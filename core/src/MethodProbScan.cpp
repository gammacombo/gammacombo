/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#include <MethodProbScan.h>

#include <Combiner.h>
#include <OptParser.h>
#include <PValueCorrection.h>
#include <RooSlimFitResult.h>
#include <Utils.h>

#include <RooAbsPdf.h>
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooFitResult.h>
#include <RooFormulaVar.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMarker.h>
#include <TMath.h>
#include <TStopwatch.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
#include <iostream>
#include <limits>
#include <memory>

namespace {
  // TODO this may stay here, or go to Utils
  int getNFloatingParameters(const RooAbsPdf* pdf) {
    auto allPars = std::unique_ptr<RooArgSet>(pdf->getVariables());
    // Cast required for ROOT versions < 6.34
    auto floatPars = std::unique_ptr<RooArgSet>(static_cast<RooArgSet*>(allPars->selectByAttrib("Constant", false)));
    return floatPars->size();
  }
}  // namespace

MethodProbScan::MethodProbScan(Combiner* comb) : MethodAbsScan(comb) { methodName = "Prob"; }

MethodProbScan::MethodProbScan(const OptParser* opt) : MethodAbsScan(opt) { methodName = "Prob"; }

///
/// Default constructor
///
MethodProbScan::MethodProbScan() { methodName = "Prob"; }

