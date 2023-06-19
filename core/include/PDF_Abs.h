/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef PDF_Abs_h
#define PDF_Abs_h

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "RooFormulaVar.h"
#include "RooGlobalFunc.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooAddition.h"
#include "RooMultiVarGaussian.h"
#include "RooFitResult.h"
#include "RooRandom.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooLognormal.h"
#include "RooPoisson.h"
#include "RooProdPdf.h"
#include "RooPlot.h"
#include "RooArgSet.h"
#include "RooCategory.h"
#include "RooMultiPdf.h"

#include "TCanvas.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMarker.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TRandom3.h"
#include "TLegend.h"

#include "Utils.h"
#include "ParametersAbs.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_Abs
{
	public:
		PDF_Abs(int nObs);
		PDF_Abs(int nObs, ParametersAbs &pars);
		virtual             ~PDF_Abs();
		virtual void				build();
		virtual void        buildPdf();
		virtual void        buildCov();
		bool                bkgpdfset(){return isBkgPdfSet;};
		bool                bkgmultipdfset(){return isBkgMultipdfSet;};
		virtual bool        checkConsistency();
		void                deleteToys(){delete toyObservables;};
		inline TString			getCorrelationSourceString(){return corSource;};
		TString             getBaseName();
		inline TString			getErrorSourceString(){return obsErrSource;};
		inline int					getGcId(){return gcId;}
		inline TString      getName(){return name;};
		inline int          getNobs(){return nObs;};
		inline TString      getUniqueID(){return uniqueID;};
		inline unsigned long long getUniqueGlobalID(){return uniqueGlobalID;}
		inline RooArgList*  getObservables(){return observables;};
		inline vector<TString> getLatexObservables(){return latexObservables;};
		inline TString		getObservableSourceString(){return obsValSource;};
		float 				getObservableValue(TString obsname);
		inline RooArgList*  getParameters(){return parameters;};
		inline RooAbsPdf*   getPdf(){return pdf;};
		inline RooAbsPdf*   getBkgPdf(){return pdfBkg;};
        inline RooMultiPdf* getMultipdf(){return multipdf;};
        inline RooMultiPdf* getBkgMultipdf(){return multipdfBkg;};
		void 				getSubCorrelationStat(TMatrixDSym& target, vector<int>& indices);
		void 		 		getSubCorrelationSyst(TMatrixDSym& target, vector<int>& indices);
		inline RooArgList*  getTheory(){return theory;};
		inline TString      getTitle(){return title;};
		bool 	            hasObservable(TString obsname);
		inline bool			isCrossCorPdf(){return m_isCrossCorPdf;}
		virtual void        initParameters();
		virtual void        initRelations();
		virtual void        initObservables();
		void                loadExtParameters(RooFitResult *r);
		void                print() const;
		void                printParameters();
		void                printObservables();
		bool                ScaleError(TString obsname, float scale);
		virtual void        setCorrelations(TString c);
		inline void					setErrorSourceString(TString source){obsErrSource=source;};
		inline void					setGcId(int id){gcId=id;};
		inline void 				setName(TString myName){this->name = myName;}
		void                setObservable(TString name, float value);
		virtual void        setObservables(TString c);
		void                setObservablesTruth();
		void                setObservablesToy();
		inline void					setObservableSourceString(TString source){obsValSource=source;};
    inline void         setNObs(int val){ nObs = val; };
		inline void         setTitle(TString t){title=t;};
		void                setUncertainty(TString obsName, float stat, float syst);
		virtual void        setUncertainties(TString c);
		void                setSystCorrelation(TMatrixDSym &corSystMatrix);
		void                storeErrorsInObs();
		void                resetCorrelations();
		virtual bool        test();
		void                uniquify(int uID);  ///< used to uniquify all names when added

		// covariance
		TMatrixDSym covMatrix;
		TMatrixDSym corMatrix;
		TMatrixDSym corStatMatrix;
		TMatrixDSym corSystMatrix;
		vector<double> StatErr;
		vector<double> SystErr;
		TString corSource;
		TString obsValSource;
		TString obsErrSource;

  protected:

		void                    addToTrash(TObject*);
		void 					getSubMatrix(TMatrixDSym& target, TMatrixDSym& source, vector<int>& indices);

		RooArgList*             parameters;   // holds all fit parameters of this PDF
		RooArgList*             theory;       // holds all truth relations
		RooArgList*             observables;  // holds all observables
		TString                 name;
		TString                 title;        // to be printed in human readable summaries
		RooAbsPdf*              pdf;          // the PDF
		RooAbsPdf*      		pdfBkg;		  // Bkg PDF for building CLs teststatistic
        RooMultiPdf*            multipdf;     // the multipdf
        RooMultiPdf*            multipdfBkg;  // Bkg version of the multipdf
        RooCategory*            multipdfCat;  // the multipdf category
    	bool 					isBkgPdfSet;     //> Flag deciding if Bkg PDF is set
    	bool 					isBkgMultipdfSet;//> Flag deciding if Bkg MultiPDF is set
		int                     nObs;         // number of observables
		map<string,TObject*>    trash;        // trash bin, gets emptied in destructor
		bool					m_isCrossCorPdf;	// Cross correlation PDFs need some extra treatment in places, e.g. in uniquify()
		vector<TString>         latexObservables; // holds latex labels for observables

		// The following three members are to gain performance during
		// toy generation - generating 1000 toys is much faster than 1000 times one toy.
		int                     nToyObs;        // Number of toy observables to be pregenerated.
		RooAbsData*             toyObservables; // A dataset holding nToyObs pregenerated bkg only toy observables.
		RooAbsData*             toyBkgObservables; // A dataset holding nToyObs pregenerated toy observables.
		int                     iToyObs;        // Index of next unused set of toy observables.
		int						gcId;			// ID of this PDF inside a GammaCombo object. Used to refer to this PDF.

	private:
		void					  printCorMatrix(TString title, TString source, const TMatrixDSym& cor) const;
		TString                   uniquifyThisString(TString s, int uID);
		TString                   uniqueID;       	// see also uniquify()
		static unsigned long long counter;        	// Counts the total number of PDF_Abs objects that are created.
		unsigned long long        uniqueGlobalID; 	// Used only in the Combiner::delPdf() mechanism. Uses the counter, so will depend on creation order.
};

#endif
