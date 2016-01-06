/*
 * Gamma Combination
 * Author: Maximilian Schlupp, max.schlupp@cern.ch
 * Date: November 2013
 *
 * DEBUG mode produces a lot of output, do not use with many toys!
 * 
 * ToDo:
 *      - come up with smarter way to fill probValue histo in readScan1dTrees()
 */

#include "MethodDatasetsPluginScan.h"
#include "TRandom3.h"
#include <algorithm>
#include <ios>
#include <iomanip>
///
/// This should be the default for the GenericScan
///
MethodDatasetsPluginScan::MethodDatasetsPluginScan(PDF_Datasets_Abs* PDF, OptParser* opt, bool provideFitResult, RooFitResult* result)
{
  obsDataset          = 0;
  methodName          = "DatasetsPlugin";
  combiner            = NULL;
  pdf                 = PDF;
  w                   = pdf->getWorkspace();
  name                = pdf->getName();
  title               = pdf->getTitle();
  arg                 = opt;
  scanVar1            = arg->var[0];
  if ( arg->var.size()>1 ) scanVar2 = arg->var[1];
  verbose             = arg->verbose;
  drawSolution        = 0;
  nToys               = arg->ntoys;
  nPoints1d           = arg->npoints1d;
  nPoints2dx          = arg->npoints2dx;
  nPoints2dy          = arg->npoints2dy;
  pdfName             = pdf->getPdfName();
  obsName             = pdf->getObsName();
  parsName            = pdf->getParName();
  chi2minGlobal       = 0.0;
  chi2minGlobalFound  = false;
  lineStyle           = 0;
  lineColor           = kBlue-8;
  textColor           = kBlack;
  hCL                 = 0;
  hCL2d               = 0;
  hChi2min            = 0;
  hChi2min2d          = 0;
  obsDataset          = 0;
  startPars           = 0;
  globalMin           = 0;
  nWarnings           = 0;
  probPValues         = 0;
  drawPlots           = false;
  explicitInputFile   = false;
  doProbScanOnly      = false;
  externalProfileLH   = false;
  dataFreeFitResult   = NULL;
  fileBase            = "none";
  inputFiles.clear();        
  if(provideFitResult){
    chi2minGlobal      = 2*result->minNll();
    std::cout << "=============== Global Minimum (2*-Log(Likelihood)) set to: 2*" << result->minNll() << " = " << chi2minGlobal << endl;
    chi2minGlobalFound = true;
    dataFreeFitResult = result;
  }
  // check workspace content IS PDF CHECK NECESSARY? Don't think so for generic scan!
  //if ( !w->pdf(pdfName) ) { cout << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : not found in workspace : " << pdfName  << endl; exit(1); }
  if ( !w->set(obsName) ) { cout << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no 'obsName' set found in workspace : " << obsName << endl; exit(1); }
  if ( !w->set(parsName) ){ cout << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no 'parsName' set found in workspace : " << parsName << endl; exit(1); }

}

float MethodDatasetsPluginScan::getParValAtScanpoint(float point, TString parName){
  TBranch* b    = (TBranch*)this->profileLHPoints->GetBranch("scanpoint");
  int entries   = b->GetEntries();
  for(int i = 0; i<entries; i++){
    this->profileLHPoints->GetEntry(i);
    TLeaf* l          = b->GetLeaf("scanpoint");
    float treePoint   = l->GetValue();
    float diff = (point != 0) ? fabs((treePoint-point)/point) : fabs((treePoint-point)/1e-10);
    
    if(diff < 1e-5){
      TLeaf* var      = this->profileLHPoints->GetLeaf(parName);
      if(!var){
        cout << "MethodDatasetsPluginScan::getParValAtScanpoint() : ERROR : variable (" << parName << ") found!" << endl;
        return -1e12; 
      }
      return var->GetValue();
      }
  }
};
void MethodDatasetsPluginScan::initScan(){
  if ( arg->debug ) cout << "MethodDatasetsPluginScan::initScan() : initializing ..." << endl;

  // Init the 1-CL histograms. Range is taken from the scan range, unless
  // the --scanrange command line argument is set.
  RooRealVar *par1 = w->var(scanVar1);
  if ( !par1 ){
    cout << "MethodDatasetsPluginScan::initScan() : ERROR : No such scan parameter: " << scanVar1 << endl;
    cout << "MethodDatasetsPluginScan::initScan() :         Choose an existing one using: --var par" << endl << endl;
    cout << "  Available parameters:" << endl;
    cout << "  ---------------------" << endl << endl << "  ";
    int parcounter = 0;
    TIterator* it = pdf->getParameters()->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ){
      cout << p->GetName() << " ";
      parcounter += 1;
      if ( parcounter%5==0 ) cout << endl << "  ";
    }
    delete it;
    cout << endl << endl;
    exit(1);
  }
  if ( arg->scanrangeMin != arg->scanrangeMax ) par1->setRange("scan", arg->scanrangeMin, arg->scanrangeMax);
  Utils::setLimit(w, scanVar1, "scan");
  float min1 = par1->getMin();
  float max1 = par1->getMax();
  hCL = new TH1F("hCL"+getUniqueRootName(), "hCL"+pdfName, nPoints1d, min1, max1);
  if ( hChi2min ) delete hChi2min; 
  hChi2min = new TH1F("hChi2min"+getUniqueRootName(), "hChi2min"+pdfName, nPoints1d, min1, max1);
  
  // fill the chi2 histogram with very unlikely values such
  // that inside scan1d() the if clauses work correctly
  for ( int i=1; i<=nPoints1d; i++ ) hChi2min->SetBinContent(i,1e6);  

  if ( scanVar2!="" )
  {
    RooRealVar *par2 = w->var(scanVar2);
    if(!par2){
      cout << "MethodDatasetsPluginScan::initScan() : ERROR : No such scan parameter: " << scanVar2 << endl;
      cout << "MethodDatasetsPluginScan::initScan() :         Choose an existing one using: --var par" << endl;
      exit(1);
    }
    setLimit(w, scanVar2, "scan");
    float min2 = par2->getMin();
    float max2 = par2->getMax();
    hCL2d      = new TH2F("hCL"+getUniqueRootName(),      "hCL2d"+pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
    hChi2min2d = new TH2F("hChi2min"+getUniqueRootName(), "hChi2min",      nPoints2dx, min1, max1, nPoints2dy, min2, max2);
    for ( int i=1; i<=nPoints2dx; i++ )
      for ( int j=1; j<=nPoints2dy; j++ ) hChi2min2d->SetBinContent(i,j,1e6);
  }
  
  // Set up storage for the fit results.
  // Clear before so we can call initScan() multiple times.
  // Note that allResults still needs to hold all results, so don't delete the RooFitResults.

  // 1d:
  curveResults.clear();
  for ( int i=0; i<nPoints1d; i++ ) curveResults.push_back(0);
  
  // 2d:
  curveResults2d.clear();
  for ( int i=0; i<nPoints2dx; i++ )
  {
    vector<RooSlimFitResult*> tmp;
    for ( int j=0; j<nPoints2dy; j++ ) tmp.push_back(0);
    curveResults2d.push_back(tmp);
  }
  if(arg->debug){
    std::cout << "DEBUG in MethodDatasetsPluginScan::initScan() - Global Minimum externally provided: " << std::endl;
    std::cout << "DEBUG in MethodDatasetsPluginScan::initScan() - Minimum: " << getChi2minGlobal() << std::endl << std::endl;
  }  
  // turn off some messages
  RooMsgService::instance().setStreamStatus(0,kFALSE);
  RooMsgService::instance().setStreamStatus(1,kFALSE);
  if(arg->debug){
    std::cout << "DEBUG in MethodDatasetsPluginScan::initScan() - Scan initialized successfully!\n" << std::endl;
  }     
}

///
/// load Parameter limits
/// by default the "free" limit is loaded, can be changed to "phys" by command line argument
///
void MethodDatasetsPluginScan::loadParameterLimits(){
  TString rangeName = arg->enforcePhysRange ? "phys" : "free";
  if ( arg->debug ) cout << "DEBUG in Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << endl;
  TIterator* it = w->set("par_"+pdfName)->createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ) setLimit(w, p->GetName(), rangeName);
  delete it;
}

///
/// If an external profile likelihood is given the 
/// parameter evolution point do have to get loaded
///
/// Method looks up the external tree index or compares scanpoint values
///
/// \param point    value of the current scanpoint 
///
/// \param index    load Entry of the external TTree (if the entry does not match 
///                 scanpoint, Warning will be printed)
///
bool MethodDatasetsPluginScan::loadPLHPoint(float point, int index){
  if(index!=-1){
    return loadPLHPoint(index);
  }
  else{
    int ind=-2;
    TBranch* b    = (TBranch*)this->profileLHPoints->GetBranch("scanpoint");
    int entries   = b->GetEntries();
    for(int i = 0; i<entries; i++){
      this->profileLHPoints->GetEntry(i);
      TLeaf* l          = b->GetLeaf("scanpoint");
      float treePoint   = l->GetValue();
      if(fabs((treePoint-point)/point) < 1e-5){
        ind=i;
      }
    }
    if(ind==-2){
      cout << "MethodDatasetsPluginScan::loadPLHPoint(float point, int index) : ERROR : no scanpoint (" << point << ") found!" << endl; 
      return false;
    }
    else return loadPLHPoint(ind);
  }
};

bool MethodDatasetsPluginScan::loadPLHPoint(int index){
  int fail_count = 0;
  bool success = 0;
  this->profileLHPoints->GetEntry(index);
  RooArgSet* pars          = (RooArgSet*)this->pdf->getWorkspace()->set(parsName);
  TIterator* it;
  if(pars){
    it = pars->createIterator();
  }
  else{
    cout << "MethodDatasetsPluginScan::loadPLHPoint(int index) : ERROR : no parameter set (" 
         << parsName << ") found in workspace!" << endl; 
    return success;
  }
  while ( RooRealVar* p = (RooRealVar*)it->Next() ){
    TString parName     = p->GetName();
    TLeaf* parLeaf      = (TLeaf*)this->profileLHPoints->GetLeaf(parName+"_start");
    if(parLeaf){
      float scanParVal    = parLeaf->GetValue();
      p->setVal(scanParVal);
    }
    else{
        cout << "MethodDatasetsPluginScan::loadPLHPoint(int index) : ERROR : no var (" << parName 
        << ") found in PLH scan file!" << endl;
        fail_count++;
    }
  }
  if(fail_count>0){
    cout << "MethodDatasetsPluginScan::loadPLHPoint(int index) : ERROR : some values not loaded: \n" 
    << fail_count << " out of " << pars->getSize() << " parameters not found!" 
        << " unable to fully load PLH scan point!" << endl;
    return false;
  }
  else{
      return success;
  } 
};
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
  cout << "\t --- " << "PDF Name: \t\t\t\t" << pdfName << endl;
  cout << "\t --- " << "Observables Name: \t\t\t" <<  obsName << endl;
  cout << "\t --- " << "Parameters Name: \t\t\t" <<  parsName << endl;
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
  int _nFilesMissing = 0;
  int _nFilesRead = 0;
  // Align files names with scan1d/scan1d

  TString dirname = "root/scan1dDatasetsPlugin_"+this->pdf->getPdfName()+"_"+scanVar1;
  TString fileNameBase = (fileNameBaseIn.EqualTo("default")) ? dirname+"/scan1dDatasetsPlugin_"+this->pdf->getPdfName()+"_"+scanVar1+"_run" : fileNameBaseIn;
  
  if(!explicitInputFile){
    for (int i=runMin; i<=runMax; i++){
      TString file = Form(fileNameBase+"%i.root", i);
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : opening " << file << "\r";
      if ( !FileExists(file) ){
        if ( arg->verbose ) cout << "MethodDatasetsPluginScan::readScan1dTrees() : ERROR : File not found: " + file + " ..." << endl;
        _nFilesMissing += 1;
        continue;
      }
      if ( arg->verbose ) cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading " + file + " ..." << endl;
      c->Add(file);
      _nFilesRead += 1;
    }
    if(inputFiles.size()!=0){
      for(TString &file : inputFiles){
        if ( !FileExists(file) ){
          if ( arg->verbose ) cout << "MethodDatasetsPluginScan::readScan1dTrees() : ERROR : File not found: " + file + " ..." << endl;
          _nFilesMissing += 1;
        }
        cout << "MethodDatasetsPluginScan::readScan1dTrees() : " << file << endl;
        c->Add(file);
        _nFilesRead += 1;
      }
    }
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : read files: " << _nFilesRead 
         << ", missing files: " << _nFilesMissing 
         << "                                                               "
         << "                    " << endl; // many spaces to overwrite the above \r
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : " << fileNameBase+"*.root" << endl;
    if ( _nFilesRead==0 ){
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : no files read!" << endl;
      exit(1);
    }
  }
  else{
    for(TString &file : inputFiles){
      if ( !FileExists(file) ){
        if ( arg->verbose ) cout << "MethodDatasetsPluginScan::readScan1dTrees() : ERROR : File not found: " + file + " ..." << endl;
        _nFilesMissing += 1;
      }
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : " << file << endl;
      c->Add(file);
      _nFilesRead += 1;
    }
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : read files: " << _nFilesRead << endl  
         << ", missing files: " << _nFilesMissing 
         << "                                                               "
         << "                    " << endl; // many spaces to overwrite the above \r
  }
  nFilesRead = _nFilesRead;
  nFilesMissing = _nFilesMissing;
  return c;
};
void MethodDatasetsPluginScan::readScan1dTrees(int runMin, int runMax, TString fileNameBaseIn)
{
  int nFilesRead, nFilesMissing;
  TChain* c = this->readFiles(runMin, runMax, nFilesRead, nFilesMissing, fileNameBaseIn);
  ToyTree t(this->pdf, c);
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
  cout << "MethodDatasetsPluginScan::readScan1dTrees() : total number of toys per scanpoint: " << (double) nentries / (double)21 << endl;
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
      cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading toys " 
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
    if ( valid ){//inPhysicalRegion ){
      // not efficient! TMath::Prob evaluated each toy, only needed once.
      // come up with smarter way
      h_all->Fill(t.scanpoint);
      h_probPValues->SetBinContent(h_probPValues->FindBin(t.scanpoint), TMath::Prob(t.chi2min-this->chi2minGlobal,1)); //t.chi2minGlobal, 1));
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
    cout << "====== number of toys for pValue calculation: " << nbetter << endl;
    float p = nbetter/nall;
    hCL->SetBinContent(i, p);
    hCL->SetBinError(i, sqrt(p * (1.-p)/nall));
    /*if(i==1){
      cout << "\n\n ========= Summary for BR_Bd = 0  >>>>\n" << endl;
      cout << "Number of all toys: " << ntot << endl;
      cout << "Number of good toys: " << nall << endl;

      cout  << "Number of failed toys: " << n0failed << " fraction relative to all toys: " 
            << (float)n0failed/(float)h_tot->GetBinContent(i) * 100 << " %." << endl;
      
      cout  << "Number of bkg toys: " << nbackground << " fraction relative to good toys: " 
            << (float)nbackground/(float)nall * 100 << " %." <<  endl;
      
      cout << std::scientific << std::resetiosflags(std::ios::fixed) << std::setprecision(3);
      cout << std::setw(72) << std::setfill('=') << "" <<  endl;
      cout << std::right << std::setw(45) << std::setfill(' ') << " pVal at BF_Bd=0:   " << p << " \\pm " << sqrt(p * (1.-p)/nall) << endl;
      cout << std::right << std::setw(45) << " pVal at BF_Bd=0 with NLL vars:   " << p_pdf << " \\pm " << sqrt(p_pdf * (1.-p_pdf)/nall_pdf) << endl << endl;
      cout << std::right << std::setw(45) << " pVal at BF_Bd=0 with bf_bd as test stat:   " << p_poi << " \\pm " << sqrt(p_poi * (1.-p_poi)/nall) << endl << endl;
      cout << std::resetiosflags(std::ios::right);
    }*/
  }
  TCanvas* cans = new TCanvas("cans","cans",1024,786);
  cans->cd();
  h_fracGoodToys->SetXTitle(scanVar1);
  h_fracGoodToys->SetYTitle("fraction of good toys");
  h_fracGoodToys->Draw();
    
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
  TCanvas* can1 = new TCanvas("can1","can1",1024,786);
  can1->cd();
  h_pVals->Draw();
  TCanvas* can11 = new TCanvas("can11","can11",1024,786);
  can11->cd();
  h_background->Draw();

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

///
/// Perform the 1d Plugin scan.
/// Saves chi2 values and the prob-Scan p-values in a root tree
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodDatasetsPluginScan::scan1d(int nRun)
{
  // Necessary for parallelization 
  RooRandom::randomGenerator()->SetSeed(0);
  // Set limit to all parameters. 
  this->loadParameterLimits(); /// Default is "free", if not changed by cmd-line parameter
  if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - limits set" << endl;

  // Define scan parameter and scan range.
  RooRealVar *par = w->var(scanVar1);
  float min = hCL->GetXaxis()->GetXmin();
  float max = hCL->GetXaxis()->GetXmax();
  double freeDataFitValue = par->getVal();

  // Define outputfile   
  TString dirname = "root/scan1dDatasetsPlugin_"+this->pdf->getPdfName()+"_"+scanVar1;
  system("mkdir -p "+dirname);
  TString fName;
  if(doProbScanOnly){
    fName = (fileBase=="none") ? Form("root/scan1dDatasetsProb_"+this->pdf->getPdfName()+"_%ip"+"_"+scanVar1+"_run%i.root",arg->npoints1d,nRun) : fileBase ;
  }
  else{
    fName = Form(dirname+"/scan1dDatasetsPlugin_"+this->pdf->getPdfName()+"_"+scanVar1+"_run%i.root", nRun);
  }
  TFile *f2       = new TFile(fName, "RECREATE");
  
  // Define a TH1D for prob values of the scan
  probPValues = new TH1F("probPValues","p Values of a prob Scan", nPoints1d, min, max);

  if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - probPValues defined, min/max found" << endl;

  // Set up toy root tree
  ToyTree t(this->pdf);
  if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - ToyTree constructed" << endl;
  t.init();
  if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - ToyTree init finished" << endl;
  t.nrun = nRun;
  
  if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - ToyTree initialized" << endl;

  // Save parameter values that were active at function
  // call. We'll reset them at the end to be transparent
  // to the outside.
  RooDataSet* parsFunctionCall = new RooDataSet("parsFunctionCall", "parsFunctionCall", *w->set(parsName));
  parsFunctionCall->add(*w->set(parsName));
      
  // for the progress bar: if more than 100 steps, show 50 status messages.
  int allSteps = nPoints1d*nToys;
  float printFreq = allSteps>51 ? 50 : allSteps;
  int curStep  = 0;
  
  // define constraint rooargset
  RooArgSet globalVars = *w->set(pdf->getGlobVarsName());
  // start scan
  cout << "MethodDatasetsPluginScan::scan1d() : starting ... with " << nPoints1d << " scanpoints..." << endl;
  for ( int i=0; i<nPoints1d; i++ )
  {
    // scanpoint is calculated using min, max, which are the hCL x-Axis limits set in this->initScan()
    // this uses the "scan" range, as expected 
    // don't add half the bin size. try to solve this within plotting method

    float scanpoint; 
    if(nPoints1d==1){
      scanpoint = min; 
    }
    else scanpoint = (max - min == 0 ) ? min : (min + (max-min)*(double)i/((double)nPoints1d-1));// + hCL->GetBinWidth(1)/2.);
    
    t.scanpoint = scanpoint;
    
    if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - scanpoint calculated in scanpoint " << i+1 << " as: " << scanpoint << endl;

    // don't scan in unphysical region
    // by default this means checking against "free" range
    if ( scanpoint < par->getMin() || scanpoint > par->getMax()+2e-13 ){ 
      cout << "not obvious: " << scanpoint << " < " << par->getMin() << " and " << scanpoint << " > " << par->getMax()+2e-13 << endl;
      continue;
    }

    if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - scanVar set constant and to scanpoint value " 
      << scanpoint <<" at scanpoint " << i+1 << endl;

    if(!externalProfileLH || doProbScanOnly){
      // Do initial fit
      // here the scanvar has to be fixed -> this is done once per scanpoint 
      // and provides the scanner with the DeltaChi2 for the data as reference
      // additionally the nuisances are set to the resulting fit values
      if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - DO DATA SCAN FIT" << endl;
      // set and fix scan point
      par->setVal(scanpoint);
      par->setConstant(true);

      RooFitResult *result = this->pdf->fit(kFALSE); // false -> fit on data
      assert(result);
      if(arg->debug){ 
        cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - minNll data scan fix " << 2*result->minNll() << endl;
        cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - Data Scan fit result" << endl;
        result->Print("v");
      }
      if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - RooSlimFitResult saved, fit converged for scanpoint " << i+1 << endl;
      t.statusScanData = result->status();
      
      // set chi2 of fixed fit: scan fit on data
      t.chi2min           = 2*result->minNll();
      t.covQualScanData   = result->covQual();
      if(doProbScanOnly)  t.scanbest  = freeDataFitValue;


      if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - parameters value stored in ToyTree for scanpoint " << i+1 << endl;
      this->pdf->deleteNLL();
    }
    else{
      if(this->loadPLHPoint(scanpoint,i)){
        if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - scan point " << i+1 
          << " loaded from external PLH scan file" << endl; 
      }
      // Get chi2 and status from tree

      t.statusScanData  = this->getParValAtScanpoint(scanpoint,"statusScanData");
      t.chi2min         = this->getParValAtScanpoint(scanpoint,"chi2min");
      t.covQualScanData = this->getParValAtScanpoint(scanpoint,"covQualScanData");
    }

      // cout << "CHECK PAR EVOL LOADING: ####" << endl 
      // << "######## -\t scanpoint: " << t.scanpoint << " - scanpoint " << scanpoint << endl
      // << "######## -\t statusScanData: " << t.statusScanData << endl 
      // << "######## -\t chi2min: " << t.chi2min << endl 
      // << "######## -\t Nlam_br_th_start: " << this->pdf->getWorkspace()->var("Nlam_br_th")->getVal() << endl;

    // Set nuisances. This is the point in parameter space where
    // the toys need to be generated.
    // this->loadParevolPoscanpoint; // Do not use it here, as this use to provided by the data fit with fixed scanparameter
    // save nuisances for start parameters 
    // after the initial data fit using the fixed scanvar
    RooArgSet allVars = w->allVars();
    TString plhName = Form("profileLHPoint_%i",i);
    w->saveSnapshot(plhName,allVars);

    RooDataSet* parsGlobalMinScanPoint = new RooDataSet("parsGlobalMinScanPoint", "parsGlobalMinScanPoint", *w->set(parsName));
    parsGlobalMinScanPoint->add(*w->set(parsName));
    
    if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - stored parameter values for scanpoint " << i+1 << endl;
      

    // get the chi2 of the data
    if(this->chi2minGlobalFound){
      t.chi2minGlobal     = this->getChi2minGlobal();
    }
    else{
      cout << "FATAL in MethodDatasetsPluginScan::scan1d() - Global Minimum not set!" << endl;
      cout << "FATAL in MethodDatasetsPluginScan::scan1d() - Cannot continue with scan" << endl;
      exit(-1);
    }
    
    t.storeParsPll();

    // Importance sampling ** interesting idea think about it
    // int nActualToys = nToys;
    // if ( arg->importance ){
    float plhPvalue = TMath::Prob(t.chi2min-t.chi2minGlobal,1);
    probPValues->SetBinContent(probPValues->FindBin(scanpoint), plhPvalue);
    t.genericProbPValue = plhPvalue;
    //if(arg->debug && (i<=10 || fmod(i,printFreq)==0) ) 
    cout << "INFO in MethodDatasetsPluginScan::scan1d() - Chi2 pValue " << plhPvalue 
    << " filled in bin " << i+1 << " at: " << scanpoint << endl;
    
    // Draw all toy datasets in advance. This is much faster. ** Check this statement for Generic usecase

    if(doProbScanOnly) nToys = 0;
    // Begin toy loop
    for ( int j = 0; j<nToys; j++ )
    {
      bool useConstrPDFforRandomization = true;
      //curStep++;
      //if ( curStep % (int)(allSteps/printFreq) == 0 ){
      //  cout << (float)curStep/(float)allSteps*100. << "%" << endl;
     
      //}
      if(arg->debug){
        cout << "\n>>" << endl;
        cout << ">> new toy\n" << endl;
        cout << "\n>>" << endl;
      }
      this->pdf->setMinNllFree(0);
      this->pdf->setMinNllScan(0);
      //
      // 1. Generate toys
      //    (or select the right one)
      //
      // set parameters to generate at to the values from the fit to data fixing the scanvar 
      w->loadSnapshot(plhName);
      
      this->pdf->generateToys();

      //prepare fit by randomising constraints
      this->pdf->randomizeConstraintMeans(useConstrPDFforRandomization);
      if(this->pdf->globVals) globalVars = *this->pdf->globVals->get(0);

      t.storeParsGau();

      //
      // 2. scan fit
      //
      if(arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - perform scan toy fit" << endl;
      // set parameters to data scan fit
      if(!useConstrPDFforRandomization){
        Utils::setParameters(this->pdf->getWorkspace(), parsName, parsGlobalMinScanPoint->get(0));
      }
      else{
        w->loadSnapshot(plhName);
        if(this->pdf->globVals) globalVars = *this->pdf->globVals->get(0);
      }


      par->setVal(scanpoint);
      par->setConstant(true);
      this->pdf->setFitStrategy(0);
      RooFitResult* r   = this->pdf->fit(kTRUE); // fit toys
      assert(r);
      pdf->setMinNllScan(pdf->minNll);

      if (std::isinf(pdf->getMinNllScan()) || std::isnan(pdf->getMinNllScan())) {
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
              cout << "----> emd: " << r->edm() << endl;
              break;

            case -1:
              cout << "----> fit results in status -1" << endl;
              cout << "----> NLL value: " << pdf->minNll << endl;
              cout << "----> emd: " << r->edm() << endl;
              break;

            case -99: 
              cout << "----> fit has NLL value with flag NaN or INF" << endl;
              cout << "----> NLL value: " << pdf->minNll << endl;
              cout << "----> emd: " << r->edm() << endl;
              break;
            
            default:
              cout << "unknown" << endl; 
              break;
          }
          pdf->setFitStrategy(1);
          delete r;
          r = pdf->fit(kTRUE);
          pdf->setMinNllScan(pdf->minNll);
          assert(r);
          if (std::isinf(pdf->getMinNllScan()) || std::isnan(pdf->getMinNllScan())) {
            cout << "----> nan/inf flag detected " << endl;
            cout << "----> fit status: " << pdf->getFitStatus() << endl; 
            pdf->setFitStatus(-99);
          }
          if (pdf->getFitStatus()!=0) {
            cout  << "----> problem in current fit: going to refit with strategy 2(!!), summary: " << endl
            << "----> NLL value: " << std::setprecision(9) << pdf->minNll << endl
            << "----> fit status: " << pdf->getFitStatus() << endl;
            switch(pdf->getFitStatus()){
              case 1:
                cout << "----> fit results in status 1" << endl;
                cout << "----> NLL value: " << pdf->minNll << endl;
                cout << "----> emd: " << r->edm() << endl;
                break;
  
              case -1:
                cout << "----> fit results in status -1" << endl;
                cout << "----> NLL value: " << pdf->minNll << endl;
                cout << "----> emd: " << r->edm() << endl;
                break;
  
              case -99: 
                cout << "----> fit has NLL value with flag NaN or INF" << endl;
                cout << "----> NLL value: " << pdf->minNll << endl;
                cout << "----> emd: " << r->edm() << endl;
                break;
              
              default:
                cout << "unknown" << endl; 
                break;
            }
            pdf->setFitStrategy(2);
            delete r;
            r = pdf->fit(kTRUE);
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

      /*
      if(pdf->getFitStatus() != 0 || r->minNll() < -1e27){
        cout  << "Compare pdf fit status to fit result status: " 
              << pdf->getFitStatus() << " vs fit result: " << r->status() << endl;
        cout << "###### problem in scan fit: didn't converge properly!" << endl;
        this->printDebug(*r);
        delete r;
        this->pdf->setFitStrategy(1);
        r     = this->pdf->fit(kTRUE);
        cout << std::setw(30) << std::setfill('-') << ">> refit with strat 1:" << std::setfill(' ') << endl;
        this->printDebug(*r);
        
        if(r->status()==-1){
          delete r; 
          cout << "------ last try with setFitStrategy(2)" << endl;
          this->pdf->setFitStrategy(2);
          r     = this->pdf->fit(kTRUE);
          this->printDebug(*r);
        }
        
      }
      int scanStrategy = this->pdf->getFitStrategy();
      */

      t.chi2minToy          = 2*r->minNll(); // 2*r->minNll(); //2*r->minNll();
      t.chi2minToyPDF       = 2*pdf->getMinNllScan();
      t.covQualScan         = r->covQual();
      t.statusScan          = r->status();
      t.statusScanPDF       = pdf->getFitStatus(); //r->status();
      t.storeParsScan();

      pdf->deleteNLL();
      
      RooDataSet* parsAfterScanFit = new RooDataSet("parsAfterScanFit", "parsAfterScanFit", *w->set(parsName));
      parsAfterScanFit->add(*w->set(parsName));
      
      /*cout  << std::setprecision(9);
      cout  << "===== > compare scan fit result with pdf parameters: " << endl;
      cout  << "===== > minNLL for fitResult: " << t.chi2minToy << endl 
            << "===== > minNLL for pdfResult: " << t.chi2minToyPDF << endl
            << "===== > status for pdfResult: " << pdf->getFitStatus() << endl
            << "===== > status for fitResult: " << r->status() << endl
            << "===== > BR_{Bs} value from workspace: " << w->var("BR_{Bs}")->getVal() << endl
            << "===== > BR_{Bs} value from fitResult: " << static_cast<RooRealVar*>(r->floatParsFinal().find("BR_{Bs}"))->getVal() << endl;
      */
      //
      // 2. free fit
      //
      //if(arg->debug) 
      if(arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - perform free toy fit" << endl;
      // Use parameters from the scanfit to data
      if(!useConstrPDFforRandomization){
        Utils::setParameters(this->pdf->getWorkspace(), parsName, parsGlobalMinScanPoint->get(0));
      }
      else{
        // scanpoint has to be set free again
        w->loadSnapshot(plhName);
        if(this->pdf->globVals) globalVars = *this->pdf->globVals->get(0);
      }
      par->setConstant(false);
      if(par->getVal() < 1e-13){
        par->setVal(2.0e-11);
      }
      // Fit
      // pdf->setFitStrategy(0);
      RooFitResult* r1  = this->pdf->fit(kTRUE);
      assert(r1);
      pdf->setMinNllFree(pdf->minNll);
      t.chi2minGlobalToy = 2*r1->minNll();

      if (std::isinf(pdf->getMinNllFree()) || std::isnan(pdf->getMinNllFree())) {
        cout << "----> nan/inf flag detected " << endl;
        cout << "----> fit status: " << pdf->getFitStatus() << endl; 
        pdf->setFitStatus(-99);
      }

      bool negTestStat = t.chi2minToy-t.chi2minGlobalToy<0;
      
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

        if(pdf->getFitStatus()!=0 || negTestStat ) {
          cout  << "----> problem in current fit: going to refit with strategy "<< pdf->getFitStrategy() << " , summary: " << endl
                << "----> NLL value: " << std::setprecision(9) << pdf->getMinNllFree() << endl
                << "----> fit status: " << pdf->getFitStatus() << endl
                << "----> dChi2: " << (t.chi2minToy-t.chi2minGlobalToy) << endl 
                << "----> dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;

          switch(pdf->getFitStatus()){
            case 1:
              cout << "----> fit results in status 1" << endl;
              cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
              cout << "----> emd: " << r1->edm() << endl;
              break;

            case -1:
              cout << "----> fit results in status -1" << endl;
              cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
              cout << "----> emd: " << r1->edm() << endl;
              break;

            case -99: 
              cout << "----> fit has NLL value with flag NaN or INF" << endl;
              cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
              cout << "----> emd: " << r1->edm() << endl;
              break;
            case -66:
              cout  << "----> fit has nan/inf NLL value and a negative test statistic" << endl
                    << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                    << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                    << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
              cout << "----> emd: " << r1->edm() << endl;
              break;
            case -13:
              cout  << "----> free fit has status 0 but creates a negative test statistic" << endl
                    << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                    << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                    << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
              cout  << "----> emd: " << r1->edm() << endl;
              break;
            case -12:
              cout  << "----> free fit has status 1 and creates a negative test statistic" << endl
                    << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                    << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                    << "----> free fit min nll:" << pdf->getMinNllFree() << endl;

              cout  << "----> emd: " << r1->edm() << endl;
              break;
            case -33:
              cout  << "----> free fit has status -1 and creates a negative test statistic" << endl
                    << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                    << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                    << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
              cout  << "----> emd: " << r1->edm() << endl;
              cout  << std::setprecision(6);
              break;
            default:
              cout << "-----> unknown / fitResult neg test stat, but status 0" << endl; 
              cout << "----> dChi2: " << 2*(r->minNll() - r1->minNll()) << endl;
              cout << "----> dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;
            break;
          }
          bool refit = kFALSE;
          if(pdf->getFitStrategy() == 0){pdf->setFitStrategy(1); refit = kTRUE;}
          else{
            if(pdf->getFitStrategy() == 1){pdf->setFitStrategy(2); refit = kTRUE;}
            else{
              if(pdf->getFitStrategy() == 2){cout << "----> ##FAILURE## IN FREE FIT WITH STRATEGY 2!!" << endl;}
            }
          }
          if(refit){
            cout << "----> refit with strategy: " << pdf->getFitStrategy() << endl;
            delete r1;
            r1  = this->pdf->fit(kTRUE);
            assert(r1);
            pdf->setMinNllFree(pdf->minNll);
            t.chi2minGlobalToy = 2*r1->minNll();
            if (std::isinf(pdf->getMinNllFree()) || std::isnan(pdf->getMinNllFree())) {
              cout << "----> nan/inf flag detected " << endl;
              cout << "----> fit status: " << pdf->getFitStatus() << endl; 
              pdf->setFitStatus(-99);
            } 
            negTestStat = t.chi2minToy-t.chi2minGlobalToy<0;

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

            if (pdf->getFitStatus()!=0 || negTestStat ) {
              cout  << "----> problem in current fit: going to refit with strategy "<< pdf->getFitStrategy() << " , summary: " << endl
                    << "----> NLL value: " << std::setprecision(9) << pdf->getMinNllFree() << endl
                    << "----> fit status: " << pdf->getFitStatus() << endl
                    << "----> dChi2: " << (t.chi2minToy-t.chi2minGlobalToy) << endl 
                    << "----> dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;
              switch(pdf->getFitStatus()){
                case 1:
                  cout << "----> fit results in status 1" << endl;
                  cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
                  cout << "----> emd: " << r1->edm() << endl;
                  break;

                case -1:
                  cout << "----> fit results in status -1" << endl;
                  cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
                  cout << "----> emd: " << r1->edm() << endl;
                  break;

                case -99: 
                  cout << "----> fit has NLL value with flag NaN or INF" << endl;
                  cout << "----> NLL value: " << pdf->getMinNllFree() << endl;
                  cout << "----> emd: " << r1->edm() << endl;
                  break;
                case -66:
                  cout  << "----> fit has nan/inf NLL value and a negative test statistic" << endl
                        << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                        << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                        << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
                  cout << "----> emd: " << r1->edm() << endl;
                  break;
                case -13:
                  cout  << "----> free fit has status 0 but creates a negative test statistic" << endl
                        << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                        << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                        << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
                  cout  << "----> emd: " << r1->edm() << endl;
                  break;
                case -12:
                  cout  << "----> free fit has status 1 and creates a negative test statistic" << endl
                        << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                        << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                        << "----> free fit min nll:" << pdf->getMinNllFree() << endl;

                  cout  << "----> emd: " << r1->edm() << endl;
                  break;
                case -33:
                  cout  << "----> free fit has status -1 and creates a negative test statistic" << endl
                        << "----> dChi2: " << 2*(pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                        << "----> scan fit min nll:" << pdf->getMinNllScan() << endl 
                        << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
                  cout  << "----> emd: " << r1->edm() << endl;
                  cout  << std::setprecision(6);
                  break;
                default:
                  cout << "unknown" << endl; 
                  break;
              }
              bool refit2nd = kFALSE;
              if(pdf->getFitStrategy() == 0){pdf->setFitStrategy(1); refit2nd = kTRUE;}
              else{
                if(pdf->getFitStrategy() == 1){pdf->setFitStrategy(2); refit2nd = kTRUE;}
                else{
                  if(pdf->getFitStrategy() == 2){cout << "----> ##FAILURE## IN FREE FIT WITH STRATEGY 2!!" << endl;}
                }
              }

              if(refit2nd){
                cout << "----> refit with strategy: " << pdf->getFitStrategy() << endl;
                delete r1;
                r1  = this->pdf->fit(kTRUE);
                assert(r1);
                pdf->setMinNllFree(pdf->minNll); 
                t.chi2minGlobalToy = 2*r1->minNll();
                if (std::isinf(pdf->getMinNllFree()) || std::isnan(pdf->getMinNllFree())) {
                  cout << "----> nan/inf flag detected " << endl;
                  cout << "----> fit status: " << pdf->getFitStatus() << endl; 
                  pdf->setFitStatus(-99);
                } 

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
              }
            }
          }
        }   

      if( (t.chi2minToy-t.chi2minGlobalToy) < 0){
        cout << "+++++ > still negative test statistic after whole procedure!! " << endl;
        cout << "+++++ > try to fit with different starting values" << endl;
        cout << "+++++ > dChi2: " << t.chi2minToy-t.chi2minGlobalToy << endl; 
        cout << "+++++ > dChi2PDF: " << 2*(pdf->getMinNllScan()-pdf->getMinNllFree()) << endl;
        Utils::setParameters(this->pdf->getWorkspace(), parsName, parsAfterScanFit->get(0));
        if(par->getVal() < 1e-13) par->setVal(0.67e-12);
        par->setConstant(false); 
        pdf->deleteNLL();
        RooFitResult* r_tmp = pdf->fit(kTRUE);
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
          par->setVal(static_cast<RooRealVar*>(r1->floatParsFinal().find(par->GetName()))->getVal());
          delete r_tmp;
        }
        delete parsAfterScanFit;
      };
      
      cout  << "===== > compare free fit result with pdf parameters: " << endl;
      cout  << "===== > minNLL for fitResult: " << r1->minNll() << endl 
            << "===== > minNLL for pdfResult: " << pdf->getMinNllFree() << endl
            << "===== > status for pdfResult: " << pdf->getFitStatus() << endl
            << "===== > status for fitResult: " << r1->status() << endl;
            //<< "===== > BR_{Bd} value from workspace: " << w->var("BR_{Bd}")->getVal() << endl
            //<< "===== > BR_{Bd} value from fitResult: " << static_cast<RooRealVar*>(r1->floatParsFinal().find("BR_{Bd}"))->getVal() << endl
            //<< "===== > BR_{Bs} value from workspace: " << w->var("BR_{Bs}")->getVal() << endl
            //<< "===== > BR_{Bs} value from fitResult: " << static_cast<RooRealVar*>(r1->floatParsFinal().find("BR_{Bs}"))->getVal() << endl;


      t.chi2minGlobalToy    = 2*r1->minNll(); //2*r1->minNll();
      t.chi2minGlobalToyPDF = 2*pdf->getMinNllFree(); //2*r1->minNll();
      t.statusFreePDF       = pdf->getFitStatus(); //r1->status();
      t.statusFree          = r1->status();
      t.covQualFree         = r1->covQual();
      t.scanbest            = ((RooRealVar*)w->set(parsName)->find(scanVar1))->getVal();
      t.storeParsFree();
      pdf->deleteNLL();

      cout << "#### > Fit summary: " << endl;
      cout  << "#### > free fit status: " << t.statusFree << " vs pdf: " << t.statusFreePDF << endl 
            << "#### > scan fit status: " << t.statusScan << " vs pdf: " << t.statusScanPDF<< endl 
            << "#### > free min nll: " << t.chi2minGlobalToy << " vs pdf: " << t.chi2minGlobalToyPDF << endl 
            << "#### > scan min nll: " << t.chi2minToy << " vs pdf: " << t.chi2minToyPDF << endl 
            << "#### > dChi2 fitresult: " << t.chi2minToy-t.chi2minGlobalToy << endl
            << "#### > dChi2 pdfresult: " << t.chi2minToyPDF-t.chi2minGlobalToyPDF << endl;
      cout  << std::setprecision(6);      
    
      if(t.chi2minToy - t.chi2minGlobalToy > 20 && (t.statusFree==0 && t.statusScan==0) 
          && t.chi2minToy>-1e27 && t.chi2minGlobalToy>-1e27){
        cout << std::setw(30) << std::setfill('-') << ">>> HIGH test stat value!! print fit results with fit strategy: "<< pdf->getFitStrategy() << std::setfill(' ') << endl;
        cout << "SCAN FIT Result" << endl;
        r->Print("");
        cout << "================" << endl;
        cout << "FREE FIT result" << endl;
        r1->Print("");
      }

      if(arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - ToyTree 2*minNll free fit: " << t.chi2minGlobalToy << endl;

      if(arg->debug && (j<=2 || fmod(i,printFreq)==0)){ 
        cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - ToyTree 2*minNll scan fit: " << t.chi2minToy << endl;
        cout << "DEBUG in MethodDatasetsPluginScan::scan1d() - for Toy with scanpoint: " << t.scanpoint << endl;
        //r->Print("v");
      }
      
      //
      // 4. store
      //
      t.fill();
      //remove dataset and pointers
      delete r;
      delete r1;
      pdf->deleteToys();
      pdf->deleteConstraints();
      //delete parsAfterScanFit;
      // status bar
      curStep++;
      if ( curStep % (int)(allSteps/printFreq) == 0 ){
        cout << (float)curStep/(float)allSteps*100. << "%" << endl;
      }
    } // End of toys loop
    if(doProbScanOnly) t.fill();
    // reset
    setParameters(w, parsName, parsFunctionCall->get(0));
    //delete result;
    delete parsGlobalMinScanPoint;
    //setParameters(w, obsName, obsDataset->get(0));
    t.writeToFile();
  } // End of npoints loop
  this->profileLH = new MethodProbScan(this->pdf, this->getArg(), probPValues, this->pdf->getPdfName());
  f2->Write();
  f2->Close();
  delete parsFunctionCall;
  //if (!(arg->isAction("pluginbatch"))) readScan1dTrees(nRun, nRun);
  return 0;
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
  ToyTree t(this->pdf, this->chain);
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
