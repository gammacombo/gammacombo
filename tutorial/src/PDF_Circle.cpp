/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#include "PDF_Circle.h"

	PDF_Circle::PDF_Circle(TString cObs, TString cErr, TString cCor)
: PDF_Abs(1)
{
	name = "Circle";
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
	observables->add(*(new RooRealVar("radius_obs", "radius",  0, -1e4, 1e4)));
}


void PDF_Circle::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2013") ){
		obsValSource = "year2013";
		setObservable("radius_obs",2.0);
	}
	else{
		cout << "PDF_Circle::setObservables() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Circle::setUncertainties(TString c)
{
	if ( c.EqualTo("year2013") ){
		obsErrSource = "year2013";
		StatErr[0] = 0.25;
		SystErr[0] = 0;
	}
	else{
		cout << "PDF_Circle::initCov() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Circle::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2013") ){
		corSource = "no correlations for 1 obs";
	}
	else{
		cout << "PDF_Circle::setCorrelations() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_Circle::buildPdf()
{
	pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}

