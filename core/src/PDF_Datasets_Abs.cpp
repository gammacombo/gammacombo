/**
 * Gamma Combination
 * Author: Maximilian Schlupp, max.schlupp@cern.ch
 * Date: December 2013
 *
 *  
 **/

#include "PDF_Datasets_Abs.h"


PDF_Datasets_Abs::PDF_Datasets_Abs(RooWorkspace* w, int nObs, OptParser* opt) 
: PDF_Abs(nObs){
  observables     = NULL;
  parameters      = NULL;
  wspc            = w;//new RooWorkspace(*w);
  pdfName         = "default_internal_pdf_name";
  globalVarsName  = "default_internal_global_vars_set_name";
  constraintName  = "default_internal_constraint_set_name";
  dataName        = "default_internal_dataset_name";
  pdfWspcName     = "default_internal_workspace_name";
  parName         = "default_internal_parameter_set";
  areObsSet       = areParsSet = areRangesSet = isPdfSet = isDataSet = isToyDataSet = false;
  arg             = opt;
  fitStatus       = -10;
  globVals        = NULL;
  _constraintPdf  = NULL;
  _NLL            = NULL;
  minNllFree      = 0;
  minNllScan      = 0;
  minNll          = 0;
};

PDF_Datasets_Abs::~PDF_Datasets_Abs(){
  delete wspc;
};

void PDF_Datasets_Abs::initData(const TString& name){
  if(isDataSet){
    std::cout << "WARNING in PDF_Datasets_Abs::initData -- Data already set" << std::endl; 
    std::cout << "WARNING in PDF_Datasets_Abs::initData -- Data will not be overwritten" << std::endl; 
    std::cout << "WARNING in PDF_Datasets_Abs::initData -- !!!" << std::endl; 
    return;
  }
  dataName    = name;
  data        = (RooDataSet*) wspc->data(dataName);
  if(data) isDataSet   = true;
  else{
    std::cout << "FATAL in PDF_Datasets_Abs::initData -- Data: " << dataName << " not found in workspace" << std::endl; 
    exit(-1);
  }
  std::cout << "INFO in PDF_Datasets_Abs::initData -- Data initialized" << std::endl;
  return;
};

void  PDF_Datasets_Abs::initObservables(const TString& setName){
    if( areObservablesSet() ){ 
    std::cout << "WARNING in PDF_Datasets_Abs::initObservables -- Observables already set" << std::endl; 
    return;
  }
  if(! isPdfInitialized() ){
    std::cout << "FATAL in PDF_Datasets_Abs::initObservables -- first call PDF_Datasets_Abs::initPdf to init the PDF!" << std::endl;
    exit(-1);
  }
  wspc->renameSet(setName, obsName);
  observables = (RooArgList*) wspc->set(obsName);
  areObsSet = true;
};

void  PDF_Datasets_Abs::initGlobalObservables(const TString& setName){
  wspc->renameSet(setName, globalObsName);
  wspc->saveSnapshot(globalObsDataSnapshotName,*wspc->set(globalObsName));
};





void  PDF_Datasets_Abs::initObservables(){
    std::cout << "ERROR in PDF_Datasets_Abs::initObservables():"<<endl;
    std::cout << "This function is not supported for dataset scans." << std::endl; 
    std::cout << "You must define the RooArgSet of observables in the Workspace." << std::endl; 
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " <<std::endl;
    std::cout << "PDF_Datasets_Abs::initObservables(const TString& setName)" << std::endl; 
    exit(-1);
};

void  PDF_Datasets_Abs::initParameters(const TString& setName){
  if( areParametersSet() ){ 
    std::cout << "WARNING in PDF_Datasets_Abs::initParameters -- Parameters already set" << std::endl; 
    return;
  }
  if(! isPdfInitialized() ){
    std::cout << "FATAL in PDF_Datasets_Abs::initParameters -- first call PDF_Datasets_Abs::initPdf to init the PDF!" << std::endl;
    exit(-1);
  }
  wspc->renameSet(setName,parName);
  parameters = (RooArgList*) wspc->set(parName);
  areParsSet = true;
};

void  PDF_Datasets_Abs::initConstraints(const TString& setName){
  wspc->renameSet(setName,constraintName);
};

void  PDF_Datasets_Abs::initParameters(){
    std::cout << "ERROR in PDF_Datasets_Abs::initParameters():"<<endl;
    std::cout << "This function is not supported for dataset scans." << std::endl; 
    std::cout << "You must define the RooArgSet of parameters in the Workspace." << std::endl; 
    std::cout << "The name of the set in the workspace must be passed to the PDF object via " <<std::endl;
    std::cout << "PDF_Datasets_Abs::initObservables(const TString& setName)" << std::endl; 
    exit(EXIT_FAILURE);
};

void PDF_Datasets_Abs::initPDF(const TString& name){
  if(isPdfSet){
    std::cout << "ERROR in PDF_Datasets_Abs::initPDF -- PDF already set" << std::endl; 
    exit(EXIT_FAILURE);
  }
  pdfWspcName   = name;
  pdf           = wspc->pdf(pdfWspcName);
  if(pdf) isPdfSet  = true;
  else{
    std::cout << "FATAL in PDF_Datasets_Abs::initPDF -- PDF: " << pdfWspcName << " not found in workspace" << std::endl; 
    exit(EXIT_FAILURE);
  }

  std::cout << "INFO in PDF_Datasets_Abs::initPDF -- PDF initialized" << std::endl;
  return;
};

void PDF_Datasets_Abs::setVarRange(const TString &varName, const TString &rangeName, 
                                  const double &rangeMin, const double &rangeMax){
  RooRealVar* var = wspc->var(varName);
  if(!var){
    std::cout << "ERROR in PDF_Datasets_Abs::setVarRange -- No Var with Name: " 
              << varName << " found!!" << std::endl;
    return;
  }
  if(!(rangeName== "free" || rangeName== "phys" || rangeName == "scan" || rangeName == "bboos" || rangeName == "force")){
    std::cout << "ERROR in PDF_Datasets_Abs::setVarRange -- UNKNOWN range name! -- return" << std::endl;
  }
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  if(rangeMin == rangeMax){
    std::cout << "WARNING in PDF_Datasets_Abs::setVarRange -- rangeMin == rangeMax! If you want to set parameter constant "
              << "use e.g. RooRealVar::setConstant. Expect crash in CL calculation!" << std::endl;
  }
  var->setRange(rangeName, rangeMin, rangeMax);
  RooMsgService::instance().setGlobalKillBelow(INFO);
};
void PDF_Datasets_Abs::setPdfName(const TString& name){
  this->pdfName = name;
  this->obsName = "obs_"+pdfName; // these are the internal names of the observable
  this->parName = "par_"+pdfName; // and the parameter sets. 
  // \todo: Instead of re-naming the datasets, use the names that the user specified in initObservables and initParameters directly.
};

void PDF_Datasets_Abs::setToyData(RooDataSet* ds){
  toyObservables  = ds; 
  isToyDataSet    = kTRUE;
  return;
};

void PDF_Datasets_Abs::print(){
  if(isPdfSet){
    std::cout << "PDF:\t" << this->getPdfName() << std::endl;
  }
  if(wspc){
    std::cout << "Workspace:\t" << std::endl;
    wspc->Print();
  }
  return;
};


OptParser*   PDF_Datasets_Abs::getArg(){
  std::cout<<"ERROR: getting the options parser from the pdf has been deprecated"<<std::endl;
  std::cout<<"(This is up for discussion of course)"<<std::endl;
  exit(EXIT_FAILURE);
}