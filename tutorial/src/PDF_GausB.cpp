/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: November 2014
 *
 **/

#include "PDF_GausB.h"

	PDF_GausB::PDF_GausB(TString cObs, TString cErr, TString cCor)
: PDF_Abs(1)
{
	name = "GausB";
	initParameters();
	initRelations();
	initObservables();
	setObservables(cObs);
	setUncertainties(cErr);
	setCorrelations(cCor);
	buildCov();
	buildPdf();
}


PDF_GausB::~PDF_GausB(){}


void PDF_GausB::initParameters()
{
	ParametersTutorial p;
	parameters = new RooArgList("parameters");
  parameters->add(*(p.get("a_gaus")));
  parameters->add(*(p.get("b_gaus")));
}


void PDF_GausB::initRelations()
{
	theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
	theory->add(*(new RooFormulaVar("b_gaus_th", "b_gaus_th", "b_gaus", *(RooArgSet*)parameters)));
}


void PDF_GausB::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("b_gaus_obs", "b_gaus_obs",  0, -1e4, 1e4)));
}


void PDF_GausB::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2013") ){
		obsValSource = c;
		setObservable("b_gaus_obs",1.5);
	}
	else if ( c.EqualTo("year2014") ){
		obsValSource = c;
		setObservable("b_gaus_obs",1.5);
	}
	else{
		cout << "PDF_GausB::setObservables() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_GausB::setUncertainties(TString c)
{
	if ( c.EqualTo("year2013") ){
		obsErrSource = c;
		StatErr[0] = 0.25; // a_gaus
		SystErr[0] = 0; // a_gaus
	}
	else if ( c.EqualTo("year2014") ){
		obsErrSource = c;
		StatErr[0] = 0.5; // a_gaus
		SystErr[0] = 0.15; // a_gaus
	}
	else{
		cout << "PDF_GausB::setUncertainties() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_GausB::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2013") ){
		corSource = "no correlations for 1 obs";
	}
	else if ( c.EqualTo("year2014") ){
		corSource = "no correlations for 1 obs";
	}
	else{
		cout << "PDF_GausB::setCorrelations() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_GausB::buildPdf()
{
	pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}

