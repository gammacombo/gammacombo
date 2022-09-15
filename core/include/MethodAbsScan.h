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
#include "TSpline.h"

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
        MethodAbsScan();
        MethodAbsScan(Combiner* c);
        MethodAbsScan(OptParser* opt);
        ~MethodAbsScan();

        virtual void                    calcCLintervals(int CLsType = 0, bool calc_expected=false, bool quiet=false);
        void                            confirmSolutions();
        void                            doInitialFit(bool force=false);
        inline OptParser*               getArg(){return arg;};
        inline const vector<RooSlimFitResult*>& getAllResults(){return allResults;};
        inline const vector<RooSlimFitResult*>& getCurveResults(){return curveResults;};
        inline float                    getChi2minGlobal(){return chi2minGlobal;}
        inline float                    getChi2minBkg(){return chi2minBkg;}
        float                           getCL(double val);
        CLInterval                      getCLintervalCentral(int sigma=1, bool quiet=false);
        CLInterval                      getCLinterval(int iSol=0, int sigma=1, bool quiet=false);
        inline Combiner*                getCombiner() const {return combiner;};
        int                             getDrawSolution();
        inline bool                     getFilled(){return drawFilled;};
        inline TH1F*                    getHCL(){return hCL;};
        inline TH1F*                    getHCLs(){return hCLs;};
        inline TH1F*                    getHCLsFreq(){return hCLsFreq;};
        inline TH1F*                    getHCLsExp(){return hCLsExp;};
        inline TH1F*                    getHCLsErr1Up(){return hCLsErr1Up;};
        inline TH1F*                    getHCLsErr1Dn(){return hCLsErr1Dn;};
        inline TH1F*                    getHCLsErr2Up(){return hCLsErr2Up;};
        inline TH1F*                    getHCLsErr2Dn(){return hCLsErr2Dn;};
        inline TH2F*                    getHCL2d(){return hCL2d;};
        inline TH2F*                    getHCLs2d(){return hCLs2d;};
        inline TH1F*                    getHchisq(){return hChi2min;};
        inline TH2F*                    getHchisq2d(){return hChi2min2d;};
        inline int                      getLineColor(){return lineColor;};
        inline int                      getLineStyle(){return lineStyle;};
        inline int                      getLineWidth(){return lineWidth;};
        inline int                      getFillStyle(){return fillStyle;};
        inline int                      getFillColor(){return fillColor;};
        inline float                    getFillTransparency(){return fillTransparency;};
        inline TString                  getMethodName() const {return methodName;};
        inline TString                  getName() const {return name;};
        inline int                      getNObservables(){return w->set(obsName)->getSize();}
        inline int                      getNPoints1d(){return nPoints1d;}
        inline int                      getNPoints2dx(){return nPoints2dx;}
        inline int                      getNPoints2dy(){return nPoints2dy;}
        inline const RooArgSet*         getObservables(){return w->set(obsName);}
        inline TString                  getObsName(){return obsName;};
        inline TString                  getParsName(){return parsName;};
        float                           getScanVarSolution(int iVar, int iSol);
        RooRealVar*                     getScanVar1();
        TString                         getScanVar1Name();
        float                           getScanVar1Solution(int i=0);
        RooRealVar*                     getScanVar2();
        TString                         getScanVar2Name();
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
        virtual bool                    loadScanner(TString fName="");
        void                            plot2d(TString varx, TString vary);
        void                            plot1d(TString var);
        void                            plotOn(OneMinusClPlotAbs *plot, int CLsType=0); // CLsType: 0 (off), 1 (naive CLs t_s+b - t_b), 2 (freq CLs)
        void                            plotPulls(int nSolution=0);
        virtual void                    print();
        void                            printCLintervals(int CLsType, bool calc_expected=false);
        void                            printLocalMinima();
        void                            saveLocalMinima(TString fName="");
        void                            saveScanner(TString fName="");
        virtual int                     scan1d();
        virtual int                     scan2d();
        inline void                     setDrawSolution(int code=0){drawSolution = code;};
        inline void                     setPValueCorrector(PValueCorrection *pvalCor) { pvalueCorrector = pvalCor; pvalueCorrectorSet=true; }
        inline void                     setScanVar1(TString var){scanVar1 = var;};
        inline void                     setScanVar2(TString var){scanVar2 = var;};
        inline void                     setNPoints1d(int n){nPoints1d = n;};
        inline void                     setNPoints2dx(int n){nPoints2dx = n;};
        inline void                     setNPoints2dy(int n){nPoints2dy = n;};
        inline void                     setFilled(bool filled){ drawFilled = filled;};
        inline void                     setLineColor(int c){lineColor = c;};
        inline void                     setLineStyle(int c){lineStyle = c;};
        inline void                     setLineWidth(int c){lineWidth = c;};
        inline void                     setTextColor(int c){textColor = c;};
        inline void                     setFillStyle(int c){fillStyle = c;};
        inline void                     setFillColor(int c){fillColor = c;};
        inline void                     setFillTransparency(float c){fillTransparency = c;};
        inline void                     setTitle(TString s){title = s;};
        void                            setChi2minGlobal(double x);
        void                            setSolutions(vector<RooSlimFitResult*> s);
        inline void                     setVerbose(bool yesNo=true){verbose = yesNo;};
        inline void                     setHCL( TH1F *h ) { hCL = h; };
        inline void                     setHchisq( TH1F *h ) { hChi2min = h; };
        void                            setXscanRange(float min, float max);
        void                            setYscanRange(float min, float max);
        void                            calcCLintervalsSimple(int CLsType=0, bool calc_expected=false);
        const std::pair<double, double> getBorders(const TGraph& graph, const double confidence_level, bool qubic=false);
        const std::pair<double, double> getBorders_CLs(const TGraph& graph, const double confidence_level, bool qubic=false);
        virtual bool                    checkCLs();

        vector<RooSlimFitResult*> allResults;           ///< All fit results we encounter along the scan.
        vector<RooSlimFitResult*> curveResults;         ///< All fit results of the the points that make it into the 1-CL curve.
        ///< Index is the bin number of hCL bins -1.
        vector<vector<RooSlimFitResult*> > curveResults2d;  ///< All fit results of the the points that make it into the 1-CL curve.
        ///< Index is the gobal bin number of hCL2d -1.
        vector<RooSlimFitResult*> solutions;            ///< Local minima filled by saveSolutions() and saveSolutions2d().

        ///< The names of the CL interval vectors might be misleading. They correspond to the default CL intervals.
        ///< If the option --CL is given, the 1-3 sigma correspond to the first, second,... given value of the CL.
        vector<CLInterval> clintervals1sigma;  ///< all 1 sigma intervals that were found by calcCLintervals()
        vector<CLInterval> clintervals2sigma;  ///< all 2 sigma intervals that were found by calcCLintervals()
        vector<CLInterval> clintervals3sigma;  ///< all 3 sigma intervals that were found by calcCLintervals()
        vector<CLInterval> clintervalsuser;    ///< all intervals with an additional user specific CL that were found by calcCLintervals()
        RooFitResult* globalMin;               ///< parameter values at a global minimum

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
        TH1F* hCL;                  ///< 1-CL curve
        TH1F* hCLs;                 ///< 1-CL curve
        TH1F* hCLsFreq;             ///< 1-CL curve
        TH1F* hCLsExp;              ///< 1-CL curve
        TH1F* hCLsErr1Up;           ///< 1-CL curve
        TH1F* hCLsErr1Dn;           ///< 1-CL curve
        TH1F* hCLsErr2Up;           ///< 1-CL curve
        TH1F* hCLsErr2Dn;           ///< 1-CL curve
        TH2F* hCL2d;                ///< 1-CL curve
        TH2F* hCLs2d;               ///< 1-CL curve
        TH1F* hChi2min;             ///< histogram for the chi2min values before Prob()
        TH2F* hChi2min2d;           ///< histogram for the chi2min values before Prob()
        double chi2minGlobal;       ///< chi2 value at global minimum
        double chi2minBkg;          ///< chi2 value at global minimum
        bool chi2minGlobalFound;    ///< flag to avoid finding minimum twice
        int lineColor;
        int textColor;              ///< color used for plotted central values
        int lineStyle;
        int lineWidth;
        int fillStyle;
        int fillColor;
        float fillTransparency;
        bool drawFilled;            ///< choose if Histogram is drawn filled or not
        int drawSolution;           ///< Configure how to draw solutions on the plots.
        ///< 0=don't plot, 1=plot at central value (1d) or markers (2d)
        ///< Default is taken from arg, unless disabled by setDrawSolution().
        bool verbose;
        int nWarnings;              ///< number of warnings printed in getScanVarSolution()
        OptParser* arg;             ///< command line options
        Combiner* combiner;         ///< the combination
        bool m_xrangeset;           ///< true if the x range was set manually (setXscanRange())
        bool m_yrangeset;           ///< true if the y range was set manually (setYscanRange())
        bool m_initialized;         ///< true if initScan() was called
        std::vector<double> ConfidenceLevels;   ///< container of the confidence levels to be computed

    private:

        bool  compareSolutions(RooSlimFitResult* r1, RooSlimFitResult* r2);
        float pq(float p0, float p1, float p2, float y, int whichSol=0);
        void  removeDuplicateSolutions();
        bool  interpolate(TH1F* h, int i, float y, float central, bool upper, float &val, float &err);
        void  interpolateSimple(TH1F* h, int i, float y, float &val);
};

#endif
