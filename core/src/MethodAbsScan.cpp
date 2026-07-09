/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#include <MethodAbsScan.h>

#include <CLInterval.h>
#include <CLIntervalMaker.h>
#include <CLIntervalPrinter.h>
#include <Combiner.h>
#include <FileNameBuilder.h>
#include <FitResultCache.h>
#include <OneMinusClPlotAbs.h>
#include <OptParser.h>
#include <PullPlotter.h>
#include <RooSlimFitResult.h>
#include <Utils.h>

#include <RooAbsPdf.h>
#include <RooDataSet.h>
#include <RooFitResult.h>
#include <RooFormulaVar.h>
#include <RooMsgService.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <TSpline.h>
#include <TString.h>
#include <TStyle.h>

#include <algorithm>
#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {
  ///
  /// Solve a quadratic equation by means of a modified pq formula:
  /// @f[x^2 + \frac{p_1}{p_2} x + \frac{p_0-y}{p2} = 0@f]
  ///
  double pq(const double p0, const double p1, const double p2, const double y, const int whichSol) {
    if (whichSol == 0)
      return -p1 / 2. / p2 + sqrt(Utils::sq(p1 / 2. / p2) - (p0 - y) / p2);
    else
      return -p1 / 2. / p2 - sqrt(Utils::sq(p1 / 2. / p2) - (p0 - y) / p2);
  }
}  // namespace

MethodAbsScan::MethodAbsScan(Combiner* c) : MethodAbsScan(c->getArg()) {
  combiner = c;
  w = c->getWorkspace();
  name = c->getName();
  title = c->getTitle();
  pdfName = "pdf_" + combiner->getPdfName();
  obsName = "obs_" + combiner->getPdfName();
  parsName = "par_" + combiner->getPdfName();
  thName = "th_" + combiner->getPdfName();

  // check workspace content
  auto error = [](const TString& name) {
    Utils::errBase("MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : ", std::string(name));
  };
  if (!w->pdf(pdfName)) error(pdfName);
  if (!w->set(obsName)) error(obsName);
  if (!w->set(parsName)) error(parsName);
  if (!w->set(thName)) error(thName);
}

// constructor without combiner, this is atm still needed for the datasets stuff
MethodAbsScan::MethodAbsScan(const OptParser* opt)
    : rndm(), arg(opt), scanVar1(opt->var[0]), verbose(opt->verbose), nPoints1d(opt->npoints1d),
      nPoints2dx(opt->npoints2dx), nPoints2dy(opt->npoints2dy) {
  if (opt->var.size() > 1) scanVar2 = opt->var[1];
  if (opt->CL.size() > 0) {
    for (auto level : opt->CL) { ConfidenceLevels.push_back(level / 100.); }
  } else {
    ConfidenceLevels.push_back(0.6827);  // 1sigma
    ConfidenceLevels.push_back(0.9545);  // 2sigma
    ConfidenceLevels.push_back(0.9973);  // 3sigma
  }
}

MethodAbsScan::~MethodAbsScan() {
  for (int i = 0; i < allResults.size(); i++) {
    if (allResults[i]) delete allResults[i];
  }
  if (hCL) delete hCL;
  if (hCLs) delete hCLs;
  if (hCLsFreq) delete hCLsFreq;
  if (hCLsExp) delete hCLsExp;
  if (hCLsErr1Up) delete hCLsErr1Up;
  if (hCLsErr1Dn) delete hCLsErr1Dn;
  if (hCLsErr2Up) delete hCLsErr2Up;
  if (hCLsErr2Dn) delete hCLsErr2Dn;
  if (hCLs2d) delete hCLs2d;
  if (hCL2d) delete hCL2d;
  if (hChi2min) delete hChi2min;
  if (hChi2min2d) delete hChi2min2d;
  if (obsDataset) delete obsDataset;
  if (startPars) delete startPars;
  if (globalMin) delete globalMin;
}

///
/// Try to find global mininum of the PDF.
/// Despite its name this often finds a local minimum. It's merely
/// used as a starting point. When the scans stumbles upon a better
/// minimum, we'll keep that one.
///
/// Resets parameters to the values they had at function call.
/// Save the RooFitResult of the global minimum (or whatever minimum it found...)
/// into the globalMin member.
///
/// \param force If set to true it fits again, even it the fit was already run before.
///
void MethodAbsScan::doInitialFit(bool force) {
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  if (arg->debug) {
    std::cout << "\n============================================================" << std::endl;
    std::cout << "MethodAbsScan::doInitialFit() : MAKE FIRST FIT ..." << std::endl;
    std::cout << "MethodAbsScan::doInitialFit() : PDF " << pdfName << std::endl << std::endl;
  }
  if (!force && chi2minGlobalFound) {
    if (arg->debug) {
      std::cout << "MethodAbsScan::doInitialFit() : Already found previously: chi2minGlobal = " << chi2minGlobal
                << std::endl;
      std::cout << "\n============================================================" << std::endl;
    }
    return;
  }

  // Save parameter values that were active at function call.
  if (startPars) delete startPars;
  startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
  startPars->add(*w->set(parsName));

  // load parameter range
  combiner->loadParameterLimits();

  Utils::fixParameters(w, obsName);     ///< fix observables
  Utils::floatParameters(w, parsName);  ///< physics parameters need to be floating to find global minimum

  // load again the parameter values that were specified on the command line -
  // loading a set of start parameters might have changed them
  combiner->setParametersConstant();

  // fix parameters we decided to keep constant (to keep a parameter constant
  // add them manually to the workspace set 'const')
  Utils::fixParameters(w, "const");

  // check choice of start parameters
  double nsigma = 10.;
  PullPlotter p(this);
  if (p.hasPullsAboveNsigma(nsigma)) {
    std::cout << "MethodAbsScan::doInitialFit() : WARNING : Chosen start parameter values result in pulls larger\n"
                 "                                WARNING : than "
              << nsigma
              << " sigma. Check the values in your\n"
                 "                                WARNING : ParametersAbs class!\n"
                 "Offending pulls:"
              << std::endl;
    p.printPulls(nsigma);
    std::cout << std::endl;
  }

  // print init parameters
  if (arg->debug) {
    std::cout << "MethodAbsScan::doInitialFit() : init parameters:" << std::endl;
    w->set(parsName)->Print("v");
    std::cout << "MethodAbsScan::doInitialFit() : init pulls:" << std::endl;
    p.printPulls(0.);
    std::cout << "MethodAbsScan::doInitialFit() : PDF evaluated at init parameters: ";
    std::cout << w->pdf(pdfName)->getVal() << std::endl;
    RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*w->pdf(pdfName)));
    std::cout << "MethodAbsScan::doInitialFit() : Chi2 at init parameters: ";
    std::cout << ll.getVal() << std::endl;
  }

  int quiet = arg->debug ? 1 : -1;
  RooFitResult* r = Utils::fitToMinBringBackAngles(w->pdf(pdfName), true, quiet);
  // RooFitResult* r = Utils::fitToMin(w->pdf(pdfName), true, quiet);
  if (arg->debug) r->Print("v");
  // globalMin = new RooSlimFitResult(r);
  globalMin = r;
  chi2minGlobal = globalMin->minNll();
  chi2minGlobalFound = true;

  // reset parameters to their values at function call
  Utils::setParameters(w, parsName, startPars->get(0));

  if (arg->debug) std::cout << "============================================================\n" << std::endl;
  RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
}

///
/// Set the global minimum manually.
///
void MethodAbsScan::setChi2minGlobal(double x) {
  chi2minGlobalFound = true;
  chi2minGlobal = x;
}

