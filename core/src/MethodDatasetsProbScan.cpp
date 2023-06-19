/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
 *
 */

#include "MethodDatasetsProbScan.h"
#include "TRandom3.h"
#include <algorithm>
#include <ios>
#include <iomanip>

MethodDatasetsProbScan::MethodDatasetsProbScan(PDF_Datasets* PDF, OptParser* opt)
    : MethodProbScan(opt),
      pdf                 (PDF),
      probPValues         (NULL),
      drawPlots           (false),
      explicitInputFile   (false),
      bkgOnlyFitResult    (NULL),
      probScanTree        (NULL)
{
    chi2minGlobalFound = true; // the free fit to data must be done and must be saved to the workspace before gammacombo is even called
    methodName = "DatasetsProb";

    /////////////////////////////
    //Titus: add these variable initializations for compatibility
    //scanDisableDragMode = false; //Titus: Not needed at the moment
    nScansDone              = 0;
    parsName = PDF->getParName();
    ////////////////////////////

    w = PDF->getWorkspace();
    title = PDF->getTitle();
    name =  PDF->getName();
	pdfName = name;

    inputFiles.clear();

    if ( !w->set(pdf->getObsName()) ) {
        cerr << "MethodDatasetsProbScan::MethodDatasetsProbScan() : ERROR : no '" + pdf->getObsName() + "' set found in workspace" << endl;
        cerr << " You can specify the name of the set in the workspace using the pdf->initObservables(..) method.";
        exit(EXIT_FAILURE);
    }
    if ( !w->set(pdf->getParName()) ) {
        cerr << "MethodDatasetsProbScan::MethodDatasetsProbScan() : ERROR : no '" + pdf->getParName() + "' set found in workspace" << endl;
        exit(EXIT_FAILURE);
    }

    // setup a dummy empty combiner to help with file naming and global option later
    combiner = new Combiner( opt, name, title);
}

