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
#include "RooFitResult.h"

using namespace std;
using namespace RooFit;

// Builds a workspace containing a multipdf which can be given to GammaCombo
// The background pdfs here are an exponential, 2nd order polynomial and power law
// We define a S+B model using each background pdf
// We also define a RooCategory "pdf_index" which indexes the S+B models
// These are combined in a RooMultiPdf

int main(int argc, char* argv[])
{

    // mass variable
    RooRealVar mass("mass","m",120,100,180);

    // create 3 background pdfs
    // 1. exponential
    RooRealVar expo_1("expo_1","slope of exponential",-0.035,-0.1,-0.0001);
    RooExponential exponential("exponential","exponential pdf",mass,expo_1);

    // 2. polynomial with 2 parameters
    RooRealVar poly_1("poly_1","T1 of chebychev polynomial",0,-3,3);
    RooRealVar poly_2("poly_2","T2 of chebychev polynomial",0,-3,3);
    RooChebychev polynomial("polynomial","polynomial pdf",mass,RooArgList(poly_1,poly_2));

    // 3. A power law function
    RooRealVar pow_1("pow_1","exponent of power law",-4.5,-10,-0.0001);
    RooGenericPdf powerlaw("powerlaw","TMath::Power(@0,@1)",RooArgList(mass,pow_1));

    // Generate some data according to a double exponential pdf
    RooRealVar expo_2("expo_2","slope of exponential",-0.005,-0.2,-0.0002);
    RooRealVar frac1("frac1","frac1",0.5,0.,1.);
    RooExponential exponential1("exponential1","exponential1",mass,expo_1);
    RooExponential exponential2("exponential2","exponential2",mass,expo_2);
    RooAddPdf doubleexponential("doubleexponential","doubleexponential",RooArgList(exponential1,exponential2),frac1);
    RooDataSet& data = *doubleexponential.generate(mass,RooFit::NumEvents(5000));
    data.SetName("data"); // the name of the dataset MUST be "data" in order for the framework to identify it.

    // First we fit the pdfs to the data (gives us a sensible starting value of parameters)
    exponential.fitTo(data);   // index 0
    polynomial.fitTo(data);   // index 1
    RooFitResult& result = *powerlaw.fitTo(data, RooFit::Save());     // index 2

    // Make a plot (data is a toy dataset)
    RooPlot *plot = mass.frame();   data.plotOn(plot);
    exponential.plotOn(plot,RooFit::LineColor(kGreen));
    polynomial.plotOn(plot,RooFit::LineColor(kBlue));
    powerlaw.plotOn(plot,RooFit::LineColor(kRed));
    plot->SetTitle("PDF fits to toy data");
    plot->Draw();

    // As usual make an extended term for the background with _norm for freely floating yield
    RooRealVar norm("roomultipdf_norm","Number of background events",1000,0,10000);

    // We will also produce a signal model
    RooRealVar sigma("sigma","sigma",1.2); sigma.setConstant(true);
    RooRealVar MH("MH","MH",115); MH.setConstant(true);
    RooGaussian signal("signal","signal",mass,MH,sigma);
    RooRealVar branchingRatio("branchingRatio", "branchingRatio", 0., -1e-6,  0.0001);  // this is the branching ratio, the parameter of interest
    RooRealVar  norm_constant("norm_constant","norm_constant", 1e-8);
    RooFormulaVar n_sig("n_sig", "branchingRatio/norm_constant", RooArgList(branchingRatio, norm_constant));
    // Now make signal+background models
    RooAddPdf SBmodel_exponential("SBmodel_exponential", "SBmodel_exponential", RooArgList(signal,exponential), RooArgList(n_sig,norm));
    RooAddPdf SBmodel_polynomial("SBmodel_polynomial", "SBmodel_polynomial", RooArgList(signal,polynomial), RooArgList(n_sig,norm));
    RooAddPdf SBmodel_powerlaw("SBmodel_powerlaw", "SBmodel_powerlaw", RooArgList(signal,powerlaw), RooArgList(n_sig,norm));

    // Make a RooCategory object. This will control which of the pdfs is "active"
    RooCategory cat("pdf_index","Index of Pdf which is active");

    // Make a RooMultiPdf object. The index corresponds to the order in which pdfs are added, ie for below
    // 0 == exponential
    // 1 == polynomial
    // 2 == powerlaw
    RooArgList mypdfs;
    mypdfs.add(SBmodel_exponential);
    mypdfs.add(SBmodel_polynomial);
    mypdfs.add(SBmodel_powerlaw);

    RooMultiPdf multipdf("roomultipdf","All Pdfs",cat,mypdfs);
    // By default the multipdf will tell combine to add 0.5 to the nll for each parameter (this is the penalty for the discrete profiling method)
    // It can be changed with
    //   multipdf.setCorrectionFactor(penalty)

    // Save to a new workspace
    TFile *fout = new TFile("multipdfworkspace.root","RECREATE");
    RooWorkspace wout("workspace","workspaace");

    // Define the RooArgSets
    RooArgSet constraint_set("constraint_set");
    RooArgSet global_observables_set("global_observables_set");
    RooArgSet dataset_observables_set(mass,"dataset_observables_set");
    RooArgSet parameters_set(*(multipdf.getParameters(RooArgSet(mass,cat))), "parameters"); // Although cat is a nuisance parameter, it must not be included in parameters_set, since it is internally treated differently to the other nuisance parameters


    wout.import(data);
    wout.import(multipdf);
    wout.import(result,"data_fit_result");
    wout.defineSet("constraint_set", constraint_set, true);
    wout.defineSet("global_observables_set", global_observables_set, true);
    wout.defineSet("datasetObservables", dataset_observables_set, true);
    wout.defineSet("parameters", parameters_set, true);

    wout.Print();
    wout.Write();
}
