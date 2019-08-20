#include "RooGaussian.h"
#include "RooLognormal.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooFormulaVar.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooFormula.h"
#include "RooExponential.h"
#include "RooExtendPdf.h"
#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooWorkspace.h"
#include "TFile.h"
#include "TCanvas.h"



int main()
{

  /////////////////////////////////////////////////////////
  //
  // First, we define all observables and variables and construct the PDF.
  // It will be included in the workspace. 
  //
  /////////////////////////////////////////////////////////

  // First, define the signal peak of the mass model

  // It consists of a gaussian to describe the signal...
  RooRealVar mass("mass","mass", 4360., 6360., "MeV");
  RooRealVar mean("mean","mean",5370);
  RooRealVar sigma("sigma","sigma", 20.9);
  RooGaussian signal_model("g","g", mass, mean, sigma);

  // ... and of an exponential function to describe the background.
  RooRealVar exponent("exponent","exponent", -1e-3, -1., 1.);
  RooRealVar n_bkg("Nbkg","Nbkg", 4900, 0, 10000);
  RooExponential background_model("background_model", "background_model", mass, exponent);
  RooExponential bkg_only_model("bkg_only_model", "bkg_only_model", mass, exponent);
  RooExtendPdf extended_bkg_model("extended_bkg_model", "extended_bkg_model", bkg_only_model, n_bkg);

  // The number of signal events is related to the branching ratio via the formula <branching ratio> = <n_sig> * <normalization factor>
  // The normalization factor is not exactly known. Instead, it has to be estimated. The estimator for the normalization factor is a global observable
  // that constrains its value via a Gaussian Constraint. 
  RooRealVar  norm_constant_obs( "norm_constant_glob_obs", "global observable of normalization constant", 
                              1e-8, 1e-20, 1e-6);     // this is the observed value, the global observable
  RooRealVar  norm_constant("norm_constant","norm_constant", 1e-8, 1e-20,  1e-6);                   // the normalization constant is a nuisance parameter.
  RooRealVar  norm_constant_sigma("norm_constant_sigma","norm_constant_sigma", 5e-10);
  RooGaussian norm_constant_constraint("norm_constant_constraint","norm_constant_constraint", norm_constant_obs, norm_constant, norm_constant_sigma);

  // Now we can build the mass model by adding the signal and background probability density functions
  RooRealVar branchingRatio("branchingRatio", "branchingRatio", 1e-7, -1e-6,  0.0001);  // this is the branching ratio, the parameter of interest
  RooFormulaVar n_sig("Nsig", "branchingRatio/norm_constant", RooArgList(branchingRatio, norm_constant));
  RooExtendPdf extended_sig_model("extended_sig_model", "extended_sig_model", signal_model, n_sig);

  RooAddPdf mass_model("mass_model","mass_model", RooArgList(signal_model, background_model), RooArgList(n_sig, n_bkg));

  /////////////////////////////////////////////////////////
  //
  // Open/create the dataset with the data that will be analyzed 
  // and set the global observables to the observed values
  //
  /////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////
  //
  // In this tutorial, we generate our data instead of using a real dataset.
  // We generate one mass dataset with 100 events.
  // We also generate a single measurement of the global observable.
  double observedValueGlobalObservable = norm_constant_constraint.generate(RooArgSet(norm_constant_obs),1)
                                                                ->get(0)
                                                                ->getRealValue("norm_constant_glob_obs");
  //
  RooDataSet& data = *mass_model.generate(RooArgSet(mass),5000); // we use a reference here to avoid copying the object
  data.SetName("data"); // the name of the dataset MUST be "data" in order for the framework to identify it.
  //
  /////////////////////////////////////////////////////////

  // Set the global observables to the observed values and make them constant for the fit.
  norm_constant_obs.setVal(observedValueGlobalObservable);
  norm_constant_obs.setConstant();

  // The workspace must also include the fit result of an initial fit of the model to data.
  RooFitResult& rooFitResult = *mass_model.fitTo(data,RooFit::Save(), RooFit::ExternalConstraints(RooArgSet(norm_constant_constraint)));

  /////////////////////////////////////////////////////////
  //
  // Plot it all so we can see what we did
  //
  /////////////////////////////////////////////////////////
  
  RooPlot* plot = mass.frame();
  data.plotOn(plot);	
  extended_bkg_model.plotOn(plot, RooFit::LineColor(kRed));
  mass_model.plotOn(plot);
  TCanvas c("c","c",1024, 768);
  plot->Draw();
  c.SaveAs("plots/pdf/data_and_fit_in_workspace.pdf");
  
  rooFitResult.Print();

  /////////////////////////////////////////////////////////
  //
  // We also must define some RooArgSets: 
  //
  /////////////////////////////////////////////////////////

  //One contaninting the constraint PDFs,
  RooArgSet constraint_set(norm_constant_constraint, "constraint_set");

  //one containing the global Observables,
  RooArgSet global_observables_set(norm_constant_obs, "global_observables_set");

  //one containing the normal Observables (the bin variables in the datasets usually) and
  RooArgSet dataset_observables_set(mass, "datasetObservables");


  //one containing the parameters
  RooArgSet parameters_set(branchingRatio, norm_constant, exponent, n_bkg, "parameters");

  /////////////////////////////////////////////////////////
  //
  // Import everything into a workspace and save it
  //
  /////////////////////////////////////////////////////////

  RooWorkspace workspace("dataset_workspace");
  workspace.import(mass_model);
  workspace.import(extended_bkg_model);
  workspace.import(data);
  workspace.import(rooFitResult, "data_fit_result"); // this MUST be called data_fit_result
  workspace.defineSet("constraint_set", constraint_set, true);
  workspace.defineSet("global_observables_set", global_observables_set, true);
  workspace.defineSet("datasetObservables", dataset_observables_set, true);
  workspace.defineSet("parameters", parameters_set, true);

  // Save the workspace to a file
  workspace.SaveAs("workspace.root");
  
  return 0;
}
