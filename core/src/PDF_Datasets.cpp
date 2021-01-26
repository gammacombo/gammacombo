/**
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
 *
 *
 **/

#include "PDF_Datasets.h"


PDF_Datasets::PDF_Datasets(RooWorkspace* w, int nObs, OptParser* opt)
    : PDF_Abs(nObs) {
    wspc            = w;//new RooWorkspace(*w);
    obsName         = "default_internal_observables_set_name";
    parName         = "default_internal_parameter_set_name";
    // globalParsName  = "default_internal_global_pars_set_name";
    globalObsName   = "default_internal_global_obs_set_name";
    constraintName  = "default_internal_constraint_set_name";
    dataName        = "default_internal_dataset_name";
    pdfName         = "default_pdf_workspace_name";
    pdfBkgName      = "default_pdf_bkg_workspace_name";
    parName         = "default_internal_parameter_set";
    areObsSet       = areParsSet = areRangesSet = isPdfSet = isBkgPdfSet = isDataSet = isToyDataSet = kFALSE;
    arg             = opt;
    fitStatus       = -10;
    _NLL            = NULL;
    minNllFree      = 0;
    minNllScan      = 0;
    minNll          = 0;
};

PDF_Datasets::PDF_Datasets(RooWorkspace* w)
    : PDF_Datasets(w, 1, NULL)
{
    name    = "PDF_Dataset";
    title   = "PDF_Dataset";
};

PDF_Datasets::~PDF_Datasets() {
    if (wspc) delete wspc;
    if (_constraintPdf) delete _constraintPdf;
};

void  PDF_Datasets::initConstraints(const TString& setName) {
    constraintName = setName;
    //obtain the part of the PDF that can generate the global observables
    this->_constraintPdf  = new RooProdPdf("constraintPdf", "", *wspc->set(constraintName));
    if (_constraintPdf == NULL) {
        std::cout << "ERROR in PDF_B_MuMu::initConstraints - constraint pdf not initialized." << endl;
        exit(EXIT_FAILURE);
    }
};

