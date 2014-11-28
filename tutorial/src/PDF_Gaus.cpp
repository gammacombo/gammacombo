/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "PDF_Gaus.h"

PDF_Gaus::PDF_Gaus(config cObs, config cErr, config cCor)
: PDF_Abs(1)
{
  name = "gaus";
  initParameters();
  initRelations();
  initObservables();
  setObservables(cObs);
  setUncertainties(cErr);
  setCorrelations(cCor);
  buildCov();
  buildPdf();
}


PDF_Gaus::~PDF_Gaus(){}


void PDF_Gaus::initParameters()
{
  ParametersTutorial p;
  parameters = new RooArgList("parameters");
  parameters->add(*(p.get("a_gaus")));
}


void PDF_Gaus::initRelations()
{
  theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
  theory->add(*(new RooFormulaVar("a_gaus_th", "a_gaus_th", "a_gaus", *(RooArgSet*)parameters)));
}


void PDF_Gaus::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("a_gaus_obs", "a_gaus_obs",  0, -1e4, 1e4)));
}


void PDF_Gaus::setObservables(config c)
{
  switch(c)
  {
    case truth:{
      setObservablesTruth();
      break;
    }
    case toy:{ 
      setObservablesToy();
      break;
    }
    case lumi1fb:{
      obsValSource = "lumi1fb";
      setObservable("a_gaus_obs",-0.5);
      break;
    }
    case lumi2fb:{
      obsValSource = "lumi2fb";
      setObservable("a_gaus_obs",1.5);
      break;
    }
    default:{
      cout << "PDF_Gaus::setObservables() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);
    }
  }
}


void PDF_Gaus::setUncertainties(config c)
{
  switch(c)
  {
    case lumi1fb:{
      obsErrSource = "lumi1fb";
      StatErr[0] = 1; // a_gaus
      SystErr[0] = 0; // a_gaus
      break;
    }
    case lumi2fb:{
      obsErrSource = "lumi2fb";
      StatErr[0] = 0.5; // a_gaus
      SystErr[0] = 0.15; // a_gaus
      break;
    }
    default:{
      cout << "PDF_Gaus::initCov() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);
    }
  }
}


void PDF_Gaus::setCorrelations(config c)
{
  resetCorrelations();
  switch(c)
  {
    case lumi1fb:
      corSource = "no correlations for 1 obs";
      break;
    case lumi2fb:
      corSource = "no correlations for 1 obs";
      break;
    default: 
      cout << "PDF_Gaus::setCorrelations() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);    
  }
}


void PDF_Gaus::buildPdf()
{
  pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}
