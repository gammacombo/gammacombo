#include <Combiner.h>

#include <OptParser.h>
#include <PDF_Abs.h>
#include <Utils.h>

#include <RooAbsPdf.h>
#include <RooArgSet.h>
#include <RooMsgService.h>
#include <RooProdPdf.h>
#include <RooRandom.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>

#include <TObjString.h>
#include <TString.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

Combiner::Combiner(const OptParser* arg, TString title) : title(title) {
  std::cout << "Combiner::Combiner() : WARNING : This constructor is deprecated. "
               "Use Combiner(const OptParser *arg, TString name, TString title) instead."
            << std::endl;
  if (arg->debug) std::cout << "Combiner::Combiner() : new combiner title=" << title << std::endl;
  this->arg = arg;
  TString wsname = "w" + Utils::getUniqueRootName();
  w = new RooWorkspace(wsname, wsname);
}

Combiner::Combiner(const OptParser* arg, TString name, TString title) : name(name), title(title) {
  if (arg->debug) std::cout << "Combiner::Combiner() : new combiner name=" << name << " title=" << title << std::endl;
  this->arg = arg;
  TString wsname = "w" + Utils::getUniqueRootName();
  w = new RooWorkspace(wsname, wsname);
}

Combiner::~Combiner() {
  if (w) delete w;
}

///
/// Clone an existing combiner.
///
Combiner* Combiner::Clone(TString name, TString title) {
  Combiner* cNew = new Combiner(this->arg, name, title);
  cNew->pdfName = this->pdfName;
  for (int i = 0; i < pdfs.size(); i++) cNew->addPdf(pdfs[i]);
  cNew->_isCombined = this->_isCombined;
  return cNew;
}

void Combiner::addPdf(PDF_Abs* p) {
  assert(p);
  pdfs.push_back(p);
}

void Combiner::delPdf(PDF_Abs* p) {
  if (_isCombined) {
    std::cout << "Combiner::delPdf() : ERROR : Can't delete pdf after the Combiner was combined!" << std::endl;
    return;
  }
  std::vector<PDF_Abs*> pdfsNew;
  for (int i = 0; i < pdfs.size(); i++) {
    if (pdfs[i]->getUniqueGlobalID() == p->getUniqueGlobalID()) continue;
    pdfsNew.push_back(pdfs[i]);
  }
  pdfs = pdfsNew;
}

void Combiner::replacePdf(PDF_Abs* from, PDF_Abs* to) {
  delPdf(from);
  addPdf(to);
}

