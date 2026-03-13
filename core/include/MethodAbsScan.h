/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#ifndef MethodAbsScan_h
#define MethodAbsScan_h

#include "CLInterval.h"

#include <TAttLine.h>
#include <TRandom3.h>
#include <TString.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class Combiner;
class OneMinusClPlotAbs;
class OptParser;
class PValueCorrection;
class RooSlimFitResult;

class RooArgSet;
class RooDataSet;
class RooFitResult;
class RooRealVar;
class RooWorkspace;

class TGraph;
class TH1F;
class TH2F;

class MethodAbsScan {
 public:
  MethodAbsScan() = default;
  MethodAbsScan(Combiner* c);
  MethodAbsScan(const OptParser* opt);

  virtual ~MethodAbsScan();

  virtual void calcCLintervals(int CLsType = 0, bool calc_expected = false, bool quiet = false);
  void confirmSolutions();
  void doInitialFit(bool force = false);

  // Getters
  inline const OptParser* getArg() const { return arg; };
  inline const std::vector<RooSlimFitResult*>& getAllResults() { return allResults; };
  std::pair<double, double> getBorders(const TGraph& graph, double confidence_level, bool qubic = false) const;
  std::pair<double, double> getBorders_CLs(const TGraph& graph, double confidence_level, bool qubic = false) const;
  inline const std::vector<RooSlimFitResult*>& getCurveResults() { return curveResults; };
  inline double getChi2minGlobal() const { return chi2minGlobal; }
  inline double getChi2minBkg() const { return chi2minBkg; }
  double getCL(double val) const;
  CLInterval getCLintervalCentral(int sigma = 1, bool quiet = false);
  CLInterval getCLinterval(int iSol = 0, int sigma = 1, bool quiet = false);
  inline Combiner* getCombiner() { return combiner; };
  inline const Combiner* getCombiner() const { return combiner; };
  inline int getDrawSolution() const { return drawSolution; }
  inline bool getFilled() const { return drawFilled; };
  inline TH1F* getHCL() { return hCL; };
  inline TH1F* getHCLs() { return hCLs; };
  inline TH1F* getHCLsFreq() { return hCLsFreq; };
  inline TH1F* getHCLsExp() { return hCLsExp; };
  inline TH1F* getHCLsErr1Up() { return hCLsErr1Up; };
  inline TH1F* getHCLsErr1Dn() { return hCLsErr1Dn; };
  inline TH1F* getHCLsErr2Up() { return hCLsErr2Up; };
  inline TH1F* getHCLsErr2Dn() { return hCLsErr2Dn; };
  inline TH2F* getHCL2d() { return hCL2d; };
  inline TH2F* getHCLs2d() { return hCLs2d; };
  inline TH1F* getHchisq() { return hChi2min; };
  inline TH2F* getHchisq2d() { return hChi2min2d; };
  inline int getLineColor() const { return lineColor; };
  inline int getLineStyle() const { return lineStyle; };
  inline int getLineWidth() const { return lineWidth; };
  inline int getFillStyle() const { return fillStyle; };
  inline int getFillColor() const { return fillColor; };
  inline float getFillTransparency() const { return fillTransparency; };
  inline int getTextColor() const { return textColor; };
  inline TString getMethodName() const { return methodName; };
  inline TString getName() const { return name; };
  int getNObservables() const;
  inline int getNPoints1d() const { return nPoints1d; }
  inline int getNPoints2dx() const { return nPoints2dx; }
  inline int getNPoints2dy() const { return nPoints2dy; }
  const RooArgSet* getObservables() const;
  inline TString getObsName() const { return obsName; };
  inline TString getParsName() const { return parsName; };
  RooRealVar* getScanVar1();
  RooRealVar* getScanVar2();
  const RooRealVar* getScanVar1() const;
  const RooRealVar* getScanVar2() const;
  inline TString getScanVar1Name() const { return scanVar1; };
  inline TString getScanVar2Name() const { return scanVar2; };
  double getScanVarSolution(int iVar, int iSol) const;
  double getScanVar1Solution(int i = 0) const;
  double getScanVar2Solution(int i = 0) const;
  inline std::vector<RooSlimFitResult*> getSolutions() { return solutions; };
  inline int getNSolutions() const { return solutions.size(); };
  RooSlimFitResult* getSolution(int i = 0);
  const RooSlimFitResult* getSolution(int i = 0) const;
  const RooArgSet* getTheory() const;
  inline TString getTitle() const { return title; };
  inline RooWorkspace* getWorkspace() { return w; };
  inline const RooWorkspace* getWorkspace() const { return w; };

