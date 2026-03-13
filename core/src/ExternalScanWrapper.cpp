#include <ExternalScanWrapper.h>

#include <Utils.h>

#include <RooRealVar.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TString.h>

#include <format>
#include <memory>
#include <string>

class OptParser;

namespace {
  bool isAngle(const TString& name) {
    return name.Contains("gamma") || name.Contains("delta") || name.Contains("phi") || name.Contains("theta") ||
           name.Contains("alpha") || name.Contains("beta");
  }
}  // namespace

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

ExternalScanWrapper* createExternalScanner(const TString filename, const TString label, const OptParser* opts) {
  auto info = [](std::string msg) { return Utils::msgBase("createExternalScanner() : ", msg); };
  auto error = [](std::string msg) { return Utils::errBase("createExternalScanner() : ERROR : ", msg); };

  auto extFile = std::unique_ptr<TFile>(TFile::Open(filename));
  if (!extFile || extFile->IsZombie()) {
    error(std::format("Cannot open external scan file {:s}", std::string(filename)));
    return nullptr;
  }

  using Utils::getUniqueRootName;
  if (auto hCL2d = dynamic_cast<TH2F*>(extFile->Get("hCL"))) {
    info("Loading 2D external scan...");

    // Clone to avoid issues when file closes
    hCL2d = dynamic_cast<TH2F*>(hCL2d->Clone("hCL_external" + getUniqueRootName()));
    hCL2d->SetDirectory(0);

    auto hCLs2d = dynamic_cast<TH2F*>(extFile->Get("hCLs"));
    if (hCLs2d) {
      hCLs2d = dynamic_cast<TH2F*>(hCLs2d->Clone("hCLs_external" + getUniqueRootName()));
      hCLs2d->SetDirectory(0);
    }

    auto extScanner = new ExternalScanWrapper(hCL2d, hCLs2d, "external_" + label, label, opts);

    // Infer from histogram axis names for common angular variables
    TString xName = hCL2d->GetXaxis()->GetName();
    TString yName = hCL2d->GetYaxis()->GetName();
    if (isAngle(xName)) extScanner->getScanVar1()->setUnit("Rad");
    if (isAngle(yName)) extScanner->getScanVar2()->setUnit("Rad");

    return extScanner;

  } else if (auto hCL1d = dynamic_cast<TH1F*>(extFile->Get("hCL"))) {
    info("Loading 1D external scan...");

    // Clone to avoid issues when file closes
    hCL1d = dynamic_cast<TH1F*>(hCL1d->Clone("hCL_external" + getUniqueRootName()));
    hCL1d->SetDirectory(0);

    auto hCLs1d = dynamic_cast<TH1F*>(extFile->Get("hCLs"));
    if (hCLs1d) {
      hCLs1d = dynamic_cast<TH1F*>(hCLs1d->Clone("hCLs_external" + getUniqueRootName()));
      hCLs1d->SetDirectory(0);
    }

    auto extScanner = new ExternalScanWrapper(hCL1d, hCLs1d, "external_" + label, label, opts);

    // Infer from histogram axis name for common angular variables
    TString xName = hCL1d->GetXaxis()->GetName();
    if (isAngle(xName)) extScanner->getScanVar1()->setUnit("Rad");

    return extScanner;

  } else {
    error("No TH1F/TH2F `hCL` histogram found");
    return nullptr;
  }
}
