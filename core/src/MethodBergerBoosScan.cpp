/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maximilian.schlupp@cern.ch
 * Date: January 2013
 */

#include "MethodBergerBoosScan.h"

///
/// Initialize from a previous Prob scan, setting the profile
/// likelihood. This should
/// be the default.
///
MethodBergerBoosScan::MethodBergerBoosScan(MethodProbScan* s, TString d)
    : MethodPluginScan(s)
{
    this->methodName = "BergerBoos";
    if(d=="default"){
        this->dir = "XX";
    }
    else{
        this->dir = d;
    }
    nBBPoints = 1;
    //std::cout << "open the file" << std::endl;
    TString fName;
    if(this->dir == "XX"){
        fName = "root/BBTree.root";
    }
    else{
        fName = this->dir+"root/BBTree.root";
    }
    file = new TFile(fName);
    //std::cout << "file opened" << std::endl;
    if(!file->IsOpen()){
        std::cout << "ROOT file not opened!" << std::endl;
    }

    BBtree = (TTree*)file->Get("BBTree");
};

MethodBergerBoosScan::~MethodBergerBoosScan(){
    file->Close();
    delete file;
};

///
/// Calculates 2D p-Values in (scanpoint,BergerBoos_id) Histogram
///
TH2F* MethodBergerBoosScan::calcPValues(TH2F better, TH2F all, TH2F bg){
    TH2F* cl = (TH2F*) better.Clone("cl");
    for (int i=1; i<=better.GetNbinsX(); i++)
    {
        for (int j = 1; j <= better.GetNbinsY(); ++j)
        {
            float nbetter = better.GetBinContent(i,j);
            float nall = all.GetBinContent(i,j);
            float nbackground = bg.GetBinContent(i,j);
            if ( nall == 0. ) continue;

            // subtract background
            // float p = (nbetter-nbackground)/(nall-nbackground);
            // hCL->SetBinContent(i, p);
            // hCL->SetBinError(i, sqrt(p * (1.-p)/(nall-nbackground)));

            // don't subtract background
            float p = nbetter/nall;
            cl->SetBinContent(i,j, p);
            cl->SetBinError(i,j, sqrt(p * (1.-p)/nall));
        }
    }
    return cl;
};

///
/// Draws a 2D Histogram in var1:var2 space
///
void MethodBergerBoosScan::drawBBPoints(TString varX, TString varY, int runMin, int runMax, bool save){
    TChain *c = new TChain("plugin");
    int nFilesMissing = 0;
    int nFilesRead    = 0;

    TString fileNameBase;
    if(this->dir=="XX"){
        //fileNameBase = "root/scan1dPlugin_"+name+"_"+scanVar1+"_run";
        fileNameBase = "root/scan1dBergerBoos_"+name+"_"+scanVar1+"_run";
    }
    else{
        //fileNameBase = this->dir+"root/scan1dPlugin_"+name+"_"+scanVar1+"_run";
        fileNameBase = this->dir+"root/scan1dBergerBoos_"+name+"_"+scanVar1+"_run";
    }
    for (int i=runMin; i<=runMax; i++)
    {
        TString file = Form(fileNameBase+"%i.root", i);
        if ( !FileExists(file) )
        {
            if ( arg->verbose ) cout << "MethodBergerBoosScan::drawBBPoints() : ERROR : File not found: " + file + " ..." << endl;
            nFilesMissing += 1;
            continue;
        }
        if ( arg->verbose ) cout << "MethodBergerBoosScan::drawBBPoints() : reading " + file + " ..." << endl;
        c->Add(file);
        // Quick hack to read in Malcolm's refitted toys:
        // t->Add(Form("k3piCrosscheck/toysForPOI_rBu_dk_dkanddpi_24_TMK_refitted_gaus-run%i.root",i));
        // t->Add(Form("k3piCrosscheck/toysForPOI_rBu_dk_dkanddpi_24_TMK_refitted_gaus_force-run%i.root",i));
        nFilesRead += 1;
    }
    cout << "MethodBergerBoosScan::drawBBPoints() : read files: " << nFilesRead << ", missing files: " << nFilesMissing << endl;
    cout << "MethodBergerBoosScan::drawBBPoints() : " << fileNameBase+"*.root" << endl;
    if ( nFilesRead==0 )
    {
        cout << "MethodBergerBoosScan::drawBBPoints() : no files read!" << endl;
        return;
    }
    ToyTree TT(combiner, c);
    TT.open();
    TTree* tree = TT.getTree();
    // 'create' Histogram
    tree->Draw(varY+":"+varX+">>hBBPoints");
    TH2F *hBBPoints = new TH2F(*(TH2F*)gDirectory->Get("hBBPoints"));
    // Reset Bin Content
    hBBPoints->Reset();
    //float nBB;
    //nBB = tree->GetLeaf("nBergerBoos")->GetValue();
    for(int entry = 0; entry < tree->GetEntries(); entry++){
        tree->GetEntry(entry);
        Int_t nBinX = hBBPoints->GetXaxis()->FindFixBin(tree->GetLeaf(varX)->GetValue());
        Int_t nBinY = hBBPoints->GetYaxis()->FindFixBin(tree->GetLeaf(varY)->GetValue());
        // If there are two BB Points generated at the same Phasespace point
        if( hBBPoints->GetBinContent(nBinX,nBinY)<=nBBPoints && hBBPoints->GetBinContent(nBinX,nBinY) != 0 && hBBPoints->GetBinContent(nBinX,nBinY) != (tree->GetLeaf("BergerBoos_id")->GetValue()+1))
        {
            // When it is not the first time filling (content == 0) and
            // the content is not equal the the value that would be filled: add the two values
            hBBPoints->SetBinContent(nBinX,nBinY,(tree->GetLeaf("BergerBoos_id")->GetValue()+1+hBBPoints->GetBinContent(nBinX,nBinY)));
        }
        else{
            hBBPoints->SetBinContent(nBinX,nBinY,(tree->GetLeaf("BergerBoos_id")->GetValue()+1));
        }
    }
    TCanvas* c1 = newNoWarnTCanvas("c",varY+" vs "+varX,800,600);
    hBBPoints->Draw("COLZTEXT");
    if(save){
        savePlot(c1,varY+"_vs_"+varX+"_BB_id");
    }
    delete c1;
    delete hBBPoints;
};

