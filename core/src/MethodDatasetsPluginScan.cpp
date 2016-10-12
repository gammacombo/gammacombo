/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
 *
 * \todo come up with smarter way to fill probValue histo in readScan1dTrees()
 *
 * Some introductory comments:
 * Despite its name, this class combines the Prob Scan and the Plugin Scan functionality. This was maybe a bad choice.
 * It leads to a lot of duplication patterns, methods which exist twice, one with _plugin suffix and once with _prob suffix, or without.
 * Most prominently, it leads to the global doProbScanOnly switch that decides if either a prob scan OR a plugin scan should be performed.
 * Basically, this is a switch which decides if the class is a MethodPluginScan or a MethodProbScan. 
 * 
 * The term "free fit do data" refers to the fit which is performed to data, where the parameter
 * of interest is left to float freely.
 * 
 * The term "constrained fit to data" refers to the fit which is performed to data, where the 
 * parameter of interest is fixed to a certain value (scanpoint)
 */

#include "MethodDatasetsPluginScan.h"
#include "TRandom3.h"
#include <algorithm>
#include <ios>
#include <iomanip>

///
/// The default constructor for the dataset plugin scan
///
MethodDatasetsPluginScan::MethodDatasetsPluginScan(PDF_Datasets* PDF, OptParser* opt):
  MethodPluginScan(opt),
  pdf                 (PDF),
  probPValues         (NULL),
  drawPlots           (false),
  explicitInputFile   (false),
  doProbScanOnly      (false),
  externalProfileLH   (false),
  dataFreeFitResult   (NULL)
  {
  chi2minGlobalFound = true; // the free fit to data must be done and must be saved to the workspace before gammacombo is even called
  methodName = "DatasetsPlugin";
  w = PDF->getWorkspace();
  title = PDF->getTitle();
  name =  PDF->getName();
  
  if ( arg->var.size()>1 ) scanVar2 = arg->var[1];
  inputFiles.clear();

  
  if (w->obj("data_fit_result") == NULL){ //\todo: support passing the name of the fit result in the workspace.
    cerr << "ERROR: The workspace must contain the fit result of the fit to data. The name of the fit result must be 'data_fit_result'. " <<endl;
    exit(EXIT_FAILURE);
  }
  dataFreeFitResult = (RooFitResult*) w->obj("data_fit_result");
  chi2minGlobal = 2*dataFreeFitResult->minNll();
  std::cout << "=============== Global Minimum (2*-Log(Likelihood)) is: 2*" << dataFreeFitResult->minNll() << " = " << chi2minGlobal << endl;

  if ( !w->set(pdf->getObsName()) ) { 
    cerr << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no '" + pdf->getObsName() + "' set found in workspace" << endl; 
    cerr << " You can specify the name of the set in the workspace using the pdf->initObservables(..) method.";
    exit(EXIT_FAILURE); 
  }
  if ( !w->set(pdf->getParName()) ) { 
    cerr << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no '" + pdf->getParName() + "' set found in workspace" << endl; 
    exit(EXIT_FAILURE); 
  }
}

///////////////////////////////////////////////
///
/// Gets values of certain parameters as they were at the given scanpoint-index after the constrained fit to data
/// 
/// \todo: rename this to getParValAtIndex. If necessary, stub out getParValAtScanpoint
///////////////////////////////////////////////
float MethodDatasetsPluginScan::getParValAtScanpoint(int index, TString parName){

  this->probScanTree->GetEntry(index);
  TLeaf* var = this->probScanTree->GetLeaf(parName); //<- pretty sure that this will give a segfault, we need to use parName + "_scan"
  if(!var){
    cout << "MethodDatasetsPluginScan::getParValAtScanpoint() : ERROR : variable (" << parName << ") not found!" << endl;
    exit(EXIT_FAILURE);
  }
  return var->GetValue();
}


void MethodDatasetsPluginScan::initScan(){
  if ( arg->debug ) cout << "MethodDatasetsPluginScan::initScan() : initializing ..." << endl;

  // Init the 1-CL histograms. Range is taken from the scan range, unless
  // the --scanrange command line argument is set.
  RooRealVar *par1 = w->var(scanVar1);
  if ( !par1 ){
    cout << "MethodDatasetsPluginScan::initScan() : ERROR : No such scan parameter: " << scanVar1 << endl;
    cout << "MethodDatasetsPluginScan::initScan() :         Choose an existing one using: --var par" << endl << endl;
    cout << "  Available parameters:" << endl << "  ---------------------" << endl << endl << "  ";
    pdf->printParameters();
    exit(EXIT_FAILURE);
  }
  if ( arg->scanrangeMin != arg->scanrangeMax ) par1->setRange("scan", arg->scanrangeMin, arg->scanrangeMax);
  Utils::setLimit(w, scanVar1, "scan");

  if (hCL) delete hCL;
  hCL = new TH1F("hCL"+getUniqueRootName(), "hCL"+pdf->getPdfName(), nPoints1d, par1->getMin(), par1->getMax());
  if ( hChi2min ) delete hChi2min; 
  hChi2min = new TH1F("hChi2min"+getUniqueRootName(), "hChi2min"+pdf->getPdfName(), nPoints1d, par1->getMin(), par1->getMax());
  
  // fill the chi2 histogram with very unlikely values such
  // that inside scan1d() the if clauses work correctly
  for ( int i=1; i<=nPoints1d; i++ ) hChi2min->SetBinContent(i,1e6);  

  if ( scanVar2!="" ){
    cout << "MethodDatasetsPluginScan::initScan(): EROR: Scanning in more than one dimension is not supported." << std::endl;
    exit(EXIT_FAILURE);
  }
  
  // Set up storage for the fit results.
  // Clear before so we can call initScan() multiple times.
  // Note that allResults still needs to hold all results, so don't delete the RooFitResults.

  curveResults.clear();
  for ( int i=0; i<nPoints1d; i++ ) curveResults.push_back(0);
  
  // turn off some messages
  RooMsgService::instance().setStreamStatus(0,kFALSE);
  RooMsgService::instance().setStreamStatus(1,kFALSE);
  if(arg->debug){
    std::cout << "DEBUG in MethodDatasetsPluginScan::initScan() - Scan initialized successfully!\n" << std::endl;
  }     
}

//////////////////////////////////////////////
///
/// This checks of a TTree which originated from a previous prob scan 
/// for compatibility with the current scan: Did it use the same number
/// of scan points, did it use the same scan range?
/// If everything is fine, keeps a pointer to the tree in this->probScanTree
///
/// \param tree    TTree from probscan, to checked for compatibility and to be assigned to class member
///
//////////////////////////////////////////////
void MethodDatasetsPluginScan::setExtProfileLH(TTree* tree){

  //make sure that the scan points in the tree match number 
  //of scan points and the scan range that we are using now.
  TBranch* b    = (TBranch*)tree->GetBranch("scanpoint");
  int entriesInTree = b->GetEntries();
  if(nPoints1d != entriesInTree){
    std::cout<<"Number of scan points in tree saved from prob scan do not match number of scan points used in plugin scan."<<std::endl;
    exit(EXIT_FAILURE);
  }


  float parameterToScan_min = hCL->GetXaxis()->GetXmin();
  float parameterToScan_max = hCL->GetXaxis()->GetXmax();

  tree->GetEntry(0);
  float minTreePoint = b->GetLeaf("scanpoint")->GetValue();
  if((minTreePoint - parameterToScan_min)/std::max(parameterToScan_max, parameterToScan_min) > 1e-5){
    std::cout<<"Lowest scan point in tree saved from prob scan does not match lowest scan point used in plugin scan."<<std::endl;
    std::cout<<"Alternatively, this could be a problem with the heuristics used for checking the equality of two floats"<<std::endl;
    exit(EXIT_FAILURE); 
  }

  tree->GetEntry(entriesInTree-1);
  float maxTreePoint = b->GetLeaf("scanpoint")->GetValue();
  if((maxTreePoint - parameterToScan_max)/std::max(parameterToScan_max, parameterToScan_min) > 1e-5){
    std::cout<<"Max scan point in tree saved from prob scan probably does not match max scan point used in plugin scan."<<std::endl;
    std::cout<<"Alternatively, this could be a problem with the heuristics used for checking the equality of two floats"<<std::endl;
    exit(EXIT_FAILURE); 
  }

  // if all is fine, assign and proceed.
  probScanTree = tree;
  externalProfileLH = true;
};