///
/// Perform 1d Prob scan.
///
/// - Scan range defined through limit "scan".
/// - Will fill the hCL histogram with the 1-CL curve.
/// - Start at a scan value that is in the middle of the allowed
///   range, preferably a solution, and scan up and down from there.
/// - use the "probforce" command line flag to enable force minimum finding
///
/// \param fast This will scan each scanpoint only once.
/// \param reverse This will scan in reverse direction.
///   When using the drag mode, this can sometimes make a difference.
/// \return status: 2 = new global minimum found, 1 = error
///
int MethodProbScan::scan1d(bool fast, bool reverse, bool quiet) {
  if (arg->debug) std::cout << "MethodProbScan::scan1d() : starting ... " << std::endl;
  nScansDone++;

  // The "improve" method doesn't need multiple scans.
  if (arg->probforce || arg->probimprove) fast = true;
  if (arg->probforce) scanDisableDragMode = true;

  // Save parameter values that were active at function call.
  if (startPars) delete startPars;
  startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
  startPars->add(*w->set(parsName));

  // // start scan from global minimum (not always a good idea as we need to set from other places as well)
  // Utils::setParameters(w, parsName, globalMin);

  // load scan parameter and scan range
  Utils::setLimit(w, scanVar1, "scan");
  RooRealVar* par = w->var(scanVar1);
  assert(par);
  double min = hCL->GetXaxis()->GetXmin();
  double max = hCL->GetXaxis()->GetXmax();
  if (fabs(par->getMin() - min) > 1e-6 || fabs(par->getMax() - max) > 1e-6) {
    std::cout << "MethodProbScan::scan1d() : WARNING : Scan range was changed after initScan()" << std::endl;
    std::cout << "                           was called so the old range will be used." << std::endl;
  }
  if (arg->verbose) {
    std::cout << "\nProb configuration:" << std::endl;
    std::cout << "  combination : " << title << std::endl;
    std::cout << "  scan variable : " << scanVar1 << std::endl;
    std::cout << "  scan range : " << min << " ... " << max << std::endl;
    std::cout << "  scan steps : " << nPoints1d << std::endl;
    std::cout << "  fast mode : " << fast << std::endl;
    std::cout << std::endl;
  }

  // Set limit to all parameters.
  combiner->loadParameterLimits();

  // fix scan parameter
  par->setConstant(true);

  // Check if there is at least one parameter to be fit
  auto likelihood = w->pdf(pdfName);
  const bool hasFreePars = getNFloatingParameters(likelihood) > 0;
  if (!hasFreePars)
    std::cout << "MethodProbScan::scan1d() : INFO : There are no free parameters. I will scan without fitting"
              << std::endl;

  RooFormulaVar nll("nll", "nll", "-2*log(@0)", RooArgSet(*likelihood));

  // j =
  // 0 : start value -> upper limit
  // 1 : upper limit -> start value
  // 2 : start value -> lower limit
  // 3 : lower limit -> start value
  double startValue = par->getVal();
  bool scanUp;

  // for the status bar
  double nTotalSteps = nPoints1d;
  nTotalSteps *= fast ? 1 : 2;
  double nStep = 0;
  double printFreq = nTotalSteps > 15 ? 10 : nTotalSteps;

  // Report on the smallest new minimum we come across while scanning.
  // Sometimes the scan doesn't find the minimum
  // that was found before. Warn if this happens.
  double bestMinOld = chi2minGlobal;
  auto bestMinFoundInScan = std::numeric_limits<double>::max();

  for (int jj = 0; jj < 4; jj++) {
    int j = jj;
    if (reverse) switch (jj) {
      case 0:
        j = 2;
        break;
      case 1:
        j = 3;
        break;
      case 2:
        j = 0;
        break;
      case 3:
        j = 1;
        break;
      }

    double scanStart, scanStop;
    switch (j) {
    case 0:
      // UP
      Utils::setParameters(w, parsName, startPars->get(0));
      scanStart = startValue;
      scanStop = par->getMax();
      scanUp = true;
      break;
    case 1:
      // DOWN
      scanStart = par->getMax();
      scanStop = startValue;
      scanUp = false;
      break;
    case 2:
      // DOWN
      Utils::setParameters(w, parsName, startPars->get(0));
      scanStart = startValue;
      scanStop = par->getMin();
      scanUp = false;
      break;
    case 3:
      // UP
      scanStart = par->getMin();
      scanStop = startValue;
      scanUp = true;
      break;
    }

    if (fast && (j == 1 || j == 3)) continue;

    for (int i = 0; i < nPoints1d; i++) {
      double scanvalue;
      if (scanUp) {
        scanvalue = min + (max - min) * (double)i / (double)nPoints1d + hCL->GetBinWidth(1) / 2.;
        if (scanvalue < scanStart) continue;
        if (scanvalue > scanStop) break;
      } else {
        scanvalue = max - (max - min) * (double)(i + 1) / (double)nPoints1d + hCL->GetBinWidth(1) / 2.;
        if (scanvalue > scanStart) continue;
        if (scanvalue < scanStop) break;
      }

      // disable drag mode
      // (the improve method doesn't work with drag mode as parameter run
      // at their limits)
      if (scanDisableDragMode) Utils::setParameters(w, parsName, startPars->get(0));

      // set the parameter of interest to the scan point
      par->setVal(scanvalue);

      // don't scan in unphysical region
      if (scanvalue < par->getMin() || scanvalue > par->getMax()) continue;

      // status bar
      if ((((int)nStep % (int)(nTotalSteps / printFreq)) == 0))
        if (!quiet)
          std::cout << "MethodProbScan::scan1d() : scanning " << (double)nStep / (double)nTotalSteps * 100. << "%   \r"
                    << std::flush;

      RooSlimFitResult* sfr = nullptr;
      auto chi2minScan = std::numeric_limits<double>::max();
      bool performFit = hasFreePars;
      if (!hasFreePars) {
        // There are no parameters to fit, just calculate NLL
        chi2minScan = nll.getVal();
        // But if we found indications of a new global minimum, perform the fit to find it!
        if (chi2minScan < chi2minGlobal) performFit = true;
      }
      if (performFit) {
        if (!hasFreePars) par->setConstant(false);
        std::unique_ptr<RooFitResult> fr;
        if (arg->probforce)
          fr = std::unique_ptr<RooFitResult>(Utils::fitToMinForce(w, combiner->getPdfName()));
        else if (arg->probimprove)
          fr = std::unique_ptr<RooFitResult>(Utils::fitToMinImprove(w, combiner->getPdfName()));
        else
          fr = std::unique_ptr<RooFitResult>(Utils::fitToMinBringBackAngles(likelihood, false, -1));
        chi2minScan = fr->minNll();
        if (std::isinf(chi2minScan))
          chi2minScan = std::numeric_limits<double>::max();    // else the toys in PDF_testConstraint don't work
        allResults.push_back(new RooSlimFitResult(fr.get()));  // save memory by using the slim fit result
        sfr = allResults.back();
        if (!hasFreePars) par->setConstant(true);
      }
      if (chi2minScan < 0) {
        TString warningChi2Neg;
        double newChi2minScan = chi2minGlobal + 25.;  // 5sigma more than best point
        warningChi2Neg = "MethodProbScan::scan1d() : WARNING : " + title;
        warningChi2Neg += TString(Form(" chi2 negative for scan point %i: %f", i, chi2minScan));
        warningChi2Neg += " setting to: " + TString(Form("%f", newChi2minScan));
        std::cout << warningChi2Neg << std::endl;
        chi2minScan = newChi2minScan;
      }
      bestMinFoundInScan = std::min(chi2minScan, bestMinFoundInScan);

      // If we find a minimum smaller than the old "global" minimum, this means that all
      // previous 1-CL values are too high.
      if (chi2minScan < chi2minGlobal) {
        if (arg->verbose)
          std::cout << "MethodProbScan::scan1d() : WARNING : '" << title << "' new global minimum found! "
                    << " chi2minScan=" << chi2minScan << std::endl;
        chi2minGlobal = chi2minScan;
        // recompute previous 1-CL values
        for (int k = 1; k <= hCL->GetNbinsX(); k++) {
          hCL->SetBinContent(k, TMath::Prob(hChi2min->GetBinContent(k) - chi2minGlobal, 1));
        }
      }

      double deltaChi2 = chi2minScan - chi2minGlobal;
      double oneMinusCL = TMath::Prob(deltaChi2, 1);
      double deltaChi2Bkg = TMath::Max(chi2minScan - hChi2min->GetBinContent(1), 0.0);
      if (i == 0) deltaChi2Bkg = 0.0;
      double oneMinusCLBkg = TMath::Prob(deltaChi2Bkg, 1);
      hCLs->SetBinContent(hCLs->FindBin(scanvalue), oneMinusCLBkg);

      if (i == 0) chi2minBkg = chi2minScan;

      // Save the 1-CL value and the corresponding fit result.
      // But only if better than before!
      if (hCL->GetBinContent(hCL->FindBin(scanvalue)) <= oneMinusCL) {
        hCL->SetBinContent(hCL->FindBin(scanvalue), oneMinusCL);
        hChi2min->SetBinContent(hCL->FindBin(scanvalue), chi2minScan);
        int iRes = hCL->FindBin(scanvalue) - 1;
        if (sfr) curveResults[iRes] = sfr;
      }
      nStep++;
    }
  }
  std::cout << "MethodProbScan::scan1d() : scan done.           " << std::endl;

  if (bestMinFoundInScan - bestMinOld > 0.01) {
    std::cout << "MethodProbScan::scan1d() : WARNING: Scan didn't find similar minimum to what was found before!"
              << std::endl;
    std::cout << "MethodProbScan::scan1d() :          Too strict parameter limits? Too coarse scan steps? Didn't load "
                 "global minimum?"
              << std::endl;
    std::cout << "MethodProbScan::scan1d() :          chi2 bestMinFoundInScan=" << bestMinFoundInScan
              << ", bestMinOld=" << bestMinOld << std::endl;
  }

  // attempt to correct for undercoverage
  if (pvalueCorrectorSet) {
    for (int k = 1; k <= hCL->GetNbinsX(); k++) {
      double pvalueProb = hCL->GetBinContent(k);
      pvalueProb = pvalueCorrector->transform(pvalueProb);
      hCL->SetBinContent(k, pvalueProb);
    }
  }

  Utils::setParameters(w, parsName, startPars->get(0));
  saveSolutions();
  if (arg->confirmsols) confirmSolutions();

  if ((bestMinFoundInScan - bestMinOld) / bestMinOld > 0.01) return 1;
  return 0;
}