///
/// Returns a 1D Histogram containing the 1-CL curve
/// Input is a 2D Histogram with the p-Values in the
/// dimensions (scanpoint, BergerBoos_id)
///
void MethodBergerBoosScan::getBestPValue(TH1F* h, TH2F* pValues){
    for (int i=1; i<=pValues->GetNbinsX(); i++)
    {
        for (int j = 1; j <= pValues->GetNbinsY(); ++j)
        {
            float pVal = pValues->GetBinContent(i,j);

            if(j==1){
                h->SetBinContent(i,pVal);
                h->SetBinError(i, pValues->GetBinError(i,j));
            }
            else if(pVal>h->GetBinContent(i)){
                h->SetBinContent(i,pVal);
                h->SetBinError(i, pValues->GetBinError(i,j));
            }
        }
    }
};

///
/// Reads in Scan1DTree result from scan1D()
/// This is a slightly modified version of
/// MethodPluginScan::readScan1dTrees()'s function
/// to account for BergerBoos specifications
///
void MethodBergerBoosScan::readScan1dTrees(int runMin, int runMax){
    TChain *c         = new TChain("plugin");
    int nFilesMissing = 0;
    int nFilesRead    = 0;
    TString fileNameBase = "root/scan1dBergerBoos_"+name+"_"+scanVar1+"_run";
    if(this->dir!=TString("XX")) fileNameBase = this->dir+fileNameBase;
    for (int i=runMin; i<=runMax; i++)
    {
        // this segfaults for --jmin 1 --jmax 10
        // if(runMin != runMax){
        //   if( (i+1) % (int)((runMin-runMax)/10) == 0 ){
        //     std::cout << "MethodBergerBoosScan::readScan1dTrees() : " << ((float)i/(float)(runMax-runMin))*100 << "\% of files read." << std::endl;
        //   }
        // }
        TString file = Form(fileNameBase+"%i.root", i);
        if ( !FileExists(file) )
        {
            if ( arg->verbose ) cout << "MethodBergerBoosScan::readScan1dTrees() : ERROR : File not found: " + file + " ..." << endl;
            nFilesMissing += 1;
            continue;
        }
        if ( arg->verbose ) cout << "MethodBergerBoosScan::readScan1dTrees() : reading " + file + " ..." << endl;
        c->Add(file);
        nFilesRead += 1;
    }
    cout << "MethodBergerBoosScan::readScan1dTrees() : read files: " << nFilesRead << ", missing files: " << nFilesMissing << endl;
    cout << "MethodBergerBoosScan::readScan1dTrees() : " << fileNameBase+"*.root" << endl;
    if ( nFilesRead==0 )
    {
        cout << "MethodBergerBoosScan::readScan1dTrees() : no files read!" << endl;
        return;
    }
    ToyTree t(combiner, c);
    t.open();
    if ( arg->controlplot ) {
        ControlPlots cp(&t);
        if ( arg->plotid==0 || arg->plotid==1 ) cp.ctrlPlotMore(profileLH);
        if ( arg->plotid==0 || arg->plotid==2 ) cp.ctrlPlotChi2();
        if ( arg->plotid==0 || arg->plotid==3 ) cp.ctrlPlotNuisances();
        if ( arg->plotid==0 || arg->plotid==4 ) cp.ctrlPlotObservables();
        if ( arg->plotid==0 || arg->plotid==5 ) cp.ctrlPlotChi2Distribution();
        if ( arg->plotid==0 || arg->plotid==6 ) cp.ctrlPlotChi2Parabola();
        if ( arg->plotid==0 || arg->plotid==7 ) cp.ctrlPlotPvalue();
        cp.saveCtrlPlots();
    }

    delete hCL;
    hCL = new TH1F("hCL", "hCL", t.getScanpointN(), t.getScanpointMin(), t.getScanpointMax());

    // Histograms used for 1-CL calculation
    // For the BergerBoos Method these have to be
    // 2 dimensional in (scanpoint,BergerBoos_id)
    TH2F *h_better = new TH2F("h_better","h_better", t.getScanpointN(),
            t.getScanpointMin(), t.getScanpointMax(),
            t.nBergerBoos,       0.,
            t.nBergerBoos);
    TH2F *h_all        = (TH2F*)h_better->Clone("h_all");
    TH2F *h_background = (TH2F*)h_better->Clone("h_background");

    Long64_t nentries = t.GetEntries();
    Long64_t nfailed  = 0;
    // t.activateCoreBranchesOnly(); ///< speeds up the event loop
    // Loop over the whole tree
    std::cout << "MethodBergerBoosScan::readScan1dTrees() : starting to read toys..." << std::endl;
    for (Long64_t i = 0; i < nentries; i++)
    {
        if( (i+1) % (int)(nentries/10) == 0){
            std::cout << "MethodBergerBoosScan::readScan1dTrees() : " << Form("%2.0f",((float)i/(float)nentries)*100) << "\% of toys read." << std::endl;
        }
        t.GetEntry(i);
        // apply global cuts
        // rejects events if the fits are "not good enough"
        if ( ! (fabs(t.chi2minToy)<500 && fabs(t.chi2minGlobalToy)<500
                    && t.statusFree==0. && t.statusScan==0.
                    // && fabs(t.chi2minGlobal-chi2minGlobal)<0.1 // reject files from other runs
               ))
        {
            if( arg->debug ){
                std::cout << ":::::::::: new entry :::::::::::" << std::endl;
                if(fabs(t.chi2minToy)>=500){
                    std::cout << "|t.chi2minToy| >= 500 " << std::endl;
                }
                if(fabs(t.chi2minGlobalToy)>=500){
                    std::cout << "|t.chi2minGlobalToy| >= 500 " << std::endl;
                }
                if(fabs(t.chi2minGlobal-chi2minGlobal)>=0.1){
                    std::cout << "|t.chi2minGlobal-chi2minGlobal| >= 0.1 " << std::endl;
                }
                if(t.statusFree==1){
                    std::cout << "t.statusFree == 1 " << std::endl;
                }
                if(t.statusScan==1){
                    std::cout << "t.statusScan == 1 " << std::endl;
                }
            }
            nfailed++;
            continue;
        }

        // Check if toys are in physical region.
        // Don't enforce t.chi2min-t.chi2minGlobal>0, else it can be hard because due
        // to little fluctuaions the best fit point can be missing from the plugin plot...
        bool inPhysicalRegion = t.chi2minToy-t.chi2minGlobalToy>0; //&& t.chi2min-t.chi2minGlobal>0

        // build test statistic
        if ( inPhysicalRegion && t.chi2minToy-t.chi2minGlobalToy > t.chi2min-t.chi2minGlobal )
        {
            h_better->Fill(t.scanpoint, t.BergerBoos_id);
        }

        if ( inPhysicalRegion )
        {
            h_all->Fill(t.scanpoint, t.BergerBoos_id);
        }

        // use the unphysical events to estimate background (be careful with this,
        // at least inspect the control plots to judge if this can be at all reasonable)
        if ( !inPhysicalRegion )
        {
            h_background->Fill(t.scanpoint, t.BergerBoos_id);
        }
    }

    // 1-CL construction controlplot
    // TCanvas* c1 = newNoWarnTCanvas("asdf");
    // c1->Divide(2,1);
    // c1->cd(1); h_better->Draw("colz");
    // c1->cd(2); h_all->Draw("colz");

    cout << "MethodBergerBoosScan::readScan1dTrees() : read an average of " << (nentries-nfailed)/nPoints1d << " toys per scan point." << endl;
    cout << "MethodBergerBoosScan::readScan1dTrees() : fraction of failed toys: " << (double)nfailed/(double)nentries*100. << "%." << endl;
    cout << "MethodBergerBoosScan::readScan1dTrees() : fraction of background toys: " << h_background->GetEntries()/(double)nentries*100. << "%." << endl;

    TH2F* hCL2d = calcPValues(*h_better, *h_all, *h_background);
    getBestPValue(hCL,hCL2d);
    TString fName = Form("root/hCL2d_"+name+"_"+scanVar1+"_run%i_to_%i.root", runMin,runMax);
    if(this->dir == "XX") fName = this->dir + fName;
    if(!arg->isAction("pluginbatch")) hCL2d->SaveAs(fName);

    //hCL->SaveAs("root/hCL.root");
    //h_better->SaveAs("root/h_better.root");
};