void Combiner::combine() {
  if (_isCombined) {
    std::cout << "Combiner::combine() : WARNING : Already combined. Skipping." << std::endl;
    return;
  }
  if (arg->debug) std::cout << "Combiner::combine() : combining name=" << name << " title=" << title << std::endl;
  if (pdfs.size() == 0) {
    std::cout << "Combiner::combine() : ERROR : Combination is empty." << std::endl;
    return;
  }
  // check for pitfalls and warn the user
  for (int i = 0; i < pdfs.size(); i++) {
    for (int j = i + 1; j < pdfs.size(); j++) {
      PDF_Abs* p = pdfs[j];
      if (pdfs[i]->getUniqueGlobalID() == p->getUniqueGlobalID()) {
        std::cout << "\nWARNING : You are trying to combine the same PDF twice!" << std::endl;
        std::cout << "          Ignore this warning if you're doing it intentionally!" << std::endl;
        std::cout << "          combiner: " << title << std::endl;
        std::cout << "          PDF: " << p->getBaseName() << std::endl;
      } else if (pdfs[i]->getBaseName() == p->getBaseName()) {
        std::cout << "\nWARNING : You are trying to combine two PDFs with the same name." << std::endl;
        std::cout << "          Ignore this warning if you're doing it intentionally!" << std::endl;
        std::cout << "          combiner: " << title << std::endl;
        std::cout << "          PDF: " << p->getBaseName() << std::endl;
      }
    }
  }

  // uniquify all input pdfs, add them to the workspace
  for (int i = 0; i < pdfs.size(); i++) {
    if (arg->debug) std::cout << "Combiner::combine() : processing PDF " << pdfs[i]->getName() << std::endl;
    // check consistency of input pdfs
    bool pdfOk = pdfs[i]->checkConsistency();
    if (!pdfOk) {
      std::cout << "Combiner::combine() : ERROR : PDF " << pdfs[i]->getName() << " is not self consitent. Exit."
                << std::endl;
      std::exit(1);
    }
    // uniquify pdf
    pdfs[i]->uniquify(i);  // Needs to be unique inside this combiner, not globally: it's important
    // that same combiner's pdfs are named the same
    // else we can't save toys from different combiners into
    // the same ToyTree in the coverage test.
    // Also, the "scan for observable" mechanism relies on the fact
    // that the ID coincides with the number of the PDF in this combiner.
    // add PDF to workspace
    RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
    if (pdfs[i]->isCrossCorPdf()) {
      // cross correlation PDFs need the same observable names as the main PDFs,
      // so link them together in the workspace
      w->import(*pdfs[i]->getPdf(), RooFit::RecycleConflictNodes());
    } else {
      w->import(*pdfs[i]->getPdf());
    }
    w->defineSet("obs_" + pdfs[i]->getName(), *pdfs[i]->getObservables());
    w->defineSet("par_" + pdfs[i]->getName(), *pdfs[i]->getParameters());
    w->defineSet("th_" + pdfs[i]->getName(), *pdfs[i]->getTheory());
    RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
    // save unique pdf name
    pdfNames.push_back((pdfs[i]->getName()).Data());
  }

  // sort pdfs alphabetically
  std::sort(pdfNames.begin(), pdfNames.end());

  // new thing here
  pdfName = "comb";
  RooArgList* pdfList = new RooArgList();
  std::vector<std::string> parStr;
  std::vector<std::string> obsStr;
  std::vector<std::string> thStr;

  for (int i = 0; i < pdfNames.size(); i++) {
    // add to pdf name
    pdfName += "_" + pdfNames[i];

    // add to pdf list
    pdfList->add(*w->pdf(TString("pdf_" + pdfNames[i])));

    // now add to set strings
    Utils::addSetNamesToList(parStr, w, "par_" + pdfNames[i]);
    Utils::addSetNamesToList(obsStr, w, "obs_" + pdfNames[i]);
    Utils::addSetNamesToList(thStr, w, "th_" + pdfNames[i]);
  }

  // make the product
  RooProdPdf* prod = new RooProdPdf("pdf_" + pdfName, "pdf_" + pdfName, *pdfList);

  // import it into the ws
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
  w->import(*prod);

  // define sets of combined parameters
  Utils::makeNamedSet(w, "par_" + pdfName, parStr);
  Utils::makeNamedSet(w, "obs_" + pdfName, obsStr);
  Utils::makeNamedSet(w, "th_" + pdfName, thStr);

  // old thing
  // combine
  // pdfName = pdfNames[0];
  // for (int i=1; i<pdfNames.size(); i++ )
  //{
  //// build name of combined pdf
  // TString newName = pdfName+"_"+pdfNames[i];
  // if ( w->pdf("pdf_"+newName) ) continue;

  //// combine pdfs by multiplying them one by one:
  // w->factory("PROD::pdf_"+newName+"(pdf_"+pdfName+", pdf_"+pdfNames[i]+")");

  //// define sets of combined parameters
  // parsName = "par_"+newName;
  // obsName = "obs_"+newName;
  // mergeNamedSets(w, parsName, "par_"+pdfName, "par_"+pdfNames[i]);
  // mergeNamedSets(w, obsName,  "obs_"+pdfName, "obs_"+pdfNames[i]);
  // mergeNamedSets(w, "th_"+newName, "th_" +pdfName, "th_" +pdfNames[i]);
  // pdfName = newName;
  //}
  setParametersConstant();
  _isCombined = true;
}

///
/// Helper function for combine(), that actually sets those parameters,
/// that we want to fix, constant in the workspace. They just get added
/// to the "const" set. Later, they are fixed in MethodAbsScan::doInitialFit().
///
void Combiner::setParametersConstant() {
  for (int i = 0; i < constVars.size(); i++) {
    if (!w->var(constVars[i].name)) {
      std::cout << "Combiner::setParametersConstant() : ERROR : requesting to set a parameter constant\n"
                   "  which is not in the workspace: "
                << constVars[i].name << " . Exit." << std::endl;
      std::exit(1);
    }
    // add parameter to the list of constant paramters; create the list, if necessary
    if (w->set("const") == 0)
      w->defineSet("const", constVars[i].name);
    else
      w->extendSet("const", constVars[i].name);
    // set the parameter value, if requested
    if (constVars[i].useValue) w->var(constVars[i].name)->setVal(constVars[i].value);
  }
}

