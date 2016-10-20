/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#ifndef MethodAbsScan_h
#define MethodAbsScan_h

#include <stdlib.h>

#include "RooDataSet.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TPaveText.h"
#include "TF1.h"
#include "TDatime.h"

#include "Utils.h"
#include "OneMinusClPlotAbs.h"
#include "OptParser.h"
#include "Combiner.h"
#include "Fitter.h"
#include "Rounder.h"
#include "PullPlotter.h"
#include "RooSlimFitResult.h"
#include "FitResultCache.h"
#include "CLInterval.h"
#include "CLIntervalPrinter.h"
#include "CLIntervalMaker.h"
#include "FileNameBuilder.h"
#include "PValueCorrection.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class OneMinusClPlotAbs;

class MethodAbsScan
{
	public:
		MethodAbsScan(Combiner* c);
		MethodAbsScan();
		~MethodAbsScan();

		void                            calcCLintervals();
		void                            confirmSolutions();
		void                            doInitialFit(bool force=false);
		inline OptParser*               getArg(){return arg;};
		inline const vector<RooSlimFitResult*>& getAllResults(){return allResults;};
		inline const vector<RooSlimFitResult*>& getCurveResults(){return curveResults;};
		inline float                    getChi2minGlobal(){return chi2minGlobal;}
		float                           getCL(double val);
		CLInterval                      getCLintervalCentral(int sigma=1);
		inline Combiner* 				getCombiner() const {return combiner;};
		int                             getDrawSolution();
		inline bool                     getFilled(){return drawFilled;};
		inline TH1F*                    getHCL(){return hCL;};
		inline TH2F*                    getHCL2d(){return hCL2d;};
		inline TH1F*                    getHchisq(){return hChi2min;};
		inline TH2F*                    getHchisq2d(){return hChi2min2d;};
		inline int                      getLineColor(){return lineColor;};
		inline int                      getLineStyle(){return lineStyle;};
		inline int                      getFillStyle(){return fillStyle;};
		inline TString                  getMethodName() const {return methodName;};
		inline TString                  getName() const {return name;};
		inline int                      getNObservables(){return w->set(obsName)->getSize();}
		inline int                      getNPoints1d(){return nPoints1d;}
		inline int                      getNPoints2dx(){return nPoints2dx;}
		inline int                      getNPoints2dy(){return nPoints2dy;}
		inline int                      getNSolutions(){return solutions.size();};
		inline const RooArgSet*         getObservables(){return w->set(obsName);}
		inline TString			getObsName(){return obsName;};
		inline TString			getParsName(){return parsName;};
		float                           getScanVarSolution(int iVar, int iSol);
		RooRealVar*                     getScanVar1();
		TString													getScanVar1Name();
		float                           getScanVar1Solution(int i=0);
		RooRealVar*                     getScanVar2();
		TString							getScanVar2Name();
		float                           getScanVar2Solution(int i=0);
		inline vector<RooSlimFitResult*>    getSolutions(){return solutions;};
		RooSlimFitResult*                   getSolution(int i=0);
		inline const RooArgSet*         getTheory(){return w->set(thName);}
		inline int                      getTextColor(){return textColor;};
		inline TString                  getTitle(){return title;};
		inline RooWorkspace*            getWorkspace(){return w;};
		virtual void                    initScan();
		void                            loadParameters(RooSlimFitResult *r);
		bool                            loadSolution(int i=0);
		bool                            loadScanner(TString fName="");
		void                            plot2d(TString varx, TString vary);
		void                            plot1d(TString var);
		void                            plotOn(OneMinusClPlotAbs *plot);
		void                            plotPulls(int nSolution=0);
		virtual void                    print();
		void                            printCLintervals();
		void                            printLocalMinima();
		void                            saveScanner(TString fName="");
		virtual int                     scan1d();
		virtual int                     scan2d();
		inline void                     setDrawSolution(int code=0){drawSolution = code;};
		inline void 					setPValueCorrector(PValueCorrection *pvalCor) { pvalueCorrector = pvalCor; pvalueCorrectorSet=true; }
		inline void                     setScanVar1(TString var){scanVar1 = var;};
		inline void                     setScanVar2(TString var){scanVar2 = var;};
		inline void                     setNPoints1d(int n){nPoints1d = n;};
		inline void                     setNPoints2dx(int n){nPoints2dx = n;};
		inline void                     setNPoints2dy(int n){nPoints2dy = n;};
		inline void                     setFilled(bool filled){ drawFilled = filled;};
		inline void                     setLineColor(int c){lineColor = c;};
		inline void                     setLineStyle(int c){lineStyle = c;};
		inline void                     setTextColor(int c){textColor = c;};
		inline void                     setFillStyle(int c){fillStyle = c;};
		inline void                     setTitle(TString s){title = s;};
		void                            setChi2minGlobal(double x);
		void                            setSolutions(vector<RooSlimFitResult*> s);
		inline void                     setVerbose(bool yesNo=true){verbose = yesNo;};
    inline void                     setHCL( TH1F *h ) { hCL = h; };
    inline void                     setHchisq( TH1F *h ) { hChi2min = h; };
		void 							setXscanRange(float min, float max);
		void 							setYscanRange(float min, float max);

