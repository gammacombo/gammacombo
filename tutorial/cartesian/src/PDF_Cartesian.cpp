/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Dec 2014
 *
 **/

#include "PDF_Cartesian.h"

	PDF_Cartesian::PDF_Cartesian(TString cObs, TString cErr, TString cCor)
: PDF_Abs(4) // <-- configure the number of observables
{
	name = "cartesian";
	initParameters();
	initRelations();
	initObservables();
	setObservables(cObs);
	setUncertainties(cErr);
	setCorrelations(cCor);
	buildCov();
	buildPdf();
}

PDF_Cartesian::~PDF_Cartesian(){}


void PDF_Cartesian::initParameters()
{
	ParametersCartesian p; // <-- use the project's parameter class
	parameters = new RooArgList("parameters");
	parameters->add(*(p.get("r_dk")));
	parameters->add(*(p.get("d_dk")));
	parameters->add(*(p.get("g")));
}


void PDF_Cartesian::initRelations()
{
	theory = new RooArgList("theory"); // <-- the order of this list must match that of the COR matrix!
	RooArgSet *p = (RooArgSet*)parameters;
	theory->add(*(new RooFormulaVar("xm_dk_th", "xm_dk_th", "r_dk*cos(d_dk-g)", *p)));
	theory->add(*(new RooFormulaVar("ym_dk_th", "ym_dk_th", "r_dk*sin(d_dk-g)", *p)));
	theory->add(*(new RooFormulaVar("xp_dk_th", "xp_dk_th", "r_dk*cos(d_dk+g)", *p)));
	theory->add(*(new RooFormulaVar("yp_dk_th", "yp_dk_th", "r_dk*sin(d_dk+g)", *p)));
}


void PDF_Cartesian::initObservables()
{
	observables = new RooArgList("observables"); // <-- the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("xm_dk_obs", "x-", 0, -1, 1)));
	observables->add(*(new RooRealVar("ym_dk_obs", "y-", 0, -1, 1)));
	observables->add(*(new RooRealVar("xp_dk_obs", "x+", 0, -1, 1)));
	observables->add(*(new RooRealVar("yp_dk_obs", "y+", 0, -1, 1)));
}


void PDF_Cartesian::setObservables(TString c)
{
	if ( c.EqualTo("truth") ){
		setObservablesTruth();
	}
	else if ( c.EqualTo("toy") ){
		setObservablesToy();
	}
	else if ( c.EqualTo("year2014") ){
		obsValSource = "arxiv:1408.2748";
		setObservable("xm_dk_obs", 2.5e-2);
		setObservable("ym_dk_obs", 7.5e-2);
		setObservable("xp_dk_obs",-7.7e-2);
		setObservable("yp_dk_obs",-2.2e-2);
	}
	else {
		cout << "PDF_Cartesian::setObservables() : ERROR : config not found: " << c << endl;
		exit(1);
	}
}


void PDF_Cartesian::setUncertainties(TString c)
{
	if ( c.EqualTo("year2014") ){
		obsErrSource = "arxiv:1408.2748";
		StatErr[0] = 0.025; // xm
		StatErr[1] = 0.029; // ym
		StatErr[2] = 0.024; // xp
		StatErr[3] = 0.025; // yp
		SystErr[0] = 0.011; // xm
		SystErr[1] = 0.015; // ym
		SystErr[2] = 0.011; // xp
		SystErr[3] = 0.011; // yp
	}
	else {
		cout << "PDF_Cartesian::setUncertainties() : ERROR : config not found: " << c << endl;
		exit(1);
	}
}


void PDF_Cartesian::setCorrelations(TString c)
{
	resetCorrelations();
	if ( c.EqualTo("year2014") ){
		corSource = "arxiv:1408.2748";
		double dataStat[]  = {
			// xm      ym      xp      yp
			 1.   , -0.247,  0.038, -0.003, // xm
			-0.247,  1.   , -0.011,  0.012, // ym
			 0.038, -0.011,  1.   ,  0.002, // xp
			-0.003,  0.012,  0.002,  1.     // yp
		};
		corStatMatrix = TMatrixDSym(nObs,dataStat);
		double dataSyst[]  = {
			// xm      ym      xp      yp
			 1.   ,  0.005, -0.025,  0.070, // xm
			 0.005,  1.   ,  0.009, -0.141, // ym
			-0.025,  0.009,  1.   ,  0.008, // xp
			 0.070, -0.141,  0.008,  1.     // yp
		};
		corSystMatrix = TMatrixDSym(nObs,dataSyst);
	}
	else {
		cout << "PDF_Cartesian::initCorrelations() : ERROR : config not found: " << c << endl;
		exit(1);
	}
}


void PDF_Cartesian::buildPdf()
{
	pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}