void PDF_Datasets::initData(const TString& name) {
    if (isDataSet) {
        std::cout << "WARNING in PDF_Datasets::initData -- Data already set" << std::endl;
        std::cout << "WARNING in PDF_Datasets::initData -- Data will not be overwritten" << std::endl;
        std::cout << "WARNING in PDF_Datasets::initData -- !!!" << std::endl;
        exit(EXIT_FAILURE);
    }
    dataName    = name;
    data        = (RooDataSet*) wspc->data(dataName);
    if (data) isDataSet   = true;
    else {
        std::cout << "FATAL in PDF_Datasets::initData -- Data: " << dataName << " not found in workspace" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(pdf) this->minNll = pdf->createNLL(*data)->getVal();
    std::cout << "INFO in PDF_Datasets::initData -- Data initialized" << std::endl;
    return;
};

//
// Sets the name of the set containing the observables, minus the global observables.
//
void  PDF_Datasets::initObservables(const TString& setName) {

    if (! isPdfInitialized() ) {
        std::cerr << "FATAL in PDF_Datasets::initObservables -- first call PDF_Datasets::initPdf to init the PDF!" << std::endl;
        exit(EXIT_FAILURE);
    }
    obsName = setName;
    observables = (RooArgList*) wspc->set(obsName);
    areObsSet = true;
};

void  PDF_Datasets::initGlobalObservables(const TString& setName) {
    globalObsName = setName;
    // The global observables in the workspace are set to their observed value.
    // This value is saved.
    wspc->saveSnapshot(globalObsDataSnapshotName, *wspc->set(globalObsName));
};

void  PDF_Datasets::initObservables() {
    std::cout << "ERROR in PDF_Datasets::initObservables():" << endl;
    std::cout << "This function is not supported for dataset scans." << std::endl;
    std::cout << "You must define the RooArgSet of observables in the Workspace." << std::endl;
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " << std::endl;
    std::cout << "PDF_Datasets::initObservables(const TString& setName)" << std::endl;
    exit(EXIT_FAILURE);
};

void  PDF_Datasets::initParameters(const TString& setName) {
    if ( areParametersSet() ) {
        std::cout << "WARNING in PDF_Datasets::initParameters -- Parameters already set" << std::endl;
        return;
    }
    if (! isPdfInitialized() ) {
        std::cout << "FATAL in PDF_Datasets::initParameters -- first call PDF_Datasets::initPdf to init the PDF!" << std::endl;
        exit(EXIT_FAILURE);
    }
    parName = setName;
    parameters = (RooArgList*) wspc->set(parName);
    areParsSet = true;
};

void  PDF_Datasets::initParameters(const vector<TString>& parNames) {
    if ( areParametersSet() ) {
        std::cout << "WARNING in PDF_Generic_Abs::initParameters -- Parameters already set" << std::endl;
        return;
    }
    if (! isPdfInitialized() ) {
        std::cout << "FATAL in PDF_Generic_Abs::initParameters -- first call PDF_Generic_Abs::initPdf to init the PDF!" << std::endl;
        exit(EXIT_FAILURE);
    }
    parameters = new RooArgList("parameters");
    Utils::fillArgList(parameters, wspc, parNames);
    wspc->defineSet(parName, *parameters);//, RooFit::Silence());
    areParsSet = true;
    if (arg->debug) std::cout << "DEBUG in PDF_Generic_Abs::initParameters --pars filled" << std::endl;
};

void  PDF_Datasets::initParameters() {
    std::cout << "ERROR in PDF_Datasets::initParameters():" << endl;
    std::cout << "This function is not supported for dataset scans." << std::endl;
    std::cout << "You must define the RooArgSet of parameters in the Workspace." << std::endl;
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " << std::endl;
    std::cout << "PDF_Datasets::initParameters(const TString& setName)" << std::endl;
    exit(EXIT_FAILURE);
};

void PDF_Datasets::initPDF(const TString& name) {
    if (isPdfSet) {
        std::cout << "ERROR in PDF_Datasets::initPDF -- PDF already set" << std::endl;
        exit(EXIT_FAILURE);
    }
    pdfName  = name;
    pdf      = wspc->pdf(pdfName);
    if (pdf) isPdfSet  = true;
    else {
        std::cout << "FATAL in PDF_Datasets::initPDF -- PDF: " << pdfName << " not found in workspace" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "INFO in PDF_Datasets::initPDF -- PDF initialized" << std::endl;
    if(data) this->minNll = pdf->createNLL(*data)->getVal();
    return;
};

void PDF_Datasets::initBkgPDF(const TString& name) {
    if (isBkgPdfSet) {
        std::cout << "ERROR in PDF_Datasets::initBkgPDF -- Bkg PDF already set" << std::endl;
        exit(EXIT_FAILURE);
    }
    pdfBkgName  = name;
    pdfBkg      = wspc->pdf(pdfBkgName);
    if (pdfBkg) isBkgPdfSet  = true;
    else {
        std::cout << "FATAL in PDF_Datasets::initBkgPDF -- PDF: " << pdfBkgName << " not found in workspace" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "INFO in PDF_Datasets::initBkgPDF -- PDF initialized. CLs method ready." << std::endl;
    return;
};

void PDF_Datasets::setVarRange(const TString &varName, const TString &rangeName,
                               const double &rangeMin, const double &rangeMax) {
    RooRealVar* var = wspc->var(varName);
    if (!var) {
        std::cout << "ERROR in PDF_Datasets::setVarRange -- No Var with Name: "
                  << varName << " found!!" << std::endl;
        return;
    }
    if (!(rangeName == "free" || rangeName == "phys" || rangeName == "scan" || rangeName == "bboos" || rangeName == "force")) {
        std::cout << "ERROR in PDF_Datasets::setVarRange -- UNKNOWN range name! -- return" << std::endl;
    }
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    if (rangeMin == rangeMax) {
        std::cout << "WARNING in PDF_Datasets::setVarRange -- rangeMin == rangeMax! If you want to set parameter constant "
                  << "use e.g. RooRealVar::setConstant. Expect crash in CL calculation!" << std::endl;
    }
    var->setRange(rangeName, rangeMin, rangeMax);
    RooMsgService::instance().setGlobalKillBelow(INFO);
};


void PDF_Datasets::setToyData(RooDataSet* ds) {
    toyObservables  = ds;
    isToyDataSet    = kTRUE;
    return;
};

void PDF_Datasets::setBkgToyData(RooDataSet* ds) {
    toyBkgObservables  = ds;
    return;
};


void PDF_Datasets::print() {
    if (isPdfSet) {
        std::cout << "PDF:\t" << this->getPdfName() << std::endl;
    }
    if (wspc) {
        std::cout << "Workspace:\t" << std::endl;
        wspc->Print();
    }
    return;
};

void PDF_Datasets::printParameters() {
    int parcounter = 0;
    TIterator* it = this->parameters->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ) {
        cout << p->GetName() << " " << p->getVal() << " ";
        parcounter += 1;
        if ( parcounter % 5 == 0 ) cout << endl << "  ";
    }
    delete it;
    cout << endl << endl;
};



OptParser*   PDF_Datasets::getArg() {
    std::cout << "ERROR: getting the options parser from the pdf has been deprecated" << std::endl;
    std::cout << "(This is up for discussion of course)" << std::endl;
    exit(EXIT_FAILURE);
};

void  PDF_Datasets::generateBkgToysGlobalObservables(int SeedShift, int index) {

    initializeRandomGenerator(SeedShift);

    // generate the global observables into a RooArgSet
    // const RooArgSet* set = pdfBkg->generate(*(wspc->set(globalObsName)), 1)->get(0);
    const RooArgSet* set = _constraintPdf->generate(*(wspc->set(globalObsName)), 1)->get(0);
    // iterate over the generated values and use them to update the actual global observables in the workspace

    TIterator* it =  set->createIterator();
    while (RooRealVar* genVal = dynamic_cast<RooRealVar*>(it->Next())) {
        wspc->var(genVal->GetName())->setVal(genVal->getVal());
    }
    TString index_string;
    index_string.Form("globalObsBkgToySnapshotName%d",index);
    // take a snapshot of the global variables in the workspace so they can be loaded later
    globalObsBkgToySnapshotName = index_string;
    wspc->saveSnapshot(globalObsBkgToySnapshotName, *wspc->set(globalObsName));
};


void  PDF_Datasets::generateToysGlobalObservables(int SeedShift) {

    initializeRandomGenerator(SeedShift);

    // generate the global observables into a RooArgSet
    const RooArgSet* set = _constraintPdf->generate(*(wspc->set(globalObsName)), 1)->get(0);
    // iterate over the generated values and use them to update the actual global observables in the workspace

    TIterator* it =  set->createIterator();
    while (RooRealVar* genVal = dynamic_cast<RooRealVar*>(it->Next())) {
        wspc->var(genVal->GetName())->setVal(genVal->getVal());
    }

    // take a snapshot of the global variables in the workspace so they can be loaded later
    wspc->saveSnapshot(globalObsToySnapshotName, *wspc->set(globalObsName));
};


RooFitResult* PDF_Datasets::fit(RooDataSet* dataToFit) {

    if (this->getWorkspace()->set(constraintName) == NULL) {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "ERROR: No RooArgSet with constraints found." << std::endl;
        std::cout << "The Workspace must contain a RooArgSet with constraint PDFs." << std::endl;
        std::cout << "These are usually Gaussians that constrain parameters via global observables." << std::endl;
        std::cout << "This set can be empty." << std::endl;
        std::cout << "By default its name should be 'default_internal_constraint_set_name'." << std::endl;
        std::cout << "Other names can be passed via PDF_Datasets::initConstraints" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);
    // Choose Dataset to fit to

    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));
    RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Extended(kTRUE));

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);
    this->fitStatus = result->status();
    // RooAbsReal* nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE));
    RooAbsReal* nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    this->minNll = nll->getVal();
    delete nll;

    return result;
};

