#include <random>

#include "PDF_DatasetTutorial.h"
#include "RooExponential.h"

PDF_DatasetTutorial::PDF_DatasetTutorial(RooWorkspace* w): PDF_Datasets(w){}
PDF_DatasetTutorial::~PDF_DatasetTutorial(){};

RooFitResult* PDF_DatasetTutorial::fit(RooDataSet* dataToFit){
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
		std::cout<< "Other names can be passed via PDF_Datasets::initConstraints"<<std::endl;
		  exit(EXIT_FAILURE);
	  }

  // Turn off RooMsg
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  RooMsgService::instance().setSilentMode(kTRUE);


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
