#include "Combiner.h"

Combiner::Combiner(OptParser *arg, TString title)
: title(title)
{
  cout << "Combiner::Combiner() : WARNING : This constructor is deprecated. "
  "Use Combiner(OptParser *arg, TString name, TString title) instead." << endl;
  if ( arg->debug ) cout << "Combiner::Combiner() : new combiner title=" << title << endl;
  name = "";
  pdfName = "";
  this->arg = arg;
  TString wsname = "w"+getUniqueRootName();
  w = new RooWorkspace(wsname, wsname);
  _isCombined = false;
}


Combiner::Combiner(OptParser *arg, TString name, TString title)
: name(name), title(title)
{
  if ( arg->debug ) cout << "Combiner::Combiner() : new combiner name=" << name << " title=" << title << endl;
  pdfName = "";
  this->arg = arg;
  TString wsname = "w"+getUniqueRootName();
  w = new RooWorkspace(wsname, wsname);
  _isCombined = false;
}


Combiner::~Combiner()
{
  delete w;
}

///
/// Clone an existing combiner.
///
Combiner* Combiner::Clone(TString name, TString title)
{
  Combiner* cNew = new Combiner(this->arg, name, title);
  cNew->pdfName = this->pdfName;
  for ( int i=0; i<pdfs.size(); i++ ) cNew->addPdf(pdfs[i]);
  cNew->_isCombined = this->_isCombined;
  return cNew;
}


void Combiner::addPdf(PDF_Abs *p)
{
  assert(p);
  pdfs.push_back(p);
}

void Combiner::addPdf(PDF_Abs *p1, PDF_Abs *p2)
{
  addPdf(p1);
  addPdf(p2);
}

void Combiner::addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3)
{
  addPdf(p1, p2);
  addPdf(p3);
}

void Combiner::addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4)
{
  addPdf(p1, p2, p3);
  addPdf(p4);
}

void Combiner::addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5)
{
  addPdf(p1, p2, p3, p4);
  addPdf(p5);
}

void Combiner::addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5, PDF_Abs *p6)
{
  addPdf(p1, p2, p3, p4, p5);
  addPdf(p6);
}


void Combiner::delPdf(PDF_Abs *p)
{
  if ( _isCombined ){
    cout << "Combiner::delPdf() : ERROR : Can't delete pdf after the Combiner was combined!" << endl;
    return;
  }
  vector<PDF_Abs*> pdfsNew;
  for (int i=0; i<pdfs.size(); i++ ){
    if ( pdfs[i]->getUniqueGlobalID() == p->getUniqueGlobalID() ) continue;
    pdfsNew.push_back(pdfs[i]);
  }
  pdfs = pdfsNew;
}

void Combiner::delPdf(PDF_Abs *p1, PDF_Abs *p2)
{
  delPdf(p1);
  delPdf(p2);
}

void Combiner::delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3)
{
  delPdf(p1, p2);
  delPdf(p3);
}

void Combiner::delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4)
{
  delPdf(p1, p2, p3);
  delPdf(p4);
}

void Combiner::delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5)
{
  delPdf(p1, p2, p3, p4);
  delPdf(p5);
}

void Combiner::delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5, PDF_Abs *p6)
{
  delPdf(p1, p2, p3, p4, p5);
  delPdf(p6);
}


void Combiner::replacePdf(PDF_Abs *from, PDF_Abs *to)
{
  delPdf(from);
  addPdf(to);
}


