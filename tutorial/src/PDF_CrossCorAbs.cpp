/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#include "PDF_CrossCorAbs.h"

PDF_CrossCorAbs::PDF_CrossCorAbs(PDF_Abs* pdf1, PDF_Abs* pdf2)
: PDF_Abs(pdf1->getNobs()+pdf2->getNobs())
{
  name = "CrossCorAbs";
	assert(pdf1);
	assert(pdf2);
	this->pdf1 = pdf1;
	this->pdf2 = pdf2;
	title = "cross correlation: ["+pdf1->getTitle()+"] vs ["+pdf2->getTitle()+"]";
	m_isCrossCorPdf = true;
	nObs1 = pdf1->getNobs();
	nObs2 = pdf2->getNobs();
  initParameters();
  initRelations();
  initObservables();
}


PDF_CrossCorAbs::~PDF_CrossCorAbs(){}


void PDF_CrossCorAbs::initParameters()
{
  ParametersTutorial p;
  parameters = new RooArgList("parameters");
	// we need the same parameters as both input PDFs:
	// copy over from first PDF
  TIterator* it = pdf1->getParameters()->createIterator();
	while ( RooRealVar* par = (RooRealVar*)it->Next() ){
		parameters->add(*(p.get(par->GetName())));
	}
	delete it;
  // copy over from second PDF
	it = pdf2->getParameters()->createIterator();
	while ( RooRealVar* par = (RooRealVar*)it->Next() ){
		if ( parameters->find(par->GetName()) ) continue;
		parameters->add(*(p.get(par->GetName())));
	}
	delete it;
}


void PDF_CrossCorAbs::initRelations()
{
  theory = new RooArgList("theory");
	// we need the same theory as both input PDFs:
	// copy over from first PDF
  TIterator* it = pdf1->getTheory()->createIterator();
	while ( RooRealVar* th = (RooRealVar*)it->Next() ){
		theory->add(*th);
	}
	delete it;
  // copy over from second PDF
	it = pdf2->getTheory()->createIterator();
	while ( RooRealVar* th = (RooRealVar*)it->Next() ){
		theory->add(*th);
	}
	delete it;
}


void PDF_CrossCorAbs::initObservables()
{
	observables = new RooArgList("observables");
	// we need the same observables as both input PDFs:
	// copy over from first PDF
  TIterator* it = pdf1->getObservables()->createIterator();
	while ( RooRealVar* obs = (RooRealVar*)it->Next() ){
		observables->add(*obs);
	}
	delete it;
  // copy over from second PDF
	it = pdf2->getObservables()->createIterator();
	while ( RooRealVar* obs = (RooRealVar*)it->Next() ){
		observables->add(*obs);
	}
	delete it;
}


void PDF_CrossCorAbs::copyMeasurementCovariance()
{
	// copy errors
	for ( int i=0; i<nObs; i++ ){
		if ( i < pdf1->getNobs() ){
			StatErr[i] = pdf1->StatErr[i];
			SystErr[i] = pdf1->SystErr[i];
		}
		else if ( i >= pdf1->getNobs() ){
			int shift = pdf1->getNobs();
			StatErr[i] = pdf2->StatErr[i-shift];
			SystErr[i] = pdf2->SystErr[i-shift];
		}
	}
	// copy correlations
	for ( int i=0; i<nObs; i++ )
	for ( int j=0; j<nObs; j++ ){
		if ( i < pdf1->getNobs() && j < pdf1->getNobs() ){
			corStatMatrix[i][j] = pdf1->corStatMatrix[i][j];
			corSystMatrix[i][j] = pdf1->corSystMatrix[i][j];
		}
		else if ( i >= pdf1->getNobs() && j >= pdf1->getNobs() ){
			int shift = pdf1->getNobs();
			corStatMatrix[i][j] = pdf2->corStatMatrix[i-shift][j-shift];
			corSystMatrix[i][j] = pdf2->corSystMatrix[i-shift][j-shift];
		}
	}
}


void PDF_CrossCorAbs::setCorrelations(config c){assert(0);};


void PDF_CrossCorAbs::buildPdf()
{
	TMatrixDSym covInverse = covMatrix;
	covInverse.Invert();
	pdf = new RooCrossCorPdf("pdf_"+name, "pdf_"+name, *observables, *theory, covInverse, nObs1);
}
