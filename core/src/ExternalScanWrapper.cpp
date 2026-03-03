#include <ExternalScanWrapper.h>

#include <RooRealVar.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TString.h>

// Constructor for 1D data
ExternalScanWrapper::ExternalScanWrapper(TH1F* hCL_ext, TH1F* hCLs_ext, TString name, TString title,
                                         const OptParser* opt)
    : MethodAbsScan(opt) {
  methodName = "ExternalProb";
  this->name = name;
  this->title = title;
  hCL = hCL_ext;
  hCLs = hCLs_ext;

  // Extract axis info from histogram
  TString xTitle = hCL_ext->GetXaxis()->GetTitle();
  TString xName = hCL_ext->GetXaxis()->GetName();
  double xMin = hCL_ext->GetXaxis()->GetXmin();
  double xMax = hCL_ext->GetXaxis()->GetXmax();
  double xCenter = (xMin + xMax) / 2.;

  scanVar1 = new RooRealVar(xName.IsNull() ? "x" : xName, xTitle.IsNull() ? "x" : xTitle, xCenter, xMin, xMax);
}

// Constructor for 2D data
ExternalScanWrapper::ExternalScanWrapper(TH2F* hCL2d_ext, TH2F* hCLs2d_ext, TString name, TString title,
                                         const OptParser* opt)
    : MethodAbsScan(opt), is2D(true) {
  methodName = "ExternalProb";
  this->name = name;
  this->title = title;
  hCL2d = hCL2d_ext;
  hCLs2d = hCLs2d_ext;

  // Extract axis info from histogram
  TString xTitle = hCL2d_ext->GetXaxis()->GetTitle();
  TString xName = hCL2d_ext->GetXaxis()->GetName();
  double xMin = hCL2d_ext->GetXaxis()->GetXmin();
  double xMax = hCL2d_ext->GetXaxis()->GetXmax();
  double xCenter = (xMin + xMax) / 2.;

  TString yTitle = hCL2d_ext->GetYaxis()->GetTitle();
  TString yName = hCL2d_ext->GetYaxis()->GetName();
  double yMin = hCL2d_ext->GetYaxis()->GetXmin();
  double yMax = hCL2d_ext->GetYaxis()->GetXmax();
  double yCenter = (yMin + yMax) / 2.;

  scanVar1 = new RooRealVar(xName.IsNull() ? "x" : xName, xTitle.IsNull() ? "x" : xTitle, xCenter, xMin, xMax);
  scanVar2 = new RooRealVar(yName.IsNull() ? "y" : yName, yTitle.IsNull() ? "y" : yTitle, yCenter, yMin, yMax);
}
