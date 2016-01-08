#include "GammaComboEngine.h"

#include "RooGaussian.h"
#include "RooRealVar.h"
#include "RooAddPdf.h"
#include "RooFormula.h"
#include "RooExponential.h"
#include "RooArgList.h"
#include "PDF_DatasetTutorial.h"
#include "PDF_Datasets_Abs.h"
#include "TFile.h"



int main(int argc, char* argv[])
{
  //////////////////////////////////////////////////////////////
  //
  // When working with datasets, the gammacombo framework relies on a workspace 
  // as the main reference for data and the statistical model.
  // Therefore, we first must construct the workspace that contains all necessary information.
  // In this tutorial, this is done in the constructWorkspace() function. 
  // In a more complex analysis, you can also do this elsewhere, for example using pyroot.
  //
  /////////////////////////////////////////////////////////////// 

  // Load the workspace from its file
  TFile f("workspace.root");
  RooWorkspace* workspace = (RooWorkspace*)f.Get("dataset_workspace");

  // Construct the PDF and pass the workspace to it
  PDF_Datasets_Abs* pdf = new PDF_DatasetTutorial(workspace);

  // Start the Gammacombo Engine
  GammaComboEngine gc("tutorial_dataset", argc, argv);

  // add the PDF
  gc.addPdf(0, pdf); 
  // there is just a single pdf, and its id is 0

  // Combiners are not supported when working with datsets. 
  // The statistical model is fully defined with the PDF
  gc.run(true);
}

void constructWorkspace()
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
  RooRealVar exponent("exponent","exponent", 0, -1., 1.);
  RooExponential background_model("background_model", "background_model", mass, exponent);

  // The number of signal events is related to the branching ratio via the formula <branching ratio> = <n_sig> * <normalization factor>
  // The normalization factor is not exactly known. Instead, it has to be estimated. The estimator for the normalization factor is a global observable
  // that constraints its value via a Gaussian Constraint. 
  RooRealVar  norm_constant_obs( "norm_constant_glob_obs", "global observable of normalization constant", 
                              7.4e-10, 0., 1e-8);     // this is the observed value, the global observable
  RooRealVar  norm_constant("norm_constant","norm_constant", 7e-10, 0,  1e-8);                   // the normalization constant is a nuisance parameter.
  RooRealVar  norm_constant_sigma("norm_constant_sigma","norm_constant_sigma", 0.5e-10);
  RooGaussian norm_constant_constraint("norm_constant_constraint","norm_constant_constraint", norm_constant_obs, norm_constant, norm_constant_sigma);

  // Now we can build the mass model by adding the signal and background probability density functions
  RooRealVar branchingRatio("branchingRatio", "branchingRatio", 1e-9, 0,  1e-6);  // this is the branching ratio, the parameter of interest
  RooFormulaVar n_sig("Nsig", "branchingRatio/norm_constant", RooArgList(branchingRatio, norm_constant));
  RooRealVar n_bkg("Nbkg","Nbkg", 30, 0, 1000);
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
  RooDataSet& data = *mass_model.generate(RooArgSet(mass),100); // we use a reference here to avoid copying the object
  //
  /////////////////////////////////////////////////////////

  // Set the global observables to the observed values and make them constant for the fit.
  norm_constant_obs.setVal(observedValueGlobalObservable);
  norm_constant_obs.setConstant();

  // The workspace must also include the fit result of an initial fit of the model to data.
  RooFitResult& rooFitResult = *mass_model.fitTo(data,RooFit::Save(), RooFit::ExternalConstraints(RooArgSet(norm_constant_constraint)));

  /////////////////////////////////////////////////////////
  //
  // We also must define some RooArgSets: 
  //
  /////////////////////////////////////////////////////////

  //One containting the constraint PDFs,
  RooArgSet constraint_set(norm_constant_constraint, "constraintPDFs");

  //one containting the global Observables,
  RooArgSet global_observables_set(norm_constant_obs, "globalObservables");

  //one containting the normal Observables and
  RooArgSet normal_observables_set(mass, "datsetObservables");

  //one containting the parameters
  RooArgSet parameters_set(branchingRatio, norm_constant, "parameters");

  /////////////////////////////////////////////////////////
  //
  // Import everything into a workspace and save it
  //
  /////////////////////////////////////////////////////////

  RooWorkspace workspace("dataset_workspace");
  workspace.import(mass_model);
  workspace.import(data);
  workspace.import(rooFitResult, "data_fit_result");
  workspace.defineSet("constraint_set", constraint_set, true);
  workspace.defineSet("global_observables_set", global_observables_set, true);

  // Save the workspace to a file
  workspace.SaveAs("bs24mu_wsp.root");

}