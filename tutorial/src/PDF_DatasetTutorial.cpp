#include "PDF_DatasetTutorial.h"
#include "RooExponential.h"

PDF_DatasetTutorial::PDF_DatasetTutorial(RooWorkspace* w)
: PDF_Datasets_Abs(w,1, NULL)
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
  
  drawFitsDebug  = kFALSE;
}
PDF_DatasetTutorial::~PDF_DatasetTutorial(){};

RooFitResult* PDF_DatasetTutorial::fit(bool fitToys){
  if(this->notSetupToFit(fitToys)){
    std::cout << "FATAL in PDF_DatasetTutorial::fit -- There is no PDF or (toy)data set to fit!" << std::endl;  
    return NULL;
  }
  	//\todo: move the following into separate method in the ABS class
	//\todo: also check if all the other argsets and co can be found
	//\todo: in exchange, get rid of the memeber variables that check initalization, except for the pdf itself.
	//\todo: Or maybe it is smarter to make the checks elsewhere?
    //\todo: Maybe in fact we could have the fit functions in the ABS class and call other functions
    // from within these fit functions that only implement the fit, but none of the checks?
	if (this->getWorkspace()->set(constraintName)==NULL){
		std::cout<<std::endl;
		std::cout<<std::endl;
		std::cout<< "ERROR: No RooArgSet with constraints found."<<std::endl;
		std::cout<< "The Workspace must contain a RooArgSet with constraint PDFs."<<std::endl;
		std::cout<< "These are usually Gaussians that constrain parameters via global observables."<<std::endl;
		std::cout<< "This set can be empty."<<std::endl;
		std::cout<< "By default its name should be 'default_internal_constraint_set_name'."<<std::endl;
		std::cout<< "Other names can be passed via PDF_Datasets_Abs::initConstraints"<<std::endl;
		  exit(EXIT_FAILURE);
	  }
  
  // Turn off RooMsg
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  RooMsgService::instance().setSilentMode(kTRUE);
  // Choose Dataset to fit to
  RooDataSet* dataToFit = (fitToys) ? this->toyObservables : this->data ;
  
  if(fitToys)   wspc->loadSnapshot(globalObsToySnapshotName);
  else          wspc->loadSnapshot(globalObsDataSnapshotName);
  
  RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() 
                                      ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                      ,RooFit::Minos(kFALSE)
                                      ,RooFit::Hesse(kFALSE)
                                      ,RooFit::Strategy(3)
                                      ,RooFit::Minimizer("Minuit2","minimize")
                                      );
  
//
//  RooPlot* plot = getWorkspace()->var("mass")->frame();
//  dataToFit->plotOn(plot);	
//  pdf->plotOn(plot);
//  TCanvas c("c","c",1024, 768);
//  plot->Draw();
//  if (fitToys){
//	  c.SaveAs("plots/pdf/testfitplot"+TString(std::to_string(rand()%10))+".pdf");
//  } else {
//	  c.SaveAs(TString("plots/pdf/fitdata/testfitplot"+std::to_string(getWorkspace()->var("branchingRatio")->getVal()*10000)+".pdf"));
//  }

  RooMsgService::instance().setSilentMode(kFALSE);
  RooMsgService::instance().setGlobalKillBelow(INFO);
  this->fitStatus = result->status();
  return result;
};

void   PDF_DatasetTutorial::generateToys(int SeedShift) {
  TRandom3 rndm(0);

  double mean_signal_events_to_be_generated = getWorkspace()->var("branchingRatio")->getVal() / getWorkspace()->var("norm_constant")->getVal();
  double mean_background_events_to_be_generated = this->getWorkspace()->var("Nbkg")->getVal();

  std::default_random_engine generator(std::random_device{}());
  std::poisson_distribution<int> signal_poisson(mean_signal_events_to_be_generated);
  std::poisson_distribution<int> background_poisson(mean_background_events_to_be_generated);

  int sig_number = signal_poisson(generator);
  int bkg_number = background_poisson(generator);

  RooDataSet* toys = this->getWorkspace()->pdf("g")->generate(*observables, sig_number);
  toys->append(*(this->getWorkspace()->pdf("background_model")->generate(*observables,bkg_number)));

  this->toyObservables  = toys; 
  this->isToyDataSet    = kTRUE;
}
