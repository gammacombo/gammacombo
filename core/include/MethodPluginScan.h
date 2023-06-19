/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#ifndef MethodPluginScan_h
#define MethodPluginScan_h

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
#include "ProgressBar.h"
#include "ToyTree.h"
#include "Utils.h"
#include "PDF_Datasets.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class MethodPluginScan : public MethodAbsScan
{
	public:
		MethodPluginScan(MethodProbScan* s);
		MethodPluginScan(MethodProbScan* s, PDF_Datasets* pdf, OptParser* opt);
		MethodPluginScan(Combiner* comb);

		inline void     setNtoysPerPoint(int n){nToys=n;};
		void            setParevolPLH(MethodProbScan* s);
		virtual int     scan1d(int nRun=1);
		virtual void    scan2d(int nRun=1);
		virtual void    readScan1dTrees(int runMin=1, int runMax=1, TString fName="default");
		void            readScan2dTrees(int runMin=1, int runMax=1);
		int             getNtoys(){return nToys;};
		double          getPvalue1d(RooSlimFitResult* plhScan, double chi2minGlobal, ToyTree* t=0, int id=0, bool quiet=false);
		void			makeControlPlotsCLs(map<int, vector<double> > bVals, map<int, vector<double> > sbVals);

	protected:
		TH1F*           	analyseToys(ToyTree* t, int id=-1, bool quiet=false);
		void          		computePvalue1d(RooSlimFitResult* plhScan, double chi2minGlobal, ToyTree* t, int id, Fitter *f, ProgressBar *pb);
		RooDataSet*				generateToys(int nToys);
		double          	importance(double pvalue);
		RooSlimFitResult*	getParevolPoint(float scanpoint);


		int             		nToys;              ///< number of toys to be generated at each scan point
		MethodProbScan* 		profileLH;          ///< external scanner holding the profile likelihood: DeltaChi2 of the scan PDF on data
		MethodProbScan* 		parevolPLH;         ///< external scanner defining the parameter evolution: set to profileLH unless for the Hybrid Plugin
		RooDataSet*				BkgToys;
		std::vector<double>		chi2minBkgBkgToysvector;	///< saving the fits of the bkg-only pdf to the bkg-only toy
		std::vector<double>		chi2minGlobalBkgToysvector; ///< saving the fits of the global pdf to the bkg-only toy
};

#endif