void MethodDatasetsProbScan::initScan() {
    if ( arg->debug ) cout << "MethodDatasetsProbScan::initScan() : initializing ..." << endl;

    // Init the 1-CL histograms. Range is taken from the scan range, unless
    // the --scanrange command line argument is set.
    RooRealVar *par1 = w->var(scanVar1);
    if ( !par1 ) {
        cout << "MethodDatasetsProbScan::initScan() : ERROR : No such scan parameter: " << scanVar1 << endl;
        cout << "MethodDatasetsProbScan::initScan() :         Choose an existing one using: --var par" << endl << endl;
        cout << "  Available parameters:" << endl << "  ---------------------" << endl << endl << "  ";
        pdf->printParameters();
        exit(EXIT_FAILURE);
    }
    if ( !m_xrangeset && arg->scanrangeMin != arg->scanrangeMax ) {
			setXscanRange(arg->scanrangeMin,arg->scanrangeMax);
	}
    // setLimit(w, scanVar1, "scan");

    if (hCL) delete hCL;
    // Titus: small change for consistency
    // float min1 = par1->getMin();
    // float max1 = par1->getMax();
    float min1 = arg->scanrangeMin;
    float max1 = arg->scanrangeMax;

    if(scanVar1==scanVar2){
        if(arg->debug) std::cout << "DEBUG: MethodDatasetsProbScan::initScan() : scanning y range" << std::endl;
        min1 = arg->scanrangeyMin;
        max1 = arg->scanrangeyMax;
    }

    hCL = new TH1F("hCL" + getUniqueRootName(), "hCL" + pdf->getPdfName(), nPoints1d, min1, max1);
    if ( hChi2min ) delete hChi2min;
    hChi2min = new TH1F("hChi2min" + getUniqueRootName(), "hChi2min" + pdf->getPdfName(), nPoints1d, min1, max1);

    if (hCLs) delete hCLs;
    hCLs = new TH1F("hCLs" + getUniqueRootName(), "hCLs" + pdf->getPdfName(), nPoints1d, min1, max1);


    // fill the chi2 histogram with very unlikely values such
    // that inside scan1d() the if clauses work correctly
    for ( int i = 1; i <= nPoints1d; i++ ) hChi2min->SetBinContent(i, 1e6);

    ///////////////////////////////////////////////////////////////////////////
    // setup everything for 2D scan
    if ( scanVar2!="" )
    {
        RooRealVar *par2 = w->var(scanVar2);
        if ( !par2 ){
            if ( arg->debug ) cout << "MethodDatasetsProbScan::initScan() : ";
            cout << "ERROR : No such scan parameter: " << scanVar2 << endl;
            cout << "        Choose an existing one using: --var par" << endl << endl;
            cout << "  Available parameters:" << endl;
            cout << "  ---------------------" << endl << endl;
            pdf->printParameters();
            exit(EXIT_FAILURE);
        }
        if ( !m_yrangeset && arg->scanrangeyMin != arg->scanrangeyMax ){
            setYscanRange(arg->scanrangeyMin,arg->scanrangeyMax);
        }
        // setLimit(w, scanVar2, "scan");
        // float min2 = par2->getMin();
        // float max2 = par2->getMax();
        float min2 = arg->scanrangeyMin;
        float max2 = arg->scanrangeyMax;

        hCL2d      = new TH2F("hCL2d"+getUniqueRootName(),      "hCL2d"+pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
        hCLs2d      = new TH2F("hCLs2d"+getUniqueRootName(),      "hCLs2d"+pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
        hChi2min2d = new TH2F("hChi2min2d"+getUniqueRootName(), "hChi2min",      nPoints2dx, min1, max1, nPoints2dy, min2, max2);

        for ( int i=1; i<=nPoints2dx; i++ )
            for ( int j=1; j<=nPoints2dy; j++ ) hChi2min2d->SetBinContent(i,j,1e6);
    }
    /////////////////////////////////////////////////////////////////////////////////


    // set start parameters

    // Set up storage for the fit results.
    // Clear before so we can call initScan() multiple times.
    // Note that allResults still needs to hold all results, so don't delete the RooFitResults.

    curveResults.clear();
    for ( int i = 0; i < nPoints1d; i++ ) curveResults.push_back(0);

    //////////////////////////////////////////////////////////
    // Titus: 2d:
    curveResults2d.clear();
    for ( int i=0; i<nPoints2dx; i++ )
    {
        vector<RooSlimFitResult*> tmp;
        for ( int j=0; j<nPoints2dy; j++ ) tmp.push_back(0);
        curveResults2d.push_back(tmp);
    }
    //////////////////////////////////////////////////////////

    // turn off some messages
    RooMsgService::instance().setStreamStatus(0, kFALSE);
    RooMsgService::instance().setStreamStatus(1, kFALSE);

    // Perform the fits needed for later (the global minimum and the background)
    // free data fit
    w->var(scanVar1)->setConstant(false);
    globalMin = loadAndFit(pdf); // fit on data free
    assert(globalMin);
    globalMin->SetName("globalMin");
    // chi2minGlobal = 2 * globalMin->minNll();
    chi2minGlobal = 2 * pdf->getMinNll();
    std::cout << "=============== Global minimum (2*-Log(Likelihood)) is: " << chi2minGlobal << endl;
    // background only
    // if ( !pdf->getBkgPdf() )
      bkgOnlyFitResult = pdf->fitBkg(pdf->getData(), arg->var[0]); // fit on data w/ bkg only hypoth
      assert(bkgOnlyFitResult);
      bkgOnlyFitResult->SetName("bkgOnlyFitResult");
      // chi2minBkg = 2 * bkgOnlyFitResult->minNll();
      chi2minBkg = 2 * pdf->getMinNllBkg();
      std::cout << "=============== Bkg minimum (2*-Log(Likelihood)) is: " << chi2minBkg << endl;
      w->var(scanVar1)->setConstant(false);
      if (chi2minBkg<chi2minGlobal)
      {
          std::cout << "WARNING: BKG MINIMUM IS LOWER THAN GLOBAL MINIMUM! The likelihoods are screwed up! Set bkg minimum to global minimum for consistency." << std::endl;
          chi2minBkg = chi2minGlobal;
          std::cout << "=============== New bkg minimum (2*-Log(Likelihood)) is: " << chi2minBkg << endl;
      }
    // else if ( arg->cls.size()!=0 ){
    //     std::cout << "**************************************************************************************************************************************" << std::endl;
    //     std::cout << "WARNING: No Bkg PDF is given! Will calculate CLs method by assuming the bkgchi2 to be the chi2 of the first bin." << std::endl;
    //     std::cout << "WARNING: This is only an approximate solution and MIGHT EVEN BE WRONG, if the first bin does not represent the background expectation!" << std::endl;
    //     std::cout << "**************************************************************************************************************************************" << std::endl;

    //     w->var(scanVar1)->setVal(min1);
    //     w->var(scanVar1)->setConstant(true);
    //     bkgOnlyFitResult = loadAndFit(pdf); // fit on data w/ bkg only hypoth
    //     assert(bkgOnlyFitResult);
    //     // chi2minBkg = 2 * bkgOnlyFitResult->minNll();
    //     chi2minBkg = 2 * pdf->getMinNll();
    //     std::cout << "=============== Bkg minimum (2*-Log(Likelihood)) is: 2*" << bkgOnlyFitResult->minNll() << " = " << chi2minBkg << endl;
    //     w->var(scanVar1)->setConstant(false);
    // }


    if (arg->debug) {
        std::cout << "DEBUG in MethodDatasetsProbScan::initScan() - Scan initialized successfully!\n" << std::endl;
    }
}

void MethodDatasetsProbScan::loadScanFromFile(TString fileNameBaseIn) {
    if ( arg->debug ) cout << "MethodDatasetsProbScan::loadFromFile() : loading ..." << endl;

    TChain* c = new TChain("plugin");
    TString file = Form("root/scan1dDatasetsProb_" + this->pdf->getName() + "_%ip" + "_" + scanVar1 + ".root", arg->npoints1d);
    Utils::assertFileExists(file);
    c->Add(file);

    // load the tree and histograms
    this->probScanTree = new ToyTree(this->pdf, this->arg, c);
    this->sethCLFromProbScanTree();

    // load the fit results
    loadFitResults(file);
}

void MethodDatasetsProbScan::loadFitResults(TString file) {

  Utils::assertFileExists(file);
  TFile *tf = TFile::Open(file);

  if ( pdf->getBkgPdf() ) {
    bkgOnlyFitResult  = (RooFitResult*)((RooFitResult*)tf->Get("bkgOnlyFitResult"))->Clone("bkgOnlyFitResult"+getUniqueRootName());

    if (!bkgOnlyFitResult) {
      cout << "MethodDatasetsProbScan::loadFitResults() : ERROR - bkgOnlyFitResult not found in file " << file << endl;
      exit(1);
    }
  }

  globalMin = (RooFitResult*)((RooFitResult*)tf->Get("globalMin"))->Clone("globalMin"+getUniqueRootName());

  if (!globalMin) {
    cout << "MethodDatasetsProbScan::loadFitResults() : ERROR - globalMin not found in file " << file << endl;
  }

  tf->Close();
  delete tf;

}


void MethodDatasetsProbScan::sethCLFromProbScanTree() {
    this->probScanTree->open();
    float halfBinWidth = (this->probScanTree->getScanpointMax() - this->probScanTree->getScanpointMin()) / ((float)this->probScanTree->getScanpointN()) / 2; //-1.)/2;
    /// \todo replace this such that there's always one bin per scan point, but still the range is the scan range.
    /// \todo Also, if we use the min/max from the tree, we have the problem that they are not exactly
    /// the scan range, so that the axis won't show the lowest and highest number.
    /// \todo If the scan range was changed after the toys were generate, we absolutely have
    /// to derive the range from the root files - else we'll have bining effects.
    delete hCL;
    this->hCL = new TH1F("hCL", "hCL", this->probScanTree->getScanpointN(), this->probScanTree->getScanpointMin() - halfBinWidth, this->probScanTree->getScanpointMax() + halfBinWidth);
    if (arg->debug) printf("DEBUG %i %f %f %f\n", this->probScanTree->getScanpointN(), this->probScanTree->getScanpointMin() - halfBinWidth, this->probScanTree->getScanpointMax() + halfBinWidth, halfBinWidth);
    delete hCLs;
    this->hCLs = new TH1F("hCLs", "hCLs", this->probScanTree->getScanpointN(), this->probScanTree->getScanpointMin() - halfBinWidth, this->probScanTree->getScanpointMax() + halfBinWidth);
    if (arg->debug) printf("DEBUG %i %f %f %f\n", this->probScanTree->getScanpointN(), this->probScanTree->getScanpointMin() - halfBinWidth, this->probScanTree->getScanpointMax() + halfBinWidth, halfBinWidth);
    Long64_t nentries     = this->probScanTree->GetEntries();
    this->probScanTree->activateAllBranches();
    // this->probScanTree->a - DATASETSctivateCoreBranchesOnly(); //< speeds up the event loop
    for (Long64_t i = 0; i < nentries; i++)
    {
        // load entry
        this->probScanTree->GetEntry(i);

        double deltaChi2 = probScanTree->chi2min - probScanTree->chi2minGlobal;
				double oneMinusCL = TMath::Prob( deltaChi2, 1);
				// save the value and corresponding fit result
				// but only if an improvement
				if ( hCL->GetBinContent(hCL->FindBin( probScanTree->scanpoint ) ) <= oneMinusCL ) {
					hCL->SetBinContent( hCL->FindBin( probScanTree->scanpoint ) , oneMinusCL );
					hChi2min->SetBinContent( hCL->FindBin( probScanTree->scanpoint ), probScanTree->chi2min );
				}
        // and whilst we here do the same relative to the background only hypothesis (i.e. for CLs method)
        double deltaChi2Bkg  = probScanTree->chi2min - probScanTree->chi2minBkg;
        //double oneMinusCLBkg = TMath::Prob( deltaChi2Bkg, 1);
        double oneMinusCLBkg = getPValueTTestStatistic( deltaChi2Bkg, true );
        if ( hCLs->GetBinCenter( hCLs->FindBin( probScanTree->scanpoint ) ) <= oneMinusCLBkg ) {
            hCLs->SetBinContent( hCLs->FindBin( probScanTree->scanpoint ), oneMinusCLBkg );
        }
    }
    // if only the plot method then set the global and bkg minimums from the file
    if ( arg->isAction("plot") ) {
      chi2minGlobal = probScanTree->chi2minGlobal;
      chi2minBkg    = probScanTree->chi2minBkg;
    }

	// put in best fit value
	hCL->SetBinContent(hCL->FindBin( probScanTree->scanbest ),1.);
	hChi2min->SetBinContent(hCL->FindBin( probScanTree->scanbest),chi2minGlobal);
    hCLs->SetBinContent(hCLs->FindBin( probScanTree->scanbest ), 1.);

	cout << "Best fit at: scanVar  = " << probScanTree->scanbest << " with Chi2Min: " << chi2minGlobal << endl;

	sortSolutions();
	//saveSolutions();
// this->probScanTree->activateAllBranches(); //< Very important!
//
}



///////////////////////////////////////////////////
///
/// Prepare environment depending on data or toy fit
///
/// \param pdf      the pdf that is to be fitted.
///
////////////////////////////////////////////////////
RooFitResult* MethodDatasetsProbScan::loadAndFit(PDF_Datasets* pdf) {

    // we want to fit to data
    // first, try to load the measured values of the global observables from a snapshot
    if (!w->loadSnapshot(pdf->globalObsDataSnapshotName)) {
        std::cout << "FATAL in MethodDatasetsProbScan::loadAndFit() - No snapshot globalObsToySnapshotName found!\n" << std::endl;
        exit(EXIT_FAILURE);
    };
    // then, fit the pdf while passing it the dataset
    return pdf->fit(pdf->getData());
};

///
/// load Parameter limits
/// by default the "free" limit is loaded, can be changed to "phys" by command line argument
///
void MethodDatasetsProbScan::loadParameterLimits() {
    TString rangeName = arg->enforcePhysRange ? "phys" : "free";
    if ( arg->debug ) cout << "DEBUG in Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << endl;
    TIterator* it = w->set(pdf->getParName())->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ) setLimit(w, p->GetName(), rangeName);
    delete it;
}


///
/// Print settings member of MethodDatasetsProbScan
///
void MethodDatasetsProbScan::print() {
    cout << "########################## Print MethodDatasetsProbScan Class ##########################" << endl;
    cout << "\t --- " << "Method Name: \t\t\t" << methodName << endl;
    cout << "\t --- " << "Instance Name: \t\t\t" << name << endl;
    cout << "\t --- " << "Instance Title: \t\t\t" << title << endl;
    cout << "\t --- " << "Scan Var Name: \t\t\t" << scanVar1 << endl;
    if ( arg->var.size() > 1 )   cout << "\t --- " << "2nd Scan Var Name: \t\t" << scanVar2 << endl;
    cout << "\t --- " << "Number of Scanpoints 1D: \t\t" << nPoints1d << endl;
    cout << "\t --- " << "Number of Scanpoints x 2D: \t" << nPoints2dx << endl;
    cout << "\t --- " << "Number of Scanpoints y 2D: \t" << nPoints2dy << endl;
    cout << "\t --- " << "PDF Name: \t\t\t\t" << pdf->getPdfName() << endl;
    cout << "\t --- " << "Observables Name: \t\t\t" <<  pdf->getObsName() << endl;
    cout << "\t --- " << "Parameters Name: \t\t\t" <<  pdf->getParName() << endl;
    cout << "\t --- " << "Global minimum Chi2: \t\t" << chi2minGlobal << endl;
    cout << "---------------------------------" << endl;
    cout << "\t --- Scan Var " << scanVar1 << " from " << getScanVar1()->getMin("scan")
         << " to " << getScanVar1()->getMax("scan") << endl;
    cout << "---------------------------------" << endl;
}

///
/// Perform the 1d Prob scan.
/// Saves chi2 values and the prob-Scan p-values in a root tree
/// For the datasets stuff, we do not yet have a MethodDatasetsProbScan class, so we do it all in
/// MethodDatasetsProbScan
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodDatasetsProbScan::scan1d(bool fast, bool reverse, bool quiet)
{
	if (fast) return 0; // tmp

	if ( arg->debug ) cout << "MethodDatasetsProbScan::scan1d() : starting ... " << endl;

    // Set limit to all parameters.
    this->loadParameterLimits(); /// Default is "free", if not changed by cmd-line parameter


    // Define scan parameter and scan range.
    RooRealVar *parameterToScan = w->var(scanVar1);
    float parameterToScan_min = hCL->GetXaxis()->GetXmin();
    float parameterToScan_max = hCL->GetXaxis()->GetXmax();

		// do a free fit
		RooFitResult *result = this->loadAndFit(this->pdf); // fit on data
		assert(result);
    RooSlimFitResult *slimresult = new RooSlimFitResult(result,true);
		slimresult->setConfirmed(true);
		solutions.push_back(slimresult);
        Utils::setParameters(w,result); // Set parameters to result (necessary to get correct freeDataFitValue if using a multipdf)
		double freeDataFitValue = w->var(scanVar1)->getVal();

    // Define outputfile
    system("mkdir -p root");
    TString probResName = Form("root/scan1dDatasetsProb_" + this->pdf->getName() + "_%ip" + "_" + scanVar1 + ".root", arg->npoints1d);
    TFile* outputFile = new TFile(probResName, "RECREATE");

    // Set up toy root tree
    this->probScanTree = new ToyTree(this->pdf, arg);
    this->probScanTree->init();
    this->probScanTree->nrun = -999; //\todo: why does this branch even exist in the output tree of the prob scan?

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    RooDataSet* parsFunctionCall = new RooDataSet("parsFunctionCall", "parsFunctionCall", *w->set(pdf->getParName()));
    parsFunctionCall->add(*w->set(pdf->getParName()));

    // start scan
    cout << "MethodDatasetsProbScan::scan1d_prob() : starting ... with " << nPoints1d << " scanpoints..." << endl;
    ProgressBar progressBar(arg, nPoints1d);
    for ( int i = 0; i < nPoints1d; i++ )
    {
        progressBar.progress();
        // scanpoint is calculated using min, max, which are the hCL x-Axis limits set in this->initScan()
        // this uses the "scan" range, as expected
        // don't add half the bin size. try to solve this within plotting method

        float scanpoint = parameterToScan_min + (parameterToScan_max - parameterToScan_min) * (double)i / ((double)nPoints1d - 1);
				if (arg->debug) cout << "DEBUG in MethodDatasetsProbScan::scan1d_prob() " << scanpoint << " " << parameterToScan_min << " " << parameterToScan_max << endl;

        this->probScanTree->scanpoint = scanpoint;

        if (arg->debug) cout << "DEBUG in MethodDatasetsProbScan::scan1d_prob() - scanpoint in step " << i << " : " << scanpoint << endl;

        // don't scan in unphysical region
        // by default this means checking against "free" range
        if ( scanpoint < parameterToScan->getMin() || scanpoint > parameterToScan->getMax() + 2e-13 ) {
            cout << "it seems we are scanning in an unphysical region: " << scanpoint << " < " << parameterToScan->getMin() << " or " << scanpoint << " > " << parameterToScan->getMax() + 2e-13 << endl;
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

        RooFitResult *result = this->loadAndFit(this->pdf); // fit on data
        assert(result);

        if (arg->debug) {
            cout << "DEBUG in MethodDatasetsProbScan::scan1d_prob() - minNll data scan at scan point " << scanpoint << " : " << 2 * result->minNll() << ": "<< 2 * pdf->getMinNll() << endl;
        }
        this->probScanTree->statusScanData = result->status();

        // set chi2 of fixed fit: scan fit on data
        // CAVEAT: chi2min from fitresult gives incompatible results to chi2min from pdf
        // this->probScanTree->chi2min           = 2 * result->minNll();
        this->probScanTree->chi2min           = 2 * pdf->getMinNll();
        this->probScanTree->covQualScanData   = result->covQual();
        this->probScanTree->scanbest  = freeDataFitValue;

        // After doing the fit with the parameter of interest constrained to the scanpoint,
        // we are now saving the fit values of the nuisance parameters. These values will be
        // used to generate toys according to the PLUGIN method.
        // After doing the fit with the parameter of interest constrained to the scanpoint,
        // we are now saving the fit values of the nuisance parameters. These values will be
        // used to generate toys according to the PLUGIN method.
        //
        // Firstly save the parameter values from the workspace using storeParsScan(). If using
        // a multipdf, this means that all parameters are close to their scan fit values, which
        // should help with convergence.
        // Then save parameter values from the best fit result using storeParsScan(result). If
        // using a multipdf, this means the values of the parameters which appear in the best
        // pdf are set to the values from the fit using that pdf, so S+B toys are generated with
        // the correct nuisance parameter values. If not using a multipdf, this command is identical
        // to storeParsScan()
        this->probScanTree->storeParsScan(); // \todo : figure out which one of these is semantically the right one
        this->probScanTree->storeParsScan();
        this->probScanTree->storeParsScan(result);
        this->probScanTree->bestIndexScanData = pdf->getBestIndex();

        this->pdf->deleteNLL();

        // also save the chi2 of the free data fit to the tree:
        this->probScanTree->chi2minGlobal = this->getChi2minGlobal();
        probScanTree->covQualFree = globalMin->covQual();
        probScanTree->statusFree = globalMin->status();
        this->probScanTree->chi2minBkg = this->getChi2minBkg();
        if(bkgOnlyFitResult){
            probScanTree->statusFreeBkg = bkgOnlyFitResult->status();
            probScanTree->covQualFreeBkg = bkgOnlyFitResult->covQual();
        }

        this->probScanTree->genericProbPValue = this->getPValueTTestStatistic(this->probScanTree->chi2min - this->probScanTree->chi2minGlobal);
        this->probScanTree->fill();

        if(arg->debug && pdf->getBkgPdf())
        {
            float pval_cls = this->getPValueTTestStatistic(this->probScanTree->chi2min - this->probScanTree->chi2minBkg, true);
            cout << "DEBUG in MethodDatasetsProbScan::scan1d() - p value CLs: " << pval_cls << endl;
        }


        // reset
        setParameters(w, pdf->getParName(), parsFunctionCall->get(0));
        //setParameters(w, pdf->getObsName(), obsDataset->get(0));
    } // End of npoints loop
    probScanTree->writeToFile();
    if (bkgOnlyFitResult) bkgOnlyFitResult->Write();
    if (globalMin) globalMin->Write();
    outputFile->Close();
    std::cout << "Wrote ToyTree to file" << std::endl;
    delete parsFunctionCall;

    // This is kind of a hack. The effect is supposed to be the same as callincg
    // this->sethCLFromProbScanTree(); here, but the latter gives a segfault somehow....
    // \todo: use this->sethCLFromProbScanTree() directly after figuring out the cause of the segfault.
    this->loadScanFromFile();

    return 0;
}


int MethodDatasetsProbScan::computeCLvalues(){
    std::cout << "Computing CL values based on test statistic decision" << std::endl;
    std::cout << "Using "<< arg->teststatistic <<"-sided test statistic" << std::endl;
    float bestfitpoint = ((RooRealVar*) globalMin->floatParsFinal().find(scanVar1))->getVal();
    float bestfitpointerr = ((RooRealVar*) globalMin->floatParsFinal().find(scanVar1))->getError();

    for (int k=1; k<=hCL->GetNbinsX(); k++){
        float scanvalue=hChi2min->GetBinCenter( k);
        float teststat_measured = hChi2min->GetBinContent(k) - chi2minGlobal;
        float CLb = 1. - (normal_cdf(TMath::Sqrt(teststat_measured) + ((scanvalue - 0.)/bestfitpointerr)) + normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.)/bestfitpointerr)) - 1.);
        if (arg->teststatistic ==1){ // use one-sided test statistic
            teststat_measured = bestfitpoint <= scanvalue ? teststat_measured : 0.; // if mu < muhat then q_mu = 0
            hCL->SetBinContent(k,1. - normal_cdf(TMath::Sqrt(teststat_measured)));
            // if (scanvalue < bestfitpoint) hCL->SetBinContent(k,1.0);    // should not be here, but looks ugly if solution is drawn as p=1
            CLb = 1. - normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.)/bestfitpointerr));
        }
        hCLs->SetBinContent(k,min(1.,hCL->GetBinContent(k)/CLb));
    }
    return 0;
}