///////////////////////////////////////////////////
///
/// Prepare environment depending on data or toy fit
///
/// \param fitToys   boolean switch that decides whether the latest simulated toys or the data should be fitted.
///
/// \param pdf      the pdf that is to be fitted.
///
////////////////////////////////////////////////////
RooFitResult* MethodDatasetsPluginScan::loadAndFit(bool fitToys, PDF_Datasets* pdf){
  if(fitToys){
    // we want to fit to the latest simulated toys
    // first, try to simulated toy values of the global observables from a snapshot
    if(!w->loadSnapshot(pdf->globalObsToySnapshotName)){
      std::cout << "FATAL in MethodDatasetsPluginScan::loadAndFit() - No snapshot globalObsToySnapshotName found!\n" << std::endl;
      exit(EXIT_FAILURE);
    };
    // then, fit the pdf while passing it the simulated toy dataset
    return pdf->fit(pdf->getToyObservables());
  }
  else{
    // we want to fit to data
    // first, try to load the measured values of the global observables from a snapshot
    if(!w->loadSnapshot(pdf->globalObsDataSnapshotName)){
      std::cout << "FATAL in MethodDatasetsPluginScan::loadAndFit() - No snapshot globalObsToySnapshotName found!\n" << std::endl;
      exit(EXIT_FAILURE);
    };
    // then, fit the pdf while passing it the dataset
    return pdf->fit(pdf->getData());
  }
};

///
/// load Parameter limits
/// by default the "free" limit is loaded, can be changed to "phys" by command line argument
///
void MethodDatasetsPluginScan::loadParameterLimits(){
  TString rangeName = arg->enforcePhysRange ? "phys" : "free";
  if ( arg->debug ) cout << "DEBUG in Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << endl;
  TIterator* it = w->set(pdf->getParName())->createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ) setLimit(w, p->GetName(), rangeName);
  delete it;
}




///
/// Print settings member of MethodDatasetsPluginScan
///
void MethodDatasetsPluginScan::print(){
  cout << "########################## Print MethodDatasetsPluginScan Class ##########################" << endl;
  cout << "\t --- " << "Method Name: \t\t\t" << methodName << endl;
  cout << "\t --- " << "Instance Name: \t\t\t" << name << endl;
  cout << "\t --- " << "Instance Title: \t\t\t" << title << endl;
  cout << "\t --- " << "Scan Var Name: \t\t\t" << scanVar1 << endl;
  if ( arg->var.size()>1 )   cout << "\t --- " << "2nd Scan Var Name: \t\t" << scanVar2 << endl;
  cout << "\t --- " << "Number of Scanpoints 1D: \t\t" << nPoints1d <<endl;
  cout << "\t --- " << "Number of Scanpoints x 2D: \t" << nPoints2dx <<endl;
  cout << "\t --- " << "Number of Scanpoints y 2D: \t" << nPoints2dy <<endl;
  cout << "\t --- " << "Number of Toys per scanpoint: \t" << nToys << endl;
  cout << "\t --- " << "PDF Name: \t\t\t\t" << pdf->getPdfName() << endl;
  cout << "\t --- " << "Observables Name: \t\t\t" <<  pdf->getObsName() << endl;
  cout << "\t --- " << "Parameters Name: \t\t\t" <<  pdf->getParName() << endl;
  cout << "\t --- " << "Global minimum Chi2: \t\t" << chi2minGlobal << endl;
  cout << "\t --- " << "nrun: \t\t\t\t" << arg->nrun << endl;
  cout << "---------------------------------" << endl;
  cout << "\t --- Scan Var " << scanVar1 << " from " << getScanVar1()->getMin("scan") 
       << " to " << getScanVar1()->getMax("scan") << endl;
  cout << "---------------------------------" << endl;
}


///
/// Compute the p-value at a certain point in parameter space using
/// the plugin method. The precision of the p-value will depend on
/// the number of toys that were generated, more than 100 should
/// be a good start (ntoys command line option).
///
/// \param runMin   defines lowest run number of toy jobs to read in
///
/// \param runMax   defines highest run number of toy jobs to read in 
TChain* MethodDatasetsPluginScan::readFiles(int runMin, int runMax, int &nFilesRead, int &nFilesMissing, TString fileNameBaseIn){
///
  TChain *c = new TChain("plugin");
  int _nFilesRead = 0;

  if(doProbScanOnly){
    TString file = Form("root/scan1dDatasetsProb_"+this->pdf->getName()+"_%ip"+"_"+scanVar1+".root", arg->npoints1d);
    Utils::assertFileExists(file);
    c->Add(file);
    _nFilesRead = 1;
  
  }else{
    TString dirname = "root/scan1dDatasetsPlugin_"+this->pdf->getName()+"_"+scanVar1;
    TString fileNameBase = (fileNameBaseIn.EqualTo("default")) ? dirname+"/scan1dDatasetsPlugin_"+this->pdf->getName()+"_"+scanVar1+"_run" : fileNameBaseIn;
    
    if(explicitInputFile){
      for(TString &file : inputFiles){
        Utils::assertFileExists(file);
        c->Add(file);
        _nFilesRead += 1;
      }
    }
    else{
      for (int i=runMin; i<=runMax; i++){
        TString file = Form(fileNameBase+"%i.root", i);
        cout << "MethodDatasetsPluginScan::readFiles() : opening " << file << "\r";
        Utils::assertFileExists(file);
        c->Add(file);
        _nFilesRead += 1;
      }
    }
  }

  nFilesRead = _nFilesRead;
  if ( nFilesRead==0 ){
    cout << "MethodDatasetsPluginScan::readFiles() : no files read!" << endl;
    exit(EXIT_FAILURE);
  }
  cout << "MethodDatasetsPluginScan::readFiles() : read files: " << nFilesRead << endl;
  return c;
}

