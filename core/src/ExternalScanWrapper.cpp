#include <ExternalScanWrapper.h>
// Constructor for 1D data
ExternalScanWrapper::ExternalScanWrapper(TH1F* hCL_ext, TH1F* hCLs_ext, TString name, TString title, OptParser* opt)
    : MethodAbsScan(opt), scannerName(name), scannerTitle(title), is2D(false) {
  methodName = "ExternalProb";
  hCL = hCL_ext;
  hCLs = hCLs_ext;
  this->name = name;
  this->title = title;

  // Extract axis info from histogram
  TString xTitle = hCL_ext->GetXaxis()->GetTitle();
  TString xName = hCL_ext->GetXaxis()->GetName();
  double xMin = hCL_ext->GetXaxis()->GetXmin();
  double xMax = hCL_ext->GetXaxis()->GetXmax();
  double xCenter = (xMin + xMax) / 2.0;

  scanVar1Dummy = new RooRealVar(xName.IsNull() ? "x" : xName, xTitle.IsNull() ? "x" : xTitle,
                                 xCenter,  // initial value
                                 xMin, xMax);
  scanVar2Dummy = nullptr;
}

// Constructor for 2D data
ExternalScanWrapper::ExternalScanWrapper(TH2F* hCL2d_ext, TH2F* hCLs2d_ext, TString name, TString title, OptParser* opt)
    : MethodAbsScan(opt), scannerName(name), scannerTitle(title), is2D(true) {
  methodName = "ExternalProb";
  hCL2d = hCL2d_ext;
  hCLs2d = hCLs2d_ext;
  this->name = name;
  this->title = title;

  // Extract axis info from histogram
  TString xTitle = hCL2d_ext->GetXaxis()->GetTitle();
  TString xName = hCL2d_ext->GetXaxis()->GetName();
  double xMin = hCL2d_ext->GetXaxis()->GetXmin();
  double xMax = hCL2d_ext->GetXaxis()->GetXmax();
  double xCenter = (xMin + xMax) / 2.0;

  TString yTitle = hCL2d_ext->GetYaxis()->GetTitle();
  TString yName = hCL2d_ext->GetYaxis()->GetName();
  double yMin = hCL2d_ext->GetYaxis()->GetXmin();
  double yMax = hCL2d_ext->GetYaxis()->GetXmax();
  double yCenter = (yMin + yMax) / 2.0;

  scanVar1Dummy = new RooRealVar(xName.IsNull() ? "x" : xName, xTitle.IsNull() ? "x" : xTitle, xCenter, xMin, xMax);
  scanVar2Dummy = new RooRealVar(yName.IsNull() ? "y" : yName, yTitle.IsNull() ? "y" : yTitle, yCenter, yMin, yMax);
}
