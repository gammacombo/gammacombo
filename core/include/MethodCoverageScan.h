/*
 * Gamma Combination
 * Author: Matthew Kenzie, matthew.kenzie@cern.ch
 * Date: January 2016
 *
 */

#ifndef MethodCoverageScan_h
#define MethodCoverageScan_h

#include <iostream>
#include <stdlib.h>

#include "RooAddition.h"
#include "RooArgSet.h"
#include "RooConstVar.h"
#include "RooCustomizer.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooGlobalFunc.h"
#include "RooMultiVarGaussian.h"
#include "RooPlot.h"
#include "RooPoisson.h"
#include "RooProdPdf.h"
#include "RooRandom.h"
#include "RooRealVar.h"
#include "RooSlimFitResult.h"
#include "RooWorkspace.h"

#include "TCanvas.h"
#include "TChain.h"
#include "TCut.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TMarker.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TStopwatch.h"
#include "TStyle.h"
#include "TTree.h"
#include "TTree.h"

#include "ControlPlots.h"
#include "FitResultCache.h"
#include "MethodAbsScan.h"
#include "MethodProbScan.h"
#include "MethodPluginScan.h"
#include "ProgressBar.h"
#include "ToyTree.h"
#include "Utils.h"
#include "ParameterCache.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class MethodCoverageScan : public MethodAbsScan
{
	public:
		MethodCoverageScan(Combiner* comb);
		MethodCoverageScan();

    ~MethodCoverageScan();

    void            setParameterCache( ParameterCache *_pCache) { pCache = _pCache; }
		virtual int     scan1d(int nRun=1);
    virtual void    readScan1dTrees(int runMin, int runMax);
    virtual void    plot();
		int             getNtoys(){return nToys;};
    void            saveScanner(TString fName="");
    bool            loadScanner(TString fName="");

  protected:
    ParameterCache *pCache;
		int             nToys;              ///< number of toys to be generated at each scan point

    // functions
    std::vector<double> fitHist( TH1* h, TString fitfunc="p1+exp", bool draw=true );
    double              transform( std::vector<double> fitParams, TString transFunc, double x );
    void                printLatexLine( float eta, float finProb, float finProbErr, float finPlug, float finPlugErr );

    // result histograms
    TH1F *h_sol;
    TH1F *h_pvalue_plugin;
    TH1F *h_pvalue_prob;
    TH1F *h_pvalue_plugin_notransf;
    TH1F *h_pvalue_prob_notransf;

    // result values
    Long64_t nentries;
    Long64_t nfailed;
    float n68plugin;
    float n95plugin;
    float n99plugin;
    float n68prob;
    float n95prob;
    float n99prob;

};

#endif

