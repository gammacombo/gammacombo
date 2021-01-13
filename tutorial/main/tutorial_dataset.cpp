#include "GammaComboEngine.h"
#include "TFile.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooWorkspace.h"
#include <iostream>
// #include "PDF_DatasetTutorial.h"
#include "PDF_DatasetCustom.h"
#include "RooMinimizer.h"

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
  //
  // 3.) If you want to add the CLs method add the option --cls 1
  //        bin/tutorial_dataset --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --cls 1
  //        bin/tutorial_dataset --var BFsig --npoints 50 --scanrange 0.:140.e-9 --cls 1
  //
  // 4.) To do a Feldman Cousins plugin scan (run a bunch in parallel and give them different names with --nrun %d
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 1 --debug --verbose
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 1
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 2
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 3
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 4
  //        bin/tutorial_dataset -a pluginbatch --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 --ntoys 100 --nrun 5
  // 5.) To read a bunch of Feldman Cousins scans back in (use the -j option to label the different run numbers)
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:1.4e-7 -j 1-5
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5
  // 6.) To just plot the Feldman Cousins stuff without having to re-scan or re-read add the -a plot option again
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot
  // 7.) The F-C CLs method is a bit overkill (to do the classic CLs thing (with the FC toys) and plot the expected values as well) use the --cls 2 option (note you can pass --cls multiple times)
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:1.4e-7 -j 1-5 -a plot --cls 1 --cls 2
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot --cls 1 --cls 2
  // 8.) There are various ways of prettyfying your plots - for CLs stuff you can try adding --qh 23 (moves the CL label) --group LHCb (adds LHCb label) --prelim (add preliminary label)
  //        bin/tutorial_dataset -a plugin --var branchingRatio --npoints 50 --scanrange 0.:1.e-6 -j 1-5 -a plot --cls 1 --cls 2 --qh 23 --group LHCb --prelim
  //
  // If you have any problems contact Matthew Kenzie (matthew.kenzie@cern.ch) or Titus Mombächer (titus.mombacher@cern.ch)

  //ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(10000);

  std::string massVarname="Lb_DTFLambdaPV_M";
  std::string brems = "Brem";
  std::string TrackType = "LL";
  std::string Run = "Run2";
  std::string category = brems+"_"+TrackType+"_"+Run;
  std::string data_string = "data_"+category;
  std::string sb_string = "model_"+category;
  std::string bg_string = "bkg_"+category;
  std::string fit_string = "data_fit_result_"+category;
  std::string workspace_location = std::getenv("LB2LEMUROOT")+std::string("/gammacombo/tutorial/Workspaces/");
  //std::string workspace_name = "Lb2Lemu_wsOld_WithPDFs.root";
  //std::string workspace_name = "Lb2Lemu_wsOld_gammaCombo2.root";
  std::string workspace_name = "Lb2Lemu_wsOld_GC3.root";

  std::cout << "Using workspace: " << (workspace_location+workspace_name).c_str() <<std::endl;

  // Load the workspace from its file
  //TFile f("workspace.root");
  //RooWorkspace* workspace = (RooWorkspace*)f.Get("dataset_workspace");
  //TFile f("/afs/cern.ch/work/p/pswallow/private/Lb2LemuAna/gammacombo/tutorial/bin/Lb2Lemu_ws_withPDFs_Old.root");
  //TFile f((std::getenv("LB2LEMUROOT")+std::string("/gammacombo/tutorial/Workspaces/Lb2Lemu_wsOld_WithPDFs.root")).c_str());
  TFile f((workspace_location+workspace_name).c_str());
  RooWorkspace* workspace = (RooWorkspace*)f.Get("w");

  if (workspace==NULL){
	  std::cout<<"No workspace found:"<<std::endl;
	  std::cout<<"This tutorial requires a .root file containting a special workspace before running it."<<std::endl;
	  std::cout<<"You can create the workspace by calling the tutorial_dataset_build_workspace command. "<<std::endl;
	  std::cout<<"The corresponding code can be found in tutorial_dataset_build_workspace.cpp"<<std::endl;
  }
  //workspace->Print();

  workspace->var("BFsig")->setVal(1e-8);
  workspace->var("BFsig")->setRange(0,140e-9);

  RooArgSet parameters = workspace->allVars();
  workspace->defineSet("parameters", parameters);
  //RooFitResult* fit_result = (RooFitResult*)workspace->genobj(fit_string.c_str());
  //fit_result->SetName("data_fit_result");
  //workspace->import(*fit_result, "data_fit_result");

  /*
  auto iterator = parameters.createIterator();
  while (RooRealVar* par = (RooRealVar*)iterator->Next() ) {
    std::cout << par->GetName() << " : " << par->getVal() <<  " Constant: " << par->isConstant() <<std::endl;
  }
  */

  std::string categories[8] = {"Brem_DD_Run1","Brem_LL_Run1","NoBrem_DD_Run1","NoBrem_LL_Run1","Brem_DD_Run2","Brem_LL_Run2","NoBrem_DD_Run2","NoBrem_LL_Run2"};

  int freeze = 0;
  workspace->var("BFsig")->setConstant(0);
  workspace->var(massVarname.c_str())->setConstant(0);
  workspace->var("Nnorm")->setConstant(freeze);
  workspace->var("BFnorm")->setConstant(freeze);
  workspace->var("frac_run1")->setConstant(freeze);
  workspace->var("frac_DD_run1")->setConstant(freeze);
  workspace->var("frac_DD_run2")->setConstant(freeze);
  workspace->var("frac_brem_run1_LL")->setConstant(freeze);
  workspace->var("frac_brem_run1_DD")->setConstant(freeze);
  workspace->var("frac_brem_run2_LL")->setConstant(freeze);
  workspace->var("frac_brem_run2_DD")->setConstant(freeze);
  
  for(int i=0; i<8; i++){
      std::string cat = categories[i];
      std::string brem="";
      std::string track="";
      std::string run="";
      if(cat.find("NoBrem") != std::string::npos){ brem="NoBrem";}
      else{ brem="Brem";}
      if(cat.find("LL") != std::string::npos){ track="LL";}
      else{ track="DD";}
      if(cat.find("Run1") != std::string::npos){ run="Run1";}
      else{ run="Run2";}

      workspace->var(("f_"+cat).c_str())->setConstant(0);
      workspace->var(("m_"+brem+"_"+track).c_str())->setConstant(freeze);
      workspace->var(("s_"+brem+"_"+track).c_str())->setConstant(freeze);
      workspace->var(("a_"+brem+"_"+track).c_str())->setConstant(freeze);
      workspace->var(("a2_"+brem+"_"+track).c_str())->setConstant(freeze);
      workspace->var(("eff_sig_"+cat).c_str())->setConstant(freeze);
      workspace->var(("eff_norm_"+track+"_"+run).c_str())->setConstant(freeze);
      workspace->var(("Nbkg_"+cat).c_str())->setConstant(0);
      workspace->var(("tau_a_"+cat).c_str())->setConstant(0);
      workspace->var(("tau_b_"+cat).c_str())->setConstant(0);

      auto val = workspace->var(("Nbkg_"+cat).c_str())->getVal();
      workspace->var(("Nbkg_"+cat).c_str())->setMin(0.5*val);
      workspace->var(("Nbkg_"+cat).c_str())->setMax(1.5*val);
      val = workspace->var(("tau_a_"+cat).c_str())->getVal();
      workspace->var(("tau_a_"+cat).c_str())->setMin(1.5*val);
      workspace->var(("tau_a_"+cat).c_str())->setMax(0.5*val);
  }

  //std::cout << ((RooFitResult*)workspace->obj("data_fit_result"))->floatParsFinal() <<std::endl;

  //workspace->Print();

  //std::cout << "Wait for input..." <<std::endl;
  //cin.get(); 

  // Construct the PDF and pass the workspace to it
  //    note that you can write your own PDF_DatasetsTutorial Class which defines your own fitting procedure etc.
  //    this should inherit from PDF_Datasets


  //PDF_Datasets* pdf = new PDF_Datasets(workspace);
  PDF_DatasetCustom* pdf = new PDF_DatasetCustom(workspace);
  pdf->initData("data"); // this is the name of the dataset in the workspace
  pdf->initPDF("model"); // this the name of the pdf in the workspace (without the constraints)
  pdf->initBkgPDF("bgModel"); // this the name of the background pdf in the workspace (without the constraints)
  pdf->initObservables("observables"); // non-global observables whose measurements are stored in the dataset (for example the mass).
  //pdf->initGlobalObservables("global_observables"); // global observables
  pdf->initGlobalObservables("constrained_variables"); // global observables
  pdf->initParameters("parameters"); // all parameters
  pdf->initConstraints("constraints"); // RooArgSet containing the "constraint" PDF's
  // the below are optional (will not effect the results but just make some plots for you)
  pdf->addFitObs(massVarname);                         // this is not required but will make some sanity plots
  //pdf->unblind(massVarname,"[4360:5260],[5460:6360]"); // have to be a bit careful about staying blind (this code isn't yet really blind friendly)
  //pdf->unblind("mass", "[4360:6360]" );
  pdf->massVarName=massVarname;

  //pdf->plotDir="FitRangePlots/pdf/LikelihoodScan/";
  pdf->plotDir="FullRangePlots/pdf/LikelihoodScan/";
  pdf->sanity = false;
  for(int i=0; i<argc; i++){
      std::string tmpStr(argv[i]);
      if(tmpStr == "pluginbatch") pdf->plotDir="plots/pdf/Toy/";
      if(tmpStr == "--sanity") pdf->sanity = true;
  }

  //pdf->printParameters();

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
