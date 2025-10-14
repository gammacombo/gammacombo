#ifndef ExternalScanWrapper_h
#define ExternalScanWrapper_h

#include <MethodAbsScan.h>

#include <RooRealVar.h>

#include <TString.h>

#include <vector>

class OptParser;

class TH1F;
class TH2F;

/**
 * Helper class to deal with external scans from LHCb + BESIII combination of D0 -> K3pi in LHCb-CONF-2025.
 */
class ExternalScanWrapper : public MethodAbsScan {
 private:
  bool is2D = false;
  RooRealVar* scanVar1Dummy = nullptr;
  RooRealVar* scanVar2Dummy = nullptr;

 public:
  /// Constructor for 1D scan.
  ExternalScanWrapper(TH1F* hCL, TH1F* hCLs, TString name, TString title, const OptParser* arg);
  /// Constructor for 2D scan.
  ExternalScanWrapper(TH2F* hCL2d, TH2F* hCLs2d, TString name, TString title, const OptParser* arg);

  ~ExternalScanWrapper() {
    if (scanVar1Dummy) delete scanVar1Dummy;
    if (scanVar2Dummy) delete scanVar2Dummy;
  }

  RooRealVar* getScanVar1() override { return scanVar1Dummy; }
  RooRealVar* getScanVar2() override { return scanVar2Dummy; }
  const RooRealVar* getScanVar1() const override { return scanVar1Dummy; }
  const RooRealVar* getScanVar2() const override { return scanVar2Dummy; }

  int scan1d() override { return 0; }
  int scan2d() override { return 0; }
  void initScan() override {}
};

#endif
