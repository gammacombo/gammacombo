/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#include "PDF_Circle.h"

PDF_Circle::PDF_Circle(config cObs, config cErr, config cCor)
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


PDF_Circle::~PDF_Circle(){}


void PDF_Circle::initParameters()
{
  ParametersTutorial p;
  parameters = new RooArgList("parameters");
  parameters->add(*(p.get("a_gaus")));
  parameters->add(*(p.get("b_gaus")));
}


void PDF_Circle::initRelations()
{
  theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
  theory->add(*(new RooFormulaVar("radius_th", "radius_th", "sqrt(a_gaus^2+b_gaus^2)", *(RooArgSet*)parameters)));
}


void PDF_Circle::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("radius_obs", "radius_obs",  0, -1e4, 1e4)));
}


void PDF_Circle::setObservables(config c)
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
      setObservable("radius_obs",2.0);
      break;
    }
    default:{
      cout << "PDF_Circle::setObservables() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);
    }
  }
}


void PDF_Circle::setUncertainties(config c)
{
  switch(c)
  {
    case lumi1fb:{
      obsErrSource = "lumi1fb";
      StatErr[0] = 0.25;
      SystErr[0] = 0;
      break;
    }
    default:{
      cout << "PDF_Circle::initCov() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);
    }
  }
}


void PDF_Circle::setCorrelations(config c)
{
  resetCorrelations();
  switch(c)
  {
    case lumi1fb:{
      corSource = "no correlations for 1 obs";
      break;
    }
    default:{
      cout << "PDF_Circle::setCorrelations() : ERROR : config "+ConfigToTString(c)+" not found." << endl;
      exit(1);
    }    
  }
}


void PDF_Circle::buildPdf()
{
  pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}