// sanity Checks for 2D scan \TODO: Idea: enlargen this function to be used for all scans
void MethodDatasetsProbScan::sanityChecks()
{
    // if ( !w->set(parsName) ){
    //     cout << "MethodDatasetsProbScan::sanityChecks() : ERROR : parsName not found: " << parsName << endl;
    //     exit(1);
    // }
    if ( !w->var(scanVar1) ){
        cout << "MethodDatasetsProbScan::sanityChecks() : ERROR : scanVar1 not found: " << scanVar1 << endl;
        exit(1);
    }
    if ( !w->var(scanVar2) ){
        cout << "MethodDatasetsProbScan::sanityChecks() : ERROR : scanVar2 not found: " << scanVar2 << endl;
        exit(1);
    }
}


// Working at 2D scan by first copying the original ProbScan
int MethodDatasetsProbScan::scan2d()
{
    if ( arg->debug ) cout << "MethodDatasetsProbScan::scan2d() : starting ..." << endl;
    nScansDone++;
    sanityChecks();
    if ( startPars ) delete startPars;

    // Define whether the 2d contours in hCL are "1D sigma" (ndof=1) or "2D sigma" (ndof=2).
    // Titus: Change this to 2, since there is no reason to do wrong hCL contours.
    int ndof = 2;

    // Set up storage for fit results of this particular
    // scan. This is used for the drag start parameters.
    // We cannot use the curveResults2d member because that
    // only holds better results.
    vector<vector<RooSlimFitResult*> > mycurveResults2d;
    for ( int i=0; i<nPoints2dx; i++ ){
        vector<RooSlimFitResult*> tmp;
        for ( int j=0; j<nPoints2dy; j++ ) tmp.push_back(0);
        mycurveResults2d.push_back(tmp);
    }

    // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // // Titus: Saving is done via saveSolutions2d() in the combination, but maybe we need it inline for now \todo: implement a saving function
    // // Define outputfile
    // system("mkdir -p root");
    // TString probResName = Form("root/scan1dDatasetsProb_" + this->pdf->getName() + "_%ip" + "_" + scanVar1 + ".root", arg->npoints1d);
    // TFile* outputFile = new TFile(probResName, "RECREATE");

    // // Set up toy root tree
    // this->probScanTree = new ToyTree(this->pdf, arg);
    // this->probScanTree->init();
    // this->probScanTree->nrun = -999; //\todo: why does this branch even exist in the output tree of the prob scan?
    // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // store start parameters so we can reset them later
    startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
    startPars->add(*w->set(parsName));

    // // start scan from global minimum (not always a good idea as we need to set from other places as well)
    // setParameters(w, parsName, globalMin);

    // Define scan parameters and scan range:
    RooRealVar *par1 = w->var(scanVar1);
    RooRealVar *par2 = w->var(scanVar2);

    // Set limit to all parameters.
    this->loadParameterLimits();

    // fix scan parameters
    par1->setConstant(true);
    par2->setConstant(true);

    // Report on the smallest new minimum we come across while scanning.
    // Sometimes the scan doesn't find the minimum
    // that was found before. Warn if this happens.
    double bestMinOld = chi2minGlobal;
    double bestMinFoundInScan = 100.;

    // for the status bar
    int nSteps = 0;
    float nTotalSteps = nPoints2dx*nPoints2dy;
    float printFreq = nTotalSteps>100 && !arg->probforce ? 100 : nTotalSteps; ///< number of messages

    // initialize some control plots
    gStyle->SetOptTitle(1);
    TCanvas *cDbg = newNoWarnTCanvas(getUniqueRootName(), Form("DeltaChi2 for 2D scan %i",nScansDone));
    cDbg->SetMargin(0.1,0.15,0.1,0.1);
    float hChi2min2dMin = hChi2min2d->GetMinimum();
    bool firstScanDone = hChi2min2dMin<1e5;
    TH2F *hDbgChi2min2d = histHardCopy(hChi2min2d, firstScanDone, true);
    hDbgChi2min2d->SetTitle(Form("#Delta#chi^{2} for scan %i, %s",nScansDone,title.Data()));
    if ( firstScanDone ) hDbgChi2min2d->GetZaxis()->SetRangeUser(hChi2min2dMin,hChi2min2dMin+25);
    hDbgChi2min2d->GetXaxis()->SetTitle(par1->GetTitle());
    hDbgChi2min2d->GetYaxis()->SetTitle(par2->GetTitle());
    hDbgChi2min2d->GetZaxis()->SetTitle("#Delta#chi^{2}");
    TH2F *hDbgStart = histHardCopy(hChi2min2d, false, true);


    // start coordinates //Titus: start at the global minimum
    // don't allow the under/overflow bins
    int iStart = min(hCL2d->GetXaxis()->FindBin(par1->getVal()), hCL2d->GetNbinsX());
    int jStart = min(hCL2d->GetYaxis()->FindBin(par2->getVal()), hCL2d->GetNbinsY());
    iStart = max(iStart, 1);
    jStart = max(jStart, 1);
    hDbgStart->SetBinContent(iStart, jStart, 500.);
    TMarker *startpointmark = new TMarker(par1->getVal(),par2->getVal(),3);

    // timer
    TStopwatch tFit;
    TStopwatch tSlimResult;
    TStopwatch tScan;
    TStopwatch tMemory;

    // set up the scan spiral
    int X = 2*nPoints2dx;
    int Y = 2*nPoints2dy;
    int x,y,dx,dy;
    x = y = dx = 0;
    dy = -1;
    int t = std::max(X,Y);
    int maxI = t*t;

    for ( int spiralstep=0; spiralstep<maxI; spiralstep++ )
    {
        if ((-X/2 <= x) && (x <= X/2) && (-Y/2 <= y) && (y <= Y/2))
        {
            int i = x+iStart;
            int j = y+jStart;
            if ( i>0 && i<=nPoints2dx && j>0 && j<=nPoints2dy )
            {
                tScan.Start(false);

                // status bar
                if (((int)nSteps % (int)(nTotalSteps/printFreq)) == 0){
                    cout << Form("MethodDatasetsProbScan::scan2d() : scanning %3.0f%%", (float)nSteps/(float)nTotalSteps*100.)
                                                             << "       \r" << flush;
                }
                nSteps++;

                // status histogram
                if ( spiralstep>0 ) hDbgStart->SetBinContent(i, j, 500./*firstScan ? 1. : hChi2min2dMin+36*/);

                // set start parameters from inner turn of the spiral
                int xStartPars, yStartPars;
                computeInnerTurnCoords(iStart, jStart, i, j, xStartPars, yStartPars, 1);
                RooSlimFitResult *rStartPars = mycurveResults2d[xStartPars-1][yStartPars-1];
                if ( rStartPars ) setParameters(w, parsName, rStartPars);

                // memory management:
                tMemory.Start(false);
                // delete old, inner fit results, that we don't need for start parameters anymore
                // for this we take the second-inner-most turn.
                int iOld, jOld;
                bool innerTurnExists = computeInnerTurnCoords(iStart, jStart, i, j, iOld, jOld, 2);
                if ( innerTurnExists ){
                    deleteIfNotInCurveResults2d(mycurveResults2d[iOld-1][jOld-1]);
                    mycurveResults2d[iOld-1][jOld-1] = 0;
                }
                tMemory.Stop();

                // alternative choice for start parameters: always from what we found at function call
                // setParameters(w, parsName, startPars->get(0));

                // set scan point
                float scanvalue1 = hCL2d->GetXaxis()->GetBinCenter(i);
                float scanvalue2 = hCL2d->GetYaxis()->GetBinCenter(j);
                par1->setVal(scanvalue1);
                par2->setVal(scanvalue2);

                // fit!
                tFit.Start(false);
                RooFitResult *fr;
                // if ( !arg->probforce ) fr = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
                // else                   fr = fitToMinForce(w, combiner->getPdfName());

                fr = this->loadAndFit(this->pdf);   //Titus: change fitting strategy to the one from the datasets \todo: should be possible to use the fittominforce etc methods
                // double chi2minScan = 2 * fr->minNll(); //Titus: take 2*minNll vs. minNll? Where is the squared in the main gammacombo?
                double chi2minScan = 2 * pdf->getMinNll();
                tFit.Stop();
                tSlimResult.Start(false);
                RooSlimFitResult *r = new RooSlimFitResult(fr); // try to save memory by using the slim fit result
                tSlimResult.Stop();
                delete fr;
                allResults.push_back(r);
                bestMinFoundInScan = TMath::Min((double)chi2minScan, (double)bestMinFoundInScan);
                mycurveResults2d[i-1][j-1] = r;

                // If we find a new global minumum, this means that all
                // previous 1-CL values are too high. We'll save the new possible solution, adjust the global
                // minimum, return a status code, and stop.
                // if ( chi2minScan > -500 && chi2minScan<chi2minGlobal ){      //Titus: the hard coded minimum chi2 to avoid ridiculous minima (e.g. at boundaries) only sensible when using the Utils::fitToMin, since the chi2 of the best fit with that fitting method is nominally 0.
                if ( chi2minScan<chi2minGlobal ){
                    // warn only if there was a significant improvement
                    if ( arg->debug || chi2minScan<chi2minGlobal-1e-2 ){
                        if ( arg->verbose ) cout << "MethodDatasetsProbScan::scan2d() : WARNING : '" << title << "' new global minimum found! chi2minGlobal="
                                                            << chi2minGlobal << " chi2minScan=" << chi2minScan << endl;
                    }
                    double deltaChi2_min = chi2minScan-chi2minGlobal;
                    chi2minGlobal = chi2minScan;
                    // recompute previous 1-CL values
                    for ( int k=1; k<=hCL2d->GetNbinsX(); k++ )
                        for ( int l=1; l<=hCL2d->GetNbinsY(); l++ ){
                            hCL2d->SetBinContent(k, l, TMath::Prob(hChi2min2d->GetBinContent(k,l)-deltaChi2_min, ndof));
                            hCLs2d->SetBinContent(k, l, TMath::Prob(hChi2min2d->GetBinContent(k,l)-chi2minBkg, ndof));
                        }
                }

                double deltaChi2 = chi2minScan - chi2minGlobal;
                double oneMinusCL = TMath::Prob(deltaChi2, ndof);
                // if ( arg->debug ) {
                //     cout << "chi2minScan: " << chi2minScan << endl;
                //     cout << "chi2minGlobal: " << chi2minGlobal << endl;
                //     cout << "deltaChi2: " << deltaChi2 << endl;
                //     cout << "ndof: " << ndof << endl;
                //     cout << "oneMinusCL: " << oneMinusCL << endl << endl;
                // }


                // Save the 1-CL value. But only if better than before!
                if ( hCL2d->GetBinContent(i, j) < oneMinusCL ){
                    hCL2d->SetBinContent(i, j, oneMinusCL);
                    double cls_pval = chi2minScan > chi2minBkg ? chi2minScan - chi2minBkg : 0.;
                    hCLs2d->SetBinContent(i, j, TMath::Prob(cls_pval, ndof));
                    hChi2min2d->SetBinContent(i, j, deltaChi2);
                    hDbgChi2min2d->SetBinContent(i, j, chi2minScan);
                    curveResults2d[i-1][j-1] = r;
                }

                // draw/update histograms - doing only every nth update
                // depending on the value of updateFreq
                // saves a lot of time for small combinations
                if ( ( arg->interactive && ((int)nSteps % arg->updateFreq == 0) ) || nSteps==nTotalSteps ){
                    hDbgChi2min2d->Draw("colz");
                    hDbgStart->Draw("boxsame");
                    startpointmark->Draw();
                    cDbg->Update();
                }
                tScan.Stop();
            }
        }
        // spiral stuff:
        if( (x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1-y)))
        {
            t = dx;
            dx = -dy;
            dy = t;
        }
        x += dx;
        y += dy;
    }
    cout << "MethodDatasetsProbScan::scan2d() : scan done.            " << endl;
    if ( arg->debug ){
        cout << "MethodDatasetsProbScan::scan2d() : full scan time:             "; tScan.Print();
        cout << "MethodDatasetsProbScan::scan2d() : - fitting:                  "; tFit.Print();
        cout << "MethodDatasetsProbScan::scan2d() : - create RooSlimFitResults: "; tSlimResult.Print();
        cout << "MethodDatasetsProbScan::scan2d() : - memory management:        "; tMemory.Print();
    }
    setParameters(w, parsName, startPars->get(0));

    saveSolutions2d();
    if ( arg->debug ) printLocalMinima();
    // confirmSolutions(); //Titus: Leave this out for now, since using it requires compatibility to Utils:fitToMin(), which is not achieved yet

    // clean all fit results that didn't make it into the final result
    for ( int i=0; i<allResults.size(); i++ ){
        deleteIfNotInCurveResults2d(allResults[i]);
    }

    if ( bestMinFoundInScan-bestMinOld > 0.1 )
    {
        cout << "MethodDatasetsProbScan::scan2d() : WARNING: Scan didn't find minimum that was found before!" << endl;
        cout << "MethodDatasetsProbScan::scan2d() :          Are you using too strict parameter limits?" << endl;
        cout << "MethodDatasetsProbScan::scan2d() :          min chi2 found in scan: " << bestMinFoundInScan << ", old min chi2: " << bestMinOld << endl;
        return 1;
    }

  // cleanup
  if (hDbgChi2min2d) delete hDbgChi2min2d;
  if (hDbgStart) delete hDbgStart;

    return 0;
}