void Combiner::combine()
{
  if ( _isCombined ){
    cout << "Combiner::combine() : WARNING : Already combined. Skipping." << endl;
    return;
  }
  if ( arg->debug ) cout << "Combiner::combine() : combining name=" << name << " title=" << title << endl;
  if ( pdfs.size()==0 ){
    cout << "Combiner::combine() : ERROR : Combination is empty." << endl;
    return;
  }
  // check for pitfalls and warn the user
  for (int i=0; i<pdfs.size(); i++ ){
    for (int j=i+1; j<pdfs.size(); j++ ){
      PDF_Abs* p = pdfs[j];
      if ( pdfs[i]->getUniqueGlobalID() == p->getUniqueGlobalID() ){
        cout << "Combiner::combine() : WARNING : You are trying to combine the same PDF twice!" << endl;
        cout << "                                Ignore this warning if you're doing it intentionally!" << endl;
        cout << "                                combiner: " << title << endl;
        cout << "                                PDF: " << p->getBaseName() << endl;
      }
      else if ( pdfs[i]->getBaseName() == p->getBaseName() ){
        cout << "Combiner::combine() : WARNING : You are trying to combine two PDFs with the same name." << endl;
        cout << "                                Ignore this warning if you're doing it intentionally!" << endl;
        cout << "                                combiner: " << title << endl;
        cout << "                                PDF: " << p->getBaseName() << endl;
      }
    }
  }

  // uniquify all input pdfs, add them to the workspace
  for (int i=0; i<pdfs.size(); i++ ){
    if ( arg->debug ) cout << "Combiner::combine() : processing PDF " << pdfs[i]->getName() << endl;
    // check consistency of input pdfs
    bool pdfOk = pdfs[i]->checkConsistency();
    if ( !pdfOk ){
			cout << "Combiner::combine() : ERROR : PDF " << pdfs[i]->getName() << " is not self consitent. Exit." << endl;
			exit(1);
		}
    // uniquify pdf
    pdfs[i]->uniquify(i); // need to be unique inside this combiner, not globally: it's important
                          // that same combiner's pdfs are named the same
                          // else we can't save toys from different combiners into
                          // the same ToyTree in the coverage test
    // add PDF to workspace
    RooMsgService::instance().setGlobalKillBelow(WARNING);
		if ( pdfs[i]->isCrossCorPdf() ){
			// cross correlation PDFs need the same observable names as the main PDFs,
			// so link them together in the workspace
			w->import(*pdfs[i]->getPdf(),RecycleConflictNodes());
		}
		else{
    	w->import(*pdfs[i]->getPdf());
		}
    w->defineSet("obs_"+pdfs[i]->getName(), *pdfs[i]->getObservables());
    w->defineSet("par_"+pdfs[i]->getName(), *pdfs[i]->getParameters());
    w->defineSet("th_" +pdfs[i]->getName(), *pdfs[i]->getTheory());
    RooMsgService::instance().setGlobalKillBelow(INFO);
    // save unique pdf name
    pdfNames.push_back((pdfs[i]->getName()).Data());
  }

  // sort pdfs alphabetically
  sort( pdfNames.begin(), pdfNames.end() );

  // combine
  pdfName = pdfNames[0];
  for (int i=1; i<pdfNames.size(); i++ )
  {
    // build name of combined pdf
    TString newName = pdfName+"_"+pdfNames[i];
    if ( w->pdf("pdf_"+newName) ) continue;

    // combine pdfs by multiplying them one by one:
    w->factory("PROD::pdf_"+newName+"(pdf_"+pdfName+", pdf_"+pdfNames[i]+")");

    // define sets of combined parameters
    parsName = "par_"+newName;
    obsName = "obs_"+newName;
    mergeNamedSets(w, parsName, "par_"+pdfName, "par_"+pdfNames[i]);
    mergeNamedSets(w, obsName,  "obs_"+pdfName, "obs_"+pdfNames[i]);
    mergeNamedSets(w, "th_"+newName, "th_" +pdfName, "th_" +pdfNames[i]);
    pdfName = newName;
  }
	setParametersConstant();
  _isCombined = true;
}

///
/// Helper function for combine(), that actually sets those parameters,
/// that we want to fix, constant in the workspace.
///
void Combiner::setParametersConstant()
{
  for ( int i=0; i<constVars.size(); i++){
    if ( !w->var(constVars[i].name) ){
      cout << "Combiner::setParametersConstant() : ERROR : requesting to set a parameter constant\n"
        "  which is not in the workspace: " << constVars[i].name << " . Exit." << endl;
      exit(1);
    }
		// add parameter to the list of constant paramters; create the list, if necessary
    if ( w->set("const")==0 ) w->defineSet("const", constVars[i].name);
    else w->extendSet("const", constVars[i].name);
		// set the parameter value, if requested
		if ( constVars[i].useValue ) w->var(constVars[i].name)->setVal(constVars[i].value);
  }
}

