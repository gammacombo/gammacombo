/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Dec 2014
 *
 **/

#ifndef PDF_rb_h
#define PDF_rb_h

#include "PDF_Abs.h"
#include "ParametersCartesian.h"

using namespace RooFit;
using namespace std;

class PDF_rb : public PDF_Abs
{
	public:
		PDF_rb(TString cObs, TString cErr, TString cCor);
		~PDF_rb();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

