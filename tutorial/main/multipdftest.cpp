#include "TFile.h"

#include "RooMultiPdf.h"
#include "RooRealVar.h"
#include "RooWorkspace.h"
#include "RooExponential.h"
#include "RooPolynomial.h"
#include "RooGaussian.h"
#include "RooChebychev.h"
#include "RooGenericPdf.h"
#include "RooDataSet.h"
#include "RooAddPdf.h"
#include "RooPlot.h"

using namespace std;
using namespace RooFit;

int main(int argc, char* argv[])
{
    // mass variable
    RooRealVar mass("CMS_hgg_mass","m_{#gamma#gamma}",120,100,180);

    // create 3 background pdfs
    // 1. exponential
    RooRealVar expo_1("expo_1","slope of exponential",-0.02,-0.1,-0.0001);
    RooExponential exponential("exponential","exponential pdf",mass,expo_1);

    // 2. polynomial with 2 parameters
    RooRealVar poly_1("poly_1","T1 of chebychev polynomial",0,-3,3);
    RooRealVar poly_2("poly_2","T2 of chebychev polynomial",0,-3,3);
    RooChebychev polynomial("polynomial","polynomial pdf",mass,RooArgList(poly_1,poly_2));

    // 3. A power law function
    RooRealVar pow_1("pow_1","exponent of power law",-3,-6,-0.0001);
    RooGenericPdf powerlaw("powerlaw","TMath::Power(@0,@1)",RooArgList(mass,pow_1));

    // Generate some data (lets use the power lay function for it)
    // Here we are using unbinned data, but binning the data is also fine
    RooDataSet *data = powerlaw.generate(mass,RooFit::NumEvents(1000));

    // First we fit the pdfs to the data (gives us a sensible starting value of parameters for, e.g - blind limits)
    exponential.fitTo(*data);   // index 0
    polynomial.fitTo(*data);   // index 1
    powerlaw.fitTo(*data);     // index 2

    // Make a plot (data is a toy dataset)
    RooPlot *plot = mass.frame();   data->plotOn(plot);
    exponential.plotOn(plot,RooFit::LineColor(kGreen));
    polynomial.plotOn(plot,RooFit::LineColor(kBlue));
    powerlaw.plotOn(plot,RooFit::LineColor(kRed));
    plot->SetTitle("PDF fits to toy data");
    plot->Draw();

    // Make a RooCategory object. This will control which of the pdfs is "active"
    RooCategory cat("pdf_index","Index of Pdf which is active");

    // Make a RooMultiPdf object. The order of the pdfs will be the order of their index, ie for below
    // 0 == exponential
    // 1 == polynomial
    // 2 == powerlaw
    RooArgList mypdfs;
    mypdfs.add(exponential);
    mypdfs.add(polynomial);
    mypdfs.add(powerlaw);

    RooMultiPdf multipdf("roomultipdf","All Pdfs",cat,mypdfs);
    // By default the multipdf will tell combine to add 0.5 to the nll for each parameter (this is the penalty for the discrete profiling method)
    // It can be changed with
    //   multipdf.setCorrectionFactor(penalty)
    // For bias-studies, this isn;t relevant however, so lets just leave the default

    // As usual make an extended term for the background with _norm for freely floating yield
    RooRealVar norm("roomultipdf_norm","Number of background events",1000,0,10000);

    // We will also produce a signal model for the bias studies
    RooRealVar sigma("sigma","sigma",1.2); sigma.setConstant(true);
    RooRealVar MH("MH","MH",125); MH.setConstant(true);
    RooGaussian signal("signal","signal",mass,MH,sigma);


    // Save to a new workspace
    TFile *fout = new TFile("workspace.root","RECREATE");
    RooWorkspace wout("workspace","workspaace");

    data->SetName("data");
    wout.import(*data);
    wout.import(cat);
    wout.import(norm);
    wout.import(multipdf);
    wout.import(signal);
    wout.Print();
    wout.Write();
}
