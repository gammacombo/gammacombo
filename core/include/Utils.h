/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef Utils_h
#define Utils_h

#include "TMath.h"
#include "TObjString.h"
#include "TString.h"
#include "TH2F.h"
#include "TStyle.h"
#include "RooAbsPdf.h"
#include "RooWorkspace.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TPaveText.h"
#include "rdtsc.h"
#include "TMatrixDSym.h"
#include "RooFormulaVar.h"
#include "RooRealVar.h"
#include "RooFitResult.h"
#include "RooSlimFitResult.h"
#include "RooDataSet.h"
#include "RooRandom.h"
#include "RooMinuit.h"
#include "TTree.h"
#include "TH1.h"
#include "TGraphErrors.h"
#include "TGraphSmooth.h"
#include "TMatrixDSymEigen.h"
#include "TVectorD.h"
#include <sys/stat.h>
#include "boost/algorithm/string.hpp"
#include "UtilsConfig.h"

using namespace std;
using namespace RooFit;

namespace Utils
{
	extern int countFitBringBackAngle;      ///< counts how many times an angle needed to be brought back
	extern int countAllFitBringBackAngle;   ///< counts how many times fitBringBackAngle() was called

	// used to fix parameters in the combination, see e.g. Combiner::fixParameter()
	struct FixPar
	{
		TString name;
		float value;
		bool useValue;
	};

	// used to set starting values in the combination, see e.g. Combiner::setValue()
	struct StartPar
	{
		TString name;
		float value;
		bool useValue;
	};

	// used to adjust ranges in the combination, see e.g. Combiner::adjustRange()
	struct RangePar
	{
		TString name;
		float min;
		float max;
	};

  // drawing HFAG label
  void HFAGLabel(const TString& label="please set label", Double_t xpos=0, Double_t ypos=0, Double_t scale=1);

	enum          histogramType { kChi2, kPvalue };
	inline double sq(double x){return x*x;}
	inline double RadToDeg(double rad){return rad/TMath::Pi()*180.;}
	inline double DegToRad(double deg){return deg/180.*TMath::Pi();}
	inline double Max(double v1, double v2){return v1>v2 ? v1 : v2;}
	int           calcNsubdigits(double value, int sigdigits=2);
	float         Round(double value, int digits);
	double        bringBackAngle(double angle);
	double        angularDifference(double angle1, double angle2);
	bool          isPosDef(TMatrixDSym* c);
	bool          isAngle(RooRealVar* v);
	int           makeNewColor(string hex);

	RooFitResult*   fitToMin(RooAbsPdf *pdf, bool thorough, int printLevel);
	RooFitResult*   fitToMinBringBackAngles(RooAbsPdf *pdf, bool thorough, int printLevel);
	RooFitResult*   fitToMinForce(RooWorkspace *w, TString name, TString forceVariables="", bool debug=true);
	RooFitResult*   fitToMinImprove(RooWorkspace *w, TString name);
	double          getChi2(RooAbsPdf *pdf);
	TH1F*           histHardCopy(const TH1F* h, bool copyContent=true, bool uniqueName=true, TString specName="");
	TH2F*           histHardCopy(const TH2F* h, bool copyContent=true, bool uniqueName=true, TString specName="");

	TTree*  convertRooDatasetToTTree(RooDataSet *d);
  TGraph* convertTH1ToTGraph(TH1* h, bool withErrors=false);
  TGraph* smoothGraph(TGraph* g, int option=0);
  TGraph* smoothHist(TH1* h, int option=0);

	void addSetNamesToList( vector<string>& list, RooWorkspace *w, TString setName);
	void makeNamedSet(RooWorkspace *w, TString mergedSet, vector<string>& names);
	void mergeNamedSets(RooWorkspace *w, TString mergedSet, TString set1, TString set2);
	void randomizeParameters(RooWorkspace* w, TString setname);
	void randomizeParametersGaussian(RooWorkspace* w, TString setname, RooSlimFitResult *r);
	void randomizeParametersUniform(RooWorkspace* w, TString setname, RooSlimFitResult *r, double sigmaRange);
	void setParameters(const RooAbsCollection* setMe, const RooAbsCollection* values);
	void setParameters(RooWorkspace* w, TString parname, const RooAbsCollection* set);
	void setParameters(RooWorkspace* w, TString parname, RooFitResult* r, bool constAndFloat=false);
	void setParameters(RooWorkspace* w, TString parname, RooSlimFitResult* r, bool constAndFloat=false);
	void setParameters(RooWorkspace* w, TString parname, RooDataSet* d);
	void setParameters(RooWorkspace* w, RooFitResult* values);
	void setParametersFloating(const RooAbsCollection* setMe, const RooAbsCollection* values);
	void setParametersFloating(RooWorkspace* w, TString parname, const RooAbsCollection* set);
	void setParametersFloating(RooWorkspace* w, TString parname, RooFitResult* r);
	void setParametersFloating(RooWorkspace* w, TString parname, RooDataSet* d);
	void fixParameters(const RooAbsCollection* set);
	void fixParameters(RooWorkspace* w, TString parname);
	void floatParameters(const RooAbsCollection* set);
	void floatParameters(RooWorkspace* w, TString parname);
	void setLimit(RooRealVar* v, TString limitname);
	void setLimit(RooWorkspace* w, TString parname, TString limitname);
	void setLimit(const RooAbsCollection* set, TString limitname);
  double getCorrelationFactor( const vector<double> &a , const vector<double> &b );