void MethodAbsScan::initScan() {
  if (arg->debug) std::cout << "MethodAbsScan::initScan() : initializing ..." << std::endl;
  if (m_initialized) {
    std::cout << "MethodAbsScan::initScan() : already initialized." << std::endl;
    std::exit(1);
  }

  // Init the 1-CL histograms. Range is taken from the scan range defined in
  // the ParameterAbs class (and derived ones), unless the --scanrange command
  // line argument is set.
  RooRealVar* par1 = w->var(scanVar1);
  if (!par1) {
    if (arg->debug) std::cout << "MethodAbsScan::initScan() : ";
    std::cout << "ERROR : No such scan parameter: " << scanVar1 << std::endl;
    std::cout << "        Choose an existing one using: --var par" << std::endl << std::endl;
    std::cout << "  Available parameters:" << std::endl;
    std::cout << "  ---------------------" << std::endl << std::endl;
    for (int i = 0; i < combiner->getParameterNames().size(); i++) {
      std::cout << "    " << combiner->getParameterNames()[i] << std::endl;
    }
    std::cout << std::endl;
    std::exit(1);
  }
  if (!m_xrangeset && arg->scanrangeMin != arg->scanrangeMax) { setXscanRange(arg->scanrangeMin, arg->scanrangeMax); }
  Utils::setLimit(w, scanVar1, "scan");
  double min1 = par1->getMin();
  double max1 = par1->getMax();
  using Utils::getUniqueRootName;
  hCL = new TH1F("hCL" + getUniqueRootName(), "hCL" + pdfName, nPoints1d, min1, max1);
  if (hChi2min) delete hChi2min;
  hChi2min = new TH1F("hChi2min" + getUniqueRootName(), "hChi2min" + pdfName, nPoints1d, min1, max1);
  if (hCLs) delete hCLs;
  hCLs = new TH1F("hCLs" + getUniqueRootName(), "hCLs" + pdfName, nPoints1d, min1, max1);
  if (hCLs) delete hCLsFreq;
  hCLsFreq = new TH1F("hCLsFreq" + getUniqueRootName(), "hCLsFreq" + pdfName, nPoints1d, min1, max1);
  if (hCLsExp) delete hCLsExp;
  hCLsExp = new TH1F("hCLsExp" + getUniqueRootName(), "hCLsExp" + pdfName, nPoints1d, min1, max1);
  if (hCLsErr1Up) delete hCLsErr1Up;
  hCLsErr1Up = new TH1F("hCLsErr1Up" + getUniqueRootName(), "hCLsErr1Up" + pdfName, nPoints1d, min1, max1);
  if (hCLsErr1Dn) delete hCLsErr1Dn;
  hCLsErr1Dn = new TH1F("hCLsErr1Dn" + getUniqueRootName(), "hCLsErr1Dn" + pdfName, nPoints1d, min1, max1);
  if (hCLsErr2Up) delete hCLsErr2Up;
  hCLsErr2Up = new TH1F("hCLsErr2Up" + getUniqueRootName(), "hCLsErr2Up" + pdfName, nPoints1d, min1, max1);
  if (hCLsErr2Dn) delete hCLsErr2Dn;
  hCLsErr2Dn = new TH1F("hCLsErr2Dn" + getUniqueRootName(), "hCLsErr2Dn" + pdfName, nPoints1d, min1, max1);

  // fill the chi2 histogram with very unlikely values such
  // that inside scan1d() the if clauses work correctly
  for (int i = 1; i <= nPoints1d; i++) hChi2min->SetBinContent(i, 1e6);

  if (scanVar2 != "") {
    RooRealVar* par2 = w->var(scanVar2);
    if (!par2) {
      if (arg->debug) std::cout << "MethodAbsScan::initScan() : ";
      std::cout << "ERROR : No such scan parameter: " << scanVar2 << std::endl;
      std::cout << "        Choose an existing one using: --var par" << std::endl << std::endl;
      std::cout << "  Available parameters:" << std::endl;
      std::cout << "  ---------------------" << std::endl << std::endl;
      for (int i = 0; i < combiner->getParameterNames().size(); i++) {
        std::cout << "    " << combiner->getParameterNames()[i] << std::endl;
      }
      std::cout << std::endl;
      std::exit(1);
    }
    if (!m_yrangeset && arg->scanrangeyMin != arg->scanrangeyMax) {
      setYscanRange(arg->scanrangeyMin, arg->scanrangeyMax);
    }
    Utils::setLimit(w, scanVar2, "scan");
    double min2 = par2->getMin();
    double max2 = par2->getMax();
    if (hCL2d) delete hCL2d;
    hCL2d = new TH2F("hCL2d" + getUniqueRootName(), "hCL2d" + pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
    if (hChi2min2d) delete hChi2min2d;
    hChi2min2d =
        new TH2F("hChi2min2d" + getUniqueRootName(), "hChi2min", nPoints2dx, min1, max1, nPoints2dy, min2, max2);
    for (int i = 1; i <= nPoints2dx; i++)
      for (int j = 1; j <= nPoints2dy; j++) hChi2min2d->SetBinContent(i, j, 1e6);
  }

  // Set up storage for the fit results.
  // Clear before so we can call initScan() multiple times.
  // Note that allResults still needs to hold all results, so don't delete the RooFitResults.

  // 1d:
  curveResults.clear();
  for (int i = 0; i < nPoints1d; i++) curveResults.push_back(0);

  // 2d:
  curveResults2d.clear();
  for (int i = 0; i < nPoints2dx; i++) {
    std::vector<RooSlimFitResult*> tmp;
    for (int j = 0; j < nPoints2dy; j++) tmp.push_back(0);
    curveResults2d.push_back(tmp);
  }

  // global minimum
  doInitialFit();

  // turn off some messages
  RooMsgService::instance().setStreamStatus(0, kFALSE);
  RooMsgService::instance().setStreamStatus(1, kFALSE);
  m_initialized = true;
}

void MethodAbsScan::dumpResult(const std::string& ofname) const {

  system("mkdir -p plots/par");
  const auto ofpath = std::format("plots/par/{:s}.dat", ofname);
  std::cout << "MethodAbsScan::dumpResult() : saving " << ofpath << std::endl;

  bool angle = Utils::isAngle(getWorkspace()->var(getScanVar1Name()));

  std::ofstream outf;
  outf.open(ofpath);
  outf << std::format("# Fit Result Summary\n"
                      "nSolutions={:d}\n"
                      "# pvalue central min max\n",
                      getNSolutions());

  for (int cl = 0; cl < 2; ++cl) {
    for (const auto& ci : clintervals[cl]) {
      auto central = ci->central;
      auto min = ci->min;
      auto max = ci->max;
      if (angle) {
        using Utils::RadToDeg;
        central = RadToDeg(central);
        min = RadToDeg(min);
        max = RadToDeg(max);
      }
      outf << std::format("{:f} {:f} {:f} {:f}\n", ci->pvalue, central, min, max);
    }
  }
  outf.close();
}

///
/// Save this scanner to a root file placed into plots/scanner.
/// It contains the 1-CL histograms and the solutions.
///
void MethodAbsScan::saveScanner(TString fName) const {
  if (fName == "") {
    FileNameBuilder fb(arg);
    fName = fb.getFileNameScanner(this);
  }
  if (arg->debug) std::cout << "MethodAbsScan::saveScanner() : saving scanner: " << fName << std::endl;
  TFile f(fName, "recreate");
  // save 1-CL histograms
  if (scanVar2 != "") {
    hCL2d->Write("hCL");
    if (hCLs2d) hCLs2d->Write("hCLs");
  } else {
    hCL->Write("hCL");
    if (hCLs) hCLs->Write("hCLs");
    if (hCLsFreq) hCLsFreq->Write("hCLsFreq");
    if (hCLsExp) hCLsExp->Write("hCLsExp");
    if (hCLsErr1Up) hCLsErr1Up->Write("hCLsErr1Up");
    if (hCLsErr1Dn) hCLsErr1Dn->Write("hCLsErr1Dn");
    if (hCLsErr2Up) hCLsErr2Up->Write("hCLsErr2Up");
    if (hCLsErr2Dn) hCLsErr2Dn->Write("hCLsErr2Dn");
  }
  // save chi2 histograms
  if (scanVar2 != "")
    hChi2min2d->Write("hChi2min");
  else
    hChi2min->Write("hChi2min");
  // save solutions
  for (int i = 0; i < solutions.size(); i++) { f.WriteObject(solutions[i], Form("sol%i", i)); }
}

///
/// Save a scanner from plots/scanner.
/// It contains the 1-CL histograms and the solutions.
///
bool MethodAbsScan::loadScanner(TString fName) {
  if (fName == "") {
    FileNameBuilder fb(arg);
    fName = fb.getFileNameScanner(this);
  }
  if (arg->debug) std::cout << "MethodAbsScan::loadScanner() : ";
  std::cout << "loading scanner: " << fName << std::endl;
  if (!Utils::FileExists(fName)) {
    std::cout << "MethodAbsScan::loadScanner() : ERROR : file not found: " << fName << std::endl;
    std::cout << "                               Run first without the '-a plot' option to produce the missing file."
              << std::endl;
    std::exit(1);
  }
  TFile* f = new TFile(fName, "ro");  // don't delete this later else the objects die
  // load 1-CL histograms
  TObject* obj = f->Get("hCL");
  if (obj == 0) {
    std::cout << "MethodAbsScan::loadScanner() : ERROR : 'hCL' not found in root file " << fName << std::endl;
    std::exit(1);
  }

  using Utils::getUniqueRootName;
  if (scanVar2 != "") {
    hCL2d = (TH2F*)obj;
    hCL2d->SetName("hCL2d" + getUniqueRootName());
  } else {
    hCL = (TH1F*)obj;
    hCL->SetName("hCL" + getUniqueRootName());
  }
  // load chi2 histograms
  obj = f->Get("hChi2min");
  if (obj == 0) {
    std::cout << "MethodAbsScan::loadScanner() : ERROR : 'hChi2min' not found in root file " << fName << std::endl;
    // std::exit(1);
    // return false;
  }
  if (scanVar2 != "") {
    hChi2min2d = (TH2F*)obj;
    hChi2min2d->SetName("hChi2min2d" + getUniqueRootName());
  } else {
    hChi2min = (TH1F*)obj;
    hChi2min->SetName("hChi2min" + getUniqueRootName());
  }
  // load CLs histograms
  if (std::find(arg->cls.begin(), arg->cls.end(), 1) != arg->cls.end()) {
    obj = f->Get("hCLs");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLs' not found in root file - you can ignore this if you're "
             "not running in dataset mode "
          << fName << std::endl;
    }
    if (scanVar2 != "") {
      hCLs2d = (TH2F*)obj;
      hCLs2d->SetName("hCLs2d" + getUniqueRootName());
    } else {
      hCLs = (TH1F*)obj;
      hCLs->SetName("hCLs" + getUniqueRootName());
    }
  }
  // load CLs histograms
  bool lookForMixedCLs =
      std::find(arg->cls.begin(), arg->cls.end(), 2) != arg->cls.end() && !methodName.Contains("Prob");
  if (lookForMixedCLs) {
    obj = f->Get("hCLsFreq");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLsFreq' not found in root file - you can ignore this if "
             "you're not running in dataset mode "
          << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsFreq = (TH1F*)obj;
      hCLsFreq->SetName("hCLsFreq" + getUniqueRootName());
    }
    obj = f->Get("hCLsExp");
    if (obj == 0) {
      std::cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsExp' not found in root file - you can ignore this if "
                   "you're not running in dataset mode "
                << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsExp = (TH1F*)obj;
      hCLsExp->SetName("hCLsExp" + getUniqueRootName());
    }
    obj = f->Get("hCLsErr1Up");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr1Up' not found in root file - you can ignore this if "
             "you're not running in dataset mode "
          << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsErr1Up = (TH1F*)obj;
      hCLsErr1Up->SetName("hCLsErr1Up" + getUniqueRootName());
    }
    obj = f->Get("hCLsErr1Dn");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr1Dn' not found in root file - you can ignore this if "
             "you're not running in dataset mode "
          << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsErr1Dn = (TH1F*)obj;
      hCLsErr1Dn->SetName("hCLsErr1Dn" + getUniqueRootName());
    }
    obj = f->Get("hCLsErr2Up");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr2Up' not found in root file - you can ignore this if "
             "you're not running in dataset mode "
          << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsErr2Up = (TH1F*)obj;
      hCLsErr2Up->SetName("hCLsErr2Up" + getUniqueRootName());
    }
    obj = f->Get("hCLsErr2Dn");
    if (obj == 0) {
      std::cout
          << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr2Dn' not found in root file - you can ignore this if "
             "you're not running in dataset mode "
          << fName << std::endl;
    } else if (scanVar2 == "") {
      hCLsErr2Dn = (TH1F*)obj;
      hCLsErr2Dn->SetName("hCLsErr2Dn" + getUniqueRootName());
    }
  }
  // load solutions: try the first one hundred
  solutions.clear();
  int nSol = 100;
  for (int i = 0; i < nSol; i++) {
    RooSlimFitResult* r = (RooSlimFitResult*)f->Get(Form("sol%i", i));
    if (!r) break;
    solutions.push_back(r);
  }
  if (f->Get(Form("sol%i", nSol))) {
    std::cout << "MethodAbsScan::loadScanner() : WARNING : Only the first 100 solutions read from: " << fName
              << std::endl;
  }

  return true;
}

