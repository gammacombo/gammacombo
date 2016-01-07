#include <RooGaussian>
#include <RooRealVar>
#include <RooAddPdf>
#include <RooFormula>
#include <RooExponential>
#include "PDF_Dataset.h"



int main(int argc, char* argv[])
{
// When working with datasets, the gammacombo framework relies on a workspace 
// as the main reference for data and the statistical model.
// Therefore, we first must construct the workspace that contains all the necessary information

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
RooRealVar  norm_constant_obs( "norm_constant_glob", "global observable of normalization constant", 
                              7.4e-10, 0., 1e-8);     // this is the observed value, the global observable
RooRealVar  norm_constant("norm_constant","norm_constant", 7e-10, 0,  1e-8);                   // the normalization constant is a nuisance parameter.
RooRealVar  norm_constant_sigma("norm_constant_sigma","norm_constant_sigma", 0.5e-10);
RooGaussian norm_constant_constraint("norm_constant_constraint","norm_constant_constraint", norm_constant_obs, norm_constant, norm_constant_sigma);

// Now we can build the mass model by adding the signal and background probability density functions
RooRealVar branchingRatio("branchingRatio", "branchingRatio", 1e-9, 0,  1e-6);  // this is the branching ratio, the parameter of interest
RooFormula Var n_sig("Nsig", "branchingRatio/norm_constant", ROOT.RooArgList(branchingRatio, norm_constant));
RooRealVar n_bkg("Nbkg","Nbkg", 30, 0, 1000);
RooAddPdf mass_model("mass_model","mass_model", ROOT.RooArgList(signal_model,data_model), ROOT.RooArgList(n_sig,n_bkg));


// We need some data to analyse.
// Because this is a tutorial, we just generate a dataset ourselves.
// We generate one mass dataset with 100 events.
// We also generate a single measurement of the global observable.
double observedValueGlobalObservable = ... = norm_constant.generate(1);
RooDataSet data = ... = mass_model.generate(100);
norm_constant_obs.setVal(observedValueGlobalObservable);
norm_constant_obs.setConstant();

// The workspace must also include the fit result of an initial fit of the model to data.
RooFitResult rooFitResult = fit_model.fitTo(data,ROOT.RooFit.Save(), ROOT.RooFit.ExternalConstraints(ROOT.RooArgSet(norm_constant_constraint)));

// We also must define two RooArgSets: One containting the constraint PDFs,
RooArgSet constraint_set(norm_constant_constraint, "constraintPDFs")
//one containting the global Observables
RooArgSet global_observables_set(norm_constant_obs, "globalObservables")
//one containting the normal Observables and
RooArgSet global_observables_set(mass, "datsetObservables")
//one containting the parameters
RooArgSet global_observables_set(branchingRatio, norm_constant, "parameters")


// Now, a workspace is being defined and als is being imported
RooWorkspace workspace("dataset_workspace")
workspace.import(fit_model)
workspace.import(data)
workspace.import(rooFitResult)
workspace.defineSet("constraintPDFs", constraint_set, True)
workspace.defineSet("globalObservables", observables_set, True)




  // std::vector<TString> observables, parameters;
  // observables.push_back("B_s0_MM");

  // parameters.push_back("exponent");
  // parameters.push_back("Nbkg");
  // parameters.push_back("branchingRatio");
  // parameters.push_back("mean");
  // parameters.push_back("sigma");
  // // parameters.push_back("norm_constant_obs");
  // parameters.push_back("norm_constant");

  
  // Create the PDF object
  PDF_Generic_Abs *pdf = new PDF_Generic_Abs(workspace);
  pdf->setConstraints("constraintPDFs");
  pdf->setGlobalVars("globalObservables");
  pdf->addGlobalConstraint("norm_constant_obs","norm_constant", W->var("norm_constant_obs")->getVal());
  pdf->initPDF("fitModel");
  pdf->initObservables(observables);
  pdf->initParameters(parameters);

  // We pass the probability density function as a fourth argument to the GammaComboEngine
  GammaComboEngine gc("tutorial_dataset", argc, argv, pdf);
  



   // do we really need this? 
  // pdf->setVarRange("branchingRatio","free", 0, 1e-6);
  // pdf->setVarRange("branchingRatio","phys", 0, 1e-6);
  // pdf->setVarRange("branchingRatio","scan", 0, 1e-6);

  // this must be done in the GammaComboEngine
  // pdf->setNToys(2500);


  // this must be done elsewhere  
  // pdf->getWorkspace()->var("mean")->setConstant(true);
  // pdf->getWorkspace()->var("sigma")->setConstant(true);
  // pdf->getWorkspace()->var("norm_constant")->setConstant(true);

  

  // this should move to the RooGenericAbsScan
  //RooFitResult* r = pdf->fit(true);

  
  // All the plotting should move to the gammacombo engine
  // OneMinusClPlot* plot = new OneMinusClPlot(arg, "bs24Mu", "bs24Mu");
  // int scanStatus = 0;

  // cout<<"--lightfiles : option always active"<<endl;
  // arg->lightfiles = true;
  
  // MethodGenericPluginScan *scanner = new MethodGenericPluginScan(pdf, arg, true, result);

  // scanner->initScan();
  
  // if(arg->isAction("plugin")){//<-- You have to pass "-a prob " if this should NOT evaluate to true. Why? no idea.
  //   cout << "plugin option is set, calling MethodGenericPluginScan.readScan1dTrees"<<endl;
  //   scanner->readScan1dTrees(arg->jmin[0], arg->jmax[0]);
  //   // this just reads in simulated Feldman-Coussins events, right?
  //   // where are these events simulated?
  // }
  // else{
  //     cout << "plugin option is not set, calling MethodGenericPluginScan.scan1d()"<<endl;
  //     //(which does a plugin scan, among others, because apparently the --plugin option refers to the caclulation of the FC-intervals using the fit results from a -a pluginbatch run.)"<<endl;
  //   if(!arg->lightfiles){
  //       cout << "FATAL in main() - GenericScan::scan1d() just works with argument --lightfiles" << endl;
  //       exit(1);
  //     }
  //   scanStatus = scanner->scan1d(arg->nrun); // CAUTION! Currently only works with --lightfiles argument!
  //   cout << "scan status: " << scanStatus << endl;

  // }


  // plot->addScanner(scanner);
  
  // MethodProbScan* sc = dynamic_cast<MethodProbScan*>(scanner->getProfileLH());
  // plot->addScanner(sc);
  
  // // if ( arg->plotpulls ) scanner->plotPulls();
  // // // if ( arg->parevol ) scanner->plotParEvolution();
  // scanner->calcCLintervals();
  // sc->calcCLintervals();
  // // scanner->plotOn(plot); // this one is equivalent to plot->addScanner()
  // plot->Draw();
  

  // if ( arg->interactive ) theApp->Run();
}