///
/// Modify the CL histograms according to the defined test statistics
///\return status 0 ->potentially can encode debug information here
///
int MethodProbScan::computeCLvalues() {
  std::cout << "Computing CL values based on test statistic decision" << std::endl;
  std::cout << "Using " << arg->teststatistic << "-sided test statistic" << std::endl;

  if (!globalMin && !this->getSolution()) {
    std::cout << "Could not find a solution so can't redefine the test statistics appropriately" << std::endl;
    return 1;
  }

  double bestfitpoint;
  double bestfitpointerr;

  if (globalMin) {
    bestfitpoint = ((RooRealVar*)globalMin->floatParsFinal().find(scanVar1))->getVal();
    bestfitpointerr = ((RooRealVar*)globalMin->floatParsFinal().find(scanVar1))->getError();
  }

  if (this->getSolution()) {
    if (this->getSolution()->floatParsFinal().find(scanVar1)) {
      bestfitpoint = ((RooRealVar*)this->getSolution()->floatParsFinal().find(scanVar1))->getVal();
      bestfitpointerr = ((RooRealVar*)this->getSolution()->floatParsFinal().find(scanVar1))->getError();
    }
  }

  for (int k = 1; k <= hCL->GetNbinsX(); k++) {
    using Utils::normal_cdf;
    double scanvalue = hChi2min->GetBinCenter(k);
    double teststat_measured = hChi2min->GetBinContent(k) - chi2minGlobal;
    double CLb = 1. - (normal_cdf(TMath::Sqrt(teststat_measured) + ((scanvalue - 0.) / bestfitpointerr)) +
                       normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.) / bestfitpointerr)) - 1.);
    if (arg->teststatistic == 1) {                                             // use one-sided test statistic
      teststat_measured = bestfitpoint <= scanvalue ? teststat_measured : 0.;  // if mu < muhat then q_mu = 0
      hCL->SetBinContent(k, 1. - normal_cdf(TMath::Sqrt(teststat_measured)));
      // if (scanvalue < bestfitpoint) hCL->SetBinContent(k,1.0);    // should not be here, but looks ugly if solution
      // is drawn as p=1
      CLb = 1. - normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.) / bestfitpointerr));
    }
    if (arg->cls.size() > 0) hCLs->SetBinContent(k, std::min(1., hCL->GetBinContent(k) / CLb));
    // std::cout << "MethodProbScan::" << k << "\t" << hCL->GetBinContent(k) << "\t" << CLb << "\t" <<
    // hCLs->GetBinContent(k) <<std::endl;
  }

  return 0;
}

