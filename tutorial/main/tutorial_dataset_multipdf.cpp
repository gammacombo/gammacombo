#include "GammaComboEngine.h"
#include "TFile.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooWorkspace.h"
// #include "PDF_DatasetTutorial.h"

int main(int argc, char* argv[])
{

  // This script initialises GammaCombo with a RooMultiPdf
  // This allows the calculation of the limit on a dataset which includes the systematic uncertainty due to model choice, using the discrete profiling method https://arxiv.org/abs/1408.6865
  //
  // Note that this is very similar to initialising with a standard pdf:
  //    - initPDF is called in the same way, with the multipdf name
  //    - The only difference is the need to call pdf->initMultipdfCat to initialise the category which indexes the multipdf 
  //
  // It is run in the same way as tutorial_dataset.cpp:
  // 1. Profile likelihood scan - For a multipdf, this will not return the correct limit, but is necessary setup for the plugin scan
  // bin/tutorial_dataset_multipdf --var branchingRatio --npoints 20 --scanrange 0.:5.e-7
  // 2. Plugin scan
  // bin/tutorial_dataset_multipdf -a pluginbatch --var branchingRatio --npoints 20 --npointstoy 20 --scanrange 0.:5.e-7 --ntoys 50
  // bin/tutorial_dataset_multipdf -a plugin --var branchingRatio --npoints 20 --npointstoy 20 --scanrange 0.:5.e-7

  // Load the workspace from its file
  TFile f("multipdfworkspace.root");
  RooWorkspace* workspace = (RooWorkspace*)f.Get("workspace");
  if (workspace==NULL){
	  std::cout<<"No workspace found:"<<std::endl;
	  std::cout<<"This tutorial requires a .root file containting a special workspace before running it."<<std::endl;
	  std::cout<<"You can create the workspace by calling the multipdf_build_workspace command. "<<std::endl;
	  std::cout<<"The corresponding code can be found in multipdf_build_workspace.cpp"<<std::endl;
  }

	// You can make any changes to your workspace on the fly here


  PDF_Datasets* pdf = new PDF_Datasets(workspace);
  pdf->initData("data"); // this is the name of the dataset in the workspace
  pdf->initPDF("roomultipdf"); // this the name of the pdf in the workspace (without the constraints)
  pdf->initMultipdfCat("pdf_index"); // this is the only extra thing that needs initialising for a multipdf
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

  // now run it
  gc.run();
}
