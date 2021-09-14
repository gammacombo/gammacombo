/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Dec 2014
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
#include "PDF_Datasets.h"
#include "ProgressBar.h"

#include "MethodProbScan.h"

using namespace std;
using namespace Utils;

///
/// Interface class for the root trees that are written
/// by the Plugin method and related functions.
///
class ToyTree
{
	public:

		ToyTree(Combiner *c, TChain* t=0, bool _quiet=false);
		ToyTree(PDF_Datasets *p, OptParser* opt, TChain* t=0, bool _quiet=false);
		~ToyTree();

		void                    activateCoreBranchesOnly();
		void				    activateAllBranches();
		void                    activateBranch(const TString& bName);
		void                    fill();
		void                    init();
		OptParser*              getArg(){return arg;};
		Long64_t                GetEntries();
		void                    GetEntry(Long64_t i);
		inline TString          getName(){return name;};
		float                   getScanpointMin();
		float                   getScanpointMax();
		int                     getScanpointN();
		float                   getScanpointyMin();
		float                   getScanpointyMax();
		int                     getScanpointyN();
		TTree*                  getTree(){return t;};
		bool					isWsVarAngle(TString var);
		void                    open();
		void                    setCombiner(Combiner* c);
		void                    storeParsPll();
		void                    storeParsFree();
		void                    storeParsScan();
		void                    storeParsScan(RooFitResult* values);
		void                    storeTheory();
		void                    storeObservables();
		void                    writeToFile(TString fName);
		void                    writeToFile();
		void                    setStoreObs(bool flag){this->storeObs = flag;};
		void                    setStoreTh(bool flag){this->storeTh = flag;};
		void                    setStoreGlob(bool flag){this->storeTh = flag;};
		void                    storeParsGau(const RooArgSet globalConstraintMeans);


		float scanpoint;        ///< the scanpoint for 1D scans, or the x scanpoint for 2D scans
		float scanpointy;       ///< the y scanpoint for 2D scans
		float chi2min;          ///< the chi2 of the fit with var fixed to scan point
		float chi2minGlobal;    ///< the chi2 of the free fit
		float chi2minBkg;		    ///< the chi2 of the fit of the bkg hypothesis (for CLs method)
		float chi2minToy;       ///< the chi2 of the fit to the toy with var fixed to scan point
		float chi2minGlobalToy; ///< the chi2 of the free fit to the toy
		float chi2minBkgToy;	  ///< the chi2 of the fit of the hypothesis value to the bkg toy distribution (for CLs method)
    	float chi2minGlobalBkgToy; ///< the chi2 of the free fit to the bkg only toys
    	float chi2minBkgBkgToy; ///< the chi2 of the bkg fit to the bkg only toys
		float scanbest;         ///< an alias to the free fit value of the scan variable
		float scanbesty;        ///< an alias to the free fit value of the scan y variable in 2D scans
    	float scanbestBkg;      ///< an alias to the free fit value of the scan variable on the bkg only toy (for CLs method)
    	float scanbestBkgfitBkg;      ///< an alias to the fit value of the scan variable of the bkg fit on the bkg only toy (for CLs method)
		float nrun;             ///< an ID to distinguish different runs, i.e. batch jobs
		float ntoy; 						///< an ID to distinguish different toys
		float npoint; 				  ///< an ID to distinguish different scan point
		float id;               ///< an ID to distinguish different conditions, e.g. different toys in a coverage test
		float statusFree;
		float covQualFree;
		float statusScan;
		float covQualScan;
		float statusFreeBkg;
		float covQualFreeBkg;
		float statusScanBkg;
		float covQualScanBkg;
		float statusBkgBkg;
		float covQualBkgBkg;
		float statusScanData;
		float covQualScanData;
        int   bestIndexScanData;
		float nBergerBoos;
		float BergerBoos_id;
		float genericProbPValue;
		float statusFreePDF;
		float statusScanPDF;
		float chi2minToyPDF;
		float chi2minGlobalToyPDF;
		float chi2minBkgToyPDF;
		TTree *t;               ///< the tree

	private:

		void         computeMinMaxN();
		void         initMembers(TChain* t=0);
		Combiner *comb;         ///< combination bringing in the arg, workspace, and names
		OptParser *arg;         ///< command line arguments
		RooWorkspace *w;        ///< holds all input pdfs, parameters, and observables, as well as the combination
		TString name;           ///< combiner name, ending up in titles and file names
		TString pdfName;        ///< PDF name in workspace, derived from name
		TString obsName;        ///< dataset name of observables, derived from name
		TString parsName;       ///< set name of physics parameters, derived from name
		TString thName;         ///< set name of theory parameters, derived from name
		TString globName;		///< set name of explicit set of global observables

		map<string,float>  parametersScan;   ///< fit result of the scan fit
		map<string,float>  parametersFree;   ///< fit result of the free fit
		map<string,float>  parametersPll;    ///< parameters of the profile likelihood curve of the data
		map<string,float>  observables;      ///< values of the observables
		map<string,float>  theory;           ///< theory parameters (=observables at profile likelihood points)
		map<TString,float> constraintMeans;  ///< values of global observables

		float scanpointMin;     ///< minimum of the scanpoint, computed by computeMinMaxN().
		float scanpointMax;     ///< maximum of the scanpoint, computed by computeMinMaxN().
		int   scanpointN;       ///< number of different values of the scanpoint, computed by computeMinMaxN().
		float scanpointyMin;    ///< minimum of the scanpointy, computed by computeMinMaxN().
		float scanpointyMax;    ///< maximum of the scanpointy, computed by computeMinMaxN().
		int   scanpointyN;      ///< number of different values of the scanpointy, computed by computeMinMaxN().

		bool storeObs;                      ///< Boolean flag to control storing ToyTree observables, can't store these for DatasetsScans
		bool storeTh;                       ///< Boolean flag to control storing ToyTree theory parameters. Not needed in DatasetsScans
		bool storeGlob;               		///< Boolean flag to control storing ToyTree global observables. Extremely handy in DatasetsScans
		bool quiet;
};

#endif