///
/// Return pointer to combined PDF.
///
RooAbsPdf* Combiner::getPdf() {
  if (!_isCombined) {
    std::cout << "Combiner::getPdf() : ERROR : Combiner needs to be combined first!" << std::endl;
    assert(0);
  }
  if (pdfName == "") assert(0);
  return w->pdf("pdf_" + pdfName);
}

///
/// Return a vector of all parameter names present
/// in this combination. This works already before
/// combine() was called (unline Combiner::getParameters()).
///
std::vector<std::string> Combiner::getParameterNames() const {
  std::vector<std::string> vars;
  if (pdfs.empty()) return vars;

  // 1. make a list of all parameters from the pdfs
  std::vector<std::string> varsAll;
  for (int i = 0; i < pdfs.size(); i++) {
    for (const auto& v : *pdfs[i]->getParameters()) varsAll.push_back(v->GetName());
  }
  // 2. remove duplicates
  std::sort(varsAll.begin(), varsAll.end());
  vars.push_back(varsAll[0]);
  std::string previous = varsAll[0];
  for (int i = 1; i < varsAll.size(); i++) {
    if (previous == varsAll[i]) continue;
    vars.push_back(varsAll[i]);
    previous = varsAll[i];
  }
  return vars;
}

///
/// Return a vector of all observables names present
/// in this combination.
/// If being called before combine() was called, a list of
/// observables contained in all PDFs is returned, without
/// unification strings. There might be duplicates in that
/// list, if more than one PDFs contain observables of the
/// same name.
/// If it is being called after combine() was called, the
/// the unification strings are included.
///
/// \return a vector of observable names
///
std::vector<std::string> Combiner::getObservableNames() const {
  std::vector<std::string> vars;
  if (!_isCombined) {
    // collect observables from all PDFs
    for (int i = 0; i < pdfs.size(); i++) {
      for (const auto& obs : *pdfs[i]->getObservables()) vars.push_back(obs->GetName());
    }
  } else {
    // get observables from the combined workspace
    const RooArgSet* observables = w->set("obs_" + pdfName);
    if (!observables) {
      std::cout << "Combiner::getObservableNames() : ERROR : Observables set not found in workspace: "
                << "obs_" + pdfName << std::endl;
      assert(0);
    }
    for (const auto& obs : *observables) vars.push_back(obs->GetName());
  }
  return vars;
}

///
/// Return a (new?) RooArgSet that contains all parameters.
///
const RooArgSet* Combiner::getParameters() const {
  if (!_isCombined) {
    std::cout << "Combiner::getParameters() : ERROR : Combiner needs to be combined first!" << std::endl;
    assert(0);
  }
  return w->set("par_" + pdfName);
}

///
/// Return a (new?) RooArgSet that contains all observables.
///
const RooArgSet* Combiner::getObservables() {
  if (!_isCombined) {
    std::cout << "Combiner::getObservables() : ERROR : Combiner needs to be combined first!" << std::endl;
    assert(0);
  }
  return w->set("obs_" + pdfName);
}

///
/// Print the combination setup.
///
void Combiner::print() {
  if (pdfs.empty()) return;
  std::cout << "\nCombiner Configuration: " << title << std::endl;
  std::cout << "=======================" << std::endl;
  // consice summary
  for (int i = 0; i < pdfs.size(); i++) {
    TString name = pdfs[i]->getName();
    name.ReplaceAll(pdfs[i]->getUniqueID(), "");
    printf("%2i. [measurement %3i] %-65s\n", i + 1, pdfs[i]->getGcId(), (pdfs[i]->getTitle()).Data());
  }
  std::cout << "=======================" << std::endl;
  // print observables of the combination
  std::vector<std::string> olist = getObservableNames();
  TString obslist = Form("%4d input observables: (", int(olist.size()));
  int indent_length = obslist.Length();
  int cur_length = obslist.Length();
  for (int o = 0; o < olist.size() - 1; o++) {
    obslist += " " + olist[o] + ",";
    cur_length += olist[o].length() + 2;
    if (cur_length > 100) {
      cur_length = indent_length;
      obslist += "\n";
      for (int i = 0; i < indent_length; i++) obslist += " ";
    }
  }
  obslist += " " + olist[olist.size() - 1] + " )";
  std::cout << obslist << std::endl;

  // print free parameters of the combination
  auto plist = getParameterNames();
  TString parlist = Form("%4d free parameters:   (", int(plist.size()));
  indent_length = parlist.Length();
  cur_length = parlist.Length();
  for (int p = 0; p < plist.size() - 1; p++) {
    parlist += " " + plist[p] + ",";
    cur_length += plist[p].length() + 2;
    if (cur_length > 100) {
      cur_length = indent_length;
      parlist += "\n";
      for (int i = 0; i < indent_length; i++) parlist += " ";
    }
  }
  parlist += " " + plist[plist.size() - 1] + " )";
  std::cout << parlist << std::endl;
  //}
  std::cout << "=======================" << std::endl;

  // verbose printout
  if (arg->verbose) {
    std::cout << "\nDetailed Configuration:" << std::endl;
    std::cout << "=======================\n" << std::endl;
    for (int i = 0; i < pdfs.size(); i++) {
      printf("%2i. ", i + 1);
      pdfs[i]->print();
    }
  }
  std::cout << std::endl;
}

