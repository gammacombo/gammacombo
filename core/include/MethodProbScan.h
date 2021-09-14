/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#ifndef MethodProb_h
#define MethodProb_h

#include <iostream>
#include <stdlib.h>

#include "RooGlobalFunc.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooAddition.h"
#include "RooMultiVarGaussian.h"
#include "RooSlimFitResult.h"
#include "RooRandom.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooPoisson.h"
#include "RooProdPdf.h"
#include "RooPlot.h"
#include "RooArgSet.h"

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

#include "MethodAbsScan.h"
#include "Utils.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class MethodProbScan : public MethodAbsScan
{
public:
  MethodProbScan(Combiner* comb);
  MethodProbScan(OptParser* opt);
  MethodProbScan();
  ~MethodProbScan();

  virtual int     computeCLvalues();  // compute CL histograms depending on desired test statistic
  float           getChi2min(float scanpoint);
  inline TH1F*    getHChi2min(){return hChi2min;};
  void            saveSolutions();
  void            saveSolutions2d();
  virtual int             scan1d(bool fast=false, bool reverse=false, bool quiet=false);
  virtual int             scan2d();
  inline void     setScanDisableDragMode(bool f=true){scanDisableDragMode = f;};

protected:
  bool            computeInnerTurnCoords(const int iStart, const int jStart, const int i, const int j,
                    int &iResult, int &jResult, int nTurn);
  bool            deleteIfNotInCurveResults2d(RooSlimFitResult *r);
  void            sanityChecks();
  bool            scanDisableDragMode;
	int							nScansDone;						// count the number of times a scan was done

};

#endif
