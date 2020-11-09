/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#include "PDF_DoubleCB.h"
#include "RooDoubleCB.h"

	PDF_DoubleCB::PDF_DoubleCB(TString cObs, TString cErr, TString cCor)
: PDF_Abs(1)
{
	name = "DoubleCB";
	initParameters();
	initObservables();
	setObservables(cObs);
	setUncertainties(cErr);
	setCorrelations(cCor);
	buildCov();
	buildPdf();
	initRelations();
}


PDF_DoubleCB::~PDF_DoubleCB(){}


void PDF_DoubleCB::initParameters()
{
	ParametersTutorial p;
	parameters = new RooArgList("parameters");
	parameters->add(*(p.get("mean_DSCB")));
	parameters->add(*(p.get("sigma_DSCB")));
	parameters->add(*(p.get("alpha1_DSCB")));
	parameters->add(*(p.get("n1_DSCB")));
	parameters->add(*(p.get("alpha2_DSCB")));
	parameters->add(*(p.get("n2_DSCB")));
}


void PDF_DoubleCB::initRelations()
{
	theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
	//theory->add(*(new RooFormulaVar("DSCB_th", "DSCB_th", "pdf", *(RooArgSet*)parameters)));
	theory->add(*(new RooFormulaVar("DSCB_th", "DSCB_th", "pdf", *(RooArgSet*)pdf)));
}


void PDF_DoubleCB::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("Lb_M", "mass_Lb",  5600, 5300, 5700)));
}


void PDF_DoubleCB::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2013") ){
		obsValSource = "year2013";
		setObservable("Lb_M",5600);
	}
	else{
		cout << "PDF_DoubleCB::setObservables() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_DoubleCB::setUncertainties(TString c)
{
	if ( c.EqualTo("year2013") ){
		obsErrSource = "year2013";
		StatErr[0] = 0;
		SystErr[0] = 0;
	}
	else{
		cout << "PDF_DoubleCB::initCov() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_DoubleCB::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2013") ){
		corSource = "no correlations for 1 obs";
	}
	else{
		cout << "PDF_DoubleCB::setCorrelations() : ERROR : config "+c+" not found." << endl;
		exit(1);
	}
}


void PDF_DoubleCB::buildPdf(){
    //ParametersTutorial p;
    //pdf = new RooFit.RooDoubleCB("pdf"+name, "pdf_"+name, *(RooArgSet*)observables, *(p.get("mean_DSCB")), *(p.get("sigma_DSCB")),
    //    *(p.get("alpha1_DSCB")), *(p.get("n1_DSCB")), *(p.get("alpha2_DSCB")), *(p.get("n2_DSCB") );
    pdf = new RooDoubleCB("pdf"+name, "pdf_"+name, *(RooRealVar*)observables->at(0), *(RooRealVar*)parameters->at(0), *(RooRealVar*)parameters->at(1) 
            , *(RooRealVar*)parameters->at(2), *(RooRealVar*)parameters->at(3), *(RooRealVar*)parameters->at(4), *(RooRealVar*)parameters->at(5));
}

