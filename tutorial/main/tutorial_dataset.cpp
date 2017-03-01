#include "GammaComboEngine.h"
#include "TFile.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooWorkspace.h"
// #include "PDF_DatasetTutorial.h"

int main(int argc, char* argv[])
{
  //////////////////////////////////////////////////////////////
  //
  // When working with datasets, the gammacombo framework relies on a workspace
  // as the main reference for data and the statistical model.
  // Therefore, we first must construct the workspace that contains all necessary information.
  // In this tutorial, this is done by calling the command tutorial_dataset_build_workspace
  // In a more complex analysis, you can also do this elsewhere, for example using pyroot.
  //
  ///////////////////////////////////////////////////////////////

  // How to run the tutorial:
  // bin/tutorial_dataset_build_workspace

  // bin/tutorial_dataset --var branchingRatio --npoints 20 --scanrange 1e-7:2e-6
  // bin/tutorial_dataset -a pluginbatch --var branchingRatio --ntoys 50  --npoints 20 --scanrange 1e-7:2e-6
  // bin/tutorial_dataset -a plugin --var branchingRatio --npoints 20 --scanrange 1e-7:2e-6 -i


  // Load the workspace from its file
  TFile f("workspace.root");
  RooWorkspace* workspace = (RooWorkspace*)f.Get("dataset_workspace");
  if (workspace==NULL){
	  std::cout<<"No workspace found:"<<std::endl;
	  std::cout<<"This tutorial requires a .root file containting a special workspace before running it."<<std::endl;
	  std::cout<<"You can create the workspace by calling the tutorial_dataset_build_workspace command. "<<std::endl;
	  std::cout<<"The corresponding code can be found in tutorial_dataset_build_workspace.cpp"<<std::endl;
  }

	// You can make any changes to your workspace on the fly here
	workspace->var("branchingRatio")->SetTitle("#font[32]{B}( B^{0}#rightarrow X )");
	workspace->var("branchingRatio")->setVal(1.e-7);
	workspace->var("branchingRatio")->setRange(0.,2.5e-6);
	workspace->var("Nbkg")->SetTitle("N_{bkg}");
	workspace->var("Nbkg")->setVal(5000);
	workspace->var("Nbkg")->setRange(4000,6000);

  // Construct the PDF and pass the workspace to it
  PDF_Datasets* pdf = new PDF_Datasets(workspace);
  // PDF_Datasets* pdf = new PDF_DatasetTutorial(workspace);
  pdf->initData("data"); // this is the name of the dataset in the workspace
  pdf->initPDF("mass_model"); // this the name of the pdf in the workspace (without the constraints)
  pdf->initObservables("datasetObservables"); // non-global observables whose measurements are stored in the dataset (for example the mass).
  pdf->initGlobalObservables("global_observables_set"); // global observables
  pdf->initParameters("parameters"); // all parameters
  pdf->initConstraints("constraint_set"); // RooArgSet containing the "constraint" PDF's

  // Start the Gammacombo Engine
  GammaComboEngine gc("tutorial_dataset", argc, argv);

  // set run on dataset option
  gc.setRunOnDataSet(true);

  // set the PDF
  gc.setPdf(pdf);

  // Combiners are not supported when working with datsets.
  // The statistical model is fully defined with the PDF
  gc.run();
}
