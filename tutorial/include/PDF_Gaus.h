/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef PDF_Gaus_h
#define PDF_Gaus_h

#include "PDF_Abs.h"
#include "ParametersTutorial.h"

using namespace RooFit;
using namespace std;

class PDF_Gaus : public PDF_Abs
{
	public:
		PDF_Gaus(TString cObs="year2014", TString cErr="year2014", TString cCor="year2014");
		~PDF_Gaus();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

