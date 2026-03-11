/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Aug 2014
 *
 **/

#ifndef GammaComboEngine_h
#define GammaComboEngine_h

#include <TStopwatch.h>
#include <TString.h>

#include <vector>

class BatchScriptWriter;
class Combiner;
class FileNameBuilder;
class MethodBergerBoosScan;
class MethodCoverageScan;
class MethodPluginScan;
class MethodProbScan;
class OneMinusClPlotAbs;
class OptParser;
class ParameterCache;
class PDF_Abs;

class TApplication;

///
/// The main GammaCombo scanning engine, controlling
/// the application.
///

class GammaComboEngine {
 public:
  GammaComboEngine(TString name, int argc, char* argv[]);
  GammaComboEngine(TString name, int argc, char* argv[], bool _runOnDataSet);
  ~GammaComboEngine();

  GammaComboEngine(GammaComboEngine&) = delete;
  GammaComboEngine& operator=(GammaComboEngine&) = delete;

  void adjustRanges(Combiner* c, int cId);
  void setupToyVariationSets(Combiner* c, int cId);
  void addPdf(int id, PDF_Abs* pdf, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, const std::vector<int>& indices, TString title = "");

  // The following methods are deprecated and maintained only to preserve backward-compatibility.
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, int i4, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, int i4, int i5, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, int i4, int i5, int i6, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, int i4, int i5, int i6, int i7, TString title = "");
  void addSubsetPdf(int id, PDF_Abs* pdf, int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8,
                    TString title = "");

  void setPdf(PDF_Abs* pdf);
  void addCombiner(int id, Combiner* cmb);
  void cloneCombiner(int newId, int oldId, TString name, TString title);
  void newCombiner(const int id, const TString name, const TString title, const std::vector<int>& pdfIds = {});

  /**
   * This method is deprecated and is maintained only for legacy reasons.
   *
   * Add a new Combiner, consisting of the specified PDFs. The pdf arguments refer to the GammaComboEngine ID of the
   * PDFs that should be combined (add them before using `addPdf()`).
   */
  void newCombiner(const int id, const TString name, const TString title, std::convertible_to<int> auto&&... pdfIds) {
    newCombiner(id, name, title, std::vector<int>{pdfIds...});
  }

  Combiner* getCombiner(int id);
  PDF_Abs* getPdf(int id);
  inline OptParser* getArg() const { return arg; };
  void print() const;
  void printPdfs() const;
  void printCombinations() const;
  void run();
  void runApplication();
  void scanStrategy1d(MethodProbScan* scanner, ParameterCache* pCache);
  void scanStrategy2d(MethodProbScan* scanner, ParameterCache* pCache);
  inline void setRunOnDataSet(bool opt) { runOnDataSet = opt; };
  PDF_Abs* operator[](int idx);

 private:
  void makeAddDelCombinations();
  void checkAsimovArg() const;
  void checkColorArg() const;
  void checkCombinationArg() const;
  void configureAsimovCombinerNames(Combiner* c, int i);
  bool combinerExists(int id) const;
  void compareCombinations();
  void customizeCombinerTitles();
  void defineColors();
  void disableSystematics();
  void fixParameters(Combiner* c, int cId);
  TString getStartParFileName(int cId) const;
  bool isScanVarObservable(Combiner* c, TString scanVar) const;
  void loadStartParameters(MethodProbScan* s, ParameterCache* pCache, int cId);
  void make1dPluginOnlyPlot(MethodPluginScan* sPlugin, int cId);
  void make1dPluginPlot(MethodPluginScan* sPlugin, MethodProbScan* sProb, int cId);
  void make1dPluginScan(MethodPluginScan* scannerPlugin, int cId);
  void make1dProbPlot(MethodProbScan* scanner, int cId);
  void make1dProbScan(MethodProbScan* scanner, int cId);
  void make1dCoverageScan(MethodCoverageScan* scanner, int cId);
  void make1dCoveragePlot(MethodCoverageScan* scanner, int cId);
  void make1dBergerBoosScan(MethodBergerBoosScan* scanner, int cId);
  void make2dPluginOnlyPlot(MethodPluginScan* sPlugin, int cId);
  void make2dPluginPlot(MethodPluginScan* sPlugin, MethodProbScan* sProb, int cId);
  void make2dPluginScan(MethodPluginScan* scannerPlugin, int cId);
  void make2dProbPlot(MethodProbScan* scanner, int cId);
  void make2dProbScan(MethodProbScan* scanner, int cId);
  void printCombinerStructure(Combiner* c) const;
  void printBanner() const;
  bool pdfExists(int id) const;
  void savePlot();
  void scaleStatErrors();
  void scaleStatAndSystErrors();
  void scaleDownErrors();  // now defunct
  void scan();
  void scanDataSet();
  void setAsimovObservables(Combiner* c);
  void setObservablesFromFile(Combiner* c, int cId);
  void loadAsimovPoint(Combiner* c, int cId);
  void setUpPlot();
  void tightenChi2Constraint(Combiner* c, TString scanVar);
  void usage() const;
  void writebatchscripts();
  void makeLatex(Combiner* c);
  void saveWorkspace(Combiner* c, int i);
  void runToys(Combiner* c);

  OptParser* arg = nullptr;
  std::vector<Combiner*> cmb;
  std::vector<int> colorsLine;
  std::vector<int> colorsText;
  std::vector<int> fillStyles;
  std::vector<int> fillColors;
  std::vector<float> fillTransparencies;
  std::vector<int> lineColors;
  std::vector<int> lineStyles;
  std::vector<int> lineWidths;
  std::vector<MethodProbScan*> comparisonScanners;
  TString execname;
  FileNameBuilder* m_fnamebuilder = nullptr;
  BatchScriptWriter* m_batchscriptwriter = nullptr;
  std::vector<PDF_Abs*> pdf;
  OneMinusClPlotAbs* plot = nullptr;
  TStopwatch t;
  TApplication* theApp = nullptr;
  bool runOnDataSet = false;
};

#endif
