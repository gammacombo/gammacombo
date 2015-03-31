/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef OneMinusClPlot_h
#define OneMinusClPlot_h

#include "OneMinusClPlotAbs.h"
#include "Utils.h"
#include "Rounder.h"

using namespace Utils;
using namespace RooFit;
using namespace std;

class OneMinusClPlot : public OneMinusClPlotAbs
{
	public:
		OneMinusClPlot(OptParser *arg, TString name="c1", TString title="c1");

		void            drawSolutions();
		void            drawCLguideLines();
		TGraph*         getGraph(MethodAbsScan* s, bool first=true, bool last=false, bool filled=true){return scan1dPlot(s,first,last,filled);};
		inline TString  getName(){return name;};
		inline void     setPluginMarkers(bool yesNo=true){plotPluginMarkers = yesNo;};
		void            Draw();
		TGraph*         addPointToGraphAtFirstMatchingX(TGraph* g, float xNew, float yNew);

	private:
		void            drawCLguideLine(float pvalue);
		void            drawVerticalLine(float x, int color, int style);
		TGraph*         scan1dPlot(MethodAbsScan* s, bool first, bool last, bool filled);
		void            scan1dPlotSimple(MethodAbsScan* s, bool first);

		bool            plotPluginMarkers;
};

#endif
