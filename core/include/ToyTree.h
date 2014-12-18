/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#ifndef ToyTree_h
#define ToyTree_h

#include "TEnv.h"
#include "TFile.h"
#include "TF1.h"
#include "TChain.h"
#include "TCut.h"
#include "TPaveText.h"
#include "PDF_Abs.h"
#include "OptParser.h"
#include "Utils.h"
#include "TPaveStats.h"
#include "PDF_Generic_Abs.h"

#include "MethodProbScan.h"

using namespace std;
using namespace Utils;

///
/// Interface class for the root trees that are written
/// by the Plugin method and related functions. Also makes
/// control plots for the plugin toys.
///
class ToyTree
{
	public:

		ToyTree(Combiner *c, TChain* t=0);
		ToyTree(PDF_Generic_Abs *p, TChain* t=0);
		~ToyTree();

		void                    activateCoreBranchesOnly();
		void				    activateAllBranches();
		void                    activateBranch(const TString& bName);
		void                    fill();
		void                    init();
		void                    ctrlPlotChi2Distribution();
		void                    ctrlPlotChi2Parabola();
		void                    ctrlPlotNuisances();
		void                    ctrlPlotObservables();
		void                    ctrlPlotSummary();
		void                    ctrlPlotMore(MethodProbScan* profileLH);
		Long64_t                GetEntries();
		void                    GetEntry(Long64_t i);
		float                   getScanpointMin();
		float                   getScanpointMax();
		int                     getScanpointN();
		TTree*                  getTree(){return t;};
		void                    open();
		void                    saveCtrlPlots();
		void                    setCombiner(Combiner* c);
		void                    storeParsPll();
		void                    storeParsFree();
		void                    storeParsScan();
		void                    storeTheory();
		void                    storeObservables();
		void                    writeToFile(TString fName);
		void                    writeToFile();
		void                    setStoreObs(bool flag){this->storeObs = flag;};
		void                    setStoreTh(bool flag){this->storeTh = flag;};
		void                    storeParsGau();


		float scanpoint;
		float chi2min;
		float chi2minGlobal;
		float chi2minToy;
		float chi2minGlobalToy;
		float scanbest;
		float nrun;         ///< an ID to distinguish different runs, i.e. batch jobs 
		float id;           ///< an ID to distinguish different conditions, e.g. different toys in a coverage test
		float statusFree;
		float covQualFree;
		float statusScan;
		float covQualScan;
		float statusScanData;
		float covQualScanData;
		float nBergerBoos;
		float BergerBoos_id;
		float genericProbPValue;
		float statusFreePDF;
		float statusScanPDF;
		float chi2minToyPDF;
		float chi2minGlobalToyPDF;
		TTree *t;               ///< the tree

	private:

		void         computeMinMaxN();
		void         makePlotsNice(TString htemp="htemp", TString Graph="Graph");
		TCanvas*     selectNewCanvas(TString title);
		TVirtualPad* selectNewPad();
		void         updateCurrentCanvas();
		void         initMembers(TChain* t=0);
		Combiner *comb;         ///< combination bringing in the arg, workspace, and names
		OptParser *arg;         ///< command line arguments
		RooWorkspace *w;        ///< holds all input pdfs, parameters, and observables, as well as the combination
		TString name;           ///< combiner name, ending up in titles and file names
		TString pdfName;        ///< PDF name in workspace, derived from name
		TString obsName;        ///< dataset name of observables, derived from name
		TString parsName;       ///< set name of physics parameters, derived from name
		TString thName;         ///< set name of theory parameters, derived from name

		map<string,float> parametersScan;   ///< fit result of the scan fit
		map<string,float> parametersFree;   ///< fit result of the free fit
		map<string,float> parametersPll;    ///< parameters of the profile likelihood curve of the data
		map<string,float> observables;      ///< values of the observables
		map<string,float> theory;           ///< theory parameters (=observables at profile likelihood points)
		map<TString,float> constraintMeans;  ///< stores gaussian constraint means for the B2MuMu combination

		float scanpointMin;     ///< minimum of the scanpoint, computed by computeMinMaxN().
		float scanpointMax;     ///< maximum of the scanpoint, computed by computeMinMaxN().
		int   scanpointN;       ///< number of different values of the scanpoint, computed by computeMinMaxN().

		vector<TCanvas*> ctrlPlotCanvases;  ///< Pointers to the canvases of the control plots, see selectNewCanvas().
		int ctrlPadId;                      ///< ID of currently selected pad, see selectNewPad().
		TCut ctrlPlotCuts;                  ///< Cuts that are applied to all control plots.
		bool storeObs;                      ///< Boolean flag to control storing ToyTree observables, can't store these for GenericScans
		bool storeTh;                       ///< Boolean flag to control storing ToyTree theory parameters. Not needed in GenericScans 
};

#endif
