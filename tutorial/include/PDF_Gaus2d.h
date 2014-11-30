/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: November 2014
 *
 **/

#ifndef PDF_Gaus2d_h
#define PDF_Gaus2d_h

#include "PDF_Abs.h"
#include "ParametersTutorial.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_Gaus2d : public PDF_Abs
{
	public:
		PDF_Gaus2d(TString cObs="year2013", TString cErr="year2013", TString cCor="year2013");
		~PDF_Gaus2d();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