RooFitResult* PDF_Datasets::fitBkg(RooDataSet* dataToFit) {

    if (this->getWorkspace()->set(constraintName) == NULL) {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "ERROR: No RooArgSet with constraints found." << std::endl;
        std::cout << "The Workspace must contain a RooArgSet with constraint PDFs." << std::endl;
        std::cout << "These are usually Gaussians that constrain parameters via global observables." << std::endl;
        std::cout << "This set can be empty." << std::endl;
        std::cout << "By default its name should be 'default_internal_constraint_set_name'." << std::endl;
        std::cout << "Other names can be passed via PDF_Datasets::initConstraints" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!pdfBkg)
    {
        std::cout << "ERROR in PDF_Datasets::fitBkg -- No background PDF given!" << std::endl;
        exit(EXIT_FAILURE);
    }
    double parvalue = getWorkspace()->var("branchingRatio")->getVal();
    bool isconst = getWorkspace()->var("branchingRatio")->isConstant();
    getWorkspace()->var("branchingRatio")->setVal(0.0);
    getWorkspace()->var("branchingRatio")->setConstant(true);

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);
    // Choose Dataset to fit to

    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdfBkg->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));

    RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Extended(kTRUE));
    // RooFitResult* result  = pdfBkg->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Extended(kTRUE));
    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);
    this->fitStatus = result->status();
    // RooAbsReal* nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE));
    // RooAbsReal* nll_bkg = pdfBkg->createNLL(*dataToFit, RooFit::Extended(kTRUE));
    RooAbsReal* nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    // RooAbsReal* nll_bkg = pdfBkg->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    this->minNllBkg = nll_bkg->getVal();

    getWorkspace()->var("branchingRatio")->setVal(parvalue);
    getWorkspace()->var("branchingRatio")->setConstant(isconst);    
    delete nll_bkg;

    return result;
};