	void buildCorMatrix(TMatrixDSym &cor);
	TMatrixDSym* buildCovMatrix(TMatrixDSym &cor, float *err);
	TMatrixDSym* buildCovMatrix(TMatrixDSym &cor, vector<double> &err);

  RooFormulaVar* makeTheoryVar(TString name, TString title, TString formula, RooArgList* pars);

	void savePlot(TCanvas *c1, TString name);
	bool FileExists( TString strFilename );
	void assertFileExists(TString strFilename);
	template<class T> inline bool isIn(vector<T> vec, T var){return (find(vec.begin(), vec.end(), var) != vec.end());};

	static int uniqueRootNameId = 0;
	inline TString          getUniqueRootName(){return (TString)Form("UID%i", ++uniqueRootNameId);}
	void                    fillArgList(RooArgList* list, RooWorkspace* w, std::vector<TString> names);
	void                    getParameters(const RooFitResult &result, std::vector<TString> &names);
	std::vector<TString>    getParsWithName(const TString& subString, const RooArgSet& set);
	bool                    checkBoundary(const RooSlimFitResult& r, std::vector<TString> lowProb, std::vector<TString> highProb);
	void                    setParsConstToBound(RooWorkspace* w, std::vector<TString> names, bool low);
	void                    setParsConstToBound(RooWorkspace* w, std::vector<TString> namesLow, std::vector<TString> namesHigh);
	void                    setParametersFloating(RooWorkspace* w, std::vector<TString> names);
	void                    setParametersFloating(RooWorkspace* w, std::vector<TString> names, std::vector<TString> names2);
  std::vector<double>     computeNormalQuantiles( std::vector<double> &values, int nsigma);
  template<typename T>
    static inline double  Lerp(T v0, T v1, T t);
  template<typename T>
    static inline std::vector<T> Quantile(const std::vector<T>& inData, const std::vector<T>& probs);
  template<typename T>
    static inline double  getVectorFracAboveValue(const std::vector<T>& vec, T val);
  template<typename T>
    static inline void print(const std::vector<T>& vec);
  template<typename T>
    static inline void print(T val);
  template<typename T>
    static inline T sum(const std::vector<T>& vec);
  template<typename T>
    static inline T sqsum(const std::vector<T>& vec);
  template<typename T>
    static inline double mean(const std::vector<T>& vec);
  template<typename T>
    static inline double stddev(const std::vector<T>& vec);

    static inline double normal_cdf (double x);

	void dump_vector(const std::vector<int>& l);
	void dump_vector(const std::vector<float>& l);
	void dump_map(const std::map<int, std::vector<int> >& map);
	void dump_matrix(const std::vector<std::vector<int> >& l);

	std::vector<std::vector<int> > transpose(std::vector<std::vector<int> >& v);
	TCanvas* newNoWarnTCanvas(TString name="NoWarnTCanvas", TString title="NoWarnTCanvas", int width=800, int height=600);
	TCanvas* newNoWarnTCanvas(TString name, TString title, int x, int y, int width, int height);
}

template<typename T>
double Utils::Lerp(T v0, T v1, T t)
{
    return (1 - t)*v0 + t*v1;
}

template<typename T>
std::vector<T> Utils::Quantile(const std::vector<T>& inData, const std::vector<T>& probs)
{
    if (inData.empty())
    {
        return std::vector<T>();
    }

    if (1 == inData.size())
    {
        return std::vector<T>(1, inData[0]);
    }

    std::vector<T> data = inData;
    std::sort(data.begin(), data.end());
    std::vector<T> quantiles;

    for (size_t i = 0; i < probs.size(); ++i)
    {
        T poi = Lerp<T>(-0.5, data.size() - 0.5, probs[i]);

        size_t left = std::max(int64_t(std::floor(poi)), int64_t(0));
        size_t right = std::min(int64_t(std::ceil(poi)), int64_t(data.size() - 1));

        T datLeft = data.at(left);
        T datRight = data.at(right);
        T quantile = Lerp<T>(datLeft, datRight, poi - left);

        quantiles.push_back(quantile);
    }

    return quantiles;
}

template<typename T>
double Utils::getVectorFracAboveValue(const std::vector<T>& vec, T val) {

  int nabove = 0;
  for (int i=0; i<vec.size(); i++) {
    if ( vec[i] >= val ) nabove++;
    // if ( vec[i] > val ) nabove++;
  }
  return double(nabove)/vec.size();
}

template<typename T>
void Utils::print(const std::vector<T>& vec){
  cout << "[ (size=" << vec.size() << ") ";
  for (int i=0; i<vec.size(); i++) {
    print(vec[i]);
    if ( i<vec.size()-1) cout << " , ";
  }
  cout << " ]";
}

template<typename T>
void Utils::print(T val){
  cout << val;
}

template<typename T>
T Utils::sum(const std::vector<T> &vec){
  T s=0;
  for (int i=0; i<vec.size(); i++){
    s += vec[i];
  }
  return s;
}

template<typename T>
T Utils::sqsum(const std::vector<T> &vec){
  T s=0;
  for (int i=0; i<vec.size(); i++){
    s += vec[i]*vec[i];
  }
  return s;
}

template<typename T>
double Utils::mean(const std::vector<T> &vec){
  return double( sum(vec) ) / vec.size();
}

template<typename T>
double Utils::stddev(const std::vector<T> &vec){
  double N = vec.size();
  return pow( sqsum(vec) / N - pow(sum(vec)/N,2), 0.5);
}

double 		Utils::normal_cdf(double x) { return 0.5*(1.+TMath::Erf(x/TMath::Sqrt(2)));}

#endif
