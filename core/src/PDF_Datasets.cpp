/**
 * Gamma Combination
 * Author: Maximilian Schlupp, max.schlupp@cern.ch
 * Date: December 2013
 *
 *  
 **/

#include "PDF_Datasets.h"


PDF_Datasets::PDF_Datasets(RooWorkspace* w, int nObs, OptParser* opt) 
: PDF_Abs(nObs){
  wspc            = w;//new RooWorkspace(*w);
  obsName         = "default_internal_observables_set_name";
  parName         = "default_internal_parameter_set_name";
  globalParsName  = "default_internal_global_pars_set_name";
  globalObsName   = "default_internal_global_obs_set_name";
  constraintName  = "default_internal_constraint_set_name";
  dataName        = "default_internal_dataset_name";
  pdfName         = "default_pdf_workspace_name";
  parName         = "default_internal_parameter_set";
  areObsSet       = areParsSet = areRangesSet = isPdfSet = isDataSet = isToyDataSet = false;
  arg             = opt;
  fitStatus       = -10;
  _NLL            = NULL;
  minNllFree      = 0;
  minNllScan      = 0;
  minNll          = 0;
};

PDF_Datasets::PDF_Datasets(RooWorkspace* w)
: PDF_Datasets(w,1, NULL)
{
  name    = "PDF_DatasetTutorial";
  title   = "PDF_DatasetTutorial";
  data = (RooDataSet*)wspc->data("data"); //> set real Dataset 
  if(data){
    isDataSet = kTRUE;
    std::cout << "INFO in PDF_DatasetTutorial::PDF_DatasetTutorial -- Dataset initialized" << std::endl;
  }
  else{
    std::cout << "FATAL in PDF_DatasetTutorial::PDF_DatasetTutorial -- no Dataset with name 'data' found in workspace!" << std::endl;
    isDataSet = kFALSE;
  }
  
};

PDF_Datasets::~PDF_Datasets(){
  delete wspc;
};

void PDF_Datasets::initData(const TString& name){
  if(isDataSet){
    std::cout << "WARNING in PDF_Datasets::initData -- Data already set" << std::endl; 
    std::cout << "WARNING in PDF_Datasets::initData -- Data will not be overwritten" << std::endl; 
    std::cout << "WARNING in PDF_Datasets::initData -- !!!" << std::endl; 
    return;
  }
  dataName    = name;
  data        = (RooDataSet*) wspc->data(dataName);
  if(data) isDataSet   = true;
  else{
    std::cout << "FATAL in PDF_Datasets::initData -- Data: " << dataName << " not found in workspace" << std::endl; 
    exit(-1);
  }
  std::cout << "INFO in PDF_Datasets::initData -- Data initialized" << std::endl;
  return;
};

//
// Sets the name of the set containing the observables, minus the global observables.
//
void  PDF_Datasets::initObservables(const TString& setName){
    
  if(! isPdfInitialized() ){
    std::cerr << "FATAL in PDF_Datasets::initObservables -- first call PDF_Datasets::initPdf to init the PDF!" << std::endl;
    exit(EXIT_FAILURE);
  }
  obsName = setName;
  observables = (RooArgList*) wspc->set(obsName);
  areObsSet = true;
};

void  PDF_Datasets::initGlobalObservables(const TString& setName){
  globalObsName = setName;
  // The global observables in the workspace are set to their observed value.
  // This value is saved.
  wspc->saveSnapshot(globalObsDataSnapshotName,*wspc->set(globalObsName));
};





void  PDF_Datasets::initObservables(){
    std::cout << "ERROR in PDF_Datasets::initObservables():"<<endl;
    std::cout << "This function is not supported for dataset scans." << std::endl; 
    std::cout << "You must define the RooArgSet of observables in the Workspace." << std::endl; 
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " <<std::endl;
    std::cout << "PDF_Datasets::initObservables(const TString& setName)" << std::endl; 
    exit(EXIT_FAILURE);
};

void  PDF_Datasets::initParameters(const TString& setName){
  if( areParametersSet() ){ 
    std::cout << "WARNING in PDF_Datasets::initParameters -- Parameters already set" << std::endl; 
    return;
  }
  if(! isPdfInitialized() ){
    std::cout << "FATAL in PDF_Datasets::initParameters -- first call PDF_Datasets::initPdf to init the PDF!" << std::endl;
    exit(-1);
  }
  parName = setName;
  parameters = (RooArgList*) wspc->set(parName);
  areParsSet = true;
};

void  PDF_Datasets::initConstraints(const TString& setName){
  constraintName = setName;
};

void  PDF_Datasets::initParameters(){
    std::cout << "ERROR in PDF_Datasets::initParameters():"<<endl;
    std::cout << "This function is not supported for dataset scans." << std::endl; 
    std::cout << "You must define the RooArgSet of parameters in the Workspace." << std::endl; 
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " <<std::endl;
    std::cout << "PDF_Datasets::initObservables(const TString& setName)" << std::endl; 
    exit(EXIT_FAILURE);
};

void PDF_Datasets::initPDF(const TString& name){
  if(isPdfSet){
    std::cout << "ERROR in PDF_Datasets::initPDF -- PDF already set" << std::endl; 
    exit(EXIT_FAILURE);
  }
  pdfName  = name;
  pdf      = wspc->pdf(pdfName);
  if(pdf) isPdfSet  = true;
  else{
    std::cout << "FATAL in PDF_Datasets::initPDF -- PDF: " << pdfName << " not found in workspace" << std::endl; 
    exit(EXIT_FAILURE);
  }

  std::cout << "INFO in PDF_Datasets::initPDF -- PDF initialized" << std::endl;
  return;
};