int MethodAbsScan::scan1d() {
  std::cout << "MethodAbsScan::scan1d() : not implemented." << std::endl;
  return 0;
}

int MethodAbsScan::scan2d() {
  std::cout << "MethodAbsScan::scan2d() : not implemented." << std::endl;
  return 0;
}

/**
 * Find the value of x for which a function takes on the value y, given a histogram that approximates the function.
 *
 * The interpolation of x that satisfies h(x) = y is done by means of a linear function between the y values of two
 * neighbouring bins i and i+1, such that y takes on a value in between h[i] and h[i+1].
 *
 * @param h The histogram to be interpolated.
 * @param i Interpolate around this bin (i and i+1 must be are above and below the interpolated value).
 * @param y The y position we want to find the interpolated x for.
 *
 * @return Interpolated x position, or std::nullopt if this value could not be calculated.
 */
std::optional<double> MethodAbsScan::interpolateLinear(TH1F* h, const int i, const double y) const {
  if (!(1 <= i && i <= h->GetNbinsX() - 1)) return {};
  double p1x = h->GetBinCenter(i);
  double p1y = h->GetBinContent(i);
  double p2x = h->GetBinCenter(i + 1);
  double p2y = h->GetBinContent(i + 1);
  if (!((p1y < y && y < p2y) || (p2y < y && y < p1y))) [[unlikely]] {
    std::cerr << "MethodAbsScan::interpolateLinear : ERROR : There is a problem in GammaCombo core" << std::endl;
    std::exit(1);
  }
  return p2x + (y - p2y) / (p1y - p2y) * (p1x - p2x);
}

void MethodAbsScan::interpolateSimple(TH1F* h, const int i, const double y, double& val) const {
  if (auto opt = interpolateLinear(h, i, y))
    val = *opt;
  else
    val = std::numeric_limits<double>::quiet_NaN();
}

/**
 * Find an interpolated x value near a certain bin position of a histogram that is the best estimate for h(x)=y.
 *
 * Interpolates by means of fitting a second grade polynomial to up to five adjacent points.
 * Because that's giving two solutions, we use the central value and knowledge about if it is supposed to be an upper
 * or lower boundary to pick one.
 *
 * @param h       The histogram to be interpolated.
 * @param i       Interpolate around this bin (must be such that i and i+1 are above and below the interpolated value).
 * @param y       The y position we want to find the interpolated x for.
 * @param central Central value of the solution we're trying to get the CL interval for.
 * @param upper   Set to true if we're computing an upper interval boundary.
 *
 * @return        std::nullopt if the interpolation failed, otherwise a pair {val, err} where val is the interpolated
 *                x position, and err the estimated interpolation error.
 */
std::optional<std::pair<double, double>> MethodAbsScan::interpolate(TH1F* h, const int i, const double y,
                                                                    const double central, const bool upper) const {
  // std::cout << "MethodAbsScan::interpolate(): i=" << i << " y=" << y << " central=" << central << std::endl;
  if (i > h->GetNbinsX() - 2 || i < 3) return {};

  // If method Prob, don't interpolate (no proper error estimate)
  if (methodName.Contains("Prob")) {
    for (int k = 0; k < h->GetNbinsX(); k++) { h->SetBinError(k + 1, 0.); }
  }

  // compute pol2 fit interpolation
  TGraphErrors* g = new TGraphErrors(3);
  g->SetPoint(0, h->GetBinCenter(i - 1), h->GetBinContent(i - 1));
  g->SetPointError(0, h->GetBinWidth(i - 1) / 2., h->GetBinError(i - 1));
  g->SetPoint(1, h->GetBinCenter(i), h->GetBinContent(i));
  g->SetPointError(1, h->GetBinWidth(i) / 2., h->GetBinError(i));
  g->SetPoint(2, h->GetBinCenter(i + 1), h->GetBinContent(i + 1));
  g->SetPointError(2, h->GetBinWidth(i + 1) / 2., h->GetBinError(i + 1));

  // see if we can add a 4th and 5th point
  if ((h->GetBinContent(i - 2) - h->GetBinError(i - 2) < h->GetBinContent(i - 1) + h->GetBinError(i - 1) &&
       h->GetBinContent(i - 1) < h->GetBinContent(i)) ||
      (h->GetBinContent(i - 2) + h->GetBinError(i - 2) > h->GetBinContent(i - 1) - h->GetBinError(i - 1) &&
       h->GetBinContent(i - 1) > h->GetBinContent(i))) {
    // don't use for upper limit calculation if point is equal or below central value
    if ((upper && h->FindBin(central) < i - 2) || !upper) {
      // add to the beginning
      TGraphErrors* gNew = new TGraphErrors(g->GetN() + 1);
      gNew->SetPoint(0, h->GetBinCenter(i - 2), h->GetBinContent(i - 2));
      gNew->SetPointError(0, h->GetBinWidth(i - 2) / 2., h->GetBinError(i - 2));
      Double_t pointx, pointy;
      Double_t pointxerr, pointyerr;
      for (int i = 0; i < g->GetN(); i++) {
        g->GetPoint(i, pointx, pointy);
        pointxerr = g->GetErrorX(i);
        pointyerr = g->GetErrorY(i);
        gNew->SetPoint(i + 1, pointx, pointy);
        gNew->SetPointError(i + 1, pointxerr, pointyerr);
      }
      delete g;
      g = gNew;
    }
  }

  if ((h->GetBinContent(i + 2) - h->GetBinError(i + 2) < h->GetBinContent(i + 1) + h->GetBinError(i + 1) &&
       h->GetBinContent(i + 1) < h->GetBinContent(i)) ||
      (h->GetBinContent(i + 2) + h->GetBinError(i + 2) > h->GetBinContent(i + 1) - h->GetBinError(i + 1) &&
       h->GetBinContent(i + 1) > h->GetBinContent(i))) {
    // don't use for lower limit calculation if point is equal or above central value
    if ((!upper && h->FindBin(central) > i + 2) || upper) {
      // add to the end
      g->Set(g->GetN() + 1);
      g->SetPoint(g->GetN() - 1, h->GetBinCenter(i + 2), h->GetBinContent(i + 2));
      g->SetPointError(g->GetN() - 1, h->GetBinWidth(i + 2) / 2., h->GetBinError(i + 2));
    }
  }

  TF1* f1 = new TF1("f1", "[0]+[1]*(x-[2])", h->GetBinCenter(i - 2), h->GetBinCenter(i + 2));
  TF1* f2 = new TF1("f2", "[0]+[1]*(x-[3])+[2]*(x-[3])**2", h->GetBinCenter(i - 2), h->GetBinCenter(i + 2));
  f1->FixParameter(2, h->GetBinCenter(i));
  f2->FixParameter(3, h->GetBinCenter(i));

  f1->SetParameter(1, (h->GetBinContent(i + 1) - h->GetBinContent(i)) / h->GetBinWidth(i));
  g->Fit("f1", "q");  // fit linear to get decent start parameters
  f2->SetParameter(0, f1->GetParameter(0));
  f2->SetParameter(1, f1->GetParameter(1));
  g->Fit("f2", "qf+");  // refit with minuit to get more correct errors (TGraph fit errors bug)
  double p[3], e[3];
  // for ( int ii=0; ii<3; ii++ )
  // {
  //  p[ii] = f2->GetParameter(ii);
  //  e[ii] = f2->GetParError(ii);
  // }
  p[0] = f2->GetParameter(2) * (f2->GetParameter(3) * f2->GetParameter(3)) - f2->GetParameter(1) * f2->GetParameter(3) +
         f2->GetParameter(0);
  p[1] = f2->GetParameter(1) - 2 * f2->GetParameter(2) * f2->GetParameter(3);
  p[2] = f2->GetParameter(2);

  double sol0 = pq(p[0], p[1], p[2], y, 0);
  double sol1 = pq(p[0], p[1], p[2], y, 1);
  // std::cout << upper << " ";
  // printf("%f %f %f\n", central, sol0, sol1);

  // std::cout << central << "\t" << sol0 << "\t" <<sol1 << std::endl;

  // debug: show fitted 1-CL histogram
  if (arg->controlplot) {
    TString debugTitle = methodName + Form(" y=%.2f ", y);
    debugTitle += upper ? Form("%f upper", central) : Form("%f lower", central);
    TCanvas* c = Utils::newNoWarnTCanvas(Utils::getUniqueRootName(), debugTitle);
    g->SetMarkerStyle(3);
    g->SetHistogram(h);
    h->Draw();
    g->Draw("p");
    f2->Draw("SAME");
    Utils::savePlot(c, TString(name + "_" + scanVar1 + "_boundary_interpolation_" + methodName + "_" +
                               TString(h->GetName()) + "_" + std::to_string(y)));
  }

  if ((h->GetBinCenter(i - 2) > sol0 || sol0 > h->GetBinCenter(i + 2)) &&
      (h->GetBinCenter(i - 2) > sol1 || sol1 > h->GetBinCenter(i + 2))) {
    if (arg->verbose || arg->debug) {
      std::cout << "MethodAbsScan::interpolate(): Quadratic interpolation out of bounds [" << h->GetBinCenter(i - 2)
                << ", " << h->GetBinCenter(i + 2) << "]:" << std::endl;
      std::cout << "Solutions are " << central << "(free fit result)\t" << sol0 << "(bound solution 0) \t" << sol1
                << "(bound solution 1)." << std::endl;
    }
    return {};
  } else if (sol0 != sol0 || sol1 != sol1) {
    if (arg->verbose || arg->debug) {
      std::cout << "MethodAbsScan::interpolate(): Quadratic interpolation leads to NaN:" << std::endl;
      std::cout << "Solutions are " << central << "(free fit result)\t" << sol0 << "(bound solution 0) \t" << sol1
                << "(bound solution 1)." << std::endl;
    }
    return {};
  }

  int useSol = 0;
  if ((sol0 < central && sol1 > central) || (sol1 < central && sol0 > central)) {
    if (upper) {
      useSol = (sol0 < sol1) ? 1 : 0;
    } else {
      useSol = (sol0 < sol1) ? 0 : 1;
    }
  } else {
    useSol = (std::abs(h->GetBinCenter(i) - sol0) < std::abs(h->GetBinCenter(i) - sol1)) ? 0 : 1;
  }

  double val = (useSol == 0) ? sol0 : sol1;

  // TODO try error propagation: sth is wrong in the formulae
  /*
  double err0 =
      TMath::Max(sq(val - pq(p[0] + e[0], p[1], p[2], y, useSol)), sq(val - pq(p[0] - e[0], p[1], p[2], y, useSol)));
  double err1 =
      TMath::Max(sq(val - pq(p[0], p[1] + e[1], p[2], y, useSol)), sq(val - pq(p[0], p[1] - e[1], p[2], y, useSol)));
  double err2 =
      TMath::Max(sq(val - pq(p[0], p[1], p[2] + e[2], y, useSol)), sq(val - pq(p[0], p[1], p[2] - e[2], y, useSol)));
  auto err = sqrt(err0 + err1 + err2);
  printf("%f %f %f\n", val, pq(p[0] + e[0], p[1], p[2], y, useSol), pq(p[0] - e[0], p[1], p[2], y, useSol));
  printf("%f %f %f\n", val, pq(p[0], p[1] + e[1], p[2], y, useSol), pq(p[0], p[1] - e[1], p[2], y, useSol));
  printf("%f %f %f\n", val, pq(p[0], p[1], p[2] + e[2], y, useSol), pq(p[0], p[1], p[2] - e[2], y, useSol));
  */
  auto err = h->GetBinWidth(i) / std::numbers::sqrt3 / 2.;  // Assume uniform distribution in the bin
  return std::make_pair(val, err);
}

