/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: November 2014
 *
 **/

#include "PDF_rb.h"

	PDF_rb::PDF_rb(TString cObs, TString cErr, TString cCor)
: PDF_Abs(1)
{
	name = "rb";
	initParameters();
	initRelations();
	initObservables();
	setObservables(cObs);
	setUncertainties(cErr);
	setCorrelations(cCor);
	buildCov();
	buildPdf();
}


PDF_rb::~PDF_rb(){}


void PDF_rb::initParameters()
{
	ParametersCartesian p;
	parameters = new RooArgList("parameters");
	parameters->add(*(p.get("r_dk")));
}


void PDF_rb::initRelations()
{
	theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
	theory->add(*(new RooFormulaVar("r_dk_th", "r_dk_th", "r_dk", *(RooArgSet*)parameters)));
}


void PDF_rb::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("r_dk_obs", "r_dk_obs",  0, -1e4, 1e4)));
}


void PDF_rb::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2013") ){
		obsValSource = c;
		setObservable("r_dk_obs",0.1);
	}
	else {
		cout << "PDF_rb::setObservables() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_rb::setUncertainties(TString c)
{
	if ( c.EqualTo("year2013") ){
		obsErrSource = c;
		StatErr[0] = 0.01;	// r_dk
		SystErr[0] = 0;		// r_dk
	}
	else {
		cout << "PDF_rb::setUncertainties() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_rb::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2013") ){
		corSource = "no correlations for 1 obs";
	}
	else { 
		cout << "PDF_rb::setCorrelations() : ERROR : config "+c+" not found." << endl;
		exit(1);    
	}
}


void PDF_rb::buildPdf()
{
	pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}

