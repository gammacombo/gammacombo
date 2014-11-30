/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: July 2014
 *
 **/

#ifndef PDF_Circle_h
#define PDF_Circle_h

#include "PDF_Abs.h"
#include "ParametersTutorial.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

///
/// Part of the tutorial. Just a PDF that defines a circle
/// in a_gaus and b_gaus.
///
class PDF_Circle : public PDF_Abs
{
	public:
		PDF_Circle(TString cObs="year2013", TString cErr="year2013", TString cCor="year2013");
		~PDF_Circle();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