///
/// Adjust the physical range of a parameter as requested. Only possible
/// after combining.
///
void Combiner::adjustPhysRange(TString varName, double min, double max) {
  if (!_isCombined) {
    std::cout << "Combiner::adjustPhysRange() : ERROR : Can't adjust parameters range before ";
    std::cout << "combine() was called. Skipping." << std::endl;
    return;
  }
  if (!w->var(varName)) {
    std::cout << "Combiner::adjustPhysRange() : ERROR : requesting to set a parameter constant\n";
    std::cout << "                                      which is not in the workspace: ";
    std::cout << varName << ". Skipping." << std::endl;
    return;
  }
  if (min <= -999. && max <= -999.) {
    w->var(varName)->removeMin();
    w->var(varName)->removeMax();
  } else {
    w->var(varName)->setRange("phys", min, max);
  }
}

///
/// Fix a parameter in the fit to the specified value.
/// To do that, add the parameter to the list of constant parameters.
///
/// \param var - the parameter name
/// \param value - the value to fix the parameter to
///
void Combiner::fixParameter(TString var, double value) {
  if (_isCombined) {
    std::cout << "Combiner::fixParameter() : WARNING : Can't set parameters constant "
                 "after combine() was called. Skipping."
              << std::endl;
    return;
  }
  // check if the parameter is already in the list
  bool save = true;
  for (int j = 0; j < constVars.size(); j++) {
    if (constVars[j].name == var) save = false;
  }
  Utils::FixPar p;
  p.name = var;
  p.value = value;
  p.useValue = true;  // actually use the FixPar's value field to change the parameter value
  if (save)
    constVars.push_back(p);
  else {
    std::cout << "Combiner::fixParameter() : WARNING : fixing a parameter that was fixed before: " << var << "="
              << value << std::endl;
  }
}

///
/// Fix one or more parameters in the fit to their current values
/// (the start parameters, if no user code modifies them before
/// calling combine()). To do that, add the parameters to
/// the list of constant parameters.
///
/// \param vars Comma separated list of parameteters to fix.
///
void Combiner::fixParameters(TString vars) {
  if (_isCombined) {
    std::cout << "Combiner::fixParameters() : WARNING : Can't set parameters constant "
                 "after combine() was called. Skipping."
              << std::endl;
    return;
  }
  // parse the comma separated list
  vars.ReplaceAll(" ", "");
  TObjArray* a = vars.Tokenize(",");
  for (int i = 0; i < a->GetEntries(); i++) {
    TString var = ((TObjString*)a->At(i))->GetString();
    // add variables to list of constants, if not already in
    bool save = true;
    for (int j = 0; j < constVars.size(); j++) {
      if (constVars[j].name == var) save = false;
    }
    Utils::FixPar p;
    p.name = var;
    p.useValue = false;  // don't use the FixPar's value field to change the parameter value
    if (save) constVars.push_back(p);
  }
}