///
/// Delete a pointer if it is not included in
/// the curveResults2d vector. Also removes it
/// from the allResults vector by setting the entry
/// to 0.
/// \return true if r was deleted, or if it is 0
///
bool MethodProbScan::deleteIfNotInCurveResults2d(RooSlimFitResult* r) {
  if (r == 0) return true;
  bool del = true;
  for (int j = 0; j < hCL2d->GetNbinsX(); j++)
    for (int k = 0; k < hCL2d->GetNbinsY(); k++) {
      if (r == curveResults2d[j][k]) {
        del = false;
        break;
      }
    }
  if (del) {
    delete r;
    // remove also from allResults vector
    for (int j = 0; j < allResults.size(); j++) {
      if (r == allResults[j]) allResults[j] = 0;
    }
  }
  return del;
}

///
/// Compute coordinates that can be used as coordinates
/// of that fit result, that we want to use as start parameters
/// in the drag mode. The i,j coordinates spiral out. This function
/// computes coordinates from inner turns of the spiral.
/// If the inner turn doesn't exist, the start coordinates are set
/// as the result, and false is returned.
/// \param iStart center of the spiral
/// \param jStart center of the spiral
/// \param i current coordinates on the outmost turn of the spiral
/// \param j current coordinates on the outmost turn of the spiral
/// \param iResult resulting coordinates of an inner turn
/// \param jResult resulting coordinates of an inner turn
/// \param nTurn number of inner turn to jump to, 1=next-to-outer turn, 2=second-next, etc.
///
bool MethodProbScan::computeInnerTurnCoords(const int iStart, const int jStart, const int i, const int j, int& iResult,
                                            int& jResult, int nTurn) {
  // compute bin coordinates of start parameters: connect center of
  // the spiral to the scan point with a straight line, go back by sqrt(2)
  // units, take bin this ends us in
  using Utils::sq;
  iResult = iStart;
  jResult = jStart;
  if (sq(i - iStart) + sq(j - jStart) > 0) {
    iResult = round((double)i - double(nTurn) * 1.41 * double(i - iStart) / sqrt(sq(i - iStart) + sq(j - jStart)));
    jResult = round((double)j - double(nTurn) * 1.41 * double(j - jStart) / sqrt(sq(i - iStart) + sq(j - jStart)));
  }
  if (iResult - 1 >= curveResults2d.size()) iResult = iStart;
  if (jResult - 1 >= curveResults2d[0].size()) jResult = jStart;
  // check result
  if (iResult - 1 >= curveResults2d.size() || jResult - 1 >= curveResults2d[0].size() || iResult - 1 < 0 ||
      jResult - 1 < 0) {
    std::cout << "MethodProbScan::computeInnerTurnCoords() : ERROR : resulting coordinates out of range! "
              << iResult - 1 << " " << jResult - 1 << std::endl;
  }
  if (iResult == iStart && jResult == jStart) return false;
  return true;
}

void MethodProbScan::sanityChecks() const {
  auto error = [](const std::string& msg) { Utils::errBase("MethodProbScan::sanityChecks() : ERROR : ", msg); };

  if (!w->set(parsName)) error(std::format("parsName not found: {:s}", std::string(parsName)));
  if (!w->var(scanVar1)) error(std::format("scanVar1 not found: {:s}", std::string(scanVar1)));
  if (!w->var(scanVar2)) error(std::format("scanVar2 not found: {:s}", std::string(scanVar2)));
}

