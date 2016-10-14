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
      dataFreeFitResult   (NULL),
      probScanTree        (NULL)
{
    chi2minGlobalFound = true; // the free fit to data must be done and must be saved to the workspace before gammacombo is even called
    methodName = "DatasetsProb";
    w = PDF->getWorkspace();
    title = PDF->getTitle();
    name =  PDF->getName();

    if ( arg->var.size() > 1 ) scanVar2 = arg->var[1];
    inputFiles.clear();

    if (w->obj("data_fit_result") == NULL) { //\todo: support passing the name of the fit result in the workspace.
        cerr << "ERROR: The workspace must contain the fit result of the fit to data. The name of the fit result must be 'data_fit_result'. " << endl;
        exit(EXIT_FAILURE);
    }
    dataFreeFitResult = (RooFitResult*) w->obj("data_fit_result");
    chi2minGlobal = 2 * dataFreeFitResult->minNll();
    std::cout << "=============== Global Minimum (2*-Log(Likelihood)) is: 2*" << dataFreeFitResult->minNll() << " = " << chi2minGlobal << endl;

    if ( !w->set(pdf->getObsName()) ) {
        cerr << "MethodDatasetsProbScan::MethodDatasetsProbScan() : ERROR : no '" + pdf->getObsName() + "' set found in workspace" << endl;
        cerr << " You can specify the name of the set in the workspace using the pdf->initObservables(..) method.";
        exit(EXIT_FAILURE);
    }
    if ( !w->set(pdf->getParName()) ) {
        cerr << "MethodDatasetsProbScan::MethodDatasetsProbScan() : ERROR : no '" + pdf->getParName() + "' set found in workspace" << endl;
        exit(EXIT_FAILURE);
    }
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
    if ( arg->scanrangeMin != arg->scanrangeMax ) par1->setRange("scan", arg->scanrangeMin, arg->scanrangeMax);
    Utils::setLimit(w, scanVar1, "scan");

    if (hCL) delete hCL;
    hCL = new TH1F("hCL" + getUniqueRootName(), "hCL" + pdf->getPdfName(), nPoints1d, par1->getMin(), par1->getMax());
    if ( hChi2min ) delete hChi2min;
    hChi2min = new TH1F("hChi2min" + getUniqueRootName(), "hChi2min" + pdf->getPdfName(), nPoints1d, par1->getMin(), par1->getMax());

    // fill the chi2 histogram with very unlikely values such
    // that inside scan1d() the if clauses work correctly
    for ( int i = 1; i <= nPoints1d; i++ ) hChi2min->SetBinContent(i, 1e6);

    if ( scanVar2 != "" ) {
        cout << "MethodDatasetsProbScan::initScan(): EROR: Scanning in more than one dimension is not supported." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set up storage for the fit results.
    // Clear before so we can call initScan() multiple times.
    // Note that allResults still needs to hold all results, so don't delete the RooFitResults.

    curveResults.clear();
    for ( int i = 0; i < nPoints1d; i++ ) curveResults.push_back(0);

    // turn off some messages
    RooMsgService::instance().setStreamStatus(0, kFALSE);
    RooMsgService::instance().setStreamStatus(1, kFALSE);
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

    this->probScanTree = new ToyTree(this->pdf, this->arg, c);
    this->sethCLFromProbScanTree();

}


void MethodDatasetsProbScan::sethCLFromProbScanTree() {
    std::cout << probScanTree->t << std::endl;
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
    Long64_t nentries     = this->probScanTree->GetEntries();
    // this->probScanTree->activateCoreBranchesOnly(); //< speeds up the event loop
    for (Long64_t i = 0; i < nentries; i++)
    {
        // load entry
        this->probScanTree->GetEntry(i);
        this->hCL->SetBinContent(this->hCL->FindBin(this->probScanTree->scanpoint), this->probScanTree->genericProbPValue);
    }
    // this->probScanTree->activateAllBranches(); //< Very important!

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
int MethodDatasetsProbScan::scan1d()
{
    // Set limit to all parameters.
    this->loadParameterLimits(); /// Default is "free", if not changed by cmd-line parameter


    // Define scan parameter and scan range.
    RooRealVar *parameterToScan = w->var(scanVar1);
    float parameterToScan_min = hCL->GetXaxis()->GetXmin();
    float parameterToScan_max = hCL->GetXaxis()->GetXmax();

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
            cout << "DEBUG in MethodDatasetsProbScan::scan1d_prob() - minNll data scan at scan point " << scanpoint << " : " << 2 * result->minNll() << endl;
        }
        this->probScanTree->statusScanData = result->status();

        // set chi2 of fixed fit: scan fit on data
        this->probScanTree->chi2min           = 2 * result->minNll();
        this->probScanTree->covQualScanData   = result->covQual();
        this->probScanTree->scanbest  = freeDataFitValue;

        // After doing the fit with the parameter of interest constrained to the scanpoint,
        // we are now saving the fit values of the nuisance parameters. These values will be
        // used to generate toys according to the PLUGIN method.
        this->probScanTree->storeParsScan(); // \todo : figure out which one of these is semantically the right one

        this->pdf->deleteNLL();

        // also save the chi2 of the free data fit to the tree:
        this->probScanTree->chi2minGlobal = this->getChi2minGlobal();

        this->probScanTree->genericProbPValue = this->getPValueTTestStatistic(this->probScanTree->chi2min - this->probScanTree->chi2minGlobal);
        this->probScanTree->fill();

        // reset
        setParameters(w, pdf->getParName(), parsFunctionCall->get(0));
        //setParameters(w, pdf->getObsName(), obsDataset->get(0));
        this->probScanTree->writeToFile();
    } // End of npoints loop


    outputFile->Write();
    outputFile->Close();
    std::cout << "Wrote ToyTree to file" << std::endl;
    delete parsFunctionCall;

    // This is kind of a hack. The effect is supposed to be the same as callincg
    // this->sethCLFromProbScanTree(); here, but the latter gives a segfault somehow....
    // \todo: use this->sethCLFromProbScanTree() directly after figuring out the cause of the segfault.
    this->loadScanFromFile();
}

double MethodDatasetsProbScan::getPValueTTestStatistic(double test_statistic_value) {
    if ( test_statistic_value > 0) {
        // this is the normal case
        return TMath::Prob(test_statistic_value, 1);
    } else {
        cout << "MethodDatasetsProbScan::scan1d_prob() : WARNING : Test statistic is negative, forcing it to zero" << std::endl
             << "Fit at current scan point has higher likelihood than free fit." << std::endl
             << "This should not happen except for very small underflows when the scan point is at the best fit value. " << std::endl
             << "Value of test statistic is " << test_statistic_value << std::endl
             << "An equal upwards fluctuaion corresponds to a p value of " << TMath::Prob(abs(test_statistic_value), 1) << std::endl;
        // TMath::Prob will return 0 if the Argument is slightly below zero. As we are working with a float-zero we can not rely on it here:
        // TMath::Prob( 0 ) returns 1
        return 1.;
    }
}
