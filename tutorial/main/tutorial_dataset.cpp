#include "GammaComboEngine.h"
#include "TFile.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooWorkspace.h"
#include <iostream>
#include <vector>
// #include "PDF_DatasetTutorial.h"
#include "PDF_DatasetCustom.h"
#include "PDF_DatasetManual.h"

int main(int argc, char* argv[])
{
  //////////////////////////////////////////////////////////////
  // When working with datasets, the gammacombo framework relies on a workspace
  // as the main reference for data and the statistical model.
  // Therefore, we first must construct the workspace that contains all necessary information.
  ///////////////////////////////////////////////////////////////

  // How to run the CLs Limit procedure:
  // 1.) Running a Profile Likelihood Scan
  //        bin/tutorial_dataset --var BFsig --npoints 50 --scanrange 0.:140.e-9
  // 2.) If you want to just remake the plot (without rescanning) add the -a plot option
  //        bin/tutorial_dataset --var BFsig --npoints 50 --scanrange 0.:140.e-9 -a plot
  //
  // 3.) If you want to add the CLs method add the option --cls 1
  //        bin/tutorial_dataset --var BFsig --npoints 50 --scanrange 0.:140.e-9 --cls 1
  //
  // 4.) To do a Feldman Cousins plugin scan (run a bunch in parallel and give them different names with --nrun %d
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 1
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 2
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 3
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 4
  //        bin/tutorial_dataset -a pluginbatch --var BFsig --npoints 50 --scanrange 0.:140.e-9 --ntoys 100 --nrun 5
  // 5.) To read a bunch of Feldman Cousins scans back in (use the -j option to label the different run numbers)
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:140.e-9 -j 1-5
  // 6.) To just plot the Feldman Cousins stuff without having to re-scan or re-read add the -a plot option again
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:140.e-9 -j 1-5 -a plot
  // 7.) The F-C CLs method is a bit overkill (to do the classic CLs thing (with the FC toys) and plot the expected values as well) 
  //     use the --cls 2 option (note you can pass --cls multiple times).
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:140.e-9 -j 1-5 -a plot --cls 1 --cls 2
  // 8.) There are various ways of prettyfying your plots - for CLs stuff you can try adding --qh 23 (moves the CL label) 
  //      --group LHCb (adds LHCb label) --prelim (add preliminary label)
  //        bin/tutorial_dataset -a plugin --var BFsig --npoints 50 --scanrange 0.:140.e-9 -j 1-5 -a plot --cls 1 --cls 2 --qh 23 --group LHCb --prelim
  //
  // If you have any problems contact Matthew Kenzie (matthew.kenzie@cern.ch) or Titus Mombächer (titus.mombacher@cern.ch)

  //////////////////////////////////////
  //=====Parse additional options=====//
  //////////////////////////////////////
  gROOT->SetBatch(true);

  int count=0;
  std::vector<char*> argvTmp;
  bool fullLimitFlag = false;
  bool sanityFlag = false;
  bool blindFlag = false;
  bool freezeFlag = false;
  bool useToy = false;
  bool isManualFit = false;
  bool process = true;
  std::string plotDirectory = "plots/pdf/LikelihoodScan/";
  TString dataName="data";
  std::string workspaceName="";

  for(int i=0; i<argc; i++){
      if(std::string(argv[i]) == "-a"){
          if(std::string(argv[i+1]) == "pluginbatch"){ plotDirectory="plots/pdf/PluginBatch/"; }
          if(std::string(argv[i+1]) == "plugin"){ plotDirectory="plots/pdf/Plugin/"; }
      }
  }

  for(int i=0; i<argc; i++){
      if(std::string(argv[i]) == "--sanity"){ 
          sanityFlag = true;
          count+=1;
      }
      else if(std::string(argv[i]) == "--full"){ 
          fullLimitFlag = true;
          count+=1;
      }
      else if(std::string(argv[i]) == "--freeze"){ 
          freezeFlag = true;
          count+=1;
      }
      else if(std::string(argv[i]) == "--blind"){
          blindFlag = true;
          plotDirectory+="blind/";
          count+=1;
      }
      else if(std::string(argv[i]) == "--toy"){
          useToy = true;
          plotDirectory+="toyData/";
          count+=1;
      }
      else if(std::string(argv[i]) == "--manual"){
          isManualFit = true;
          plotDirectory+="manual/";
          count+=1;
      }
      else if(std::string(argv[i]) == "--unprocessed"){
          process = false;
          count+=1;
      }
      else if(std::string(argv[i]) == "--work"){
          workspaceName=std::string(argv[i+1]);
          count+=2;
          i++;
      }
      else{ argvTmp.push_back(argv[i]); }
  }
  
  int argcNew = argc-count;
  char* argvNew[argcNew];
  for(int i=0; i<argcNew; i++){ argvNew[i] = argvTmp[i]; }

  /////////////////////////////////////////////
  //// Load the workspace from its file    ////
  /////////////////////////////////////////////
  std::string workspace_location = std::getenv("LB2LEMUROOT")+std::string("/gammacombo/tutorial/Workspaces/");
  if(workspaceName==""){ workspaceName = "Lb2Lemu_wsOld_GC3.root";}
  std::cout << "Using workspace: " << (workspace_location+workspaceName).c_str() <<std::endl;
  TFile f((workspace_location+workspaceName).c_str());
  RooWorkspace* workspace = (RooWorkspace*)f.Get("w");

  if (workspace==NULL){
	  std::cout<<"No workspace found:"<<std::endl;
	  std::cout<<"This tutorial requires a .root file containting a special workspace before running it."<<std::endl;
	  std::cout<<"You can create the workspace by calling the tutorial_dataset_build_workspace command. "<<std::endl;
	  std::cout<<"The corresponding code can be found in tutorial_dataset_build_workspace.cpp"<<std::endl;
  }
  //workspace->Print();
  
  /////////////////////////////////////////////
  ////    Pre-Processing before GC run     ////
  /////////////////////////////////////////////
  std::string massVarname="Lb_DTFLambdaPV_M";

  RooArgSet parameters = workspace->allVars();
  workspace->defineSet("parameters", parameters);

  workspace->var("BFsig")->setVal(1e-8);
  workspace->var("BFsig")->setRange(0,140e-9);
  workspace->var("BFsig")->setConstant(false);

  if (process){
      /* //Check parameter values and if they're constant
         auto iterator = parameters.createIterator();
         while (RooRealVar* par = (RooRealVar*)iterator->Next() ) {
         std::cout << par->GetName() << " : " << par->getVal() <<  " Constant: " << par->isConstant() <<std::endl;
         }*/

      std::vector<std::string> signalPar{"a_Brem_DD","a_Brem_LL","a_NoBrem_DD","a_NoBrem_LL","a2_Brem_DD","a2_Brem_LL","a2_NoBrem_DD","a2_NoBrem_LL",
          "l_Brem_DD","l_Brem_LL","l_NoBrem_DD","l_NoBrem_LL","m_Brem_DD","m_Brem_LL","m_NoBrem_DD","m_NoBrem_LL",
          "s_Brem_DD","s_Brem_LL","s_NoBrem_DD","s_NoBrem_LL"};

      //Hold constrained variables constant or make "free"
      auto iterator = workspace->set("constrained_variables")->createIterator();
      while ( RooRealVar* constr_var = (RooRealVar*)iterator->Next() ){
          std::string cnst_free="";
          if(freezeFlag) cnst_free = "constant";
          else{ cnst_free = "free";}
          //std::cout << "Setting " << constr_var->GetName() << " " << cnst_free<<std::endl;

          bool isSignalPar = std::find(signalPar.begin(),signalPar.end(),constr_var->GetName()) != signalPar.end(); 

          if(!isSignalPar){ 
              constr_var->setConstant(freezeFlag); 
              constr_var->setMin(constr_var->getVal() - 5*constr_var->getError());
              constr_var->setMax(constr_var->getVal() + 5*constr_var->getError());
          }
      }
      delete iterator;

      //Setting Some Reasonable Limits
      std::string categories[8] = {"Brem_DD_Run1","Brem_LL_Run1","NoBrem_DD_Run1","NoBrem_LL_Run1","Brem_DD_Run2","Brem_LL_Run2","NoBrem_DD_Run2","NoBrem_LL_Run2"};
      for(int i=0; i<8; i++){
          std::string cat = categories[i];

          auto val = workspace->var(("Nbkg_"+cat).c_str())->getVal();
          workspace->var(("Nbkg_"+cat).c_str())->setMin(0.5*val);
          workspace->var(("Nbkg_"+cat).c_str())->setMax(1.5*val);

          auto val_tau_a = workspace->var(("tau_a_"+cat).c_str())->getVal();
          workspace->var(("tau_a_"+cat).c_str())->setMin(1.5*val_tau_a);
          workspace->var(("tau_a_"+cat).c_str())->setMax(0.5*val_tau_a);

          //auto val_tau_b = workspace->var(("tau_b_"+cat).c_str())->getVal();
          //workspace->var(("tau_b_"+cat).c_str())->setMin(1.5*val_tau_a);
          //workspace->var(("tau_b_"+cat).c_str())->setMax(0.5*val_tay_a);
          //workspace->var(("tau_b_"+cat).c_str())->setMin(1.5*val_tau_b);
          //workspace->var(("tau_b_"+cat).c_str())->setMax(0.5*val_tau_b);
      }
  }

  //Setting the limits for non-constant parameters to free.
  if (fullLimitFlag){
      auto iterator = parameters.createIterator();
      while (RooRealVar* par = (RooRealVar*)iterator->Next() ){
          if(!par->isConstant()){
              if(par->getMax() == 1E30 && par->getMin()==-1E30){ par->setRange("free",1E-5*par->getVal(),1E5*par->getVal()); }
              else{ par->setRange("free", par->getMin(), par->getMax());}
          }
      }
  }

  //////////////////////////////////////////////////////////
  //Create a Toy full dataset to use as BG-only proxy data//
  //////////////////////////////////////////////////////////
  if (useToy){
      OptParser* arg = new OptParser();
      arg->bookAllOptions();
      arg->parseArguments(argcNew, argvNew);

      PDF_Datasets* pdfToy;
      if(isManualFit){ pdfToy= new PDF_DatasetManual(workspace,1,arg); }
      else{ pdfToy= new PDF_DatasetCustom(workspace,1,arg); }

      //PDF_DatasetManual* pdfToy= new PDF_DatasetManual(workspace,1,arg); 
      //PDF_DatasetCustom* pdfToy= new PDF_DatasetCustom(workspace,1,arg); 
      
      RooMsgService::instance().setGlobalKillBelow(ERROR);
      RooMsgService::instance().setSilentMode(kTRUE);

      pdfToy->initData("data"); // this is the name of the dataset in the workspace
      pdfToy->initPDF("model"); // this the name of the pdfToy in the workspace (without the constraints)
      pdfToy->initBkgPDF("bgModel"); // this the name of the background pdfToy in the workspace (without the constraints)
      pdfToy->initObservables("observables"); // non-global observables whose measurements are stored in the dataset (for example the mass).
      pdfToy->initGlobalObservables("constrained_variables"); // global observables
      pdfToy->initParameters("parameters"); // all parameters
      pdfToy->initConstraints("constraints"); // RooArgSet containing the "constraint" PDF's
      pdfToy->addFitObs(massVarname);                         // this is not required but will make some sanity plots
      pdfToy->massVarName=massVarname;
      pdfToy->sanity = sanityFlag;
      pdfToy->blindFlag = true;
      pdfToy->isToyDataset=useToy;
      pdfToy->plotDir = plotDirectory;

      //RooFitResult* dataFitResult = (RooFitResult*)workspace->genobj("data_fit_result");
      //pdfToy->loadExtParameters(dataFitResult);
      RooFitResult* dataToyBGFitResult = (RooFitResult*)workspace->genobj("data_toy_fit_result");
      pdfToy->loadExtParameters(dataToyBGFitResult);
      //pdfToy->fitBkg(pdfToy->getData());
      //RooFitResult* toyFitResult = pdfToy->fitBkg(pdfToy->getData());
      //pdfToy->loadExtParameters(toyFitResult);
      //toyFitResult->SetName("data_fit_result");
      pdfToy->generateBkgToys(10); //Set a seed for a reproducible dataToy set
      RooDataSet* toyData = pdfToy->getBkgToyObservables();
      toyData->SetName("dataToy");

      RooMsgService::instance().setSilentMode(kFALSE);
      RooMsgService::instance().setGlobalKillBelow(INFO);

      workspace->import(*toyData);
      dataName="dataToy";
  }

  // Construct the PDF and pass the workspace to it
  //    note that you can write your own PDF_DatasetsTutorial Class which defines your own fitting procedure etc.
  //    this should inherit from PDF_Datasets

  PDF_Datasets* pdf;
  if(isManualFit){pdf = new PDF_DatasetManual(workspace); }
  else{pdf = new PDF_DatasetCustom(workspace); }

  //PDF_Datasets* pdf = new PDF_Datasets(workspace);
  //PDF_DatasetCustom* pdf = new PDF_DatasetCustom(workspace);
  //PDF_DatasetManual* pdf = new PDF_DatasetManual(workspace);
  pdf->initData(dataName); // this is the name of the dataset in the workspace
  pdf->initPDF("model"); // this the name of the pdf in the workspace (without the constraints)
  pdf->initBkgPDF("bgModel"); // this the name of the background pdf in the workspace (without the constraints)
  pdf->initObservables("observables"); // non-global observables whose measurements are stored in the dataset (for example the mass).
  pdf->initGlobalObservables("constrained_variables"); // global observables
  pdf->initParameters("parameters"); // all parameters
  pdf->initConstraints("constraints"); // RooArgSet containing the "constraint" PDF's
  // the below are optional (will not effect the results but just make some plots for you)
  pdf->addFitObs(massVarname);                         // this is not required but will make some sanity plots
  //pdf->unblind(massVarname,"[4360:5260],[5460:6360]"); // have to be a bit careful about staying blind (this code isn't yet really blind friendly)
  //pdf->unblind("mass", "[4360:6360]" );

  //Additional Options given to custom pdf
  pdf->massVarName = massVarname;
  pdf->plotDir = plotDirectory;
  pdf->sanity = sanityFlag;
  pdf->blindFlag = blindFlag;
  pdf->isToyDataset = useToy;

  system( ("mkdir -p " + plotDirectory).c_str() );

  //pdf->printParameters();

  // Start the Gammacombo Engine
  GammaComboEngine gc("tutorial_dataset", argcNew, argvNew);

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