bool MethodAbsScan::interpolate(TH1F* h, int i, double y, double central, bool upper, double& val, double& err) const {
  if (auto pair = interpolate(h, i, y, central, upper)) {
    val = pair->first;
    err = pair->second;
    return true;
  }
  return false;
}

/**
 * Calculate the CL intervals from the CL curve.
 *
 * Start from known local minima and scan upwards and downwards to find the interval boundaries. Then scan again from
 * the boundaries of the scan range to cover the case where an CL interval is not closed yet at the boundary.
 * Use a fit-based interpolation (@see interpolate) if we have more than 25 bins, else revert to a straight line
 * interpolation (@see interpolateLinear).
 */
void MethodAbsScan::calcCLintervals(const int CLsType, const bool calc_expected, const bool quiet) {
  auto debug = [](const std::string& msg) { Utils::msgBase("MethodAbsScan::calcCLintervals() : DEBUG : ", msg); };
  auto info = [](const std::string& msg) { Utils::msgBase("MethodAbsScan::calcCLintervals() : ", msg); };
  auto warning = [](const std::string& msg) { Utils::msgBase("MethodAbsScan::calcCLintervals() : WARNING : ", msg); };
  auto error = [](const std::string& msg) {
    Utils::errBase("MethodAbsScan::calcCLintervals() : ERROR : ", msg, false);
  };

  if (arg->debug) debug(std::format("Calling arguments: {:d}, {:s}, {:s}", CLsType, calc_expected, quiet));

  TH1F* histogramCL = this->getHCL();
  // calc CL intervals with CLs method
  if (CLsType == 1 && this->getHCLs()) {
    histogramCL = this->getHCLs();
  } else if (CLsType == 2 && this->getHCLsFreq()) {
    histogramCL = this->getHCLsFreq();
  }
  if (CLsType == 2 && calc_expected && hCLsExp) {
    histogramCL = hCLsExp;
    std::cout << "Determine expected upper limit:" << std::endl;
  }
  if (!histogramCL) {
    error("Could not retrieve the histogram. Will not calculate the CLs");
    return;
  }

  if (CLsType != 0) { std::cout << std::format("Confidence Intervals for CLs method {:d}:", CLsType) << std::endl; }

  if (arg->isQuickhack(8)) {
    // TODO Switch to the new CLIntervalMaker mechanism. It can be activated already using --qh 8,
    //      but it really is in beta stage still
    // TODO Add user specific CL interval
    std::cout << "\n";
    info(std::format("USING NEW CLIntervalMaker for {:s}\n", std::string(name)));
    CLIntervalMaker clm(arg, *histogramCL);
    clm.findMaxima(0.04);  // TODO ignore maxima under pvalue=0.04
    for (int iSol = 0; iSol < solutions.size(); iSol++) {
      double sol = getScanVar1Solution(iSol);
      clm.provideMorePreciseMaximum(sol, "max PLH");
    }
    clm.calcCLintervals();
    // print
    TString unit = w->var(scanVar1)->getUnit();
    CLIntervalPrinter clp(arg, name, scanVar1, unit,
                          std::string(methodName) + (calc_expected ? "_expected_standardCLs" : ""));
    clp.setDegrees(Utils::isAngle(w->var(scanVar1)));
    clp.addIntervals(clm.getClintervals1sigma());
    clp.addIntervals(clm.getClintervals2sigma());
    clp.print();
  }

  if (solutions.empty()) {
    info("Solutions std::vector empty. Using simple method with linear splines");
    this->calcCLintervalsSimple(CLsType, calc_expected);
    return;
  } else if ((CLsType == 1 || CLsType == 2) && !this->getHCLs()) {
    info("Using simple method with linear splines");
    this->calcCLintervalsSimple(CLsType, calc_expected);
  }

  if (!quiet)
    std::cout << std::format("\nCONFIDENCE INTERVALS for combination `{:s}`\n", std::string(name)) << std::endl;

  clintervals.clear();
  clintervals.resize(ConfidenceLevels.size());

  // Pairs of starting points and relative bins for the histogram scan
  std::vector<std::pair<double, int>> starts;
  const auto n = histogramCL->GetNbinsX();
  for (int i = 0; i < solutions.size(); i++) {
    const auto sol = getScanVar1Solution(i);
    int bin = histogramCL->FindBin(sol);
    if (histogramCL->IsBinOverflow(bin) || histogramCL->IsBinUnderflow(bin)) {
      warning(std::format(
          "Solution {:d} is outside of the scanrange, I will not try to find the relative confidence interval", i));
      starts.emplace_back(std::numeric_limits<double>::quiet_NaN(), -1);
    } else {
      if (bin == 1 || bin == n)
        warning(std::format("Solution {:d} lies at the border of the scanrange, you should increase it", i));
      starts.emplace_back(sol, bin);
    }
  }
  starts.emplace_back(histogramCL->GetXaxis()->GetXmin(), 1);
  starts.emplace_back(histogramCL->GetXaxis()->GetXmax(), n);

  const int minBinsForInterpolation = 25;
  if (n <= minBinsForInterpolation) info("Low number of scan points. Will use linear interpolation");

  // TODO
  RooRealVar* par = w->var(scanVar1);

  // Find and save a confidence interval for each solution within the scan range (plus boundary points)
  for (const auto [start, sBin] : starts) {
    if (arg->debug) {
      if (sBin == 1)
        debug("Start scan of low boundary");
      else if (sBin == n)
        debug("Start scan of up boundary");
    }

    for (int c = 0; c < ConfidenceLevels.size(); c++) {
      const double y = 1. - ConfidenceLevels[c];
      auto CLmin = std::numeric_limits<double>::quiet_NaN();
      auto CLmax = std::numeric_limits<double>::quiet_NaN();
      auto CLminErr = std::numeric_limits<double>::quiet_NaN();
      auto CLmaxErr = std::numeric_limits<double>::quiet_NaN();
      bool CLminClosed = false;
      bool CLmaxClosed = false;

      // Case that the solution is outside of scan region, or starting point does not fall into the CL region
      // (e.g. because we are doing a border scan or because the solution is a shallow local minimum)
      if (std::isnan(start) || histogramCL->GetBinContent(sBin) < y) {
        clintervals[c].push_back(nullptr);
        continue;
      }

      // Find lower interval bound
      for (int i = sBin; i > 0; i--) {
        if (i == 1) {
          CLmin = histogramCL->GetXaxis()->GetXmin();
          if (sBin != 1)
            warning(std::format(
                "I am using the lowest bin of histogramCL to calculate the lower end of the CL interval #{:d}.\n"
                "This will lead to wrong results - you need to decrease the minimum of the scan range",
                c));
        }
        if (histogramCL->GetBinContent(i) < y) {
          bool linearInterpolation = true;
          if (n > minBinsForInterpolation) {
            if (auto pair = interpolate(histogramCL, i, y, start, false)) {
              CLmin = pair->first;
              CLminErr = pair->second;
              CLminClosed = true;
              linearInterpolation = false;
            }
          }
          if (linearInterpolation) {
            if (n > minBinsForInterpolation && (arg->verbose || arg->debug)) info("Reverting to linear interpolation.");
            if (auto val = interpolateLinear(histogramCL, i, y)) {
              CLmin = *val;
              CLminErr =
                  histogramCL->GetBinWidth(i) / std::numbers::sqrt3 / 2.;  // Assume uniform distribution in the bin
              CLminClosed = true;
            } else
              warning(std::format("Could not interpolate for bin {:d}", i));
          }
          break;
        }
      }

      // Find upper interval bound
      for (int i = sBin; i <= n; i++) {
        if (i == n) {
          CLmax = histogramCL->GetXaxis()->GetXmax();
          if (sBin != n)
            warning(std::format(
                "I am using the highest bin of histogramCL to calculate the upper end of the CL interval #{:d}.\n"
                "This will lead to wrong results - you need to increase the maximum of the scan range",
                c));
        }
        if (histogramCL->GetBinContent(i) < y) {
          bool linearInterpolation = true;
          if (n > minBinsForInterpolation) {
            if (auto pair = interpolate(histogramCL, i - 1, y, start, true)) {
              CLmax = pair->first;
              CLmaxErr = pair->second;
              CLmaxClosed = true;
              linearInterpolation = false;
            }
          }
          if (linearInterpolation) {
            if (n > minBinsForInterpolation && (arg->verbose || arg->debug)) info("Reverting to linear interpolation.");
            if (auto val = interpolateLinear(histogramCL, i - 1, y)) {
              CLmax = *val;
              CLmaxErr =
                  histogramCL->GetBinWidth(i) / std::numbers::sqrt3 / 2.;  // Assume uniform distribution in the bin
              CLmaxClosed = true;
            } else
              warning(std::format("Could not interpolate for bin {:d}", i - 1));
          }
          break;
        }
      }

      // Save the interval
      auto cli = std::make_unique<CLInterval>();
      cli->pvalue = y;
      cli->central = (sBin != 1 && sBin != n) ? start : std::numeric_limits<double>::quiet_NaN();
      cli->min = CLmin;
      cli->max = CLmax;
      cli->minerr = CLminErr;
      cli->maxerr = CLmaxErr;
      cli->minclosed = CLminClosed;
      cli->maxclosed = CLmaxClosed;
      if (arg->debug) cli->print();
      if (!cli->checkPrecision(arg->errtol)) error(std::format("Precision of the CI #{:d} is too low", c));

      clintervals[c].push_back(std::move(cli));
    }
  }

  // Compute the cover of all 1sigma intervals
  // TODO this does not make much sense (especially for angles)
  if (arg->largest) {
    for (int k = 0; k < clintervals[0].size(); k++) {
      auto i = std::make_unique<CLInterval>();
      i->central = clintervals[0][k]->central;
      i->pvalue = clintervals[0][k]->pvalue;
      i->minmethod = "largest";
      i->maxmethod = "largest";
      i->min = clintervals[0][0]->min;
      i->max = clintervals[0][0]->max;
      i->minerr = clintervals[0][0]->minerr;
      i->maxerr = clintervals[0][0]->maxerr;
      i->minclosed = clintervals[0][0]->minclosed;
      i->maxclosed = clintervals[0][0]->maxclosed;
      for (const auto& cli : clintervals[0]) {
        if (!cli) continue;
        if (cli->min < i->min) {
          i->min = cli->min;
          i->minerr = cli->minerr;
          i->minclosed = cli->minclosed;
        }
        if (cli->max > i->max) {
          i->max = cli->max;
          i->maxerr = cli->maxerr;
          i->maxclosed = cli->maxclosed;
        }
      }
      clintervals[0].push_back(std::move(i));
    }
  }
  if (!quiet) printCLintervals(CLsType, calc_expected);

  // Print fit chi2 etc. (not done for datasets running)
  if (!combiner || !combiner->isCombined()) return;
  const auto chi2 = this->getSolution(0)->minNll();
  const auto nObs = combiner->getObservables()->getSize();
  const auto nPar = combiner->getParameters()->getSize();
  if (nObs == nPar) {
    if (std::abs(chi2) > 1e-3)
      error(std::format("Chi2 is not zero ({:.4f}), even if the number of degrees of freedom is zero\n", chi2));
    else
      std::cout
          << std::format(
                 "Fit quality is meaningless for zero degrees of freedom (chi2 = 0): (nObs, nPar) = ({:d}, {:d})\n",
                 nObs, nPar)
          << std::endl;
  } else {
    const auto prob = TMath::Prob(chi2, nObs - nPar);
    std::cout << std::format("Fit quality: chi2/(nObs-nPar) = {:.2f}/({:d}-{:d}), P = {:4.1f}%\n", chi2, nObs, nPar,
                             prob * 100.)
              << std::endl;
  }
}

