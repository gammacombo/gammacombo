#include "PDF_Dataset.h"
#include "RooExponential.h"

PDF_Dataset::PDF_Dataset(RooWorkspace* w, OptParser* opt)
: PDF_Generic_Abs(w,1,opt)
{
  name    = "PDF_Dataset";
  title   = "PDF_Dataset";
  data = (RooDataSet*)wspc->data("data"); //> set real Dataset 
  if(data){
    isDataSet = kTRUE;
    std::cout << "INFO in PDF_Dataset::PDF_Dataset -- Dataset initialized" << std::endl;
  }
  else{
    std::cout << "FATAL in PDF_Dataset::PDF_Dataset -- no Dataset found in worspace!" << std::endl;
    isDataSet = kFALSE;
  }
  this->setNToys(0);
  drawFitsDebug = kFALSE;
}
PDF_Dataset::~PDF_Dataset(){};

RooFitResult* PDF_Dataset::fit(bool fitToys){
  if(this->notSetupToFit(fitToys)){
    std::cout << "FATAL in PDF_Dataset::fit -- There is no PDF or (toy)data set to fit!" << std::endl;  
    return NULL;
  }
  // Turn off RooMsg
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  RooMsgService::instance().setSilentMode(kTRUE);
  // Choose Dataset to fit to
  RooDataSet* dataToFit = (fitToys) ? this->toyObservables : this->data ;
  if(fitToys) this->setGlobalObservablesToToys(); // renamed "resetConstraintMeansToData"
  else this->setGlobalObservablesToData();

  RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() 
                                      ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                      ,RooFit::Minos(kFALSE)
                                      ,RooFit::Hesse(kFALSE)
                                      ,RooFit::Strategy(3)
                                      ,RooFit::Minimizer("Minuit2","minimize")
                                      ); 

  RooMsgService::instance().setSilentMode(kFALSE);
  RooMsgService::instance().setGlobalKillBelow(INFO);
  this->fitStatus = result->status();
  return result;
};

void   PDF_Dataset::generateToys(int SeedShift){
  TRandom3 rndm(0);
  if(this->getNToys()==0){
    std::cout << "FATAL in PDF_Dataset::generateToys -- I am supposed to generate 0 Toys? Can't do that!" << std::endl;  
  }
  
  double mean_signal_events_to_be_generated = getWorkspace()->var("branchingRatio")->getVal() / getWorkspace()->var("norm_constant")->getVal();
  double mean_background_events_to_be_generated = this->getWorkspace()->var("Nbkg")->getVal();

  std::default_random_engine generator(std::random_device{}());
  std::poisson_distribution<int> signal_poisson(mean_signal_events_to_be_generated);
  std::poisson_distribution<int> background_poisson(mean_background_events_to_be_generated);

  int sig_number = signal_poisson(generator);
  int bkg_number = background_poisson(generator);

  RooDataSet* toys = this->getWorkspace()->pdf("g")->generate(*observables, sig_number);

  toys->append(*(this->getWorkspace()->pdf("e")->generate(*observables,bkg_number)));

  this->toyObservables  = toys; 
  this->sampleConstraintObservables();
  this->isToyDataSet    = kTRUE;
}