//////////
/// readScan1dTrees implements inherited method
/// \param runMin   defines lowest run number of toy jobs to read in
///
/// \param runMax   defines highest run number of toy jobs to read in 
///
/// \param fileNameBaseIn    optional, define the directory from which the files are to be read
///
/// This is the Plugin version of the method, there is also a version of the method for the prob scan, with _prob suffix.
/// \todo: Like for the normal gammacombo stuff, use a seperate class for the PROB scan! This is MethodDatasetsPLUGINScan.cpp, after all
/////////////
void MethodDatasetsPluginScan::readScan1dTrees(int runMin, int runMax, TString fileNameBaseIn)
{
  int nFilesRead, nFilesMissing;
  TChain* c = this->readFiles(runMin, runMax, nFilesRead, nFilesMissing, fileNameBaseIn);
  ToyTree t(this->pdf, this->arg, c);
  t.open();
  
  float halfBinWidth = (t.getScanpointMax()-t.getScanpointMin())/((float)t.getScanpointN())/2;//-1.)/2;
  /// \todo replace this such that there's always one bin per scan point, but still the range is the scan range.
  /// \todo Also, if we use the min/max from the tree, we have the problem that they are not exactly
  /// the scan range, so that the axis won't show the lowest and highest number.
  /// \todo If the scan range was changed after the toys were generate, we absolutely have
  /// to derive the range from the root files - else we'll have bining effects.
  delete hCL;
  hCL = new TH1F("hCL", "hCL", t.getScanpointN(), t.getScanpointMin()-halfBinWidth, t.getScanpointMax()+halfBinWidth);
  if(arg->debug){
    printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin()-halfBinWidth, t.getScanpointMax()+halfBinWidth, halfBinWidth);
  }
  // histogram to store number of toys which enter p Value calculation
  TH1F *h_better        = (TH1F*)hCL->Clone("h_better");
  // numbers for all toys
  TH1F *h_all           = (TH1F*)hCL->Clone("h_all");
  // numbers of toys failing the selection criteria
  TH1F *h_failed        = (TH1F*)hCL->Clone("h_failed");
  // numbers of toys which are not in the physical region dChi2<0
  TH1F *h_background    = (TH1F*)hCL->Clone("h_background");
  // histo for GoF test
  TH1F *h_gof           = (TH1F*)hCL->Clone("h_gof");
  // likelihood scan p values
  TH1F *h_probPValues   = (TH1F*)hCL->Clone("h_probPValues");
  // total number of toys
  TH1F *h_tot           = (TH1F*)hCL->Clone("h_tot");
  // histogram illustrating the failure rate
  TH1F *h_fracGoodToys  = (TH1F*)hCL->Clone("h_fracGoodToys");
  TH1F *h_pVals         = new TH1F("p","p", 200, 0.0, 1e-2);
  Long64_t nentries     = t.GetEntries();
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : average number of toys per scanpoint: " << (double) nentries / (double)nPoints1d << endl;
  Long64_t nfailed      = 0;
  Long64_t nwrongrun    = 0;
  Long64_t n0better     = 0;
  Long64_t n0all        = 0;
  Long64_t n0tot        = 0;
  Long64_t n0failed     = 0;
  Long64_t totFailed    = 0;

  
  float printFreq = nentries>101 ? 100 : nentries;    ///< for the status bar
  t.activateCoreBranchesOnly();                       ///< speeds up the event loop
  TString alternateTestStatName = scanVar1+"_free";
  t.activateBranch(alternateTestStatName);
  for (Long64_t i = 0; i < nentries; i++)
  {
    // status bar
    if (((int)i % (int)(nentries/printFreq)) == 0)
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading entries " 
           << Form("%.0f",(float)i/(float)nentries*100.) << "%   \r" << flush;
    // load entry
    t.GetEntry(i);
    
    bool valid    = true;

    h_tot->Fill(t.scanpoint);
    if(t.scanpoint == 0.0) n0tot++;
    // criteria for GammaCombo
    bool convergedFits      = (t.statusFree==0. && t.statusScan==0.);
    bool tooHighLikelihood  = !( abs(t.chi2minToy)<1e27 && abs(t.chi2minGlobalToy)<1e27);
    
    // apply cuts
    if ( tooHighLikelihood || !convergedFits  )
    {
      h_failed->Fill(t.scanpoint);
      if(t.scanpoint == 0) n0failed++;
      valid = false;
      nfailed++;
      //continue;
    }
    
    // Check if toys are in physical region.
    // Don't enforce t.chi2min-t.chi2minGlobal>0, else it can be hard because due
    // to little fluctuaions the best fit point can be missing from the plugin plot...
    bool inPhysicalRegion     = ((t.chi2minToy-t.chi2minGlobalToy) >= 0 ); 
    
    // build test statistic
    if ( valid && (t.chi2minToy-t.chi2minGlobalToy) >= (t.chi2min - this->chi2minGlobal) ){ //t.chi2minGlobal ){
      h_better->Fill(t.scanpoint);
    }
      if(t.scanpoint == 0.0) n0better++;

    // goodness-of-fit
    if ( inPhysicalRegion && t.chi2minGlobalToy > this->chi2minGlobal ){ //t.chi2minGlobal ){
      h_gof->Fill(t.scanpoint);
    }
    // all toys
    if ( valid || doProbScanOnly){//inPhysicalRegion ){
      // not efficient! TMath::Prob evaluated each toy, only needed once.
      // come up with smarter way
      h_all->Fill(t.scanpoint);
      h_probPValues->SetBinContent(h_probPValues->FindBin(t.scanpoint), this->getPValueTTestStatistic(t.chi2min-this->chi2minGlobal)); //t.chi2minGlobal));
      if(t.scanpoint == 0.0) n0all++;
    }

    // use the unphysical events to estimate background (be careful with this,
    // at least inspect the control plots to judge if this can be at all reasonable)
    if ( valid && !inPhysicalRegion ){
      h_background->Fill(t.scanpoint);
    }
    
    if(n0tot % 1500 == 0 && n0all!=0){
      //cout << "better: " << n0better << " all: " << n0all << " p: " << (float)n0better/(float)n0all << endl << endl;
      h_pVals->Fill((float)n0better/(float)n0all);
      n0tot = 0;
      n0better = 0;
      n0all = 0;
    }
  }
  cout << std::fixed << std::setprecision(2);
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading done.           \n" << endl;
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : read an average of " << ((double)nentries-(double)nfailed)/(double)nPoints1d << " toys per scan point." << endl;
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of failed toys: " << (double)nfailed/(double)nentries*100. << "%." << endl;
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of background toys: " << h_background->GetEntries()/(double)nentries*100. << "%." << endl;
  if ( nwrongrun>0 ){
    cout << "\nMethodDatasetsPluginScan::readScan1dTrees() : WARNING : Read toys that differ in global chi2min (wrong run) : "
          << (double)nwrongrun/(double)(nentries-nfailed)*100. << "%.\n" << endl;
  }
  
  for (int i=1; i<=h_better->GetNbinsX(); i++){
    float nbetter = h_better->GetBinContent(i);
    float nall = h_all->GetBinContent(i);
    // get number of background and failed toys
    float nbackground     = h_background->GetBinContent(i);

    nfailed       = h_failed->GetBinContent(i);

    //nall = nall - nfailed + nbackground;
    float ntot = h_tot->GetBinContent(i);
    if ( nall == 0. ) continue;
    h_background->SetBinContent(i,nbackground/nall);
    h_fracGoodToys->SetBinContent(i,(nall)/(float)ntot);
    // subtract background
    // float p = (nbetter-nbackground)/(nall-nbackground);
    // hCL->SetBinContent(i, p);
    // hCL->SetBinError(i, sqrt(p * (1.-p)/(nall-nbackground)));
    
    // don't subtract background
    float p = nbetter/nall;
    hCL->SetBinContent(i, p);
    hCL->SetBinError(i, sqrt(p * (1.-p)/nall));
  cout << "At scanpoint "<< std::scientific << hCL->GetBinCenter(i)<<": ===== number of toys for pValue calculation: " << nbetter << endl;
  cout << "At scanpoint "<<hCL->GetBinCenter(i)<<": ===== pValue: " << p << endl;
  }

  if(arg->debug || drawPlots){
    TCanvas* can = new TCanvas("can","can",1024,786);
    can->cd();
    gStyle->SetOptTitle(0);
    //gStyle->SetOptStat(0);
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadBottomMargin(0.17);
    gStyle->SetPadLeftMargin(0.16);
    gStyle->SetLabelOffset(0.015, "X");
    gStyle->SetLabelOffset(0.015, "Y");
    h_fracGoodToys->SetXTitle(scanVar1);
    h_fracGoodToys->SetYTitle("fraction of good toys");
    h_fracGoodToys->Draw();
    TCanvas *canvas = new TCanvas("canvas","canvas",1200,1000);
    canvas->Divide(2,2);
    canvas->cd(1);
    h_all->SetXTitle("h_all");
    h_all->SetYTitle("number of toys");
    h_all->Draw();
    canvas->cd(2);
    h_better->SetXTitle("h_better");
    h_better->Draw();
    canvas->cd(3);
    h_gof->SetXTitle("h_gof");
    h_gof->Draw();
    canvas->cd(4);
    h_background->SetXTitle("h_bkg");
    h_background->SetYTitle("fraction of neg. test stat toys");
    h_background->Draw();
  }

  this->profileLH = new MethodProbScan(pdf, this->getArg(), h_probPValues);
  // goodness-of-fit
 
  int iBinBestFit = hCL->GetMaximumBin();
  float nGofBetter = h_gof->GetBinContent(iBinBestFit);
  float nall = h_all->GetBinContent(iBinBestFit);
  float fitprobabilityVal = nGofBetter/nall;
  float fitprobabilityErr = sqrt(fitprobabilityVal * (1.-fitprobabilityVal)/nall);
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : fit prob of best-fit point: "
    << Form("(%.1f+/-%.1f)%%", fitprobabilityVal*100., fitprobabilityErr*100.) << endl;
}


