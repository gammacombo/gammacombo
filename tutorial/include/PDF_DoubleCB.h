/**
 * Implementing RooDoubleCB from tools-easyanalysis
 * Author: Paul Nathaniel Swallow, paul.nathaniel.swallow@cern.ch
 * Date: Nov 2020
 *
 **/

#ifndef PDF_DoubleCB_h
#define PDF_DoubleCB_h

#include "PDF_Abs.h"
#include "ParametersTutorial.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_DoubleCB : public PDF_Abs
{
	public:
		PDF_DoubleCB(TString cObs="year2013", TString cErr="year2013", TString cCor="year2013");
		~PDF_DoubleCB();
		void          buildPdf();
		void          initObservables();
		virtual void  initParameters();
		virtual void  initRelations();
		void          setCorrelations(TString c);
		void          setObservables(TString c);
		void          setUncertainties(TString c);
};

#endif

