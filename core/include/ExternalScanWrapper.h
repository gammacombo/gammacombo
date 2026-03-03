#ifndef ExternalScanWrapper_h
#define ExternalScanWrapper_h

#include <MethodAbsScan.h>

#include <RooRealVar.h>

#include <TString.h>

class OptParser;

class TH1F;
class TH2F;

/**
 * Helper class to deal with external scans from LHCb + BESIII combination of D0 -> K3pi in LHCb-CONF-2025.
 */
class ExternalScanWrapper : public MethodAbsScan {
 private:
  bool is2D = false;
  RooRealVar* scanVar1 = nullptr;
  RooRealVar* scanVar2 = nullptr;

 public:
  /// Constructor for 1D scan.
  ExternalScanWrapper(TH1F* hCL, TH1F* hCLs, TString name, TString title, const OptParser* arg);
  /// Constructor for 2D scan.
  ExternalScanWrapper(TH2F* hCL2d, TH2F* hCLs2d, TString name, TString title, const OptParser* arg);

  ~ExternalScanWrapper() override {
    if (scanVar1) delete scanVar1;
    if (scanVar2) delete scanVar2;
  }

  RooRealVar* getScanVar1() override { return scanVar1; }
  RooRealVar* getScanVar2() override { return scanVar2; }
  const RooRealVar* getScanVar1() const override { return scanVar1; }
  const RooRealVar* getScanVar2() const override { return scanVar2; }

  int scan1d() override { return 0; }
  int scan2d() override { return 0; }
  void initScan() override {}
};

#endif