///
/// Print CL intervals.
///
void MethodAbsScan::printCLintervals(int CLsType, bool calc_expected) {
  TString unit = w->var(scanVar1)->getUnit();
  CLIntervalPrinter clp(arg, name, scanVar1, unit, methodName, CLsType);
  if (calc_expected) {
    clp = CLIntervalPrinter(arg, name, scanVar1, unit, methodName + TString("_expected_standardCLs"));
  }
  clp.setDegrees(Utils::isAngle(w->var(scanVar1)));
  clp.addIntervals(clintervals);
  clp.print();
  clp.savePython();
  std::cout << std::endl;

  // print solutions not contained in the 1sigma and 2sigma intervals
  for (int i = 0; i < solutions.size(); i++) {
    double sol = getScanVar1Solution(i);
    bool contained = false;
    for (const auto& clis : clintervals) {
      for (const auto& cli : clis) {
        if (!cli) continue;
        if (cli->min < sol && sol < cli->max) contained = true;
      }
    }
    if (contained) continue;
    if (w->var(scanVar1)->getUnit() == TString("Rad")) sol = Utils::RadToDeg(sol);
    int d = arg->digits;
    if (d <= 0) d = 3;
    printf("%s = %7.*f", w->var(scanVar1)->GetName(), d, sol);
    if (unit != "") std::cout << " [" << unit << "]";
    std::cout << std::endl;
  }
}

///
/// Get the CL interval that includes the best-fit value.
/// \param sigma 1,2
///
CLInterval MethodAbsScan::getCLintervalCentral(int sigma, bool quiet) { return getCLinterval(0, sigma, quiet); }

/**
 * Get the CL interval that includes a given solution.
 *
 * @param iSol  Index of the interval (by default, solutions 0, 1, 2... followed by border scans).
 * @param index Index identifying the CL (by default, i = 0,1,2 correspond to 1,2,3 sigma).
 * @bool quiet  Sets whether the calculation of CL intervals (in case they have not been calculated yet) should be
 *              verbose or not.
 * @return      A copy of the desired CLInterval.
 */
CLInterval MethodAbsScan::getCLinterval(const int iSol, const int index, const bool quiet) {
  auto error = [](const std::string& msg) { Utils::errBase("MethodAbsScan::getCLinterval : ERROR : ", msg); };

  if (clintervals.empty()) calcCLintervals(0, false, quiet);
  if (clintervals.empty()) error("This should never happen");
  if (index < 0 || index >= clintervals.size() || iSol < 0 || iSol >= clintervals[index].size())
    error(
        std::format("There are no CL intervals with CL identified by index {:d} and solution identified by index {:d}",
                    index, iSol));

  // Compute largest interval.
  // TODO this does not make sense at all, in case there are multiple solutions
  if (arg->largest) {
    const auto& intervals = clintervals[index];

    CLInterval i;
    i.pvalue = intervals[iSol]->pvalue;
    i.min = intervals[iSol]->min;
    i.max = intervals[iSol]->max;
    for (const auto& cli : intervals) {
      i.min = std::min(i.min, cli->min);
      i.max = std::max(i.max, cli->max);
    }
    return i;
  }

  return CLInterval(*clintervals[index][iSol]);
}