///
/// Perform a 2d Prob scan.
/// Scan range defined through limit "scan".
/// Fills the hCL2d histogram with the 1-CL curve.
/// Saves all encountered fit results to allResults.
/// Saves the fit results that make it into the 1-CL curve into curveResults2d.
/// Scan strategy: Spiral out!
///
int MethodProbScan::scan2d() {
  if (arg->debug) std::cout << "MethodProbScan::scan2d() : starting ..." << std::endl;
  nScansDone++;
  sanityChecks();
  if (startPars) delete startPars;

  // Define whether the 2d contours in hCL are "1D sigma" (ndof=1) or "2D sigma" (ndof=2).
  // Leave this at 1 for now, as the "2D sigma" contours are computed from hChi2min2d, not hCL.
  int ndof = 1;

  // Set up storage for fit results of this particular
  // scan. This is used for the drag start parameters.
  // We cannot use the curveResults2d member because that
  // only holds better results.
  std::vector<std::vector<RooSlimFitResult*>> mycurveResults2d;
  for (int i = 0; i < nPoints2dx; i++) {
    std::vector<RooSlimFitResult*> tmp;
    for (int j = 0; j < nPoints2dy; j++) tmp.push_back(0);
    mycurveResults2d.push_back(tmp);
  }

  // store start parameters so we can reset them later
  startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
  startPars->add(*w->set(parsName));

  // // start scan from global minimum (not always a good idea as we need to set from other places as well)
  // Utils::setParameters(w, parsName, globalMin);

  // Define scan parameters and scan range:
  RooRealVar* par1 = w->var(scanVar1);
  RooRealVar* par2 = w->var(scanVar2);

  // Set limit to all parameters.
  combiner->loadParameterLimits();

  // fix scan parameters
  par1->setConstant(true);
  par2->setConstant(true);

  // Check if there is at least one parameter to be fit
  auto likelihood = w->pdf(pdfName);
  const bool hasFreePars = getNFloatingParameters(likelihood) > 0;
  if (!hasFreePars)
    std::cout << "MethodProbScan::scan2d() : INFO : There are not free parameters. I will scan without fitting"
              << std::endl;

  RooFormulaVar nll("nll", "nll", "-2*log(@0)", RooArgSet(*likelihood));

  // Report on the smallest new minimum we come across while scanning.
  // Sometimes the scan doesn't find the minimum
  // that was found before. Warn if this happens.
  const auto bestMinOld = chi2minGlobal;
  auto bestMinFoundInScan = std::numeric_limits<double>::max();

  // for the status bar
  int nSteps = 0;
  double nTotalSteps = nPoints2dx * nPoints2dy;
  double printFreq = nTotalSteps > 100 && !arg->probforce ? 100 : nTotalSteps;  ///< number of messages

  // initialize some control plots
  gStyle->SetOptTitle(1);
  TCanvas* cDbg = Utils::newNoWarnTCanvas(Utils::getUniqueRootName(), Form("DeltaChi2 for 2D scan %i", nScansDone));
  cDbg->SetMargin(0.1, 0.15, 0.1, 0.1);
  double hChi2min2dMin = hChi2min2d->GetMinimum();
  bool firstScanDone = hChi2min2dMin < 1e5;
  TH2F* hDbgChi2min2d =
      Utils::histHardCopy(hChi2min2d, firstScanDone, true, TString(hChi2min2d->GetName()) + TString("_Dbg"));
  hDbgChi2min2d->SetTitle(Form("#Delta#chi^{2} for scan %i, %s", nScansDone, title.Data()));
  if (firstScanDone) hDbgChi2min2d->GetZaxis()->SetRangeUser(hChi2min2dMin, hChi2min2dMin + 81);
  hDbgChi2min2d->GetXaxis()->SetTitle(par1->GetTitle());
  hDbgChi2min2d->GetYaxis()->SetTitle(par2->GetTitle());
  hDbgChi2min2d->GetZaxis()->SetTitle("#Delta#chi^{2}");
  TH2F* hDbgStart = Utils::histHardCopy(hChi2min2d, false, true, TString(hChi2min2d->GetName()) + TString("_DbgSt"));

  // start coordinates
  // don't allow the under/overflow bins
  int iStart = std::min(hCL2d->GetXaxis()->FindBin(par1->getVal()), hCL2d->GetNbinsX());
  int jStart = std::min(hCL2d->GetYaxis()->FindBin(par2->getVal()), hCL2d->GetNbinsY());
  iStart = std::max(iStart, 1);
  jStart = std::max(jStart, 1);
  hDbgStart->SetBinContent(iStart, jStart, 500.);
  TMarker* startpointmark = new TMarker(par1->getVal(), par2->getVal(), 3);

  // timer
  TStopwatch tFit;
  TStopwatch tSlimResult;
  TStopwatch tScan;
  TStopwatch tMemory;

  // set up the scan spiral
  int X = 2 * nPoints2dx;
  int Y = 2 * nPoints2dy;
  int x, y, dx, dy;
  x = y = dx = 0;
  dy = -1;
  int t = std::max(X, Y);
  int maxI = t * t;
  for (int spiralstep = 0; spiralstep < maxI; spiralstep++) {
    if ((-X / 2 <= x) && (x <= X / 2) && (-Y / 2 <= y) && (y <= Y / 2)) {
      int i = x + iStart;
      int j = y + jStart;
      if (i > 0 && i <= nPoints2dx && j > 0 && j <= nPoints2dy) {
        tScan.Start(false);

        // status bar
        if (((int)nSteps % (int)(nTotalSteps / printFreq)) == 0) {
          std::cout << Form("MethodProbScan::scan2d() : scanning %3.0f%%", (double)nSteps / (double)nTotalSteps * 100.)
                    << "       \r" << std::flush;
        }
        nSteps++;

        // status histogram
        if (spiralstep > 0) hDbgStart->SetBinContent(i, j, 500. /*firstScan ? 1. : hChi2min2dMin+36*/);

        // set start parameters from inner turn of the spiral
        int xStartPars, yStartPars;
        computeInnerTurnCoords(iStart, jStart, i, j, xStartPars, yStartPars, 1);
        RooSlimFitResult* rStartPars = mycurveResults2d[xStartPars - 1][yStartPars - 1];
        if (rStartPars) Utils::setParameters(w, parsName, rStartPars);

        // memory management:
        tMemory.Start(false);
        // delete old, inner fit results, that we don't need for start parameters anymore
        // for this we take the second-inner-most turn.
        int iOld, jOld;
        bool innerTurnExists = computeInnerTurnCoords(iStart, jStart, i, j, iOld, jOld, 2);
        if (innerTurnExists) {
          deleteIfNotInCurveResults2d(mycurveResults2d[iOld - 1][jOld - 1]);
          mycurveResults2d[iOld - 1][jOld - 1] = 0;
        }
        tMemory.Stop();

        // alternative choice for start parameters: always from what we found at function call
        // Utils::setParameters(w, parsName, startPars->get(0));

        // set scan point
        double scanvalue1 = hCL2d->GetXaxis()->GetBinCenter(i);
        double scanvalue2 = hCL2d->GetYaxis()->GetBinCenter(j);
        par1->setVal(scanvalue1);
        par2->setVal(scanvalue2);

        RooSlimFitResult* sfr = nullptr;
        auto chi2minScan = std::numeric_limits<double>::max();
        bool fitConverged = true;
        bool performFit = hasFreePars;
        if (!hasFreePars) {
          // There are no parameters to fit, just calculate NLL
          chi2minScan = nll.getVal();
          // But if we found another minimum, perform the fit!
          if (chi2minScan < bestMinFoundInScan) performFit = true;
        }
        if (performFit) {
          if (!hasFreePars) {
            par1->setConstant(false);
            par2->setConstant(false);
          }
          tFit.Start(false);
          std::unique_ptr<RooFitResult> fr;
          if (arg->probforce)
            fr = std::unique_ptr<RooFitResult>(Utils::fitToMinForce(w, combiner->getPdfName()));
          else
            fr = std::unique_ptr<RooFitResult>(Utils::fitToMinBringBackAngles(likelihood, false, -1));
          chi2minScan = fr->minNll();
          if (std::isinf(chi2minScan))
            chi2minScan = std::numeric_limits<double>::max();  // else the toys in PDF_testConstraint don't work
          // A fit that didn't converge can report a spuriously low minNll(); don't let such a
          // result move the global minimum or overwrite a previously converged fit.
          fitConverged = fr->status() == 0 && fr->edm() < 1;
          if (!fitConverged && arg->debug)
            std::cout << "MethodProbScan::scan2d() : WARNING : fit didn't converge at (" << i << "," << j
                      << "): status=" << fr->status() << " edm=" << fr->edm() << " minNll=" << chi2minScan << std::endl;
          tFit.Stop();
          tSlimResult.Start(false);
          allResults.push_back(new RooSlimFitResult(fr.get()));  // save memory by using the slim fit result
          sfr = allResults.back();
          if (sfr) mycurveResults2d[i - 1][j - 1] = sfr;
          tSlimResult.Stop();
          if (!hasFreePars) {
            par1->setConstant(true);
            par2->setConstant(true);
          }
        }
        if (fitConverged) bestMinFoundInScan = std::min(chi2minScan, bestMinFoundInScan);

        // If we find a new global minumum, this means that all previous 1-CL values are too high.
        // We'll save the new possible solution, adjust the global minimum, return a status code, and stop.
        if (fitConverged && chi2minScan > -500.0 && chi2minScan < chi2minGlobal) {
          // warn only if there was a significant improvement
          if (arg->debug || chi2minScan < chi2minGlobal - 1e-2) {
            if (arg->verbose)
              std::cout << "MethodProbScan::scan2d() : WARNING : '" << title
                        << "' new global minimum found! chi2minGlobal=" << chi2minGlobal
                        << " chi2minScan=" << chi2minScan << std::endl;
          }
          chi2minGlobal = chi2minScan;
          // recompute previous 1-CL values
          for (int k = 1; k <= hCL2d->GetNbinsX(); k++)
            for (int l = 1; l <= hCL2d->GetNbinsY(); l++) {
              hCL2d->SetBinContent(k, l, TMath::Prob(hChi2min2d->GetBinContent(k, l) - chi2minGlobal, ndof));
            }
        }

        double deltaChi2 = chi2minScan - chi2minGlobal;
        double oneMinusCL = TMath::Prob(deltaChi2, ndof);

        // Save the 1-CL value. But only if better than before, and only if the fit that produced
        // it actually converged (an unconverged fit can report a spuriously low minNll(), which would
        // otherwise silently overwrite a meaningful value from a previous scan).
        if (fitConverged && hCL2d->GetBinContent(i, j) < oneMinusCL) {
          hCL2d->SetBinContent(i, j, oneMinusCL);
          hChi2min2d->SetBinContent(i, j, chi2minScan);
          hDbgChi2min2d->SetBinContent(i, j, chi2minScan);
          curveResults2d[i - 1][j - 1] = sfr;
        }

        // draw/update histograms - doing only every nth update
        // depending on value of updateFreq
        // saves a lot of time for small combinations
        if ((arg->interactive && ((int)nSteps % arg->updateFreq == 0)) || nSteps == nTotalSteps) {
          hDbgChi2min2d->Draw("colz");
          hDbgStart->Draw("boxsame");
          startpointmark->Draw();
          cDbg->Update();
          cDbg->Modified();
          gSystem->ProcessEvents();
        }
        tScan.Stop();
      }
    }
    // spiral stuff:
    if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y))) {
      t = dx;
      dx = -dy;
      dy = t;
    }
    x += dx;
    y += dy;
  }
  std::cout << "MethodProbScan::scan2d() : scan done.            " << std::endl;
  if (arg->debug) {
    std::cout << "MethodProbScan::scan2d() : full scan time:             ";
    tScan.Print();
    std::cout << "MethodProbScan::scan2d() : - fitting:                  ";
    tFit.Print();
    std::cout << "MethodProbScan::scan2d() : - create RooSlimFitResults: ";
    tSlimResult.Print();
    std::cout << "MethodProbScan::scan2d() : - memory management:        ";
    tMemory.Print();
  }
  Utils::setParameters(w, parsName, startPars->get(0));
  saveSolutions2d();
  if (arg->debug) printLocalMinima();
  if (arg->confirmsols) confirmSolutions();

  // clean all fit results that didn't make it into the final result
  for (int i = 0; i < allResults.size(); i++) { deleteIfNotInCurveResults2d(allResults[i]); }

  if (bestMinFoundInScan - bestMinOld > 0.1) {
    std::cout << "MethodProbScan::scan2d() : WARNING: Scan didn't find minimum that was found before!" << std::endl;
    std::cout << "MethodProbScan::scan2d() :          Are you using too strict parameter limits?" << std::endl;
    std::cout << "MethodProbScan::scan2d() :          min chi2 found in scan: " << bestMinFoundInScan
              << ", old min chi2: " << bestMinOld << std::endl;
    return 1;
  }

  // cleanup
  if (hDbgChi2min2d) delete hDbgChi2min2d;
  if (hDbgStart) delete hDbgStart;

  return 0;
}