void MethodDatasetsPluginScan::readScan1dTrees_prob()
{
  int nFilesRead, nFilesMissing;
  TChain* c = this->readFiles(1, 1, nFilesRead, nFilesMissing, TString("default"));
  ToyTree t(this->pdf, this->arg, c);
  t.open();
  
  float halfBinWidth = (t.getScanpointMax()-t.getScanpointMin())/((float)t.getScanpointN())/2;//-1.)/2;
  /// \todo replace this such that there's always one bin per scan point, but still the range is the scan range.
  /// \todo Also, if we use the min/max from the tree, we have the problem that they are not exactly
  /// the scan range, so that the axis won't show the lowest and highest number.
  /// \todo If the scan range was changed after the toys were generate, we absolutely have
  /// to derive the range from the root files - else we'll have bining effects.
  delete hCL;
  hCL = new TH1F("hCL", "hCL", t.getScanpointN(), t.getScanpointMin()-halfBinWidth, t.getScanpointMax()+halfBinWidth);
  if(arg->debug){
    printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin()-halfBinWidth, t.getScanpointMax()+halfBinWidth, halfBinWidth);
  }
 
  TH1F *h_probPValues   = (TH1F*)hCL->Clone("h_probPValues");
  Long64_t nentries     = t.GetEntries();
 

  t.activateCoreBranchesOnly();                       ///< speeds up the event loop
  TString alternateTestStatName = scanVar1+"_free";
  t.activateBranch(alternateTestStatName);
  for (Long64_t i = 0; i < nentries; i++)
  {
    // status bar
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading entries " 
           << Form("%.0f",(float)i/(float)nentries*100.) << "%   \r" << flush;
    // load entry
    t.GetEntry(i);
    h_probPValues->SetBinContent(h_probPValues->FindBin(t.scanpoint), this->getPValueTTestStatistic(t.chi2min-this->chi2minGlobal)); //t.chi2minGlobal));
  }
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading done.           \n" << endl;

  this->profileLH = new MethodProbScan(pdf, this->getArg(), h_probPValues);
  // goodness-of-fit
}


///
/// Switch between prob and plugin scan.
/// For the datasets stuff, we do not yet have a MethodDatasetsProbScan class, so we do it all in 
/// MethodDatasetsPluginScan
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodDatasetsPluginScan::scan1d(int nRun)
{
  if(doProbScanOnly){
    cout<<"CALLING MethodDatasetsPluginScan::scan1d_prob"<<std::endl;
    this->scan1d_prob();
  } else {
    cout<<"CALLING MethodDatasetsPluginScan::scan1d_plugin"<<std::endl;
    this->scan1d_plugin(nRun);
  }

}


///
/// Perform the 1d Prob scan.
/// Saves chi2 values and the prob-Scan p-values in a root tree
/// For the datasets stuff, we do not yet have a MethodDatasetsProbScan class, so we do it all in 
/// MethodDatasetsPluginScan
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
void MethodDatasetsPluginScan::scan1d_prob()
{
  // Necessary for parallelization 
  RooRandom::randomGenerator()->SetSeed(0);
  // Set limit to all parameters. 
  this->loadParameterLimits(); /// Default is "free", if not changed by cmd-line parameter


  // Define scan parameter and scan range.
  RooRealVar *parameterToScan = w->var(scanVar1);
  float parameterToScan_min = hCL->GetXaxis()->GetXmin();
  float parameterToScan_max = hCL->GetXaxis()->GetXmax();

  double freeDataFitValue = w->var(scanVar1)->getVal();

  // Define outputfile   
  system("mkdir -p root");
  TString probResName = Form("root/scan1dDatasetsProb_"+this->pdf->getName()+"_%ip"+"_"+scanVar1+".root", arg->npoints1d);
  TFile* outputFile = new TFile(probResName, "RECREATE");  
  
  // Set up toy root tree
  ToyTree probTree(this->pdf, arg);
  probTree.init();
  probTree.nrun = -999; //\todo: why does this branch even exist in the output tree of the prob scan?
  
  // Save parameter values that were active at function
  // call. We'll reset them at the end to be transparent
  // to the outside.
  RooDataSet* parsFunctionCall = new RooDataSet("parsFunctionCall", "parsFunctionCall", *w->set(pdf->getParName()));
  parsFunctionCall->add(*w->set(pdf->getParName()));


  // start scan
  cout << "MethodDatasetsPluginScan::scan1d_prob() : starting ... with " << nPoints1d << " scanpoints..." << endl;
  ProgressBar progressBar(arg, nPoints1d);
  for ( int i=0; i<nPoints1d; i++ )
  {
    progressBar.progress();
    // scanpoint is calculated using min, max, which are the hCL x-Axis limits set in this->initScan()
    // this uses the "scan" range, as expected 
    // don't add half the bin size. try to solve this within plotting method

    float scanpoint = parameterToScan_min + (parameterToScan_max-parameterToScan_min)*(double)i/((double)nPoints1d-1);
  
    probTree.scanpoint = scanpoint;
    
    if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d_prob() - scanpoint in step " << i << " : " << scanpoint << endl;

    // don't scan in unphysical region
    // by default this means checking against "free" range
    if ( scanpoint < parameterToScan->getMin() || scanpoint > parameterToScan->getMax()+2e-13 ){ 
      cout << "it seems we are scanning in an unphysical region: " << scanpoint << " < " << parameterToScan->getMin() << " or " << scanpoint << " > " << parameterToScan->getMax()+2e-13 << endl;
      exit(EXIT_FAILURE);
    }

    // FIT TO REAL DATA WITH FIXED HYPOTHESIS(=SCANPOINT).
    // THIS GIVES THE NUMERATOR FOR THE PROFILE LIKELIHOOD AT THE GIVEN HYPOTHESIS
    // THE RESULTING NUISANCE PARAMETERS TOGETHER WITH THE GIVEN HYPOTHESIS ARE ALSO 
    // USED WHEN SIMULATING THE TOY DATA FOR THE FELDMAN-COUSINS METHOD FOR THIS HYPOTHESIS(=SCANPOINT)
    // Here the scanvar has to be fixed -> this is done once per scanpoint 
    // and provides the scanner with the DeltaChi2 for the data as reference
    // additionally the nuisances are set to the resulting fit values
    
    parameterToScan->setVal(scanpoint);
    parameterToScan->setConstant(true);
    
    RooFitResult *result = this->loadAndFit(kFALSE, this->pdf); // false -> fit on data
    assert(result);

    if(arg->debug){ 
      cout << "DEBUG in MethodDatasetsPluginScan::scan1d_prob() - minNll data scan at scan point " << scanpoint << " : " << 2*result->minNll() << endl;
    }
    probTree.statusScanData = result->status();
    
    // set chi2 of fixed fit: scan fit on data
    probTree.chi2min           = 2*result->minNll();
    probTree.covQualScanData   = result->covQual();
    probTree.scanbest  = freeDataFitValue;

    // After doing the fit with the parameter of interest constrained to the scanpoint,
    // we are now saving the fit values of the nuisance parameters. These values will be
    // used to generate toys according to the PLUGIN method.
    probTree.storeParsScan(); // \todo : figure out which one of these is semantically the right one

    this->pdf->deleteNLL();
    
    // also save the chi2 of the free data fit to the tree:
    probTree.chi2minGlobal = this->getChi2minGlobal();
    
    probTree.genericProbPValue = this->getPValueTTestStatistic(probTree.chi2min-probTree.chi2minGlobal);
    probTree.fill();
  
    // reset
    setParameters(w, pdf->getParName(), parsFunctionCall->get(0));
    //setParameters(w, pdf->getObsName(), obsDataset->get(0));
    probTree.writeToFile();
  } // End of npoints loop


  outputFile->Write();
  outputFile->Close();
  std::cout<<"Wrote ToyTree to file"<<std::endl;
  delete parsFunctionCall;
}