void MethodAbsScan::plotOn(OneMinusClPlotAbs* plot, int CLsType) { plot->addScanner(this, CLsType); }

// Definitions of short getters
double MethodAbsScan::getCL(double val) const { return 1. - hCL->Interpolate(val); }
RooRealVar* MethodAbsScan::getScanVar1() { return w->var(scanVar1); }
RooRealVar* MethodAbsScan::getScanVar2() { return w->var(scanVar2); }
const RooRealVar* MethodAbsScan::getScanVar1() const { return w->var(scanVar1); }
const RooRealVar* MethodAbsScan::getScanVar2() const { return w->var(scanVar2); }
int MethodAbsScan::getNObservables() const { return w->set(obsName)->getSize(); }
const RooArgSet* MethodAbsScan::getObservables() const { return w->set(obsName); }
const RooArgSet* MethodAbsScan::getTheory() const { return w->set(thName); }

void MethodAbsScan::print() const {
  std::cout << std::format("MethodAbsScan::print() : Method: {:s}, Scanner: {:s}", std::string(methodName),
                           std::string(name))
            << std::endl;
  w->set(parsName)->Print("v");
}

///
/// Make a 1d plot of the NLL in var
///
void MethodAbsScan::plot1d(TString var) {
  std::cout << "MethodAbsScan::plot1d() : Method: " << methodName;
  std::cout << ", Scanner: " << name << std::endl;

  //   RooRealVar* vx = w->var(var);
  //   assert(vx);
  // Utils::setLimit(w, var, "plot");
  //
  //   // std::cout << "MethodAbsScan::plot1d() : loading global minimum ..." << std::endl;
  //   // if ( !globalMin ){ std::cout << "MethodAbsScan::plot1d() : no global minimum. Call doInitialFit() first!" <<
  //   std::endl; std::exit(1); }
  //   // Utils::setParameters(w, parsName, globalMinP);
  //   // print();
  //
  //   RooNLLVar nll("nll", "nll", *(w->pdf(pdfName)), *(w->data(dataName))) ;
  //
  //   TString plotName = "plot1d_"+name+"_"+var;
  //   TCanvas* c1 = Utils::newNoWarnTCanvas();
  //   RooPlot *frame = vx->frame();
  //   // w->pdf(pdfName)->plotOn(frame);
  //   nll.plotOn(frame);
  //   frame->Draw();
  //
  //   Utils::savePlot(c1, plotName);
}

///
/// Make a 2d plot of the PDF in varx and vary.
///
void MethodAbsScan::plot2d(TString varx, TString vary) {
  std::cout << "MethodAbsScan::plot2d() : Method: " << methodName;
  std::cout << ", scanner: " << name << std::endl;

  RooRealVar* vx = w->var(varx);
  RooRealVar* vy = w->var(vary);
  assert(vx);
  assert(vy);
  Utils::setLimit(w, varx, "plot");
  Utils::setLimit(w, vary, "plot");

  std::cout << "MethodAbsScan::plot2d() : loading global minimum ..." << std::endl;
  if (!globalMin) {
    std::cout << "MethodAbsScan::plot2d() : no global minimum. Call doInitialFit() first!" << std::endl;
    std::exit(1);
  }

  Utils::setParameters(w, parsName, globalMin);
  print();

  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.14);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  gStyle->SetPalette(1);

  TString plotName = "plot2d_" + name + "_" + varx + "_" + vary;
  TCanvas* c1 = Utils::newNoWarnTCanvas(plotName, plotName);
  TH1* h = w->pdf(pdfName)->createHistogram(plotName, *vx, RooFit::YVar(*vy));
  h->Draw("colz");

  Utils::savePlot(c1, plotName + arg->plotext);
}

///
/// Load the values at a specific minimum into
/// the workspace. This way we can use it for
/// goodness of fit, start points, etc.
/// \param i Index of the solution, i=0 corresponds to the best one.
///
bool MethodAbsScan::loadSolution(int i) {
  if (arg->debug) std::cout << "MethodAbsScan::loadSolution() : loading solution " << i << std::endl;
  if (i < 0 || i >= solutions.size()) {
    std::cout << "MethodAbsScan::loadSolution() : ERROR : solution ID out of range." << std::endl;
    return false;
  }
  RooArgSet* tmp = new RooArgSet();
  tmp->add(solutions[i]->floatParsFinal());
  tmp->add(solutions[i]->constPars());
  Utils::setParameters(w, parsName, tmp);
  delete tmp;
  return true;
}

///
/// Load the values given by an (external) fit result.
///
void MethodAbsScan::loadParameters(RooSlimFitResult* r) {
  if (arg->debug) std::cout << "MethodAbsScan::loadParameters() : loading a RooSlimFitResult " << std::endl;
  RooArgSet* tmp = new RooArgSet();
  tmp->add(r->floatParsFinal());
  tmp->add(r->constPars());
  Utils::setParameters(w, parsName, tmp);
  delete tmp;
}

///
/// Print local minima solutions.
///
void MethodAbsScan::printLocalMinima() const {
  if (arg->debug) std::cout << "MethodAbsScan::printLocalMinima() : LOCAL MINIMA for " << title << "\n" << std::endl;
  TDatime date;
  for (int i = 0; i < solutions.size(); i++) {
    std::cout << "SOLUTION " << i << ":\n" << std::endl;
    std::cout << "  combination: " << name << std::endl;
    std::cout << "  title:       " << title << std::endl;
    std::cout << "  date:        " << date.AsString() << std::endl;
    solutions[i]->Print(arg->verbose, arg->printcor);
  }
}

///
/// Save local minima solutions.
///
void MethodAbsScan::saveLocalMinima(TString fName) const {
  if (arg->debug) std::cout << "MethodAbsScan::saveLocalMinima() : LOCAL MINIMA for " << title << "\n" << std::endl;
  TDatime date;
  std::ofstream outfile;
  outfile.open(fName.Data());

  for (int i = 0; i < solutions.size(); i++) {
    outfile << "\%SOLUTION " << i << ":\n" << std::endl;
    outfile << "\%  combination: " << name << std::endl;
    outfile << "\%  title:       " << title << std::endl;
    outfile << "\%  date:        " << date.AsString() << std::endl;
    solutions[i]->SaveLatex(outfile, arg->verbose, arg->printcor);
  }
  outfile.close();
}

///
/// Get value of scan parameter at a certain solution.
/// \param iVar - Index of scan variable, 1 or 2.
/// \param iSol - Index of solution. 0 corresponds to the best one,
///               indices increase in order of chi2.
/// \return central value of the solution
/// \return -999 no solutions available
/// \return -99 solution not found
/// \return -9999 no such variable
///
double MethodAbsScan::getScanVarSolution(int iVar, int iSol) const {
  auto error = [](const std::string& msg, const double val) {
    Utils::errBase("MethodAbsScan::getScanVarSolution() : ERROR : ", msg, false);
    return val;
  };

  if (solutions.empty()) error("There are no solutions", -999);
  if (iSol >= solutions.size()) error(std::format("No solution with id {:d}", iSol), -99.);
  const RooSlimFitResult* r = getSolution(iSol);
  assert(r);
  TString varName;
  if (iVar == 1)
    varName = getScanVar1Name();
  else if (iVar == 2)
    varName = getScanVar2Name();
  else
    error(std::format("No such variable {:d}", iVar), -9999.);

  if (r->isConfirmed()) {
    return r->getFloatParFinalVal(varName);
  } else {
    if (nWarnings == 0)
      std::cout << "MethodAbsScan::getScanVarSolution() : WARNING : Using unconfirmed solution." << std::endl;
    ++nWarnings;
    return r->getConstParVal(varName);
  }
}

///
/// Get value of scan parameter 1 a certain solution.
/// \param iSol Index of solution. 0 corresponds to the best one,
/// indices increase in order of chi2.
///
double MethodAbsScan::getScanVar1Solution(int iSol) const { return getScanVarSolution(1, iSol); }

///
/// Get value of scan parameter 2 a certain solution
/// (only meaningful for 2d scan).
/// \param iSol Index of solution. 0 corresponds to the best one,
/// indices increase in order of chi2.
///
double MethodAbsScan::getScanVar2Solution(int iSol) const { return getScanVarSolution(2, iSol); }

///
/// Sort solutions in order of increasing chi2.
///
void MethodAbsScan::sortSolutions() {
  if (arg->debug) std::cout << "MethodAbsScan::sortSolutions() : sorting solutions ..." << std::endl;
  std::vector<RooSlimFitResult*> solutionsUnSorted = solutions;
  std::vector<RooSlimFitResult*> tmp;
  solutions = tmp;
  int nSolutions = solutionsUnSorted.size();
  for (int i = 0; i < nSolutions; i++) {
    double min = solutionsUnSorted[0]->minNll();
    int iMin = 0;
    for (int i = 0; i < solutionsUnSorted.size(); i++) {
      if (solutionsUnSorted[i]->minNll() < min) {
        min = solutionsUnSorted[i]->minNll();
        iMin = i;
      }
    }
    solutions.push_back(solutionsUnSorted[iMin]);
    solutionsUnSorted.erase(solutionsUnSorted.begin() + iMin);
  }
  if (arg->debug) std::cout << "MethodAbsScan::sortSolutions() : solutions sorted: " << solutions.size() << std::endl;
}

