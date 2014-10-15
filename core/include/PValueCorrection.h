#ifndef PValueCorrection_h
#define PValueCorrection_h

#include <iostream>
#include <algorithm>
#include <vector>
#include "TString.h"
#include "TH1.h"
#include "TF1.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TChain.h"
#include "TIterator.h"
#include "TList.h"
#include "TMath.h"
#include "TFile.h"

class PValueCorrection {

	public:

		PValueCorrection(TString _transFunc="none", bool _verbose=false);
		PValueCorrection(int id, bool _verbose=false);
		~PValueCorrection();

		void setTransFunc(TString tf) { transFunc = tf; }
		void setFitParams(std::vector<double> fP) { fitParams = fP; }
		void setFitParam(int i, double val);
		void readFiles(TString name, int id=0, bool isPlugin=false);
		void fitHist(TH1* h);
		double transform(double x);

		void checkValid();
		void checkParams();
		void printCoverage(float,float,float,float,TString name="");

		void write(TString fname);
		void write(TFile *f);

	private:
		TString transFunc;
		bool verbose;
		TString fitString;
		TF1 fitFunc;
		std::vector<double> fitParams;
		std::vector<TString> allowedFuncs;
		TH1F *h_pvalue_before;
		TH1F *h_pvalue_after;



};

#endif
