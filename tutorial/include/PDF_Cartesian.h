/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: November 2014
 *
 **/

#ifndef PDF_Cartesian_h
#define PDF_Cartesian_h

#include "PDF_Abs.h"
#include "ParametersCartesian.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_Cartesian : public PDF_Abs
{
	public:
		PDF_Cartesian(TString cObs, TString cErr, TString cCor);
		~PDF_Cartesian();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

