/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef OptParser_h
#define OptParser_h

#include "Utils.h"
#include "tclap/CmdLine.h"

#include <TString.h>

#include <map>
#include <string>
#include <vector>

class OptParser {
 public:
  OptParser();

  void bookOption(TString opt);
  void bookAllOptions();
  void bookPlottingOptions();
  void bookPluginOptions();
  void bookProbOptions();
  void bookFlowcontrolOptions();
  void parseArguments(int argc, char* argv[]);
  bool isAction(TString s) const;
  bool isAsimovCombiner(int id) const;
  bool isQuickhack(int id) const;

  std::vector<TString> action;
  std::vector<int> asimov;
  std::vector<TString> asimovfile;
  bool cacheStartingValues;
  std::vector<double> CL;
  std::vector<int> cls;
  std::vector<int> color;
  std::vector<int> combid;
  std::vector<std::vector<int>> combmodifications;  // encodes requested modifications to the combiner ID through the -c
                                                    // 26:+12 syntax,format is [cmbid:[+pdf1,-pdf2,...]]
  bool compare = false;
  bool confirmsols = true;
  bool controlplot = false;
  int coverageCorrectionID = 0;
  int coverageCorrectionPoint = 0;
  bool debug = false;
  int digits = -99;
  bool enforcePhysRange = false;
  std::vector<int> fillstyle;
  std::vector<int> fillcolor;
  std::vector<float> filltransparency;
  std::vector<int> linewidth;
  std::vector<int> linecolor;
  std::vector<int> linestyle;
  std::vector<std::string> hexfillcolor;
  std::vector<std::string> hexlinecolor;
  TString filenamechange;
  TString filenameaddition;
  std::vector<std::vector<Utils::FixPar>> fixParameters;
  std::vector<std::vector<Utils::StartPar>> startVals;
  std::vector<std::vector<Utils::RangePar>> physRanges;
  std::vector<std::vector<TString>> removeRanges;
  std::vector<std::vector<TString>> randomizeToyVars;
  bool grid = false;
  TString group = "GammaCombo";
  TString groupPos;
  TString hfagLabel;
  TString hfagLabelPos;
  int id = -99;
  bool importance = false;
  bool info = false;
  bool interactive = false;
  std::vector<int> jmax;
  std::vector<int> jmin;
  TString jobdir = ".";
  bool largest = false;
  bool latex = false;
  std::vector<TString> loadParamsFile;
  bool lightfiles = false;
  int batchstartn = 1;
  bool batcheos = false;
  bool batchsubmit = false;
  TString batchout;
  TString batchreqs;
  int nbatchjobs = -99;
  int nBBpoints = -99;
  int ndiv = 407;
  int ndivy = 407;
  bool nosyst = false;
  double errtol = 5e-2;
  int npoints1d = -99;
  int npoints2dx = -99;
  int npoints2dy = -99;
  int npointstoy = -99;
  int ncoveragetoys = -99;
  int nrun = -99;
  int ntoys = -99;
  int nsmooth = 1;
  TString parsavefile;
  bool parevol = false;
  std::vector<int> pevid;
  std::vector<int> plot2dcl;
  TString plotdate = "";
  TString plotext = "";
  int plotid = -99;
  bool plotlog = false;
  bool plotlegend = true;
  double plotlegx = -99;
  double plotlegy = -99;
  double plotlegsizex = -99;
  double plotlegsizey = -99;
  TString plotlegstyle = "default";
  int plotlegcols = 1;
  bool plotlegbox = false;
  double plotlegboxx = -99;
  double plotlegboxy = -99;
  double plotgroupx = -99;
  double plotgroupy = -99;
  Double_t plotHFAGLabelPosX = 0.;
  Double_t plotHFAGLabelPosY = 0.;
  Double_t plotHFAGLabelScale = 1.;
  bool plotmagnetic = false;
  int plotnsigmacont = 2;
  std::map<int, std::vector<int>> contourlabels;
  bool plotpluginonly = false;
  bool plotpulls = false;
  bool plotprelim = false;
  double plotoriginx = -99;
  double plotoriginy = -99;
  std::vector<int> plotsolutions;
  std::vector<int> plotsoln;
  bool plotunoff = false;
  double plotymin = -99;
  double plotymax = -99;
  bool intprob = false;
  double pluginPlotRangeMin = -100;
  double pluginPlotRangeMax = -100;
  bool probforce = false;
  bool probimprove = false;
  TString probScanResult = "notSet";
  bool printcor = false;
  double printSolX = -999;
  double printSolY = -999;
  std::vector<int> qh;
  // TString queue;
  std::vector<std::vector<TString>> readfromfile;
  std::vector<TString> relation;
  bool runCLs;
  TString save;
  bool saveAtMin = false;
  std::vector<double> savenuisances1d;
  std::vector<double> savenuisances2dx;
  std::vector<double> savenuisances2dy;
  bool scanforce = false;
  double scanrangeMin = -101;
  double scanrangeMax = -101;
  double scanrangeyMin = -102;
  double scanrangeyMax = -102;
  double scaleerr = -999;
  double scalestaterr = -999;
  bool smooth2d = false;
  bool square = false;
  int teststatistic = 2;
  std::vector<TString> title;
  TString xtitle;
  TString ytitle;
  TString toyFiles;
  int updateFreq = 10;
  bool usage = false;
  std::vector<TString> var;
  bool verbose = false;

  TCLAP::CmdLine cmd{"", ' ', ""};

 private:
  void defineOptions();
  std::vector<TString> availableOptions;
  std::vector<TString> bookedOptions;
};

#endif
