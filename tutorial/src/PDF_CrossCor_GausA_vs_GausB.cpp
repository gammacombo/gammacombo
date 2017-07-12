/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#include "PDF_CrossCor_GausA_vs_GausB.h"

PDF_CrossCor_GausA_vs_GausB::PDF_CrossCor_GausA_vs_GausB(PDF_Abs* pdf1, PDF_Abs* pdf2, TString cCor)
: PDF_CrossCorAbs(pdf1,pdf2)
{
  name = "CrossCor_GausA_vs_GausB";
  setCorrelations(cCor);
	buildCov();
  buildPdf();
}


PDF_CrossCor_GausA_vs_GausB::~PDF_CrossCor_GausA_vs_GausB(){}


void PDF_CrossCor_GausA_vs_GausB::setCorrelations(TString c)
{
  resetCorrelations();
	copyMeasurementCovariance();
  if ( c.EqualTo("year2013") ) {
    corSource = "cross correlations";
    float c = 0.6;
    corStatMatrix[0][0+nObs1] = c;	//
    corStatMatrix[0+nObs1][0] = c;	//
  }
  else{
    cout << "PDF_CrossCor_GausA_vs_GausB::setCorrelations() : ERROR : config "+c+" not found." << endl;
    exit(1);
  }
}