///
/// Perform the 1d BergerBoos scan.
/// Saves chi2 values in a root tree, together with the full fit result for each toy.
/// If a combined PDF for the toy generation is given by setToyGenCombo(), this
/// will be used to generate the toys.
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodBergerBoosScan::scan1d(int nRun)
{
    TString fName = "";
    if(this->dir=="XX"){
        //fName = Form("root/scan1dPlugin_"+name+"_"+scanVar1+"_run%i.root", nRun);
        fName = Form("root/scan1dBergerBoos_"+name+"_"+scanVar1+"_run%i.root", nRun);
    }
    else{
        //fName = this->dir+Form("root/scan1dPlugin_"+name+"_"+scanVar1+"_run%i.root", nRun);
        fName = this->dir+Form("root/scan1dBergerBoos_"+name+"_"+scanVar1+"_run%i.root", nRun);
    }

    TFile *f2 = new TFile(fName, "recreate");

    Fitter *myFit = new Fitter(arg, w, combiner->getPdfName());
    RooRandom::randomGenerator()->SetSeed(0);

    // Set limit to all parameters.
    combiner->loadParameterLimits();

    // Define scan parameter and scan range.
    RooRealVar *par = w->var(scanVar1);
    float min = hCL->GetXaxis()->GetXmin();
    float max = hCL->GetXaxis()->GetXmax();

    if ( arg->verbose )
    {
        cout << "Berger-Boos configuration:" << endl;
        cout << "  combination : " << title << endl;
        cout << "  scan variable : " << scanVar1 << endl;
        cout << "  scan range : " << min << " ... " << max << endl;
        cout << "  scan steps : " << nPoints1d << endl;
        cout << "  par. evolution : " << (parevolPLH!=profileLH?parevolPLH->getTitle():"same as combination") << endl;
        cout << "  nToys : " << nToys << endl;
        cout << endl;
    }

    // Set up toy root tree
    cout << pdfName << endl;
    ToyTree t(combiner);
    t.init();
    t.nrun = nRun;

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    FitResultCache frCache(arg);
    frCache.storeParsAtFunctionCall(w->set(parsName));
    frCache.initRoundRobinDB(w->set(parsName));

    // for the progress bar: if more than 100 steps, show 50 status messages.
    int allSteps = nPoints1d*nToys*nBBPoints;
    float printFreq = allSteps>51 ? 50 : allSteps;
    int curStep  = 0;
    int StepCounter = 0;
    // start scan
    cout << "MethodBergerBoosScan::scan1d() : starting ..." << endl;
    for ( int i=0; i<nPoints1d; i++ )
    {
        float scanpoint = min + (max-min)*(double)i/(double)nPoints1d + hCL->GetBinWidth(1)/2.;
        t.scanpoint = scanpoint;

        // Tell tree how many BergerBoos points were sampled
        t.nBergerBoos = nBBPoints;

        // don't scan in unphysical region
        if ( scanpoint < par->getMin() || scanpoint > par->getMax() ) continue;

        for(int ii=0; ii<nBBPoints; ii++) // Berger Boos nuisance Loop
        {
            // Store BergerBoos_id to tree to be able to separate the Berger Boos
            // points a posteriori
            t.BergerBoos_id = ii;

            // Set nuisances. This is the point in parameter space where
            // the toys need to be generated.

            // The first Berger Boos scanpoint is equal to the best fit values (Plugin scan point)
            RooSlimFitResult* plhScan = getParevolPoint(scanpoint);
            setParameters(w, parsName, plhScan, true);

            if(ii>0){
                // From the second point in the nuisance parameter space onwards, new points are drawn randomly
                // from their Berger Boos ranges
                this->setNewBergerBoosPoint(StepCounter);
                //continue;
            }
            StepCounter++;

            // set and fix scan point
            par->setConstant(true);
            par->setVal(scanpoint);

            // save nuisances for start parameters
            frCache.storeParsAtGlobalMin(w->set(parsName));

            t.storeParsPll();
            t.storeTheory();

            // get the chi2 of the data
            t.chi2min = profileLH->getChi2min(scanpoint);
            t.chi2minGlobal = profileLH->getChi2minGlobal();

            // Draw all toy datasets in advance. This is much faster.
            RooDataSet *toyDataSet = w->pdf(pdfName)->generate(*w->set(obsName), nToys, AutoBinned(false));

            for ( int j = 0; j<nToys; j++ )
            {
                curStep++;
                if ( curStep % (int)(allSteps/printFreq) == 0 )
                {
                    cout << (float)curStep/(float)allSteps*100. << "%" << endl;
                }

                //
                // 1. Generate toys
                //    (or select the right one)
                //
                const RooArgSet* toyData = toyDataSet->get(j);
                setParameters(w, obsName, toyData);
                t.storeObservables();

                //
                // 2. scan fit
                //
                par->setVal(scanpoint);
                par->setConstant(true);
                // myFit->setStartparsFirstFit(profileLH->getSolution(0));
                myFit->setStartparsFirstFit(frCache.getRoundRobinNminus(0));
                myFit->setStartparsSecondFit(frCache.getParsAtGlobalMin());
                myFit->fit();
                t.chi2minToy = myFit->getChi2();
                t.statusScan = myFit->getStatus();
                t.storeParsScan();

                //
                // 3. free fit
                //
                par->setConstant(false);
                myFit->fit();
                t.chi2minGlobalToy = myFit->getChi2();
                t.statusFree = myFit->getStatus();
                t.scanbest = ((RooRealVar*)w->set(parsName)->find(scanVar1))->getVal();
                t.storeParsFree();

                //
                // 4. store
                //
                if ( t.statusFree==0 ) frCache.storeParsRoundRobin(w->set(parsName));
                t.fill();
            }

            // reset
            setParameters(w, parsName, frCache.getParsAtFunctionCall());
            setParameters(w, obsName, obsDataset->get(0));
            delete toyDataSet;
        }
    }
    myFit->print();
    t.writeToFile();
    f2->Close();
    delete f2;
    delete myFit;
    readScan1dTrees(nRun, nRun);
    return nBBPoints;
}

void MethodBergerBoosScan::setNewBergerBoosPoint(int m){
    // Get new BB Point from BBTree Class member
    TIterator* iter = w->set(parsName)->createIterator();
    //int p=0;
    while ( RooRealVar* par = (RooRealVar*)iter->Next() ) {
        // Set new parameter values by reading the BBTree
        float VAL = -666;
        BBtree->GetBranch(par->GetName())->GetEntry(m-1);
        VAL = BBtree->GetLeaf(par->GetName())->GetValue();
        par->setVal(VAL);
    }
    delete iter;
};