void   PDF_Datasets::generateToys(int SeedShift) {

    initializeRandomGenerator(SeedShift);
    RooDataSet* toys = this->pdf->generate(*observables, RooFit::NumEvents(wspc->data(dataName)->numEntries()), RooFit::Extended(kTRUE));

    // Having the delete in here causes a segmentation fault, likely due to a double free
    // related to Root's internal memory management. Therefore we do not delete,
    // which might or might not cause a memory leak.
    // if(this->toyObservables) delete this->toyObservables;
    this->toyObservables  = toys;
    this->isToyDataSet    = kTRUE;
};

void   PDF_Datasets::generateBkgToys(int SeedShift) {

    initializeRandomGenerator(SeedShift);

    if(isBkgPdfSet){
        double parvalue = getWorkspace()->var("branchingRatio")->getVal();
        bool isconst = getWorkspace()->var("branchingRatio")->isConstant();
        getWorkspace()->var("branchingRatio")->setVal(0.0);
        getWorkspace()->var("branchingRatio")->setConstant(true);
        // RooDataSet* toys = pdfBkg->generate(*observables, RooFit::NumEvents(wspc->data(dataName)->numEntries()), RooFit::Extended(kTRUE));
        RooDataSet* toys = pdf->generate(*observables, RooFit::NumEvents(wspc->data(dataName)->numEntries()), RooFit::Extended(kTRUE));
        getWorkspace()->var("branchingRatio")->setVal(parvalue);
        getWorkspace()->var("branchingRatio")->setConstant(isconst);    
        this->toyBkgObservables  = toys;
    }
    else{
        std::cerr << "Error in PDF_Datasets::generateBkgToys: No bkg pdf given." << std::endl;
        exit(EXIT_FAILURE);
    }
};

/*! \brief Initializes the random generator
 *
 *  If seedShift is set to zero, the machine environment is used to generate
 *  a hopefully unique random seed.
 *  If seedShift is nonzero, a deterministic seed is calculated from the seedShift
 *  several command line call parameters.
 */
void PDF_Datasets::initializeRandomGenerator(int seedShift) {

    if (seedShift == 0) {
        // From the ROOT documentation:
        // if seed is 0 [...] a TUUID is generated and used to fill the first 8 integers of the seed array.
        // In this case the seed is guaranteed to be unique in space and time.
        RooRandom::randomGenerator()->SetSeed(0);
    } else {
        // calculate unique seed for deterministic random generation
        if (arg == NULL) {
            std::cerr << "Error in PDF_Datasets::initializeRandomGenerator." << std::endl;
            std::cerr << "You must pass the OptParser in the constructor in order to use this function." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout<<"random seed is not 0" <<std::endl;
        RooRandom::randomGenerator()->SetSeed(seedShift + (arg->nrun) * (arg->ntoys) * (arg->npoints1d));
    }
};

void PDF_Datasets::unblind(TString var, TString unblindRegs) {

  TString unblindString = "";
  TObjArray *regs = unblindRegs.Tokenize(","); // split string at ","
  for (int i=0; i<regs->GetEntries(); i++){
    TString range = ((TObjString*)regs->At(i))->GetString();
    TString minStr = range;
    TString maxStr = range;
    minStr.ReplaceAll("[","");
    minStr.Replace(minStr.Index(":"), minStr.Sizeof(), "");
    maxStr.ReplaceAll("]","");
    maxStr.Replace(0,maxStr.Index(":")+1,"");
    float min = minStr.Atof();
    float max = maxStr.Atof();
    wspc->var(var)->setRange(Form("unblind%d",i),min,max);
    unblindString += Form("unblind%d",i);
    if (i < regs->GetEntries()-1) unblindString += ",";
  }
  unblindRegions[var] = unblindString;
};