  virtual void initScan();
  void loadParameters(RooSlimFitResult* r);
  bool loadSolution(int i = 0);
  virtual bool loadScanner(TString fName = "");
  void plot2d(TString varx, TString vary);
  void plot1d(TString var);
  void plotOn(OneMinusClPlotAbs* plot, int CLsType = 0);  // CLsType: 0 (off), 1 (naive CLs t_s+b - t_b), 2 (freq CLs)
  void plotPulls(int nSolution = 0);
  virtual void print() const;
  void printCLintervals(int CLsType, bool calc_expected = false);
  void printLocalMinima() const;

  // Save/dump results
  void dumpResult(const std::string& ofname) const;
  void saveLocalMinima(TString fName = "") const;
  void saveScanner(TString fName = "") const;

  virtual int scan1d();
  virtual int scan2d();

  // Setters
  inline void setDrawSolution(int code = 0) { drawSolution = code; };
  inline void setPValueCorrector(PValueCorrection* pvalCor) {
    pvalueCorrector = pvalCor;
    pvalueCorrectorSet = true;
  }
  inline void setScanVar1(TString var) { scanVar1 = var; };
  inline void setScanVar2(TString var) { scanVar2 = var; };
  inline void setNPoints1d(int n) { nPoints1d = n; };
  inline void setNPoints2dx(int n) { nPoints2dx = n; };
  inline void setNPoints2dy(int n) { nPoints2dy = n; };
  inline void setFilled(bool filled) { drawFilled = filled; };
  inline void setLineColor(int c) { lineColor = c; };
  inline void setLineStyle(int c) { lineStyle = c; };
  inline void setLineWidth(int c) { lineWidth = c; };
  inline void setTextColor(int c) { textColor = c; };
  inline void setFillStyle(int c) { fillStyle = c; };
  inline void setFillColor(int c) { fillColor = c; };
  inline void setFillTransparency(float c) { fillTransparency = c; };
  inline void setTitle(TString s) { title = s; };
  void setChi2minGlobal(double x);
  void setSolutions(std::vector<RooSlimFitResult*> s);
  inline void setVerbose(bool yesNo = true) { verbose = yesNo; };
  inline void setHCL(TH1F* h) { hCL = h; };
  inline void setHchisq(TH1F* h) { hChi2min = h; };
  void setXscanRange(double min, double max);
  void setYscanRange(double min, double max);

  void calcCLintervalsSimple(int CLsType = 0, bool calc_expected = false);
  virtual bool checkCLs();

  /// All fit results we encounter along the scan.
  std::vector<RooSlimFitResult*> allResults;
  /// All fit results of the the points that make it into the 1-CL curve. Index is the bin number of hCL bins -1.
  std::vector<RooSlimFitResult*> curveResults;
  /// All fit results of the the points that make it into the 1-CL curve. Index is the gobal bin number of hCL2d -1.
  std::vector<std::vector<RooSlimFitResult*>> curveResults2d;
  /// Local minima filled by saveSolutions() and saveSolutions2d().
  std::vector<RooSlimFitResult*> solutions;

  /**
   * All CL intervals found by `calcCLintervals`.
   *
   * The first index corresponds to the CL values set (1, 2, 3 sigma by default).
   * The second index corresponds to the solutions from 0 to n (in case they fall in the scan range, otherwise the value
   * will be nullptr). There may be also two more CL intervals at the end, corresponding e.g. to scans starting from the
   * minimum and maximum values of the scan range.
   */
  std::vector<std::vector<std::unique_ptr<CLInterval>>> clintervals;

  RooFitResult* globalMin = nullptr;  ///< Parameter values at a global minimum.

 protected:
  void sortSolutions();

