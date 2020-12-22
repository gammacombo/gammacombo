/**
 * Author: Matthew Kenzie <matthew.kenzie@cern.ch>
 * Date: December 2020
 *
 **/

#include "PDF_GGSZ.h"

	PDF_GGSZ::PDF_GGSZ(TString cObs, TString cErr, TString cCor)
: PDF_Abs(4)
{
	p = new ParametersGamma();
	name = "ggsz_dk";
	initParameters();
	initRelations();
	initObservables();
	setObservables(cObs);
	setUncertainties(cErr);
	setCorrelations(cCor);
	buildCov();
	buildPdf();
}

PDF_GGSZ::~PDF_GGSZ(){}


void PDF_GGSZ::initParameters()
{
	parameters = new RooArgList("parameters");
	parameters->add(*(p->get("r_dk")));
	parameters->add(*(p->get("d_dk")));
	parameters->add(*(p->get("g")));
}


void PDF_GGSZ::initRelations()
{
	theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
	RooArgSet *p = (RooArgSet*)parameters;
	theory->add(*(new RooFormulaVar("xp_dk_th", "x+ (DK)", "r_dk*cos(d_dk+g)", *p)));
	theory->add(*(new RooFormulaVar("yp_dk_th", "y+ (DK)", "r_dk*sin(d_dk+g)", *p)));
	theory->add(*(new RooFormulaVar("xm_dk_th", "x- (DK)", "r_dk*cos(d_dk-g)", *p)));
	theory->add(*(new RooFormulaVar("ym_dk_th", "y- (DK)", "r_dk*sin(d_dk-g)", *p)));
}


void PDF_GGSZ::initObservables()
{
	observables = new RooArgList("observables"); ///< the order of this list must match that of the COR matrix!
	observables->add(*(new RooRealVar("xp_dk_obs", "x+ (DK GGSZ) obs", 0, -1, 1)));
	observables->add(*(new RooRealVar("yp_dk_obs", "y+ (DK GGSZ) obs", 0, -1, 1)));
	observables->add(*(new RooRealVar("xm_dk_obs", "x- (DK GGSZ) obs", 0, -1, 1)));
	observables->add(*(new RooRealVar("ym_dk_obs", "y- (DK GGSZ) obs", 0, -1, 1)));
}


void PDF_GGSZ::setObservables(TString c)
{
  if ( c.EqualTo("truth") ) {
    setObservablesTruth();
  }
  else if ( c.EqualTo("toy") ) {
    setObservablesToy();
  }
  else if ( c.EqualTo("belle") ) {
    obsValSource = "Some name";
    setObservable("xp_dk_obs"  , -0.107 );
    setObservable("yp_dk_obs"  , -0.067 );
    setObservable("xm_dk_obs"  ,  0.105 );
    setObservable("ym_dk_obs"  ,  0.177 );
  }
	else {
    cout << "PDF_GGSZ::setObservables() : ERROR : obs config " << c << " not found." << endl;
    exit(1);
	}
}


void PDF_GGSZ::setUncertainties(TString c)
{
  if ( c.EqualTo("belle") ) {
    obsErrSource = "Some name";
    StatErr[0] =  0.043 ; // xp_dk
    StatErr[1] =  0.059 ; // yp_dk
    StatErr[2] =  0.047 ; // xm_dk
    StatErr[3] =  0.060 ; // ym_dk
    SystErr[0] =  0.011 ; // xp_dk
    SystErr[1] =  0.018 ; // yp_dk
    SystErr[2] =  0.011 ; // xm_dk
    SystErr[3] =  0.018 ; // ym_dk
  }
	else {
    cout << "PDF_GGSZ::setUncertainties() : ERROR : obs config " << c << " not found." << endl;
    exit(1);
	}
}


void PDF_GGSZ::setCorrelations(TString c)
{
	resetCorrelations();
  if ( c.EqualTo("none") || c.EqualTo("off") ) {
    corSource = "correlations off";
  }
  else if ( c.EqualTo("belle") ) {
    const double statCorrs[16] = {
       1.000 ,  0.110 ,  0.000 ,  0.000 ,
       0.110 ,  1.000 ,  0.000 ,  0.000 ,
       0.000 ,  0.000 ,  1.000 , -0.289 ,
       0.000 ,  0.000 , -0.289 ,  1.000 };

    corStatMatrix = TMatrixDSym( nObs, statCorrs );

    const double systCorrs[16] = {
       1.000 , -0.273 , -0.725 , -0.257 ,
      -0.273 ,  1.000 ,  0.455 , -0.686 ,
      -0.725 ,  0.455 ,  1.000 , -0.260 ,
      -0.257 , -0.686 , -0.260 ,  1.000 };

     corSystMatrix = TMatrixDSym( nObs, systCorrs );

  }
	else {
    cout << "PDF_GGSZ::setCorrelations() : ERROR : obs config " << c << " not found." << endl;
    exit(1);
	}
}


void PDF_GGSZ::buildPdf()
{
	pdf = new RooMultiVarGaussian("pdf_"+name, "pdf_"+name, *(RooArgSet*)observables, *(RooArgSet*)theory, covMatrix);
}
