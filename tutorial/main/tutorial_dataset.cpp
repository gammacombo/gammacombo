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

  // 1.) Running a Profile Likelihood Scan
  //        bin/tutorial_dataset --var branchingRatio --npoints 50 --scanrange 0.:1.e-6
  // 2.) If you want to just remake the plot (without rescanning) add the -a plot option
  //        bin/tutorial_dataset --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -a plot
  // 3.) If you want to add the CLs method add the option --cls 1
  //        bin/tutorial_dataset --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --cls 1
  // 4.) To do a Feldman Cousins plugin scan (run a bunch in parallel and give them different names with --nrun %d
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 1
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 2
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 3
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 4
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 5
  // 5.) To read a bunch of Feldman Cousins scans back in (use the -j option to label the different run numbers)
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5
  // 6.) To just plot the Feldman Cousins stuff without having to re-scan or re-read add the -a plot option again
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot
  // 7.) The F-C CLs method is a bit overkill (to do the classic CLs thing (with the FC toys) and plot the expected values as well) use the --cls 2 option (note you can pass --cls multiple times)
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot --cls 1 --cls 2
  // 8.) There are various ways of prettyfying your plots - for CLs stuff you can try adding --qh 23 (moves the CL label) --group LHCb (adds LHCb label) --prelim (add preliminary label)
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot --cls 1 --cls 2 --qh 23 --group LHCb --prelim
  //
  // If you have any problems contact Matthew Kenzie (matthew.kenzie@cern.ch) or Titus Mombächer (titus.mombacher@cern.ch)

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
	workspace->var("branchingRatio")->setRange(-1.e-6,2.5e-6);
	workspace->var("Nbkg")->SetTitle("N_{bkg}");
	workspace->var("Nbkg")->setVal(5000);
	workspace->var("Nbkg")->setRange(4000,6000);

  // Construct the PDF and pass the workspace to it
  //    note that you can write your own PDF_DatasetsTutorial Class which defines your own fitting procedure etc.
  //    this should inherit from PDF_Datasets

  PDF_Datasets* pdf = new PDF_Datasets(workspace);
  // PDF_Datasets* pdf = new PDF_DatasetTutorial(workspace); // put your inherited fitter if you want to
  pdf->initData("data"); // this is the name of the dataset in the workspace
  pdf->initBkgPDF("extended_bkg_model"); // this the name of the background pdf in the workspace (without the constraints)
  pdf->initPDF("mass_model"); // this the name of the pdf in the workspace (without the constraints)
  pdf->initObservables("datasetObservables"); // non-global observables whose measurements are stored in the dataset (for example the mass).
  pdf->initGlobalObservables("global_observables_set"); // global observables
  pdf->initParameters("parameters"); // all parameters
  pdf->initConstraints("constraint_set"); // RooArgSet containing the "constraint" PDF's
  // the below are optional (will not effect the results but just make some plots for you)
  pdf->addFitObs("mass");                         // this is not required but will make some sanity plots
  //pdf->unblind("mass","[4360:5260],[5460:6360]"); // have to be a bit careful about staying blind (this code isn't yet really blind friendly)
  pdf->unblind("mass", "[4360:6360]" );

  // pdf->printParameters();

  // Start the Gammacombo Engine
  GammaComboEngine gc("tutorial_dataset", argc, argv);

  // set run on dataset option
  gc.setRunOnDataSet(true);

  // set the PDF
  gc.setPdf(pdf);

  // Combiners are not supported when working with datsets.
  // The statistical model is fully defined with the PDF
  // In some other use cases you will see lines like
  // gc.newCombiner(1, "Combiner Name", "Combiner Title", 2,3,4 );
  // these have no meaning in the datasets case

  // now run it
  gc.run();
}