double MethodDatasetsProbScan::getPValueTTestStatistic(double test_statistic_value, bool isCLs) {
    if ( test_statistic_value > 0) {
        // this is the normal case
        return TMath::Prob(test_statistic_value, 1);
    } else if(!isCLs){
        if (arg->verbose) {
        cout << "MethodDatasetsProbScan::scan1d_prob() : WARNING : Test statistic is negative, forcing it to zero" << std::endl
             << "Fit at current scan point has higher likelihood than free fit." << std::endl
             << "This should not happen except for very small underflows when the scan point is at the best fit value. " << std::endl
             << "Value of test statistic is " << test_statistic_value << std::endl
             << "An equal upwards fluctuaion corresponds to a p value of " << TMath::Prob(abs(test_statistic_value), 1) << std::endl;
        }
        // TMath::Prob will return 0 if the Argument is slightly below zero. As we are working with a float-zero we can not rely on it here:
        // TMath::Prob( 0 ) returns 1
        return 1.;
    }
    else {
        if (arg->verbose) cout << "MethodDatasetsProbScan::scan1d_prob() : WARNING : CLs test statistic is negative, forcing it to zero" << std::endl;
        return 1.;
    }
}
//////////////////////////////////////////////
// Have to overload the loadScanner function
// as we need to pick up the tree as well
// when loading
//////////////////////////////////////////////
bool MethodDatasetsProbScan::loadScanner(TString fName) {
	MethodAbsScan::loadScanner(fName);
	if ( scanVar2=="" ) this->loadScanFromFile();
	return true;
}

