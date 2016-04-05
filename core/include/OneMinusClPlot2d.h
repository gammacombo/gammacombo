/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef OneMinusClPlot2d_h
#define OneMinusClPlot2d_h

#include <TROOT.h>
#include "TMultiGraph.h"
#include "TSystem.h"

#include "OneMinusClPlotAbs.h"
#include "Utils.h"
#include "ColorBuilder.h"
#include "ConfidenceContours.h"

using namespace Utils;
using namespace RooFit;
using namespace std;

class OneMinusClPlot2d : public OneMinusClPlotAbs
{
	public:

		OneMinusClPlot2d(OptParser *arg, TString name="c1", TString title="c1");

		void            addScanner(MethodAbsScan* s);
		void            addFile(TString fName);
		void            Draw();
		void            DrawFull();
		void		        drawCLcontent(bool isFull=false);
		void            drawMarker(float x, float y, int color=0, int style=3, float size=2.0);
		void            drawGroup();
		void            drawSolutions();
		inline int      getNumberOfDefinedColors(){return linecolor[0].size();}
		inline void     setContoursOnly(){contoursOnly = true;};
		inline void     setXaxisTitle(TString s){xTitle=s;};
		inline void     setYaxisTitle(TString s){yTitle=s;};

	protected:

		vector<TH2F*>	histos;
		TString		xTitle;
		TString 	yTitle;
		bool		contoursOnly;
		vector<vector<int> > linecolor;   ///< defines colors of 1 sigma lines and solutions of different scanners
		vector<vector<int> > fillcolor;   ///< defines colors of 1 sigma areas of different scanners
		vector<vector<int> > linestyle;   ///< defines the style of 1 sigma line of different scanners
		vector<vector<int> > fillstyle;
		vector<int>          markerstyle; ///< defines marker styles of the solutions of different scanners
		vector<float>        markersize;

	private:

		void            drawLegend();
		bool 		    hasHistoType(histogramType t);
		void            makeNewPlotStyle(TString htmlColor, int ROOTColor=-1);

		vector<histogramType>       histosType; ///< defines if histogram is interpreted as p-value or chi2
		vector<ConfidenceContours*> m_contours; ///< holds the contours for each scanner
		vector<bool>                m_contours_computed; ///< true if the contours were computed for that scanner by computeContours()
		TLegend*                    m_legend;   ///< pointer to the plot legend. Filled by drawLegend().
};

#endif
