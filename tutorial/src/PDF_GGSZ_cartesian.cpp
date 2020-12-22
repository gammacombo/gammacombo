/**
 * Author: Matthew Kenzie <matthew.kenzie@cern.ch>
 * Date: December 2020
 *
 **/

#include "PDF_GGSZ_cartesian.h"

	PDF_GGSZ_cartesian::PDF_GGSZ_cartesian(TString cObs, TString cErr, TString cCor)
: PDF_GGSZ(cObs,cErr,cCor)
{
	name = "ggsz_dk_cart";
	initParameters();
	initRelations();
	delete pdf; // it was built already by the parent constructor
	buildPdf();
}

PDF_GGSZ_cartesian::~PDF_GGSZ_cartesian(){}


void PDF_GGSZ_cartesian::initParameters()
{
	parameters = new RooArgList("parameters");
	parameters->add(*(p->get("xp_dk")));
	parameters->add(*(p->get("yp_dk")));
	parameters->add(*(p->get("xm_dk")));
	parameters->add(*(p->get("ym_dk")));
}


void PDF_GGSZ_cartesian::initRelations()
{
	RooArgSet *p = (RooArgSet*)parameters;
	delete theory;
  theory = new RooArgList("theory"); ///< the order of this list must match that of the COR matrix!
	theory->add(*(new RooFormulaVar("xp_dk_th", "x+ (DK)", "xp_dk", *p)));
	theory->add(*(new RooFormulaVar("yp_dk_th", "y+ (DK)", "yp_dk", *p)));
	theory->add(*(new RooFormulaVar("xm_dk_th", "x- (DK)", "xm_dk", *p)));
	theory->add(*(new RooFormulaVar("ym_dk_th", "y- (DK)", "ym_dk", *p)));
}