///
/// Refit all possible solutions with the scan parameter left
/// free to confirm the solutions. We will reject solutions as
/// fake if the free fit using them as the starting point will
/// move too far away. Or, if their Delta chi2 value is above 25.
///
void MethodAbsScan::confirmSolutions() {
  if (arg->debug) std::cout << "MethodAbsScan::confirmSolutions() : Confirming solutions ..." << std::endl;
  FitResultCache frCache(arg);
  frCache.storeParsAtFunctionCall(w->set(parsName));

  std::vector<RooSlimFitResult*> confirmedSolutions;
  RooRealVar* par1 = w->var(scanVar1);
  RooRealVar* par2 = w->var(scanVar2);
  if (par1) par1->setConstant(false);
  if (par2) par2->setConstant(false);
  for (int i = 0; i < solutions.size(); i++) {
    bool ok = loadSolution(i);
    if (!ok) continue;
    if (arg->debug) {
      std::cout << "MethodAbsScan::confirmSolutions() : solution " << i;
      std::cout << " " << par1->GetName() << "=" << par1->getVal();
      if (par2) std::cout << " " << par2->GetName() << "=" << par2->getVal();
      std::cout << std::endl;
    }

    // refit the solution
    // true uses thorough fit with HESSE, -1 silences output
    RooFitResult* r = Utils::fitToMinBringBackAngles(w->pdf(pdfName), true, -1);

    // Check scan parameter shift.
    // We'll allow for a shift equivalent to 3 step sizes.
    // Express the scan step size in terms of sigmas of the fitted parameters.
    double allowedSigma;
    if (arg->var.size() == 1) {
      // 1d scan
      double par1stepsize = (par1->getMax("scan") - par1->getMin("scan")) / arg->npoints1d;
      RooRealVar* par1New = (RooRealVar*)r->floatParsFinal().find(par1->GetName());
      double par1stepsizeInSigma = par1New->getError() > 0 ? par1stepsize / par1New->getError() : 0.2;
      allowedSigma = 3. * par1stepsizeInSigma;
    } else if (arg->var.size() == 2) {
      // 2d scan
      double par1stepsize = (par1->getMax("scan") - par1->getMin("scan")) / arg->npoints2dx;
      double par2stepsize = (par2->getMax("scan") - par2->getMin("scan")) / arg->npoints2dy;
      RooRealVar* par1New = (RooRealVar*)r->floatParsFinal().find(par1->GetName());
      RooRealVar* par2New = (RooRealVar*)r->floatParsFinal().find(par2->GetName());
      double par1stepsizeInSigma = par1New->getError() > 0 ? par1stepsize / par1New->getError() : 1.;
      double par2stepsizeInSigma = par2New->getError() > 0 ? par2stepsize / par2New->getError() : 1.;
      allowedSigma = TMath::Max(3. * par1stepsizeInSigma, 3. * par2stepsizeInSigma);
    }

    // Warn if a parameter is close to its limit
    for (const auto& pAbs : r->floatParsFinal()) {
      const auto p = static_cast<RooRealVar*>(pAbs);
      if (p->getMax() - p->getVal() < p->getError() || p->getVal() - p->getMin() < p->getError()) {
        std::cout << "\nMethodAbsScan::confirmSolutions() : WARNING : " << p->GetName() << " is close to its limit!"
                  << std::endl;
        std::cout << "                                  : ";
        p->Print();
        std::cout << std::endl;
      }
    }

    // check migration of the parameters
    RooArgList listOld = solutions[i]->floatParsFinal();
    listOld.add(solutions[i]->constPars());
    RooArgList listNew = r->floatParsFinal();
    listNew.add(r->constPars());
    bool isConfirmed = true;
    TString rejectReason = "";
    for (const auto& p : *w->set(parsName)) {
      RooRealVar* pOld = (RooRealVar*)listOld.find(p->GetName());
      RooRealVar* pNew = (RooRealVar*)listNew.find(p->GetName());
      if (!pOld && !pNew) {
        std::cout << "MethodAbsScan::confirmSolutions() : ERROR : parameter not found: " << p->GetName() << std::endl;
        continue;
      }
      if (pNew->getError() > 0) {
        double shift = fabs(pOld->getVal() - pNew->getVal());
        if (Utils::isAngle(pOld)) shift = Utils::angularDifference(pOld->getVal(), pNew->getVal());
        if (shift / pNew->getError() > allowedSigma) {
          if (arg->debug) {
            std::cout << "MethodAbsScan::confirmSolutions() : solution " << i
                      << ", too large parameter shift:" << std::endl;
            pOld->Print();
            pNew->Print();
          }
          isConfirmed = false;
          rejectReason = TString("too large shift in ") + pNew->GetName();
        }
      }
    }
    if (r->minNll() - chi2minGlobal > 25) {
      std::cout << "MethodAbsScan::confirmSolutions() : WARNING : local minimum has DeltaChi2>25." << std::endl;
      isConfirmed = false;
      rejectReason =
          Form("too large chi2: DeltaChi2>25 - chi2minGlobal: %e and confirmed NLL: %e", chi2minGlobal, r->minNll());
    }
    if (isConfirmed) {
      if (arg->debug) std::cout << "MethodAbsScan::confirmSolutions() : solution " << i << " accepted." << std::endl;
      RooSlimFitResult* sr = new RooSlimFitResult(r, true);  // true saves correlation matrix
      sr->setConfirmed(true);
      confirmedSolutions.push_back(sr);
      delete r;
    } else {
      std::cout << "MethodAbsScan::confirmSolutions() : WARNING : solution " << i
                << " rejected "
                   "("
                << rejectReason << ")" << std::endl;
    }
  }
  // do NOT delete the old solutions! They are still in allResults and curveResults.
  solutions = confirmedSolutions;
  sortSolutions();
  if (arg->debug) printLocalMinima();
  removeDuplicateSolutions();
  // reset parameters
  Utils::setParameters(w, parsName, frCache.getParsAtFunctionCall());
}

///
/// Remove duplicate solutions from the common solutions storage
/// ('solutions' vector). Duplicate solutions can occur when two
/// unconfirmed solutions converge to the same true local minimum
/// when refitted by confirmSolutions().
///
/// No solutions will be removed if --qh 9 is given.
/// \todo upgrade the quickhack to a proper option
///
void MethodAbsScan::removeDuplicateSolutions() {
  if (arg->isQuickhack(9)) return;
  std::vector<RooSlimFitResult*> solutionsNoDup;
  for (int i = 0; i < solutions.size(); i++) {
    bool found = false;
    for (int j = i + 1; j < solutions.size(); j++) {
      if (compareSolutions(solutions[i], solutions[j])) found = true;
      if (found == true) continue;
    }
    if (!found)
      solutionsNoDup.push_back(solutions[i]);
    else {
      if (arg->debug)
        std::cout << "MethodAbsScan::removeDuplicateSolutions() : removing duplicate solution " << i << std::endl;
    }
  }
  if (solutions.size() != solutionsNoDup.size()) {
    std::cout << std::endl;
    if (arg->debug) std::cout << "MethodAbsScan::removeDuplicateSolutions() : ";
    std::cout << "INFO : some equivalent solutions were removed. In case of 2D scans" << std::endl;
    std::cout << "       many equivalent solutions may lay on a contour of constant chi2, in" << std::endl;
    std::cout << "       that case removing them is perhaps not desired. You can keep all solutions" << std::endl;
    std::cout << "       using --qh 9\n" << std::endl;
  }
  solutions = solutionsNoDup;
}

///
/// Compare two solutions.
/// \param r1 First solution
/// \param r2 Second solution
/// \return true, if both are equal inside a certain margin
///
bool MethodAbsScan::compareSolutions(RooSlimFitResult* r1, RooSlimFitResult* r2) {
  // compare chi2
  if (fabs(r1->minNll() - r2->minNll()) > 0.05) return false;
  // construct parameter lists
  RooArgList list1 = r1->floatParsFinal();
  list1.add(r1->constPars());
  RooArgList list2 = r2->floatParsFinal();
  list2.add(r2->constPars());
  // compare each parameter
  for (const auto& p : *w->set(parsName)) {
    RooRealVar* p1 = (RooRealVar*)list1.find(p->GetName());
    RooRealVar* p2 = (RooRealVar*)list2.find(p->GetName());
    if (!p1 && !p2) {
      std::cout << "MethodAbsScan::compareSolutions() : ERROR : parameter not found: " << p->GetName() << std::endl;
      continue;
    }
    // We accept two parameters to be equal if they agree within 0.1 sigma.
    using Utils::sq;
    double sigma1 = p1->getError() > 0 ? p1->getError() : p1->getVal() / 10.;
    double sigma2 = p2->getError() > 0 ? p2->getError() : p2->getVal() / 10.;
    if (fabs(p1->getVal() - p2->getVal()) / (sqrt(sq(sigma1) + sq(sigma2))) > 0.1) return false;
  }
  return true;
}

///
/// Return a solution corresponding to a minimum of the profile
/// likelihoood.
/// \param i Index of the solution, they are orderd after increasing chi2,
///         i=0 is that with the smallest chi2.
///
const RooSlimFitResult* MethodAbsScan::getSolution(int i) const {
  if (i >= solutions.size()) {
    std::cerr << std::format("MethodAbsScan::getSolution() : ERROR : No solution with id {:d}", i) << std::endl;
    return nullptr;
  }
  return solutions[i];
}

RooSlimFitResult* MethodAbsScan::getSolution(int i) {
  return const_cast<RooSlimFitResult*>(static_cast<const MethodAbsScan*>(this)->getSolution(i));
}

///
/// Helper function to copy over solutions from another
/// scanner. Clears the solutions vector and sets the one
/// given.
///
void MethodAbsScan::setSolutions(std::vector<RooSlimFitResult*> s) {
  solutions.clear();
  for (int i = 0; i < s.size(); i++) { solutions.push_back(s[i]); }
}

