#ifndef ExternalScanWrapper_h
#define ExternalScanWrapper_h

#include <MethodAbsScan.h>

#include <TString.h>

#include <vector>

class RooSlimFitResult;

class ExternalScanWrapper : public MethodAbsScan {
 private:
  TString scannerTitle;
  TString scannerName;
  bool is2D;
  RooRealVar* scanVar1Dummy;
  RooRealVar* scanVar2Dummy;

 public:
  // Constructor for 1D
  ExternalScanWrapper(TH1F* hCL, TH1F* hCLs, TString name, TString title, OptParser* arg);
  // Constructor for 2D
  ExternalScanWrapper(TH2F* hCL2d, TH2F* hCLs2d, TString name, TString title, OptParser* arg);

  ~ExternalScanWrapper() {
    if (scanVar1Dummy) delete scanVar1Dummy;
    if (scanVar2Dummy) delete scanVar2Dummy;
  }

  // Required implementations
  virtual TH1F* getHCL() { return hCL; }
  virtual TH1F* getHCLs() { return hCLs; }
  virtual TString getName() { return scannerName; }
  virtual TString getTitle() { return scannerTitle; }
  virtual TString getMethodName() { return "ExternalProb"; }

  // Properly implemented - no longer stubs
  virtual RooRealVar* getScanVar1() { return scanVar1Dummy; }
  virtual RooRealVar* getScanVar2() { return is2D ? scanVar2Dummy : nullptr; }

  // Stubs (never called for external data)
  virtual int scan1d(bool fast = false) { return 0; }
  virtual int scan2d() { return 0; }
  virtual void initScan() {}
  virtual std::vector<RooSlimFitResult*> getSolutions() { return std::vector<RooSlimFitResult*>(); }
};

#endif