///
/// Set the observables in the workspace to toy values,
/// that were generated from the current parameter values
/// in the workspace.
///
/// This can be used to perform a coverage test.
///
/// Only possible after the combiner was combined.
///
void Combiner::setObservablesToToyValues() {
  if (!_isCombined) {
    std::cout << "Combiner::setObservablesToToyValues() : ERROR : Can't set observables to toy values before "
                 "combine() was called. Exit."
              << std::endl;
    std::exit(1);
  }
  if (arg->debug) {
    std::cout << "Combiner::setObservablesToToyValues() : setting observables to toy values generated from:"
              << std::endl;
    getParameters()->Print("v");
  }
  RooRandom::randomGenerator()->SetSeed(0);
  RooMsgService::instance().setStreamStatus(0, kFALSE);
  RooMsgService::instance().setStreamStatus(1, kFALSE);
  RooDataSet* dataset = w->pdf("pdf_" + pdfName)->generate(*w->set("obs_" + pdfName), 1, RooFit::AutoBinned(false));
  RooMsgService::instance().setStreamStatus(0, kTRUE);
  RooMsgService::instance().setStreamStatus(1, kTRUE);
  const RooArgSet* toyData = dataset->get(0);
  if (arg->debug) {
    std::cout << "Combiner::setObservablesToToyValues() : generated toy observables from:" << std::endl;
    getObservables()->Print("v");
    std::cout << "Combiner::setObservablesToToyValues() : generated toy observables to:" << std::endl;
    toyData->Print("v");
  }
  Utils::setParameters(w, "obs_" + pdfName, toyData);
  delete dataset;
}

///
/// Activate limits for all parameters. This affects
/// the limits used in
///  - finding the globabl minimum (MethodAbsScan::doInitialFit())
///  - the scan fits in the Prob method (MethodProbScan::scan1d(), MethodProbScan::scan2d())
///  - the scan and free fits in the Plugin method (MethodPluginScan::scan1d(), MethodPluginScan::scan2d())
///  - the Berger-Boos scans (MethodBergerBoosScan::scan1d())
///  - generic plugin scans (MethodDatasetsPluginScan::scan1d())
///
/// If a Feldman-Cousins like behaviour is needed, we need
/// to load the 'phys' limits, which will prevent any parameter
/// going outside the 'phys' range. Else the 'free' limit is loaded,
/// which should be wide enough to get never hit. This is controlled
/// over the command line option "physrange".
///
void Combiner::loadParameterLimits() {
  if (!_isCombined) {
    std::cout << "Combiner::loadParameterLimits() : ERROR : Combiner needs to be combined first! Exit." << std::endl;
    std::exit(1);
  }
  TString rangeName = arg->enforcePhysRange ? "phys" : "free";
  if (arg->debug) std::cout << "Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << std::endl;
  for (const auto& p : *w->set("par_" + pdfName)) Utils::setLimit(w, p->GetName(), rangeName);
}

///
/// Set the combiner name. Only possible before combine() was called.
///
void Combiner::setName(TString name) {
  if (_isCombined) {
    std::cout << "Combiner::setName() : ERROR : Name can only be changed before combiner is combined. Exit."
              << std::endl;
    std::exit(1);
  }
  this->name = name;
}

///
/// Get the PDF that provides a certain observable.
/// The observable needs to contain the unique ID, which
/// corresponds to the number of the PDF in this combiner.
/// This works before combine() was called. Note that delPdf()
/// may change the order of the PDFs, and therefore the unique ID.
///
/// \param obsname  - name of the observable including the unique string
/// \return pointer to the PDF
///
PDF_Abs* Combiner::getPdfProvidingObservable(TString obsname) {
  // find PDF ID from the unique string
  TString obsnameparse = obsname;
  TString UID = "UID";
  if (!obsnameparse.Contains(UID)) {
    std::cout << "Combiner::getPdfProvidingObservable() : ERROR : observable name doesn't contain the string " << UID
              << std::endl;
    return 0;
  }
  obsnameparse.Replace(0, obsnameparse.Index(UID) + UID.Length(),
                       "");  // deleting from beginning of string until end of UID. That should leave only the number.
  if (!obsnameparse.IsDigit()) {
    std::cout << "Combiner::getPdfProvidingObservable() : ERROR : observable name doesn't end with an integer ID"
              << std::endl;
    return 0;
  }
  int id = obsnameparse.Atoi();
  // get the PDF of given ID
  if (id < 0 || id >= pdfs.size()) {
    std::cout << "Combiner::getPdfProvidingObservable() : ERROR : observable ID not found in Combiner: ID=" << id
              << std::endl;
    return 0;
  }
  PDF_Abs* foundpdf = pdfs[id];
  // check that the observable name exists in the PDF
  obsnameparse = obsname;
  obsnameparse.Replace(obsnameparse.Index(UID), obsnameparse.Length(),
                       "");  // delete the unique ID. That should leave just the observable name.
  if (!(foundpdf->hasObservable(obsname) || foundpdf->hasObservable(obsnameparse))) {
    std::cout << "Combiner::getPdfProvidingObservable() : ERROR : observable '" << obsname << "' not found in PDF '"
              << foundpdf->getName() << "'" << std::endl;
    return 0;
  }
  return foundpdf;
}
