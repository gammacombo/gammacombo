/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: November 2014
 *
 **/

#include "PDF_Gaus2d.h"

PDF_Gaus2d::PDF_Gaus2d(TString cObs, TString cErr, TString cCor)
: PDF_Abs(2)
{
  name = "Gaus2D";
  initParameters();
  initRelations();
  initObservables();
  setObservables(cObs);
  setUncertainties(cErr);
  setCorrelations(cCor);
  buildCov();
  buildPdf();
}


PDF_Gaus2d::~PDF_Gaus2d(){}


void PDF_Gaus2d::initParameters()
{
  ParametersTutorial p;
  parameters = new RooArgList("parameters");
  parameters->add(*(p.get("a_gaus")));
  parameters->add(*(p.get("b_gaus")));
}


void PDF_Gaus2d::initRelations()
{
  RooArgSet *p = (RooArgSet*)parameters;
  theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
  theory->add(*(new RooFormulaVar("a_gaus_th", "a_gaus_th", "a_gaus", *p)));
  theory->add(*(new RooFormulaVar("b_gaus_th", "b_gaus_th", "b_gaus", *p)));
}


void PDF_Gaus2d::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("a_gaus_obs", "a (Gaus 2D)",  0, -1e4, 1e4)));
	observables->add(*(new RooRealVar("b_gaus_obs", "b (Gaus 2D)",  0, -1e4, 1e4)));
}


void PDF_Gaus2d::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2013") ){
		obsValSource = "1fb-1 just some values";
		setObservable("a_gaus_obs", 0.1);
		setObservable("b_gaus_obs", 1.5);
	}
	else{
		cout << "PDF_Gaus2d::setObservables() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Gaus2d::setUncertainties(TString c)
{
	if ( c.EqualTo("year2013") ){
		obsErrSource = "1fb-1 just some errors";
		StatErr[0] = 1; // a
		StatErr[1] = 1; // b
		SystErr[0] = 0; // a
		SystErr[1] = 0; // b
	}
	else{
		cout << "PDF_Gaus2d::setUncertainties() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Gaus2d::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2013") ){
		corSource = "1fb-1 just some correlations";
		corStatMatrix[1][0] = 0.6; // a, b
		corSystMatrix[1][0] = 0.0; // a, b
	}
	else{
		cout << "PDF_Gaus2d::setCorrelations() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Gaus2d::buildPdf()
{
  pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}