///
/// Return pointer to combined PDF.
///
RooAbsPdf* Combiner::getPdf()
{
  if ( _isCombined ){
    cout << "Combiner::getPdf() : ERROR : Combiner needs to be combined first!" << endl;
    assert(0);
  }
  if ( pdfName=="" ) assert(0);
  return w->pdf("pdf_"+pdfName);
}

///
/// Return a vector of all parameter names present
/// in this combination. This works already before
/// combine() was called (unline Combiner::getParameters()).
///
vector<string>& Combiner::getParameterNames()
{
  vector<string>* vars = new vector<string>();
  if ( pdfs.size()==0 ) return *vars;

  // 1. make a list of all parameters from the pdfs
  vector<string> varsAll;
  for (int i=0; i<pdfs.size(); i++ ){
    TIterator* it = pdfs[i]->getParameters()->createIterator();
    while ( RooAbsReal* v = (RooAbsReal*)it->Next() ){
      varsAll.push_back(v->GetName());
    }
  }
  // 2. remove duplicates
  sort(varsAll.begin(), varsAll.end());
  vars->push_back(varsAll[0]);
  string previous = varsAll[0];
  for ( int i=1; i<varsAll.size(); i++ ){
    if ( previous==varsAll[i] ) continue;
    vars->push_back(varsAll[i]);
    previous=varsAll[i];
  }
  return *vars;
}

///
/// Return a vector of all observables names present
/// in this combination. This works only after 
/// combine() was called as this adds the unification strings.
///
vector<string>& Combiner::getObservableNames()
{
	if ( !_isCombined ){
		cout << "Combiner::getObservableNames() : ERROR : Combiner needs to be combined first!" << endl;
		assert(0);
	}
	vector<string>* vars = new vector<string>();
	const RooArgSet* obs = w->set("obs_"+pdfName);
	if ( !obs ){
		cout << "Combiner::getObservableNames() : ERROR : Observables set not found in workspace: " << "obs_"+pdfName << endl;
		assert(0);
	}
	TIterator* it = obs->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) vars->push_back(p->GetName());
	delete it;
	return *vars;
}

///
/// Return a (new?) RooArgSet that contains all parameters.
///
const RooArgSet* Combiner::getParameters()
{
  if ( !_isCombined ){
    cout << "Combiner::getParameters() : ERROR : Combiner needs to be combined first!" << endl;
    assert(0);
  }
  return w->set("par_"+pdfName);
}

///
/// Return a (new?) RooArgSet that contains all observables.
///
const RooArgSet* Combiner::getObservables()
{
  if ( !_isCombined ){
    cout << "Combiner::getObservables() : ERROR : Combiner needs to be combined first!" << endl;
    assert(0);
  }
  return w->set("obs_"+pdfName);
}

///
/// Print the combination setup.
///
void Combiner::print()
{
  if ( pdfs.size()==0 ) return;
  cout << "\nCombiner Configuration: " << title << endl;
  cout <<   "=======================\n" << endl;
  // consice summary
  for (int i=0; i<pdfs.size(); i++ ){
    TString name = pdfs[i]->getName();
    name.ReplaceAll(pdfs[i]->getUniqueID(),"");
    printf("%2i. [PDF %3i] %-65s\n", i+1, pdfs[i]->getGcId(), (pdfs[i]->getTitle()).Data());
  }
  // verbose printout
  if ( arg->verbose ){
    cout << "\nDetailed Configuration:" << endl;
    cout <<   "=======================\n" << endl;
    for (int i=0; i<pdfs.size(); i++ ){
      printf("%2i. ", i+1);
      pdfs[i]->print();
    }
  }
  cout << endl;
}

///
/// Fix a parameters in the fit to the specified value.
/// To do that, add the parameters to
/// the list of constant parameters.
/// \param vars Comma separated list of parameteters to fix.
///

void Combiner::fixParameter(TString var, float value)
{
  if ( _isCombined ){
    cout << "Combiner::fixParameter() : WARNING : Can't set parameters constant "
			"after combine() was called. Skipping." << endl;
    return;
  }
	// check if the parameter is already in the list
	bool save=true;
	for ( int j=0; j<constVars.size(); j++){
	  if ( constVars[j].name==var ) save=false;
	}
	FixPar p;
	p.name = var;
	p.value = value;
	p.useValue = true;	// actually use the FixPar's value field to change the parameter value
  if ( save ) constVars.push_back(p);
	else{
		cout << "Combiner::fixParameter() : WARNING : fixing a parameter that was fixed before: " <<
		var << "=" << value << endl;
	}
}