/////////////////////////////////////////////
// Have a nice plotting functiong
//
/////////////////////////////////////////////
void MethodDatasetsProbScan::plotFitRes(TString fName) {

  for (int i=0; i<pdf->getFitObs().size(); i++) {
    TString fitVar = pdf->getFitObs()[i];

      if(!w->var(fitVar)){
        std::cerr << "ERROR::MethodDatasetsProbScan::plotFitRes(): the variable " << fitVar << " is not present in the workspace."<< std::endl;
        std::cerr << "Candidates are: ";
        TIterator* it =  pdf->getObservables()->createIterator();
        while (RooRealVar* obs = dynamic_cast<RooRealVar*>(it->Next())) {
            std::cerr <<" "<<obs->GetName();
        }
        std::cerr<<". Will not plot."<<std::endl;
        return;
      }
    TCanvas *fitCanv = newNoWarnTCanvas( getUniqueRootName(), Form("S+B and B only fits to the dataset for %s",fitVar.Data()) );
    TLegend *leg = new TLegend(0.6,0.7,0.92,0.92);
    leg->SetFillColor(0);
    leg->SetLineColor(0);
    RooPlot *plot = w->var(fitVar)->frame();
    // bkg pdf
    if ( !bkgOnlyFitResult ) {
      cout << "MethodDatasetsProbScan::plotFitRes() : ERROR : bkgOnlyFitResult is NULL" << endl;
      exit(1);
    }
    setParameters(w, bkgOnlyFitResult);
    // if ( !w->pdf(pdf->getBkgPdfName()) ) {
    //   cout << "MethodDatasetsProbScan::plotFitRes() : ERROR : No background pdf " << pdf->getBkgPdfName() << " found in workspace" << endl;
    //   exit(1);
    // }
    if( pdf->getBkgPdf() ){
        w->pdf(pdf->getBkgPdfName())->plotOn( plot, LineColor(kRed), RooFit::Normalization( w->pdf( pdf->getBkgPdfName() )->expectedEvents( *pdf->getObservables() ), RooAbsReal::NumEvent ) );
        leg->AddEntry( plot->getObject(plot->numItems()-1), "Background Only Fit", "L");
    }
    else{
        cout << "MethodDatasetsProbScan::plotFitRes() : WARNING : No background pdf is given. Will plot S+B hypothesis with S=0." << std::endl;
        std::cout <<   w->pdf( pdf->getPdfName() )->expectedEvents( *pdf->getObservables() ) << std::endl;
        w->pdf(pdf->getPdfName())->plotOn( plot, LineColor(kRed), RooFit::Normalization( w->pdf( pdf->getPdfName() )->expectedEvents( *pdf->getObservables() ), RooAbsReal::NumEvent ) );
        leg->AddEntry( plot->getObject(plot->numItems()-1), "Background Only Fit", "L");
    }
    // free fit
    if ( !globalMin ) {
      cout << "MethodDatasetsProbScan::plotFitRes() : ERROR : globalMin is NULL" << endl;
      exit(1);
    }
    setParameters(w, globalMin);
    if ( !w->pdf(pdf->getPdfName()) ) {
      cout << "MethodDatasetsProbScan::plotFitRes() : ERROR : No pdf " << pdf->getPdfName() << " found in workspace" << endl;
      exit(1);
    }
    w->pdf(pdf->getPdfName())->plotOn(plot, RooFit::Normalization( w->pdf( pdf->getPdfName() )->expectedEvents( *pdf->getObservables() ), RooAbsReal::NumEvent ) );
    leg->AddEntry( plot->getObject(plot->numItems()-1), "Free Fit", "L");
    // data unblinded if needed
    map<TString,TString> unblindRegs = pdf->getUnblindRegions();
    if ( unblindRegs.find( fitVar ) != unblindRegs.end() ) {
      w->data(pdf->getDataName())->plotOn( plot, CutRange(pdf->getUnblindRegions()[fitVar]) );
      leg->AddEntry( plot->getObject(plot->numItems()-1), "Data", "LEP");
    }
    plot->Draw();
    leg->Draw("same");
    savePlot(fitCanv, fName);
  }

}