void PDF_Datasets::setVarRange(const TString &varName, const TString &rangeName, 
                                  const double &rangeMin, const double &rangeMax){
  RooRealVar* var = wspc->var(varName);
  if(!var){
    std::cout << "ERROR in PDF_Datasets::setVarRange -- No Var with Name: " 
              << varName << " found!!" << std::endl;
    return;
  }
  if(!(rangeName== "free" || rangeName== "phys" || rangeName == "scan" || rangeName == "bboos" || rangeName == "force")){
    std::cout << "ERROR in PDF_Datasets::setVarRange -- UNKNOWN range name! -- return" << std::endl;
  }
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  if(rangeMin == rangeMax){
    std::cout << "WARNING in PDF_Datasets::setVarRange -- rangeMin == rangeMax! If you want to set parameter constant "
              << "use e.g. RooRealVar::setConstant. Expect crash in CL calculation!" << std::endl;
  }
  var->setRange(rangeName, rangeMin, rangeMax);
  RooMsgService::instance().setGlobalKillBelow(INFO);
};


void PDF_Datasets::setToyData(RooDataSet* ds){
  toyObservables  = ds; 
  isToyDataSet    = kTRUE;
  return;
};

void PDF_Datasets::print(){
  if(isPdfSet){
    std::cout << "PDF:\t" << this->getPdfName() << std::endl;
  }
  if(wspc){
    std::cout << "Workspace:\t" << std::endl;
    wspc->Print();
  }
  return;
};


OptParser*   PDF_Datasets::getArg(){
  std::cout<<"ERROR: getting the options parser from the pdf has been deprecated"<<std::endl;
  std::cout<<"(This is up for discussion of course)"<<std::endl;
  exit(EXIT_FAILURE);
}


void  PDF_Datasets::generateToysGlobalObservables(int SeedShift){

  initializeRandomGenerator(SeedShift);
  
  //obtain the part of the PDF that can generate the global observables
  auto constraintPdf  = new RooProdPdf("constraintPdf","",*wspc->set(constraintName));
  // generate the global observables into a RooArgSet
  const RooArgSet* set = constraintPdf->generate(*(wspc->set(globalObsName)),1)->get(0);
  // iterate over the generated values and use them to update the actual global observables in the workspace
  TIterator* it =  set->createIterator();
  while (RooRealVar* genVal = dynamic_cast<RooRealVar*>(it->Next())) {
    wspc->var(genVal->GetName())->setVal(genVal->getVal());
  }
  // take a snapshot of the global variables in the workspace so they can be loaded later
  wspc->saveSnapshot(globalObsToySnapshotName, *wspc->set(globalObsName));
}


RooFitResult* PDF_Datasets::fit(bool fitToys){

  if(this->notSetupToFit(fitToys)){
    std::cout << "FATAL in PDF_DatasetTutorial::fit -- There is no PDF or (toy)data set to fit!" << std::endl;  
    return NULL;
  }

  if (this->getWorkspace()->set(constraintName)==NULL){
    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<< "ERROR: No RooArgSet with constraints found."<<std::endl;
    std::cout<< "The Workspace must contain a RooArgSet with constraint PDFs."<<std::endl;
    std::cout<< "These are usually Gaussians that constrain parameters via global observables."<<std::endl;
    std::cout<< "This set can be empty."<<std::endl;
    std::cout<< "By default its name should be 'default_internal_constraint_set_name'."<<std::endl;
    std::cout<< "Other names can be passed via PDF_Datasets::initConstraints"<<std::endl;
      exit(EXIT_FAILURE);
    }
  
  // Turn off RooMsg
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  RooMsgService::instance().setSilentMode(kTRUE);
  // Choose Dataset to fit to
  RooDataSet* dataToFit = (fitToys) ? this->toyObservables : this->data ;
  
  if(fitToys)   wspc->loadSnapshot(globalObsToySnapshotName);
  else          wspc->loadSnapshot(globalObsDataSnapshotName);
  
  RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));


  RooMsgService::instance().setSilentMode(kFALSE);
  RooMsgService::instance().setGlobalKillBelow(INFO);
  this->fitStatus = result->status();
  return result;
};

void   PDF_Datasets::generateToys(int SeedShift) {

  initializeRandomGenerator(SeedShift);
  RooDataSet* toys = this->pdf->generate(*observables, RooFit::Extended(kTRUE));

  this->toyObservables  = toys; 
  this->isToyDataSet    = kTRUE;
}

/*! \brief Initializes the random generator
 *
 *  If seedShift is set to zero, the machine environment is used to generate
 *  a hopefully unique random seed. 
 *  If seedShift is nonzero, a deterministic seed is calculated from the seedShift 
 *  several command line call parameters.
 */
void PDF_Datasets::initializeRandomGenerator(int seedShift){
  
  if(seedShift == 0){
    // If the seed is zero the seed is set to a random value which [...]
    // depends on the lowest 4 bytes of TUUID. 
    // The UUID will be identical if SetSeed(0) is called with time smaller than 100 ns.
    RooRandom::randomGenerator()->SetSeed(0);
  } else {
    // calculate unique seed for deterministic random generation
    if(arg==NULL){
      std::cerr<<"Error in PDF_Datasets::initializeRandomGenerator."<<std::endl;
      std::cerr<<"You must pass the OptParser in the constructor in order to use this function."<<std::endl;
      exit(EXIT_FAILURE);
    }
    RooRandom::randomGenerator()->SetSeed(seedShift + (arg->nrun)*(arg->ntoys)*(arg->npoints1d));
  }
}