double MethodDatasetsPluginScan::getPValueTTestStatistic(double test_statistic_value){
    if ( test_statistic_value > 0){
      // this is the normal case
      return TMath::Prob(test_statistic_value,1);
    } else {
      cout << "MethodDatasetsPluginScan::scan1d_prob() : WARNING : Test statistic is negative, forcing it to zero" << std::endl
           << "Fit at current scan point has higher likelihood than free fit." << std::endl
           << "This should not happen except for very small underflows when the scan point is at the best fit value. " << std::endl
           << "Value of test statistic is " << test_statistic_value << std::endl
           << "An equal upwards fluctuaion corresponds to a p value of " << TMath::Prob(abs(test_statistic_value),1) << std::endl;
           // TMath::Prob will return 0 if the Argument is slightly below zero. As we are working with a float-zero we can not rely on it here:
           // TMath::Prob( 0 ) returns 1
      return 1.;
    }
}


///
/// Perform the 1d Plugin scan.
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
void MethodDatasetsPluginScan::scan1d_plugin(int nRun)
{

  // //current working directory
  // boost::filesystem::path full_path( boost::filesystem::initial_path<boost::filesystem::path>() );
  // std::cout<<"initial path according to boost "<<full_path<<std::endl;

  // Necessary for parallelization 
  RooRandom::randomGenerator()->SetSeed(0);
  // Set limit to all parameters. 
  this->loadParameterLimits(); /// Default is "free", if not changed by cmd-line parameter


  // Define scan parameter and scan range.
  RooRealVar *parameterToScan = w->var(scanVar1);
  float parameterToScan_min = hCL->GetXaxis()->GetXmin();
  float parameterToScan_max = hCL->GetXaxis()->GetXmax();
  double freeDataFitValue = w->var(scanVar1)->getVal();

  TString probResName = Form("root/scan1dDatasetsProb_"+this->pdf->getName()+"_%ip"+"_"+scanVar1+".root", arg->npoints1d);
  TFile* probResFile = TFile::Open(probResName);
  if(!probResFile){
      std::cout << "ERROR in MethodDatasetsPluginScan::scan1d - Prob scan result file not found in " << std::endl
                << probResName << std::endl
                << "Please run the prob scan before running the plugin scan. " << std::endl
                << "The result file of the prob scan can be specified via the --probScanResult command line argument." << std::endl;
      exit(EXIT_FAILURE);
  }
  this->setExtProfileLH((TTree*) probResFile->Get("plugin"));

  // Define outputfile
  TString dirname = "root/scan1dDatasetsPlugin_"+this->pdf->getName()+"_"+scanVar1;
  system("mkdir -p " + dirname);
  TFile* outputFile = new TFile(Form(dirname+"/scan1dDatasetsPlugin_"+this->pdf->getName()+"_"+scanVar1+"_run%i.root", nRun),"RECREATE");

  // Set up toy root tree
  ToyTree toyTree(this->pdf, arg);
  toyTree.init();
  toyTree.nrun = nRun;
  
  // Save parameter values that were active at function
  // call. We'll reset them at the end to be transparent
  // to the outside.
  RooDataSet* parsFunctionCall = new RooDataSet("parsFunctionCall", "parsFunctionCall", *w->set(pdf->getParName()));
  parsFunctionCall->add(*w->set(pdf->getParName()));

      
  // start scan
  cout << "MethodDatasetsPluginScan::scan1d_plugin() : starting ... with " << nPoints1d << " scanpoints..." << endl;
  ProgressBar progressBar(arg, nPoints1d);
  for ( int i=0; i<nPoints1d; i++ )
  {
    progressBar.progress();
    // scanpoint is calculated using min, max, which are the hCL x-Axis limits set in this->initScan()
    // this uses the "scan" range, as expected 
    // don't add half the bin size. try to solve this within plotting method

    float scanpoint = parameterToScan_min + (parameterToScan_max-parameterToScan_min)*(double)i/((double)nPoints1d-1);	
    toyTree.scanpoint = scanpoint;
    
    if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - scanpoint in step " << i << " : " << scanpoint << endl;

    // don't scan in unphysical region
    // by default this means checking against "free" range
    if ( scanpoint < parameterToScan->getMin() || scanpoint > parameterToScan->getMax()+2e-13 ){ 
      cout << "not obvious: " << scanpoint << " < " << parameterToScan->getMin() << " and " << scanpoint << " > " << parameterToScan->getMax()+2e-13 << endl;
      continue;
    }

    // Load the parameter values (nuisance parameters and parameter of interest) from the fit to data with fixed parameter of interest.
    this->setParevolPointByIndex(i);

    toyTree.statusScanData  = this->getParValAtScanpoint(i, "statusScanData");
    toyTree.chi2min         = this->getParValAtScanpoint(i, "chi2min");
    toyTree.covQualScanData = this->getParValAtScanpoint(i, "covQualScanData");

    // get the chi2 of the data
    if(this->chi2minGlobalFound){
      toyTree.chi2minGlobal     = this->getChi2minGlobal();
    }
    else{
      cout << "FATAL in MethodDatasetsPluginScan::scan1d_plugin() - Global Minimum not set!" << endl;
      exit(EXIT_FAILURE);
    }
    
    toyTree.storeParsPll();
    toyTree.genericProbPValue = this->getPValueTTestStatistic(toyTree.chi2min-toyTree.chi2minGlobal);
    
    for ( int j = 0; j<nToys; j++ )
    {
      if(arg->debug) cout << ">> new toy\n" << endl;
      this->pdf->setMinNllFree(0);
      this->pdf->setMinNllScan(0);
      
      // 1. Generate toys

      // For toy generation, set all parameters (nuisance parameters and parameter of interest) to the values from the constrained 
      // fit to data with fixed parameter of interest.
      // This is called the PLUGIN method.
      this->setParevolPointByIndex(i);


      this->pdf->generateToys(); // this is generating the toy dataset
      this->pdf->generateToysGlobalObservables(); // this is generating the toy global observables and saves globalObs in snapshot

      // \todo: comment the following back in once I know what it does ...
      //      t.storeParsGau( we need to pass a rooargset of the means of the global observables here);  

      //
      // 2. Fit to toys with parameter of interest fixed to scanpoint
      //
      if(arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - perform scan toy fit" << endl;

      // set parameters to constrained data scan fit result again
      this->setParevolPointByIndex(i);

      // fixed parameter of interest
      parameterToScan->setConstant(true);
      this->pdf->setFitStrategy(0);
      RooFitResult* r   = this->loadAndFit(kTRUE,this->pdf); // kTrue makes sure the fit is to toy data and to toy global observables
      assert(r);
      pdf->setMinNllScan(pdf->minNll);

      this->setAndPrintFitStatusFreeToys(toyTree);

      if (pdf->getFitStatus()!=0) {
          pdf->setFitStrategy(1);
          delete r;
          r = this->loadAndFit(kTRUE,this->pdf);
          pdf->setMinNllScan(pdf->minNll);
          assert(r);

          this->setAndPrintFitStatusFreeToys(toyTree);

          if (pdf->getFitStatus()!=0) {
            pdf->setFitStrategy(2);
            delete r;
            r = this->loadAndFit(kTRUE,this->pdf);
            assert(r);
          }
      }   
  
      if (std::isinf(pdf->minNll) || std::isnan(pdf->minNll)) {
        cout  << "++++ > second fit gives inf/nan: "  << endl
          << "++++ > minNll: " << pdf->minNll << endl
          << "++++ > status: " << pdf->getFitStatus() << endl;
        pdf->setFitStatus(-99);
      }        
      pdf->setMinNllScan(pdf->minNll);

      
      toyTree.chi2minToy          = 2*r->minNll(); // 2*r->minNll(); //2*r->minNll();
      toyTree.chi2minToyPDF       = 2*pdf->getMinNllScan();
      toyTree.covQualScan         = r->covQual();
      toyTree.statusScan          = r->status();
      toyTree.statusScanPDF       = pdf->getFitStatus(); //r->status();
      toyTree.storeParsScan();

      pdf->deleteNLL();
      
      RooDataSet* parsAfterScanFit = new RooDataSet("parsAfterScanFit", "parsAfterScanFit", *w->set(pdf->getParName()));
      parsAfterScanFit->add(*w->set(pdf->getParName()));
      
      
      //
      // 3. Fit to toys with free parameter of interest
      //
      if(arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - perform free toy fit" << endl;
      // Use parameters from the scanfit to data
      
      this->setParevolPointByIndex(i);

      // free parameter of interest
      parameterToScan->setConstant(false);
      
      // Fit
      pdf->setFitStrategy(0);
      RooFitResult* r1  = this->loadAndFit(kTRUE,this->pdf); // kTrue makes sure the fit is to toy data and to toy global observables
      assert(r1);
      pdf->setMinNllFree(pdf->minNll);
      toyTree.chi2minGlobalToy = 2*r1->minNll();

      if (! std::isfinite(pdf->getMinNllFree())) {
        cout << "----> nan/inf flag detected " << endl;
        cout << "----> fit status: " << pdf->getFitStatus() << endl; 
        pdf->setFitStatus(-99);
      }

      bool negTestStat = toyTree.chi2minToy-toyTree.chi2minGlobalToy<0;
      
      this->setAndPrintFitStatusConstrainedToys(toyTree);


      if(pdf->getFitStatus()!=0 || negTestStat ) {
      
        pdf->setFitStrategy(1);
        
        cout << "----> refit with strategy: 1" << endl;
        delete r1;
        r1  = this->loadAndFit(kTRUE,this->pdf);
        assert(r1);
        pdf->setMinNllFree(pdf->minNll);
        toyTree.chi2minGlobalToy = 2*r1->minNll();
        if (! std::isfinite(pdf->getMinNllFree())) {
          cout << "----> nan/inf flag detected " << endl;
          cout << "----> fit status: " << pdf->getFitStatus() << endl; 
          pdf->setFitStatus(-99);
        } 
        negTestStat = toyTree.chi2minToy-toyTree.chi2minGlobalToy<0;

        this->setAndPrintFitStatusConstrainedToys(toyTree);

        if (pdf->getFitStatus()!=0 || negTestStat ) {

          pdf->setFitStrategy(2);
      
          cout << "----> refit with strategy: 2" << endl;
          delete r1;
          r1  = this->loadAndFit(kTRUE,this->pdf);
          assert(r1);
          pdf->setMinNllFree(pdf->minNll); 
          toyTree.chi2minGlobalToy = 2*r1->minNll();
          if (! std::isfinite(pdf->getMinNllFree())) {
            cout << "----> nan/inf flag detected " << endl;
            cout << "----> fit status: " << pdf->getFitStatus() << endl; 
            pdf->setFitStatus(-99);
          }
          this->setAndPrintFitStatusConstrainedToys(toyTree);
        
          if( (toyTree.chi2minToy-toyTree.chi2minGlobalToy) < 0){
            cout << "+++++ > still negative test statistic after whole procedure!! " << endl;
            cout << "+++++ > try to fit with different starting values" << endl;
            cout << "+++++ > dChi2: " << toyTree.chi2minToy-toyTree.chi2minGlobalToy << endl; 
            cout << "+++++ > dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;
            Utils::setParameters(this->pdf->getWorkspace(), pdf->getParName(), parsAfterScanFit->get(0));
            if(parameterToScan->getVal() < 1e-13) parameterToScan->setVal(0.67e-12);
            parameterToScan->setConstant(false); 
            pdf->deleteNLL();
            RooFitResult* r_tmp = this->loadAndFit(kTRUE,this->pdf);
            assert(r_tmp);
            if(r_tmp->status()==0 && r_tmp->minNll()<r1->minNll() && r_tmp->minNll()>-1e27){
              pdf->setMinNllFree(pdf->minNll); 
              cout << "+++++ > Improvement found in extra fit: Nll before: " << r1->minNll() 
              << " after: " << r_tmp->minNll() << endl;
              delete r1;
              r1 = r_tmp;
              cout << "+++++ > new minNll value: " << r1->minNll() << endl;
            }
            else{
              // set back parameter value to last fit value
              cout << "+++++ > no Improvement found, reset ws par value to last fit result" << endl;
              parameterToScan->setVal(static_cast<RooRealVar*>(r1->floatParsFinal().find(parameterToScan->GetName()))->getVal());
              delete r_tmp;
            }
            delete parsAfterScanFit;
          };
          if(arg->debug){
            cout  << "===== > compare free fit result with pdf parameters: " << endl;
            cout  << "===== > minNLL for fitResult: " << r1->minNll() << endl 
                  << "===== > minNLL for pdfResult: " << pdf->getMinNllFree() << endl
                  << "===== > status for pdfResult: " << pdf->getFitStatus() << endl
                  << "===== > status for fitResult: " << r1->status() << endl;
          }
        }
      }

      toyTree.chi2minGlobalToy    = 2*r1->minNll(); //2*r1->minNll();
      toyTree.chi2minGlobalToyPDF = 2*pdf->getMinNllFree(); //2*r1->minNll();
      toyTree.statusFreePDF       = pdf->getFitStatus(); //r1->status();
      toyTree.statusFree          = r1->status();
      toyTree.covQualFree         = r1->covQual();
      toyTree.scanbest            = ((RooRealVar*)w->set(pdf->getParName())->find(scanVar1))->getVal();
      toyTree.storeParsFree();
      pdf->deleteNLL();

      if(arg->debug){
        cout << "#### > Fit summary: " << endl;
        cout  << "#### > free fit status: " << toyTree.statusFree << " vs pdf: " << toyTree.statusFreePDF << endl 
              << "#### > scan fit status: " << toyTree.statusScan << " vs pdf: " << toyTree.statusScanPDF<< endl 
              << "#### > free min nll: " << toyTree.chi2minGlobalToy << " vs pdf: " << toyTree.chi2minGlobalToyPDF << endl 
              << "#### > scan min nll: " << toyTree.chi2minToy << " vs pdf: " << toyTree.chi2minToyPDF << endl 
              << "#### > dChi2 fitresult: " << toyTree.chi2minToy-toyTree.chi2minGlobalToy << endl
              << "#### > dChi2 pdfresult: " << toyTree.chi2minToyPDF-toyTree.chi2minGlobalToyPDF << endl;
        cout  << std::setprecision(6);
      
        if(toyTree.chi2minToy - toyTree.chi2minGlobalToy > 20 && (toyTree.statusFree==0 && toyTree.statusScan==0) 
            && toyTree.chi2minToy>-1e27 && toyTree.chi2minGlobalToy>-1e27){
          cout << std::setw(30) << std::setfill('-') << ">>> HIGH test stat value!! print fit results with fit strategy: "<< pdf->getFitStrategy() << std::setfill(' ') << endl;
          cout << "SCAN FIT Result" << endl;
          r->Print("");
          cout << "================" << endl;
          cout << "FREE FIT result" << endl;
          r1->Print("");
        }

        cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - ToyTree 2*minNll free fit: " << toyTree.chi2minGlobalToy << endl;
      }

      //
      // 4. store
      //
      toyTree.fill();
      //remove dataset and pointers
      delete r;
      delete r1;
      pdf->deleteToys();
    } // End of toys loop
    
    // reset
    setParameters(w, pdf->getParName(), parsFunctionCall->get(0));
    //delete result;
    
    //setParameters(w, pdf->getObsName(), obsDataset->get(0));
    toyTree.writeToFile();
  } // End of npoints loop
  outputFile->Write();
  outputFile->Close();
  delete parsFunctionCall;
}



void MethodDatasetsPluginScan::drawDebugPlots(int runMin, int runMax, TString fileNameBaseIn){
  int nFilesRead, nFilesMissing;
  TChain* c = this->readFiles(runMin, runMax, nFilesRead, nFilesMissing, fileNameBaseIn);
  //ToyTree t(this->pdf, c);
  //t.open();
  cout << "does it take long?" << endl;

  TString cut = "scanpoint == 0 && statusScan == 0 && statusFree == 0 && abs(chi2minToy)<300e3 && abs(chi2minGlobalToy)<300e3";
  TString isphysical = "(chi2minToy-chi2minGlobalToy)>=0";
  TCanvas* can = new TCanvas("can","DChi2Nominal",1024,786);
  TCanvas* can1 = new TCanvas("can1","BR_{Bd}",1024,786);
  TCanvas* can3 = new TCanvas("can3","Chi2distr",1024,786);

  TCanvas* can2 = new TCanvas("can2","DChi2False",1024,786);
  can->cd();
  chain->Draw("chi2minToy-chi2minGlobalToy", cut + "&&" + isphysical + " && abs(chi2minToy-chi2minGlobalToy)<1e2", "norm");
  can1->cd();
  chain->Draw("BR_{Bd}_free",cut + "&&" + isphysical, "norm");
  can2->cd();
  chain->Draw("chi2minToy-chi2minGlobalToy", "!("+cut + "&&" + isphysical+") && abs(chi2minToy-chi2minGlobalToy)<1e2", "norm");
  can3->cd();
  c->Draw("chi2minToy",cut, "norm");
  c->Draw("chi2minGlobalToy",cut, "normSAME");
  //cout << "draw takes a while" << endl;
};





///
/// Assumption: root file is given to the scanner which only has toy at a specific scanpoint, not necessary!
///
void MethodDatasetsPluginScan::performBootstrapTest(int nSamples, const TString& ext){
  TRandom3 rndm;
  TH1F* hist = new TH1F("h","h",800,1e-4,0.008);
  bootstrapPVals.clear();
  int nFilesRead(0), nFilesMissing(0);
  this->readFiles(arg->jmin[0], arg->jmax[0], 
                  nFilesRead, nFilesMissing, arg->jobdir);
  ToyTree t(this->pdf, this->arg, this->chain);
  t.open();
  t.activateCoreBranchesOnly(); ///< speeds up the event loop

  TString cut = "";
  // Define cuts
  cut += "scanpoint == 0"; 
  cut += " && statusScan == 0"; 
  cut += " && statusFree == 0";
  cut += " && abs(chi2minToy)<1e27";
  cut += " && abs(chi2minGlobalToy)<1e27";
  //cut += " && (chi2minToy-2*chi2minGlobalToy)>=0";

  double numberOfToys  = chain->GetEntries(cut);

  //numberOfToys = 500;

  std::vector<int> failed;
  std::vector<double> q;
  std::vector<double> q_Status_gt0;
  failed.clear();
  int totFailed = 0;
  // define bootstrap sample
  double q_data = 0;
  for(int i = 0; i<t.GetEntries(); i++){
    t.GetEntry(i);
    if(i==0){ 
      q_data = t.chi2min-t.chi2minGlobal;
      cout << "Test stat for data: " << q_data << endl;
    }
    if(!(t.statusScan == 0 && t.statusFree == 0 && fabs(t.chi2minToy)<1e27 
      && fabs(t.chi2minGlobalToy)<1e27 && t.scanpoint == 0))
    { 
        totFailed++;
        /*
        if( (t.statusFree == 0 && t.statusScan ==1) 
            || (t.statusFree == 1 && t.statusScan ==0) 
            || (t.statusFree == 1 && t.statusScan ==1)){
          cout  << "Check test stat for status>0: dChi2 = " 
                << t.chi2minToy-t.chi2minGlobalToy << endl;
          q_Status_gt0.push_back(t.chi2minToy-t.chi2minGlobalToy);
        }
        */
        failed.push_back(i);
        continue;
    }

    q.push_back(t.chi2minToy-t.chi2minGlobalToy);

  }
  cout  << "INFO in MethodDatasetsPluginScan::performBootstrapTest - Tree loop finished" << endl;
  cout  << "- start BootstrapTest with " << nSamples 
        << " Samples and " << numberOfToys << " Toys each" << endl;
  cout  << " Total number failed: " << totFailed << endl;

  for(int i = 0; i<nSamples; ++i){
    int nSelected   = 0;
    double nbetter  = 0;
    for(int j=0; j<numberOfToys; j++){

      int rndmInt = -1;
      do{
        rndmInt = rndm.Integer(numberOfToys);
      }while(std::find(failed.begin(), failed.end(), rndmInt)!=failed.end()); 

      if ( (q[rndmInt]) > q_data ) nbetter += 1;
    }
    double p = nbetter/numberOfToys;
    bootstrapPVals.push_back(p);
    hist->Fill(p);
    if(i % 100 == 0) cout << i << " Samples from " << nSamples << " done. p Value: " << p << " with " << nbetter << " Toys of " << numberOfToys << " total" << endl;
  }
  TCanvas* c = new TCanvas("c","c",1024,768);
  hist->SetLineColor(kRed+2);
  hist->SetLineWidth(2);
  hist->Fit("gaus");
  hist->Draw();

  c->SaveAs(Form("plots/root/"+name+"_bootStrap_%i_samples_with_%i_toys_"+ext+".root",nSamples,numberOfToys));
  c->SaveAs(Form("plots/C/"+name+"_bootStrap_%i_samples_with_%i_toys_"+ext+".C",nSamples,numberOfToys));
  c->SaveAs(Form("plots/pdf/"+name+"_bootStrap_%i_samples_with_%i_toys_"+ext+".pdf",nSamples,numberOfToys));
  c->SaveAs(Form("plots/png/"+name+"_bootStrap_%i_samples_with_%i_toys_"+ext+".png",nSamples,numberOfToys));

  return;

};

void MethodDatasetsPluginScan::printDebug(const RooFitResult& r){
    cout << std::fixed << std::scientific;
    cout << std::setw(42) << std::right << std::setfill('-') << " Minimum: "  << std::setprecision(8) << r.minNll() 
    << " with edm: " << std::setprecision(6) << r.edm() << endl;
    cout  << std::setw(42) << std::right << std::setfill('-') 
          << " Minimize status: " << r.status() << endl;
    cout  << std::setw(42) << std::right << std::setfill('-') 
          << " Number of invalid NLL evaluations: " << r.numInvalidNLL() << endl;
    cout  << std::resetiosflags(std::ios::right) 
          << std::resetiosflags(std::ios::fixed) 
          << std::resetiosflags(std::ios::scientific);
};


RooSlimFitResult* MethodDatasetsPluginScan::getParevolPoint(float scanpoint){
  std::cout << "ERROR: not implemented for MethodDatasetsPluginScan, use setParevolPointByIndex() instad" <<std::endl;
  exit(EXIT_FAILURE);
}


///
/// Load the param. values from the data-fit at a certain scan point
///
void MethodDatasetsPluginScan::setParevolPointByIndex(int index){


  this->probScanTree->GetEntry(index);
  RooArgSet* pars          = (RooArgSet*)this->pdf->getWorkspace()->set(pdf->getParName());

  //\todo: make sure this is checked during pdf init, do not check again here
  if(!pars){
    cout << "MethodDatasetsPluginScan::setParevolPointByIndex(int index) : ERROR : no parameter set found in workspace!" << endl; 
    exit(EXIT_FAILURE);
  }
  
  TIterator* it = pars->createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ){
    TString parName     = p->GetName();
    TLeaf* parLeaf      = (TLeaf*)this->probScanTree->GetLeaf(parName+"_scan");
    if(!parLeaf){
      cout << "MethodDatasetsPluginScan::setParevolPointByIndex(int index) : ERROR : no var (" << parName
      << ") found in PLH scan file!" << endl;
      exit(EXIT_FAILURE);
    }
    float scanParVal    = parLeaf->GetValue();
    p->setVal(scanParVal);
  }
}



void MethodDatasetsPluginScan::setAndPrintFitStatusConstrainedToys(const ToyTree& toyTree){

  if(pdf->getMinNllScan() != 0 && (pdf->getMinNllFree() > pdf->getMinNllScan())){
    // create unique failureflag
    switch(pdf->getFitStatus())
    {
      case 0: 
        pdf->setFitStatus(-13);
        break;
      case 1:
        pdf->setFitStatus(-12);
        break;
      case -1:
        pdf->setFitStatus(-33);
        break;
      case -99:
        pdf->setFitStatus(-66);
        break;
      default:
        pdf->setFitStatus(-100);
        break;

    }
  }

  bool negTestStat = toyTree.chi2minToy-toyTree.chi2minGlobalToy<0;

  if(pdf->getFitStatus()!=0 || negTestStat ) {
    cout  << "----> problem in current fit: going to refit with strategy "<< pdf->getFitStrategy() << " , summary: " << endl
          << "----> NLL value: " << std::setprecision(9) << pdf->getMinNllFree() << endl
          << "----> fit status: " << pdf->getFitStatus() << endl
          << "----> dChi2: " << (toyTree.chi2minToy-toyTree.chi2minGlobalToy) << endl 
          << "----> dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;

    switch(pdf->getFitStatus()){
      case 1:
        cout << "----> fit results in status 1" << endl;
        cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
        break;

      case -1:
        cout << "----> fit results in status -1" << endl;
        cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
        break;

      case -99: 
        cout << "----> fit has NLL value with flag NaN or INF" << endl;
        cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
        break;
      case -66:
        cout  << "----> fit has nan/inf NLL value and a negative test statistic" << endl
              << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
              << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
              << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
        break;
      case -13:
        cout  << "----> free fit has status 0 but creates a negative test statistic" << endl
              << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
              << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
              << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
        break;
      case -12:
        cout  << "----> free fit has status 1 and creates a negative test statistic" << endl
              << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
              << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
              << "----> free fit min nll:" << pdf->getMinNllFree() << endl;

        break;
      case -33:
        cout  << "----> free fit has status -1 and creates a negative test statistic" << endl
              << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
              << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
              << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
        cout  << std::setprecision(6);
        break;
      default:
        cout << "-----> unknown / fitResult neg test stat, but status" << pdf->getFitStatus() << endl; 
      break;
    }
  }
}


void MethodDatasetsPluginScan::setAndPrintFitStatusFreeToys(const ToyTree& toyTree){

  if (! std::isfinite(pdf->getMinNllScan())) {
        cout << "----> nan/inf flag detected " << endl;
        cout << "----> fit status: " << pdf->getFitStatus() << endl; 
        pdf->setFitStatus(-99);
      }      

  if (pdf->getFitStatus()!=0) {
      cout  << "----> problem in current fit: going to refit with strategy 1, summary: " << endl
      << "----> NLL value: " << std::setprecision(9) << pdf->minNll << endl
      << "----> fit status: " << pdf->getFitStatus() << endl;
      switch(pdf->getFitStatus()){
        case 1:
          cout << "----> fit results in status 1" << endl;
          cout << "----> NLL value: " << pdf->minNll << endl;
          // cout << "----> edm: " << r->edm() << endl;
          break;

        case -1:
          cout << "----> fit results in status -1" << endl;
          cout << "----> NLL value: " << pdf->minNll << endl;
          // cout << "----> edm: " << r->edm() << endl;
          break;

        case -99: 
          cout << "----> fit has NLL value with flag NaN or INF" << endl;
          cout << "----> NLL value: " << pdf->minNll << endl;
          // cout << "----> edm: " << r->edm() << endl;
          break;
        
        default:
          cout << "unknown" << endl; 
          break;
      }
    }
  }