///
/// Find the RooFitResults corresponding to all local
/// minima from the curveResults vector
/// and save them into the solutions vector.
/// It will be sorted for minChi2. Index 0
/// will correspond to the least chi2.
///
void MethodProbScan::saveSolutions() {
  auto error = [](const std::string& msg) { Utils::errBase("MethodProbScan::saveSolutions() : ERROR : ", msg); };
  auto info = [](const std::string& msg) { Utils::msgBase("MethodProbScan::saveSolutions() : ", msg); };
  auto warning = [](const std::string& msg) { Utils::msgBase("MethodProbScan::saveSolutions() : WARNING : ", msg); };

  if (arg->debug) info("Searching for minima in hChi2min...");

  // delete old solutions if any
  std::vector<RooSlimFitResult*> tmp;
  solutions = tmp;

  // loop over chi2 histogram to locate local maxima
  for (int i = 2; i < hChi2min->GetNbinsX() - 1; i++) {
    bool oneBinMax = hChi2min->GetBinContent(i - 1) > hChi2min->GetBinContent(i) &&
                     hChi2min->GetBinContent(i + 1) > hChi2min->GetBinContent(i);
    bool twoBinMax = hChi2min->GetBinContent(i - 1) > hChi2min->GetBinContent(i) &&
                     hChi2min->GetBinContent(i) == hChi2min->GetBinContent(i + 1) &&
                     hChi2min->GetBinContent(i + 2) > hChi2min->GetBinContent(i + 1);
    if (!(oneBinMax || twoBinMax)) continue;

    // loop over fit results to find those that produced it
    for (int j = 0; j < curveResults.size(); j++) {
      if (!curveResults[j]) {
        if (arg->debug) warning(std::format("Empty solution at index {:d}", j));
        continue;
      }

      if (hChi2min->FindBin(curveResults[j]->getConstParVal(scanVar1)) == i) {
        if (arg->debug) {
          info(std::format("Saving solution {:d}:", j));
          curveResults[j]->Print();
        }
        solutions.push_back(curveResults[j]);
      }
    }
  }

  if (solutions.empty())
    warning("No solutions found during the scan\n"
            "This is normal if there are only 1(2) free parameters in the baseline fit and the scan is 1d (2d)");

  if (solutions.empty()) {
    if (!globalMin) error("No solution was found at all");
    solutions.emplace_back(new RooSlimFitResult(globalMin));
  } else {
    sortSolutions();
  }
}