  TString name;                ///< basename, e.g. ggsz
  TString title;               ///< nice string for the legends
  TString methodName = "Abs";  ///< Prob, ...
  TString pdfName;             ///< PDF name in workspace, derived from name
  TString obsName;             ///< dataset name of observables, derived from name
  TString parsName;            ///< set name of physics parameters, derived from name
  TString thName;              ///< set name of theory parameters, derived from name
  TString toysName;            ///< set name of parameters to vary in toys
  TString scanVar1;            ///< scan parameter
  TString scanVar2;            ///< second scan parameter if we're scanning 2d
  int nPoints1d = -1;          ///< number of scan points used by 1d scan
  int nPoints2dx = -1;         ///< number of scan points used by 2d scan, x axis
  int nPoints2dy = -1;         ///< number of scan points used by 2d scan, y axis

  PValueCorrection* pvalueCorrector = nullptr;  // object which can correct the pvalue for undercoverage if required
  bool pvalueCorrectorSet = false;

  TRandom3 rndm;
  RooWorkspace* w = nullptr;
  RooDataSet* obsDataset = nullptr;  ///< save the nominal observables so we can restore them after we have fitted toys
  RooDataSet* startPars = nullptr;   ///< save the start parameter values before any scan
  TH1F* hCL = nullptr;               ///< 1-CL curve
  TH1F* hCLs = nullptr;              ///< 1-CL curve
  TH1F* hCLsFreq = nullptr;          ///< 1-CL curve
  TH1F* hCLsExp = nullptr;           ///< 1-CL curve
  TH1F* hCLsErr1Up = nullptr;        ///< 1-CL curve
  TH1F* hCLsErr1Dn = nullptr;        ///< 1-CL curve
  TH1F* hCLsErr2Up = nullptr;        ///< 1-CL curve
  TH1F* hCLsErr2Dn = nullptr;        ///< 1-CL curve
  TH2F* hCL2d = nullptr;             ///< 1-CL curve
  TH2F* hCLs2d = nullptr;            ///< 1-CL curve
  TH1F* hChi2min = nullptr;          ///< histogram for the chi2min values before Prob()
  TH2F* hChi2min2d = nullptr;        ///< histogram for the chi2min values before Prob()
  double chi2minGlobal = std::numeric_limits<double>::max();  ///< chi2 value at global minimum
  double chi2minBkg = std::numeric_limits<double>::max();     ///< chi2 value at global minimum
  bool chi2minGlobalFound = false;                            ///< flag to avoid finding minimum twice
  int lineColor = kBlue - 8;
  int textColor = kBlack;  ///< color used for plotted central values
  int lineStyle = 0;
  int lineWidth = 2;
  int fillStyle = 1001;
  int fillColor = kBlue - 8;
  float fillTransparency = 0.f;
  bool drawFilled = true;  ///< choose if Histogram is drawn filled or not
  int drawSolution = 0;    ///< Configure how to draw solutions on the plots.
  ///< 0=don't plot, 1=plot at central value (1d) or markers (2d)
  ///< Default is taken from arg, unless disabled by setDrawSolution().
  bool verbose = false;
  mutable int nWarnings = 0;             ///< number of warnings printed in getScanVarSolution()
  const OptParser* arg = nullptr;        ///< command line options
  Combiner* combiner = nullptr;          ///< the combination
  bool m_xrangeset = false;              ///< true if the x range was set manually (setXscanRange())
  bool m_yrangeset = false;              ///< true if the y range was set manually (setYscanRange())
  bool m_initialized = false;            ///< true if initScan() was called
  std::vector<double> ConfidenceLevels;  ///< container of the confidence levels to be computed

 private:
  bool compareSolutions(RooSlimFitResult* r1, RooSlimFitResult* r2);
  void removeDuplicateSolutions();

  std::optional<std::pair<double, double>> interpolate(TH1F* h, int i, double y, double central, bool upper) const;

  [[deprecated]] bool interpolate(TH1F* h, int i, double y, double central, bool upper, double& val, double& err) const;

  std::optional<double> interpolateLinear(TH1F* h, int i, double y) const;

  [[deprecated]] void interpolateSimple(TH1F* h, int i, double y, double& val) const;
};

#endif