///
/// Fix one or more parameters in the fit to their current values
/// (the start parameters, if no user code modifies them before
/// calling combine()). To do that, add the parameters to
/// the list of constant parameters.
/// \param vars Comma separated list of parameteters to fix.
///
void Combiner::fixParameters(TString vars)
{
  if ( _isCombined ){
    cout << "Combiner::fixParameters() : WARNING : Can't set parameters constant "
			"after combine() was called. Skipping." << endl;
    return;
  }
  // parse the comma separated list
  vars.ReplaceAll(" ","");
  TObjArray *a = vars.Tokenize(",");
  for ( int i=0; i<a->GetEntries(); i++ ){
    TString var = ((TObjString*)a->At(i))->GetString();
    // add variables to list of constants, if not already in
    bool save=true;
    for ( int j=0; j<constVars.size(); j++){
      if ( constVars[j].name==var ) save=false;
    }
		FixPar p;
		p.name = var;
		p.useValue = false;	// don't use the FixPar's value field to change the parameter value
    if ( save ) constVars.push_back(p);
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
void Combiner::setObservablesToToyValues()
{
  if ( !_isCombined ){
    cout << "Combiner::setObservablesToToyValues() : ERROR : Can't set observables to toy values before "
			"combine() was called. Exit." << endl;
    exit(1);
  }
	if ( arg->debug ){
		cout << "Combiner::setObservablesToToyValues() : setting observables to toy values generated from:" << endl;
		getParameters()->Print("v");
	}
	RooRandom::randomGenerator()->SetSeed(0);
  RooMsgService::instance().setStreamStatus(0,kFALSE);
  RooMsgService::instance().setStreamStatus(1,kFALSE);
	RooDataSet* dataset = w->pdf("pdf_"+pdfName)->generate(*w->set("obs_"+pdfName), 1, AutoBinned(false));
  RooMsgService::instance().setStreamStatus(0,kTRUE);
  RooMsgService::instance().setStreamStatus(1,kTRUE);
	const RooArgSet* toyData = dataset->get(0);
	if ( arg->debug ){
		cout << "Combiner::setObservablesToToyValues() : generated toy observables:" << endl;
		getObservables()->Print("v");
	}
  setParameters(w, "obs_"+pdfName, toyData);
	delete dataset;
}

///
/// Activate limits for all parameters. This affects
/// the limits used in
///  - finding the globabl minimum (MethodAbsScan::doInitialFit())
///  - the scan fits in the Prob method (MethodProbScan::scan1d(), MethodProbScan::scan2d())
///  - the scan and free fits in the Plugin method (MethodPluginScan::scan1d(), MethodPluginScan::scan2d())
///  - the Berger-Boos scans (MethodBergerBoosScan::scan1d())
///  - generic plugin scans (MethodGenericPluginScan::scan1d())
///
/// If a Feldman-Cousins like behaviour is needed, we need
/// to load the 'phys' limits, which will prevent any parameter
/// going outside the 'phys' range. Else the 'free' limit is loaded,
/// which should be wide enough to get never hit. This is controlled
/// over the command line option "physrange".
///
void Combiner::loadParameterLimits()
{
	if ( !_isCombined ){
    cout << "Combiner::loadParameterLimits() : ERROR : Combiner needs to be combined first! Exit." << endl;
    exit(1);
  }
  TString rangeName = arg->enforcePhysRange ? "phys" : "free";
	if ( arg->debug ) cout << "Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << endl;
  TIterator* it = w->set("par_"+pdfName)->createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ) setLimit(w, p->GetName(), rangeName);
  delete it;
}

///
/// Set the combiner name. Only possible before combine() was called.
///
void Combiner::setName(TString name)
{
	if ( _isCombined ){
    cout << "Combiner::setName() : ERROR : Name can only be changed before combiner is combined. Exit." << endl;
    exit(1);
  }
	this->name = name;
}