		vector<RooSlimFitResult*> allResults;           ///< All fit results we encounter along the scan.
		vector<RooSlimFitResult*> curveResults;         ///< All fit results of the the points that make it into the 1-CL curve.
		///< Index is the bin number of hCL bins -1.
		vector<vector<RooSlimFitResult*> > curveResults2d;  ///< All fit results of the the points that make it into the 1-CL curve.
		///< Index is the gobal bin number of hCL2d -1.
		vector<RooSlimFitResult*> solutions;            ///< Local minima filled by saveSolutions() and saveSolutions2d().
		vector<CLInterval> clintervals1sigma;           ///< all 1 sigma intervals that were found by calcCLintervals()
		vector<CLInterval> clintervals2sigma;           ///< all 2 sigma intervals that were found by calcCLintervals()
		vector<CLInterval> clintervals3sigma;           ///< all 3 sigma intervals that were found by calcCLintervals()

	protected:

		void    sortSolutions();

		TString name;       ///< basename, e.g. ggsz
		TString title;      ///< nice string for the legends
		TString methodName; ///< Prob, ...
		TString pdfName;    ///< PDF name in workspace, derived from name
		TString obsName;    ///< dataset name of observables, derived from name
		TString parsName;   ///< set name of physics parameters, derived from name
		TString thName;     ///< set name of theory parameters, derived from name
    TString toysName;   ///< set name of parameters to vary in toys
		TString scanVar1;   ///< scan parameter
		TString scanVar2;   ///< second scan parameter if we're scanning 2d
		int nPoints1d;      ///< number of scan points used by 1d scan
		int nPoints2dx;     ///< number of scan points used by 2d scan, x axis
		int nPoints2dy;     ///< number of scan points used by 2d scan, y axis

		PValueCorrection* pvalueCorrector; // object which can correct the pvalue for undercoverage if required
		bool pvalueCorrectorSet;

		TRandom3 rndm;
		RooWorkspace* w;
		RooDataSet* obsDataset;     ///< save the nominal observables so we can restore them after we have fitted toys
		RooDataSet* startPars;      ///< save the start parameter values before any scan
		RooFitResult* globalMin;    ///< parameter values at a global minimum
		TH1F* hCL;                  ///< 1-CL curve
		TH2F* hCL2d;                ///< 1-CL curve
		TH1F* hChi2min;             ///< histogram for the chi2min values before Prob()
		TH2F* hChi2min2d;           ///< histogram for the chi2min values before Prob()
		double chi2minGlobal;       ///< chi2 value at global minimum
		bool chi2minGlobalFound;    ///< flag to avoid finding minimum twice
		int lineColor;
		int textColor;              ///< color used for plotted central values
		int lineStyle;
    int fillStyle;
		bool drawFilled;            ///< choose if Histogram is drawn filled or not
		int drawSolution;           ///< Configure how to draw solutions on the plots.
		///< 0=don't plot, 1=plot at central value (1d) or markers (2d)
		///< Default is taken from arg, unless disabled by setDrawSolution().
		bool verbose;
		int nWarnings;              ///< number of warnings printed in getScanVarSolution()
		OptParser* arg;             ///< command line options
		Combiner* combiner;         ///< the combination
		bool m_xrangeset; 			///< true if the x range was set manually (setXscanRange())
		bool m_yrangeset; 			///< true if the y range was set manually (setYscanRange())
		bool m_initialized; 		///< true if initScan() was called

	private:

		bool    compareSolutions(RooSlimFitResult* r1, RooSlimFitResult* r2);
		float   pq(float p0, float p1, float p2, float y, int whichSol=0);
		void    removeDuplicateSolutions();
		bool    interpolate(TH1F* h, int i, float y, float central, bool upper, float &val, float &err);
		void    interpolateSimple(TH1F* h, int i, float y, float &val);
};

#endif