///
/// Find the RooFitResults corresponding to all local
/// minima from the curveResults2d vector
/// and save them into the solutions vector.
/// It will be sorted for minChi2. Index 0
/// will correspond to the least chi2.
///
/// We use a brute force minimum finding over the histogram,
/// as it is not large: at each bin, inpsect the 8 surrounding
/// ones.
///
void MethodProbScan::saveSolutions2d() {
  auto error = [](const std::string& msg, bool exit = true) {
    Utils::errBase("MethodProbScan::saveSolutions2d() : ERROR : ", msg, exit);
  };
  auto info = [](const std::string& msg) { Utils::msgBase("MethodProbScan::saveSolutions2d() : ", msg); };
  auto warning = [](const std::string& msg) { Utils::msgBase("MethodProbScan::saveSolutions2d() : WARNING : ", msg); };

  if (arg->debug) info("Searching for minima in hChi2min2d...");

  // delete old solutions if any
  for (int j = 0; j < solutions.size(); j++)
    if (solutions[j]) delete solutions[j];
  solutions.clear();

  // loop over chi2 histogram to locate local minima
  for (int i = 2; i < hChi2min2d->GetNbinsX(); i++) {
    for (int j = 2; j < hChi2min2d->GetNbinsY(); j++) {
      if (!(hChi2min2d->GetBinContent(i - 1, j) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i + 1, j) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i, j - 1) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i, j + 1) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i - 1, j - 1) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i + 1, j + 1) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i + 1, j - 1) > hChi2min2d->GetBinContent(i, j) &&
            hChi2min2d->GetBinContent(i - 1, j + 1) > hChi2min2d->GetBinContent(i, j)))
        continue;

      RooSlimFitResult* r = curveResults2d[i - 1][j - 1];  // -1 because it starts counting at 0, but histograms at 1
      if (!r) {
        error(std::format("No corresponding RooFitResult found! Skipping (i,j)=({:d},{:d})", i, j), false);
        continue;
      }
      if (arg->debug) info(std::format("Saving solution of bin ({:d},{:d})...", i, j));
      solutions.push_back((RooSlimFitResult*)curveResults2d[i - 1][j - 1]->Clone());
    }
  }

  if (solutions.empty()) {
    warning("MethodProbScan::saveSolutions2d() : WARNING : No solutions found in 2D scan!\n"
            "  This can happen when a solution is too close to the plot boundary.\n"
            "  In this case, either change the scan range using --scanrange and --scanrangey,\n"
            "  or increase the number of scan points, using --npoints or --npoints2dx, --npoints2dy");
    if (!globalMin) error("No solution was found at all");
    solutions.emplace_back(new RooSlimFitResult(globalMin));
  } else {
    sortSolutions();
  }
}

///
/// Get the chi2 value of the profile likelihood at a given
/// scan point. Requires that the scan was performed before
/// by scan1d().
///
double MethodProbScan::getChi2min(double scanpoint) const {
  assert(hChi2min);
  int iBin = hChi2min->FindBin(scanpoint);
  return hChi2min->GetBinContent(iBin);
}