///
/// Make a pull plot of observables corresponding
/// to the given solution.
///
void MethodAbsScan::plotPulls(int nSolution) {
  PullPlotter p(this);
  p.loadParsFromSolution(nSolution);
  p.savePulls();
  p.plotPulls();
}

void MethodAbsScan::setXscanRange(double min, double max) {
  if (min == max) return;
  RooRealVar* par1 = w->var(scanVar1);
  assert(par1);
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  par1->setRange("scan", min, max);
  RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
  if (arg->debug)
    std::cout << "DEBUG in MethodAbsScan::setXscanRange(): setting range for " << scanVar1 << ": " << min << ": " << max
              << std::endl;
  m_xrangeset = true;
}

void MethodAbsScan::setYscanRange(double min, double max) {
  if (min == max) return;
  RooRealVar* par2 = w->var(scanVar2);
  assert(par2);
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  par2->setRange("scan", min, max);
  RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
  m_yrangeset = true;
}

void MethodAbsScan::calcCLintervalsSimple(int CLsType, bool calc_expected) {
  const int nc = 3;
  clintervals.clear();
  clintervals.resize(nc);

  TH1F* histogramCL = this->hCL;
  if (this->hCLs && CLsType == 1) {
    histogramCL = this->hCLs;
  } else if (this->hCLsFreq && CLsType == 2) {
    histogramCL = this->hCLsFreq;
  }
  if (CLsType == 2 && calc_expected && hCLsExp) { histogramCL = hCLsExp; }
  if (CLsType == 0 || (this->hCLs && CLsType == 1) || (this->hCLsFreq && CLsType == 2)) {
    for (int c = 0; c < nc; c++) {
      const std::pair<double, double> borders = getBorders(TGraph(histogramCL), ConfidenceLevels[c]);
      auto cli = std::make_unique<CLInterval>();
      cli->pvalue = 1. - ConfidenceLevels[c];
      cli->min = borders.first;
      cli->max = borders.second;
      cli->central = -1;
      clintervals[c].push_back(std::move(cli));
      if (CLsType == 1) std::cout << "Simplified CL_s ";
      if (CLsType == 2) std::cout << "Standard CL_s";
      std::cout << "borders at " << ConfidenceLevels[c] << "    [ " << borders.first << " : " << borders.second << "]";
      std::cout << ", " << methodName << " (simple boundary scan)" << std::endl;
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////
  //// Add a hacky calculation of the CL_s intervals
  //// \todo: Do it properly from the very start by introducing a bkg model and propagate it to the entire framework.
  /// TODO: I think this can now disappear

  if ((!this->hCLs && CLsType == 1) || (!this->hCLsFreq && CLsType == 2)) {
    std::cout << "*****************************************************************************************************"
                 "*********************************"
              << std::endl;
    std::cout << "WARNING: hCLs is empty! Will calculate CLs intervals by normalising the p values to the p value of "
                 "the first bin."
              << std::endl;
    std::cout << "WARNING: This is only an approximate solution and MIGHT EVEN BE WRONG, if the first bin does not "
                 "represent the background expectation!"
              << std::endl;
    std::cout << "*****************************************************************************************************"
                 "*********************************"
              << std::endl;
    const int nc = 3;
    clintervals.clear();
    clintervals.resize(nc);

    for (int c = 0; c < nc; c++) {
      const std::pair<double, double> borders = getBorders_CLs(TGraph(histogramCL), ConfidenceLevels[c]);
      auto cli = std::make_unique<CLInterval>();
      cli->pvalue = 1. - ConfidenceLevels[c];
      cli->min = borders.first;
      cli->max = borders.second;
      cli->central = -1.;
      clintervals[c].push_back(std::move(cli));
      std::cout << std::format("CL_s borders at {:f}  [{:f} : {:f}], {:s} (simple boundary scan)", ConfidenceLevels[c],
                               borders.first, borders.second, std::string(methodName))
                << std::endl;
    }
  }
}

/*!
\brief determines the borders of the confidence interval by linear or qubic interpolation.
\param graph The graph holding the p-values.
\param confidence_level The confidence level at which the interval is to be determined.
\param qubic Optional parameter. False by default. If true, qubic interpolation is used.
*/
std::pair<double, double> MethodAbsScan::getBorders(const TGraph& graph, const double confidence_level,
                                                    bool qubic) const {

  const double p_val = 1 - confidence_level;
  TSpline* splines = nullptr;
  if (qubic) splines = new TSpline3();

  // will never return lower (higher) edge than min_edge (max_edge)
  double min_edge = graph.GetX()[0];
  double max_edge = graph.GetX()[graph.GetN() - 1];
  const int scan_steps = 1000;
  double lower_edge = min_edge;
  double upper_edge = max_edge;

  for (double point = min_edge; point < max_edge; point += (max_edge - min_edge) / scan_steps) {

    if (graph.Eval(point, splines) > p_val) {
      lower_edge = point;
      break;
    }
  }
  for (double point = max_edge; point > min_edge; point -= (max_edge - min_edge) / scan_steps) {
    if (graph.Eval(point, splines) > p_val) {
      upper_edge = point;
      break;
    }
  }
  return std::pair<double, double>(lower_edge, upper_edge);
}

////////////////////////////////////////////////////////////////////////////////////////////
//// Do a hacky calculation of the CL_s intervals, where essentially the pValue is normalized to the pValue with
/// n_sig=0. / Let's first assume that the parameter of interest is ALWAYS a parameter correlated with n_sig, so that
/// parameter=0 means n_sig=0. / Therefore the pValue(CL_s) is given by the ratio of the pValue at scanpointand the
/// pValue of the lowest bin. / \todo Do it properly from the very start by introducing a bkg model and propagate it to
/// the entire framework.

std::pair<double, double> MethodAbsScan::getBorders_CLs(const TGraph& graph, const double confidence_level,
                                                        bool qubic) const {

  const double p_val = 1 - confidence_level;
  TSpline* splines = nullptr;
  if (qubic) splines = new TSpline3();

  double min_edge = graph.GetX()[0];
  // will never return smaller edge than min_edge
  double max_edge = graph.GetX()[graph.GetN() - 1];
  // will never return higher edge than max_edge
  int scan_steps = 1000;
  double lower_edge = min_edge;
  double upper_edge = max_edge;

  for (double point = min_edge; point < max_edge; point += (max_edge - min_edge) / scan_steps) {

    // for CL_s normalize pVal to the pVal at 0 (which has to be the background model)
    if (graph.Eval(point, splines) / graph.Eval(min_edge, splines) > p_val) {
      lower_edge = point;
      break;
    }
  }
  for (double point = max_edge; point > min_edge; point -= (max_edge - min_edge) / scan_steps) {

    // for CL_s normalize pVal to the pVal at 0 (which has to be the background model)
    if (graph.Eval(point, splines) / graph.Eval(min_edge, splines) > p_val) {
      upper_edge = point;
      break;
    }
  }
  return std::pair<double, double>(lower_edge, upper_edge);
}

bool MethodAbsScan::checkCLs() {
  if (!hCLsExp || !hCLsErr1Up || !hCLsErr1Dn || !hCLsErr2Up || !hCLsErr2Dn) {
    std::cout << "ERROR: ***************************************************" << std::endl;
    std::cout << "ERROR: MethodAbsScan::checkCLs() : No CLs plot available!!" << std::endl;
    std::cout << "ERROR: ***************************************************" << std::endl;
    return false;
  }
  assert(hCLsExp->GetNbinsX() == hCLsErr1Up->GetNbinsX());
  assert(hCLsExp->GetNbinsX() == hCLsErr2Up->GetNbinsX());
  assert(hCLsExp->GetNbinsX() == hCLsErr1Dn->GetNbinsX());
  assert(hCLsExp->GetNbinsX() == hCLsErr2Dn->GetNbinsX());

  // correct for low stats in the lower error
  for (int i = 1; i <= hCLsExp->GetNbinsX(); i++) {
    if (hCLsErr1Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i)) {
      hCLsErr1Dn->SetBinContent(i, hCLsExp->GetBinContent(i) -
                                       (hCLsErr1Up->GetBinContent(i) - hCLsErr1Dn->GetBinContent(i)) / 2.);
    }
    if (hCLsErr1Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i)) {
      hCLsErr1Dn->SetBinContent(i,
                                hCLsExp->GetBinContent(i) - (hCLsErr1Up->GetBinContent(i) - hCLsExp->GetBinContent(i)));
    }
    if (((hCLsExp->GetBinContent(i) - hCLsErr1Dn->GetBinContent(i)) / hCLsExp->GetBinContent(i)) < 0.05) {
      hCLsErr1Dn->SetBinContent(i, hCLsExp->GetBinContent(i) -
                                       (hCLsErr1Up->GetBinContent(i) - hCLsErr1Dn->GetBinContent(i)) / 2.);
    }
    if (hCLsErr2Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i)) {
      hCLsErr2Dn->SetBinContent(i, hCLsExp->GetBinContent(i) -
                                       (hCLsErr2Up->GetBinContent(i) - hCLsErr2Dn->GetBinContent(i)) / 2.);
    }
    if (hCLsErr2Dn->GetBinContent(i) >= hCLsErr1Dn->GetBinContent(i)) {
      hCLsErr2Dn->SetBinContent(i, hCLsExp->GetBinContent(i) -
                                       (hCLsExp->GetBinContent(i) - hCLsErr1Dn->GetBinContent(i)) * 2.);
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//// end of CL_s part
////////////////////////////////////////////////////////////////////////////////////////////////////
