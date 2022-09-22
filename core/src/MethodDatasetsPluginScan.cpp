// vim: ts=2 sw=2 et
/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
 *
 *
 * The term "free fit do data" refers to the fit which is performed to data, where the parameter
 * of interest is left to float freely.
 *
 * The term "constrained fit to data" refers to the fit which is performed to data, where the
 * parameter of interest is fixed to a certain value (scanpoint)
 */

#include "MethodDatasetsPluginScan.h"
#include "TRandom3.h"
#include "TArrow.h"
#include "TLatex.h"
#include <algorithm>
#include <ios>
#include <iomanip>
#include "TFitResultPtr.h"
#include "TFitResult.h"
// #include <boost/accumulators/accumulators.hpp>
// #include <boost/accumulators/statistics/stats.hpp>
// #include <boost/accumulators/statistics/mean.hpp>
// #include <boost/accumulators/statistics/variance.hpp>



///
/// The default constructor for the dataset plugin scan
///
MethodDatasetsPluginScan::MethodDatasetsPluginScan(MethodProbScan* probScan, PDF_Datasets* PDF, OptParser* opt):
    MethodPluginScan(probScan, PDF, opt),
    pdf(PDF),
    explicitInputFile(false)
{
    chi2minGlobalFound = true; // the free fit to data must be done and must be saved to the workspace before gammacombo is even called
    methodName = "DatasetsPlugin";
    w = PDF->getWorkspace();
    title = PDF->getTitle();
    name  = PDF->getName();

    if ( arg->var.size() > 1 ) scanVar2 = arg->var[1];
    inputFiles.clear();


    globalMin = probScan->globalMin;
    Utils::setParameters(w,globalMin);  // reset fit parameters to the free fit
    bestfitpoint = ((RooRealVar*) globalMin->floatParsFinal().find(scanVar1))->getVal();
    // globalMin = (RooFitResult*) w->obj("data_fit_result");
    chi2minGlobal = probScan->getChi2minGlobal();
    std::cout << "=============== Global Minimum (2*-Log(Likelihood)) is: "  << chi2minGlobal << endl;

    // implement physical range a la Feldman Cousins
    bool refit_necessary = false;
    if ( arg->physRanges.size()>0 ){
        for ( int j=0; j<arg->physRanges[0].size(); j++ ){
            if ( w->var( arg->physRanges[0][j].name ) ){
                if(w->var( arg->physRanges[0][j].name )->getVal()<arg->physRanges[0][j].min){
                    if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].min << std::endl;
                    w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].min);
                    w->var( arg->physRanges[0][j].name )->setConstant(true);
                    refit_necessary = true;
                }
                else if (w->var( arg->physRanges[0][j].name )->getVal() > arg->physRanges[0][j].max){
                    if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].max << std::endl;
                    w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].max);
                    w->var( arg->physRanges[0][j].name )->setConstant(true);
                    refit_necessary = true;
                }
            }
        }
    }

    if(refit_necessary){
        std::cout << "!!!!!!!!!!! Global Minimum outside physical range, refitting ..." << std::endl;
        globalMin = pdf->fit(pdf->getData());
        chi2minGlobal = 2*pdf->getMinNll();
        if(!globalMin->floatParsFinal().find(scanVar1)){
            bestfitpoint = w->var(scanVar1)->getVal();
            std::cout << "=============== NEW Best Fit Point is: " << bestfitpoint << endl;
        }
        else{
            bestfitpoint = ((RooRealVar*) globalMin->floatParsFinal().find(scanVar1))->getVal();
            std::cout << "=============== NEW Best Fit Point is: " << bestfitpoint << endl;
        }
        std::cout << "=============== NEW Global Minimum (2*-Log(Likelihood)) is: " << chi2minGlobal << endl;
    }

    //reset parameters free from the Feldman Cousins behaviour
    if ( arg->physRanges.size()>0){
        for ( int j=0; j<arg->physRanges[0].size(); j++ ){
            if ( w->var( arg->physRanges[0][j].name ) && w->set(pdf->getParName())->find(arg->physRanges[0][j].name)){  //if somebody wants to modify a constant parameter make sure the parameter doesn't accidentally become floating...
                w->var( arg->physRanges[0][j].name )->setConstant(false);
            }
        }
    }


    chi2minBkg = probScan->getChi2minBkg();
    std::cout << "=============== Bkg minimum (2*-Log(Likelihood)) is: " << chi2minBkg << endl;
    if (chi2minBkg<chi2minGlobal)
    {
        std::cout << "WARNING: BKG MINIMUM IS LOWER THAN GLOBAL MINIMUM! The likelihoods are screwed up! Set bkg minimum to global minimum for consistency." << std::endl;
        chi2minBkg = chi2minGlobal;
        std::cout << "=============== New bkg minimum (2*-Log(Likelihood)) is: " << chi2minBkg << endl;
    }

    if ( !w->set(pdf->getObsName()) ) {
        cerr << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no '" + pdf->getObsName() + "' set found in workspace" << endl;
        cerr << " You can specify the name of the set in the workspace using the pdf->initObservables(..) method.";
        exit(EXIT_FAILURE);
    }
    if ( !w->set(pdf->getParName()) ) {
        cerr << "MethodDatasetsPluginScan::MethodDatasetsPluginScan() : ERROR : no '" + pdf->getParName() + "' set found in workspace" << endl;
        exit(EXIT_FAILURE);
    }
    dataBkgFitResult = pdf->fitBkg(pdf->getData(), arg->var[0]); // get Bkg fit parameters
    this->pdf->setBestIndexBkg(this->pdf->getBestIndex());
    Utils::setParameters(w,globalMin);  // reset fit parameters to the free fit
}

///////////////////////////////////////////////
///
/// Gets values of certain parameters as they were at the given scanpoint-index after the constrained fit to data
///
///////////////////////////////////////////////
float MethodDatasetsPluginScan::getParValAtIndex(int index, TString parName) {

    this->getProfileLH()->probScanTree->t->GetEntry(index);
    TLeaf* var = this->getProfileLH()->probScanTree->t->GetLeaf(parName); //<- pretty sure that this will give a segfault, we need to use parName + "_scan"
    if (!var) {
        cout << "MethodDatasetsPluginScan::getParValAtScanpoint() : ERROR : variable (" << parName << ") not found!" << endl;
        exit(EXIT_FAILURE);
    }
    return var->GetValue();
}


void MethodDatasetsPluginScan::initScan() {
    if ( arg->debug ) cout << "MethodDatasetsPluginScan::initScan() : initializing ..." << endl;

    // Init the 1-CL histograms. Range is taken from the scan range, unless
    // the --scanrange command line argument is set.
    RooRealVar *par1 = w->var(scanVar1);
    if ( !par1 ) {
        cout << "MethodDatasetsPluginScan::initScan() : ERROR : No such scan parameter: " << scanVar1 << endl;
        cout << "MethodDatasetsPluginScan::initScan() :         Choose an existing one using: --var par" << endl << endl;
        cout << "  Available parameters:" << endl << "  ---------------------" << endl << endl << "  ";
        pdf->printParameters();
        exit(EXIT_FAILURE);
    }
    // if ( arg->scanrangeMin != arg->scanrangeMax ) par1->setRange("scan", arg->scanrangeMin, arg->scanrangeMax);
    // Utils::setLimit(w, scanVar1, "scan");
    float min1 = arg->scanrangeMin;
    float max1 = arg->scanrangeMax;

    if (hCL) delete hCL;
    hCL = new TH1F("hCL" + getUniqueRootName(), "hCL" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLs) delete hCLs;
    hCLs = new TH1F("hCLs" + getUniqueRootName(), "hCLs" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsFreq) delete hCLsFreq;
    hCLsFreq = new TH1F("hCLsFreq" + getUniqueRootName(), "hCLsFreq" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsExp) delete hCLsExp;
    hCLsExp = new TH1F("hCLsExp" + getUniqueRootName(), "hCLsExp" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsErr1Up) delete hCLsErr1Up;
    hCLsErr1Up = new TH1F("hCLsErr1Up" + getUniqueRootName(), "hCLsErr1Up" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsErr1Dn) delete hCLsErr1Dn;
    hCLsErr1Dn = new TH1F("hCLsErr1Dn" + getUniqueRootName(), "hCLsErr1Dn" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsErr2Up) delete hCLsErr2Up;
    hCLsErr2Up = new TH1F("hCLsErr2Up" + getUniqueRootName(), "hCLsErr2Up" + pdf->getPdfName(), nPoints1d, min1, max1);
    if (hCLsErr2Dn) delete hCLsErr2Dn;
    hCLsErr2Dn = new TH1F("hCLsErr2Dn" + getUniqueRootName(), "hCLsErr2Dn" + pdf->getPdfName(), nPoints1d, min1, max1);
    if ( hChi2min ) delete hChi2min;
    hChi2min = new TH1F("hChi2min" + getUniqueRootName(), "hChi2min" + pdf->getPdfName(), nPoints1d, min1, max1);

    // fill the chi2 histogram with very unlikely values such
    // that inside scan1d() the if clauses work correctly
    for ( int i = 1; i <= nPoints1d; i++ ) hChi2min->SetBinContent(i, 1e6);

    if ( scanVar2 != "" ) {
        cout << "MethodDatasetsPluginScan::initScan(): EROR: Scanning in more than one dimension is not supported." << std::endl;
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
        std::cout << "DEBUG in MethodDatasetsPluginScan::initScan() - Scan initialized successfully!\n" << std::endl;
    }
    this->checkExtProfileLH();
}

//////////////////////////////////////////////
///
/// This checks if the TTree which originated from a previous prob scan
/// for compatibility with the current scan: Did it use the same number
/// of scan points, did it use the same scan range?
/// If everything is fine, keeps a pointer to the tree in this->probScanTree
///
//////////////////////////////////////////////
void MethodDatasetsPluginScan::checkExtProfileLH() {

    TTree* tree = this->getProfileLH()->probScanTree->t;

    //make sure that the scan points in the tree match number
    //of scan points and the scan range that we are using now.
    TBranch* b    = (TBranch*)tree->GetBranch("scanpoint");
    int entriesInTree = b->GetEntries();
    if (nPoints1d != entriesInTree) {
        std::cout << "Number of scan points in tree saved from prob scan do not match number of scan points used in plugin scan." << std::endl;
        exit(EXIT_FAILURE);
    }


    float parameterToScan_min = hCL->GetXaxis()->GetXmin();
    float parameterToScan_max = hCL->GetXaxis()->GetXmax();

    tree->GetEntry(0);
    float minTreePoint = b->GetLeaf("scanpoint")->GetValue();
    if ((minTreePoint - parameterToScan_min) / std::max(parameterToScan_max, parameterToScan_min) > 1e-5) {
        std::cout << "Lowest scan point in tree saved from prob scan does not match lowest scan point used in plugin scan." << std::endl;
        std::cout << "Alternatively, this could be a problem with the heuristics used for checking the equality of two floats" << std::endl;
        exit(EXIT_FAILURE);
    }

    tree->GetEntry(entriesInTree - 1);
    float maxTreePoint = b->GetLeaf("scanpoint")->GetValue();
    if ((maxTreePoint - parameterToScan_max) / std::max(parameterToScan_max, parameterToScan_min) > 1e-5) {
        std::cout << "Max scan point in tree saved from prob scan probably does not match max scan point used in plugin scan." << std::endl;
        std::cout << "Alternatively, this could be a problem with the heuristics used for checking the equality of two floats" << std::endl;
        exit(EXIT_FAILURE);
    }
};

///////////////////////////////////////////////////
///
/// Prepare environment for toy fit
///
/// \param pdf      the pdf that is to be fitted.
///
////////////////////////////////////////////////////
RooFitResult* MethodDatasetsPluginScan::loadAndFit(PDF_Datasets* pdf) {
    // we want to fit to the latest simulated toys
    // first, try to simulated toy values of the global observables from a snapshot
    if (!w->loadSnapshot(pdf->globalObsToySnapshotName)) {
        std::cout << "FATAL in MethodDatasetsPluginScan::loadAndFit() - No snapshot globalObsToySnapshotName found!\n" << std::endl;
        exit(EXIT_FAILURE);
    };
    // then, fit the pdf while passing it the simulated toy dataset
    return pdf->fit(pdf->getToyObservables());
};


///////////////////////////////////////////////////
///
/// Prepare environment for bkg-only toy fit
///
/// \param pdf      the pdf that is to be fitted.
///
////////////////////////////////////////////////////
RooFitResult* MethodDatasetsPluginScan::loadAndFitBkg(PDF_Datasets* pdf) {
    // we want to fit to the latest simulated toys
    // first, try to simulated toy values of the global observables from a snapshot
    if (!w->loadSnapshot(pdf->globalObsBkgToySnapshotName)) {
        std::cout << "FATAL in MethodDatasetsPluginScan::loadAndFit() - No snapshot globalObsBkgToySnapshotName found!\n" << std::endl;
        exit(EXIT_FAILURE);
    };
    // then, fit the pdf while passing it the simulated toy dataset
    return pdf->fit(pdf->getBkgToyObservables());
};


///
/// load Parameter limits
/// by default the "free" limit is loaded, can be changed to "phys" by command line argument
///
void MethodDatasetsPluginScan::loadParameterLimits() {
    TString rangeName = arg->enforcePhysRange ? "phys" : "free";
    if ( arg->debug ) cout << "DEBUG in Combiner::loadParameterLimits() : loading parameter ranges: " << rangeName << endl;
    TIterator* it = w->set(pdf->getParName())->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ) setLimit(w, p->GetName(), rangeName);
    delete it;
}


///
/// Print settings member of MethodDatasetsPluginScan
///
void MethodDatasetsPluginScan::print() {
    cout << "########################## Print MethodDatasetsPluginScan Class ##########################" << endl;
    cout << "\t --- " << "Method Name: \t\t\t" << methodName << endl;
    cout << "\t --- " << "Instance Name: \t\t\t" << name << endl;
    cout << "\t --- " << "Instance Title: \t\t\t" << title << endl;
    cout << "\t --- " << "Scan Var Name: \t\t\t" << scanVar1 << endl;
    if ( arg->var.size() > 1 )   cout << "\t --- " << "2nd Scan Var Name: \t\t" << scanVar2 << endl;
    cout << "\t --- " << "Number of Scanpoints 1D: \t\t" << nPoints1d << endl;
    cout << "\t --- " << "Number of Scanpoints x 2D: \t" << nPoints2dx << endl;
    cout << "\t --- " << "Number of Scanpoints y 2D: \t" << nPoints2dy << endl;
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
TChain* MethodDatasetsPluginScan::readFiles(int runMin, int runMax, int &nFilesRead, int &nFilesMissing, TString fileNameBaseIn) {
///
    TChain *c = new TChain("plugin");
    int _nFilesRead = 0;

    TString dirname = "root/scan1dDatasetsPlugin_" + this->pdf->getName() + "_" + scanVar1;
    TString fileNameBase = (fileNameBaseIn.EqualTo("default")) ? dirname + "/scan1dDatasetsPlugin_" + this->pdf->getName() + "_" + scanVar1 + "_run" : fileNameBaseIn;

    if (explicitInputFile) {
        for (TString &file : inputFiles) {
            Utils::assertFileExists(file);
            c->Add(file);
            _nFilesRead += 1;
        }
    }
    else {
        for (int i = runMin; i <= runMax; i++) {
            TString file = Form(fileNameBase + "%i.root", i);
            cout << "MethodDatasetsPluginScan::readFiles() : opening " << file << "\r";
            Utils::assertFileExists(file);
            c->Add(file);
            _nFilesRead += 1;
        }
    }

    nFilesRead = _nFilesRead;
    if ( nFilesRead == 0 ) {
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

    float halfBinWidth = (t.getScanpointMax() - t.getScanpointMin()) / ((float)t.getScanpointN()) / 2; //-1.)/2;
    /// \todo replace this such that there's always one bin per scan point, but still the range is the scan range.
    /// \todo Also, if we use the min/max from the tree, we have the problem that they are not exactly
    /// the scan range, so that the axis won't show the lowest and highest number.
    /// \todo If the scan range was changed after the toys were generate, we absolutely have
    /// to derive the range from the root files - else we'll have bining effects.
    delete hCL;
    hCL = new TH1F("hCL", "hCL", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    if (arg->debug) {
        printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth, halfBinWidth);
    }
    delete hCLs;
    hCLs = new TH1F("hCLs", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    if (arg->debug) {
        printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth, halfBinWidth);
    }
    TH1F* hCLb = new TH1F("hCLb", "hCLb", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    if (arg->debug) {
        printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth, halfBinWidth);
    }
    delete hCLsFreq;
    hCLsFreq = new TH1F("hCLsFreq", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth );
    if (arg->debug) {
        printf("DEBUG %i %f %f %f\n", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth, halfBinWidth);
    }
    delete hCLsExp;
    hCLsExp = new TH1F("hCLsExp", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    delete hCLsErr1Up;
    hCLsErr1Up = new TH1F("hCLsErr1Up", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    delete hCLsErr1Dn;
    hCLsErr1Dn = new TH1F("hCLsErr1Dn", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    delete hCLsErr2Up;
    hCLsErr2Up = new TH1F("hCLsErr2Up", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    delete hCLsErr2Dn;
    hCLsErr2Dn = new TH1F("hCLsErr2Dn", "hCLs", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);
    delete hChi2min;
    hChi2min = new TH1F("hChi2min", "hChi2min", t.getScanpointN(), t.getScanpointMin() - halfBinWidth, t.getScanpointMax() + halfBinWidth);


    // histogram to store number of toys which enter p Value calculation
    TH1F *h_better        = (TH1F*)hCL->Clone("h_better");
    // histogram to store number of toys which enter CLs p Value calculation
    TH1F *h_better_cls        = (TH1F*)hCL->Clone("h_better_cls");
    // histogram to store number of toys which enter CLb p Value calculation
    TH1F *h_better_clb        = (TH1F*)hCL->Clone("h_better_clb");
    // numbers for all toys
    TH1F *h_all           = (TH1F*)hCL->Clone("h_all");
    // numbers of toys failing the selection criteria
    TH1F *h_all_bkg           = (TH1F*)hCL->Clone("h_all_bkg");
    // numbers of toys failing the selection criteria
    TH1F *h_failed        = (TH1F*)hCL->Clone("h_failed");
    // numbers of bkg toys failing the selection criteria, needed for CLs method
    TH1F *h_failed_bkg        = (TH1F*)hCL->Clone("h_failed_bkg");
    // numbers of toys which are not in the physical region dChi2<0
    TH1F *h_background    = (TH1F*)hCL->Clone("h_background");
    // numbers of bkg toys which are not in the physical region dChi2<0
    TH1F *h_negtest_bkg    = (TH1F*)hCL->Clone("h_negtest_bkg");
    // histo for GoF test
    TH1F *h_gof           = (TH1F*)hCL->Clone("h_gof");
    // likelihood scan p values
    TH1F *h_probPValues   = (TH1F*)hCL->Clone("h_probPValues");
    // total number of toys
    TH1F *h_tot           = (TH1F*)hCL->Clone("h_tot");
    // histogram illustrating the failure rate
    TH1F *h_fracGoodToys  = (TH1F*)hCL->Clone("h_fracGoodToys");

    TH1F *bkg_pvals  = new TH1F("bkg_pvals", "bkg p values", 20, -0.1, 1.1);

    TH1F *h_sig_bkgtoys  = new TH1F("h_sig_bkgtoys", "signal distribution for bkg toys", 50, -5*(w->var(scanVar1)->getError()), 5*(w->var(scanVar1)->getError()));

    // map of vectors for determining signal distributions per scan point
    std::map<int,std::vector<double> > sampledBiasValues;

    // map of vectors for CLb quantiles
    std::map<int,std::vector<double> > sampledSchi2Values;
    std::map<int,std::vector<double> > sampledBValues;
    std::map<int,std::vector<double> > sampledSBValues;

    // vector to compute significance
    std::vector<double> sampledBTeststats;

    TH1F *h_pVals         = new TH1F("p", "p", 200, 0.0, 1e-2);
    Long64_t nentries     = t.GetEntries();
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : average number of toys per scanpoint: " << (double) nentries / (double)nPoints1d << endl;
    Long64_t nfailed      = 0;
    Long64_t nfailedbkg   = 0;
    Long64_t nwrongrun    = 0;
    Long64_t n0better     = 0;
    Long64_t n0all        = 0;
    Long64_t n0tot        = 0;
    Long64_t n0failed     = 0;
    Long64_t totFailed    = 0;


    float printFreq = nentries > 101 ? 100 : nentries;  ///< for the status bar
    t.activateAllBranches();
    for (Long64_t i = 0; i < nentries; i++)
    {
        // status bar
        if (((int)i % (int)(nentries / printFreq)) == 0)
            cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading entries "
                 << Form("%.0f", (float)i / (float)nentries * 100.) << "%   \r" << flush;
        // load entry
        t.GetEntry(i);

        bool valid    = true;

        h_tot->Fill(t.scanpoint);
        if (t.scanpoint == 0.0) n0tot++;
        // criteria for GammaCombo
        bool convergedFits      = (t.statusFree == 0. && t.statusScan == 0.) && (t.covQualFree == 3 && t.covQualScan == 3);
        bool tooHighLikelihood  = !( abs(t.chi2minToy) < 1e27 && abs(t.chi2minGlobalToy) < 1e27);
        // bool BadBkgFit          = (!(t.statusFreeBkg == 0 && t.statusBkgBkg == 0) && (t.covQualFreeBkg == 3 && t.covQualBkgBkg == 3))||(std::isnan(t.chi2minBkgBkgToy - t.chi2minGlobalBkgToy)||(t.chi2minBkgBkgToy - t.chi2minGlobalBkgToy<0)||(t.statusBkgBkg!=0)||(t.statusFreeBkg!=0));
        bool BadBkgFit          = (!(t.statusFreeBkg == 0 && t.statusScanBkg == 0) && (t.covQualFreeBkg == 3 && t.covQualScanBkg == 3))||(std::isnan(t.chi2minBkgToy - t.chi2minGlobalBkgToy)||(abs(t.chi2minBkgToy) > 1e27 || abs(t.chi2minGlobalBkgToy) > 1e27));
        // bool BadBkgFit          = false;

        // apply cuts
        if ( tooHighLikelihood || !convergedFits )
        {
            h_failed->Fill(t.scanpoint);
            if (t.scanpoint == 0) n0failed++;
            valid = false;
            nfailed++;
            //continue;
        }

        if ( BadBkgFit){
            if(arg->debug){
                std::cout << "MethodDatasetsPluginScan::readScan1dTrees():Bkg toy failed because of ";
                if(t.statusFreeBkg!=0) std::cout << "bad free fit status, ";
                if(t.statusScanBkg!=0) std::cout << "bad scan fit status, ";
                if(t.covQualFreeBkg!=3) std::cout << "imperfect free fit covariance matrix (status " << t.covQualFreeBkg << "), ";
                if(t.covQualScanBkg!=3) std::cout << "imperfect scan fit covariance matrix (status " << t.covQualScanBkg << "), ";
                if (std::isnan(t.chi2minBkgToy - t.chi2minGlobalBkgToy)||(abs(t.chi2minBkgToy) > 1e27 || abs(t.chi2minGlobalBkgToy) > 1e27)) std::cout << "and problem with likelihood computations.";
                std::cout << std::endl;
            }
            h_failed_bkg->Fill(t.scanpoint);
            nfailedbkg++;
        }

        // Check if toys are in physical region.
        // Don't enforce t.chi2min-t.chi2minGlobal>0, else it can be hard because due
        // to little fluctuaions the best fit point can be missing from the plugin plot...

        // std::cout << "using scanvar: " << scanVar1 << std::endl;
        // globalMin->Print();

        // best fit point is a class variable and define in constructor
        bool inPhysicalRegion     = ((t.chi2minToy - t.chi2minGlobalToy) >= 0 );

        // // build test statistic
        // double sb_teststat_measured= t.chi2min - this->chi2minGlobal;
        // sb_teststat_measured = bestfitpoint <= t.scanpoint ? sb_teststat_measured : 0.; // if mu < muhat then q_mu = 0

        // hChi2min->SetBinContent(hChi2min->FindBin(t.scanpoint), sb_teststat_measured);

        // double sb_teststat_toy= t.chi2minToy - t.chi2minGlobalToy;
        // sb_teststat_toy = t.scanbest <= t.scanpoint ? sb_teststat_toy : 0.; // if mu < muhat then q_mu = 0


        // double b_teststat_measured= t.chi2min - this->chi2minGlobal;
        // b_teststat_measured = bestfitpoint <= t.scanpoint ? b_teststat_measured : 0.; // if mu < muhat then q_mu = 0

        // double b_teststat_toy = t.chi2minBkgToy - t.chi2minGlobalBkgToy;
        // b_teststat_toy = t.scanbestBkg <= t.scanpoint ? b_teststat_toy : 0.;  // if mu < muhat then q_mu = 0

        // build test statistics
        // chi2minBkgBkgToy is the best fit of the bkg pdf of bkg-only toy, chi2minGlobalBkgToy is the best global fit of the bkg-only toy
        // chi2minBkgToy is the best fit at scanpoint of bkg-only toy
        double teststat_measured = t.chi2min - this->chi2minGlobal;
        double sb_teststat_toy= t.chi2minToy - t.chi2minGlobalToy;
        double b_teststat_toy = t.chi2minBkgToy - t.chi2minGlobalBkgToy;
        double sb_teststat_bkgtoy= t.chi2minBkgBkgToy - t.chi2minGlobalBkgToy;

        if (arg->teststatistic ==1){ // use one-sided test statistic
            teststat_measured = bestfitpoint <= t.scanpoint ? teststat_measured : 0.; // if mu < muhat then q_mu = 0 //best fit point defined in constructor
            sb_teststat_toy = t.scanbest <= t.scanpoint ? sb_teststat_toy : 0.; // if mu < muhat then q_mu = 0
            b_teststat_toy = t.scanbestBkg <= t.scanpoint ? b_teststat_toy : 0.;  // if mu < muhat then q_mu = 0
        }
        // the usage of the two-sided test statistic is default
        hChi2min->SetBinContent(hChi2min->FindBin(t.scanpoint), teststat_measured);



        int hBin = h_all->FindBin(t.scanpoint);
        if ( sampledBValues.find(hBin) == sampledBValues.end() ) sampledBValues[hBin] = std::vector<double>();
        if ( sampledSBValues.find(hBin) == sampledSBValues.end() ) sampledSBValues[hBin] = std::vector<double>();
        if ( sampledSchi2Values.find(hBin) == sampledSchi2Values.end() ) sampledSchi2Values[hBin] = std::vector<double>();
        if ( sampledBiasValues.find(hBin) == sampledBiasValues.end() ) sampledBiasValues[hBin] = std::vector<double>();

        if ( valid && (sb_teststat_toy) >= (teststat_measured) ) { //t.chi2minGlobal ){
            h_better->Fill(t.scanpoint);
        }
        if ( valid && (sb_teststat_toy) >= (t.chi2min - this->chi2minBkg) ) { //t.chi2minGlobal ){
        // if ( valid && (sb_teststat_toy) > (b_teststat_measured) ) { //t.chi2minGlobal ){
            h_better_cls->Fill(t.scanpoint);
        }

        if ( !BadBkgFit && (b_teststat_toy) >= (teststat_measured) ) {
            h_better_clb->Fill(t.scanpoint);
        }
        if (t.scanpoint == 0.0) n0better++;

        // goodness-of-fit
        if ( inPhysicalRegion && t.chi2minGlobalToy > this->chi2minGlobal ) { //t.chi2minGlobal ){
            h_gof->Fill(t.scanpoint);
        }

        // all toys
        if ( valid) { //inPhysicalRegion )
            // not efficient! TMath::Prob evaluated each toy, only needed once.
            // come up with smarter way
            h_all->Fill(t.scanpoint);
            // h_probPValues->SetBinContent(h_probPValues->FindBin(t.scanpoint), this->getPValueTTestStatistic(t.chi2min - this->chi2minGlobal)); //t.chi2minGlobal));
            h_probPValues->SetBinContent(h_probPValues->FindBin(t.scanpoint), this->getPValueTTestStatistic(teststat_measured)); //t.chi2minGlobal));
            if (t.scanpoint == 0.0) n0all++;
            sampledBiasValues[hBin].push_back(t.scanbest - t.scanpoint);
        }

        // all background toys
        if ( !BadBkgFit) { //inPhysicalRegion )
            h_all_bkg->Fill(t.scanpoint);
        }

        // chi2minToy is the best of the toy at scanpoint, chi2minGlobalToy is the best global fit of the toy
        if(valid && sb_teststat_toy>=0){
            // if(sb_teststat_toy<0&&sb_teststat_toy>=0) sb_teststat_toy = 0.0;
            sampledSchi2Values[hBin].push_back(sb_teststat_toy);
        }


        // chi2minBkgBkgToy is the best fit of the bkg pdf of bkg-only toy, chi2minGlobalBkgToy is the best global fit of the bkg-only toy
        // chi2minBkgToy is the best fit at scanpoint of bkg-only toy
        // if(b_teststat_toy<0&&b_teststat_toy>0) b_teststat_toy=0.0;

        if( !BadBkgFit && b_teststat_toy>=0){
            if(hBin==2){
                // std::cout << bkgTestStatVal << std::endl;
                bkg_pvals->Fill(TMath::Prob(b_teststat_toy,1));
                h_sig_bkgtoys->Fill(t.scanbestBkg);
                sampledBTeststats.push_back(sb_teststat_bkgtoy);
            }
            sampledBValues[hBin].push_back( b_teststat_toy );
            sampledSBValues[hBin].push_back( b_teststat_toy );
        }
        else if (!BadBkgFit && b_teststat_toy<0){
            h_negtest_bkg->Fill(t.scanpoint);
        }



        // use the unphysical events to estimate background (be careful with this,
        // at least inspect the control plots to judge if this can be at all reasonable)
        // ToDo: is that really sensible? Currently only used for control plots and background is estimated in a dedicated way
        if ( valid && !inPhysicalRegion ) {
            h_background->Fill(t.scanpoint);
        }

        if (n0tot % 1500 == 0 && n0all != 0) {
            //cout << "better: " << n0better << " all: " << n0all << " p: " << (float)n0better/(float)n0all << endl << endl;
            h_pVals->Fill((float)n0better / (float)n0all);
            n0tot = 0;
            n0better = 0;
            n0all = 0;
        }
    }
    // std::cout << "Overflow bkg: " << bkg_pvals->GetBinContent(0) << " underflow: " << bkg_pvals->GetBinContent(21) << std::endl;
    cout << std::fixed << std::setprecision(2);
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : reading done.           \n" << endl;
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : read an average of " << ((double)nentries - (double)nfailed) / (double)nPoints1d << " toys per scan point." << endl;
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of failed toys: " << (double)nfailed / (double)nentries * 100. << "%." << endl;
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of failed background toys: " << (double)nfailedbkg / (double)nentries * 100. << "%." << endl;
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of unphysical (negative test stat) toys: " << h_background->GetEntries() / (double)nentries * 100. << "%." << endl;
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : fraction of unphysical (negative test stat) bkg toys: " << (h_negtest_bkg->GetEntries() / (double)h_all_bkg->GetEntries()) * 100. << "%." << endl;
    double pval_significance=getVectorFracAboveValue(sampledBTeststats, chi2minBkg - chi2minGlobal);
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : signal significance: naive p-val: " << TMath::Prob(chi2minBkg - chi2minGlobal,1) << " ("<< sqrt(chi2minBkg - chi2minGlobal) <<" sigma)" << " vs. value from toys: " << pval_significance<<" (" << sqrt(2)*TMath::ErfInverse(1.-pval_significance) <<" sigma)"<<endl;
    if ( nwrongrun > 0 ) {
        cout << "\nMethodDatasetsPluginScan::readScan1dTrees() : WARNING : Read toys that differ in global chi2min (wrong run) : "
             << (double)nwrongrun / (double)(nentries - nfailed) * 100. << "%.\n" << endl;
    }

    // //Test median error...
    // TH1F *mederr_bootstrap        = (TH1F*)hCL->Clone("mederr_bootstrap");
    // mederr_bootstrap->SetLineColor(kRed);
    // mederr_bootstrap->SetMarkerColor(kRed);
    // mederr_bootstrap->SetMarkerStyle(3);
    // mederr_bootstrap->SetLineWidth(2);
    // TH1F *mederr_nonpar        = (TH1F*)hCL->Clone("mederr_nonpar");
    // mederr_nonpar->SetLineColor(kBlue);
    // mederr_nonpar->SetLineWidth(2);
    // TH1F *mederr_asymptotic        = (TH1F*)hCL->Clone("mederr_asymptotic");
    // mederr_asymptotic->SetLineColor(kBlack);
    // mederr_asymptotic->SetLineWidth(2);
    // TH1F *mederr_poisson_cls        = (TH1F*)hCL->Clone("mederr_poisson_cls");
    // mederr_poisson_cls->SetLineColor(kOrange);
    // mederr_poisson_cls->SetLineWidth(2);
    // TH1F *mederr_poisson_clsb_clb        = (TH1F*)hCL->Clone("mederr_poisson_clsb_clb");
    // mederr_poisson_clsb_clb->SetLineColor(kCyan);
    // mederr_poisson_clsb_clb->SetLineWidth(2);
    // //...

    for (int i = 1; i <= h_better->GetNbinsX(); i++) {
        float nbetter = h_better->GetBinContent(i);
        float nbetter_cls = h_better_cls->GetBinContent(i);
        float nbetter_clb = h_better_clb->GetBinContent(i);
        float nall = h_all->GetBinContent(i);
        float nall_bkg = h_all_bkg->GetBinContent(i);
        // std::cout<<nall_bkg << "\t" << nbetter_clb <<"\t"<< nbetter_clb / nall_bkg << std::endl;
        // get number of background and failed toys
        float nbackground     = h_background->GetBinContent(i);

        nfailed       = h_failed->GetBinContent(i);

        //nall = nall - nfailed + nbackground;
        float ntot = h_tot->GetBinContent(i);
        if ( nall == 0. ) continue;
        h_background->SetBinContent(i, nbackground / nall);
        h_background->SetBinError(i, sqrt(h_background->GetBinContent(i)*(1.- h_background->GetBinContent(i))/nall));
        h_negtest_bkg->SetBinContent(i, ((float)h_negtest_bkg->GetBinContent(i)) / nall);
        h_negtest_bkg->SetBinError(i, sqrt(h_negtest_bkg->GetBinContent(i)*(1.- h_negtest_bkg->GetBinContent(i))/nall));
        h_fracGoodToys->SetBinContent(i, (nall) / (float)ntot);
        h_fracGoodToys->SetBinError(i, sqrt(((nall) / (float)ntot)*(1.-((nall) / (float)ntot))/ntot));
        // subtract background
        // float p = (nbetter-nbackground)/(nall-nbackground);
        // hCL->SetBinContent(i, p);
        // hCL->SetBinError(i, sqrt(p * (1.-p)/(nall-nbackground)));

        // don't subtract background
        float p = nbetter / nall;
        float p_cls = nbetter_cls / nall;
        float p_clb = nbetter_clb / nall_bkg;
        // std::cout << "p val. bkg. Prob: " << TMath::Prob(chi2minBkg - chi2minGlobal,1) << " Plugin: " << p_clb << " +/- " << sqrt(p_clb * (1. - p_clb) / nall) << std::endl;
        // float p_clb = TMath::Prob(chi2minBkg - chi2minGlobal,1); //Since the fitting of the global pdf is biased, use Prob to determine p_clb

        hCL->SetBinContent(i, p);
        hCL->SetBinError(i, sqrt(p * (1. - p) / nall));
        hCLs->SetBinContent(i, p_cls);
        hCLs->SetBinError(i, sqrt(p_cls * (1. - p_cls) / nall));
        hCLb->SetBinContent(i, p_clb);
        hCLb->SetBinError(i, sqrt(p_clb * (1. - p_clb) / nall));


        std::vector<double> clsb_vals;
        std::vector<double> clb_vals;
        std::vector<double> cls_vals;

        if(sampledBValues[i].size()!=sampledSBValues[i].size()){
            std::cout << "MethodDatasetsPluginScan::readScan1dTrees: Not the same number of entries in sampledBValues and sampledSBValues!" <<std::endl;
            exit(EXIT_FAILURE);
        }

        for(int j=0; j<sampledBValues[i].size(); j++){
            double clsb_val = getVectorFracAboveValue( sampledSchi2Values[i], sampledSBValues[i][j]); // p_cls+b value for each bkg-only toy
            double clb_val = getVectorFracAboveValue( sampledBValues[i], sampledSBValues[i][j]); // p_clb value for each bkg-only toy CAUTION: duplicate use of sampledBValues
            double cls_val = clsb_val/clb_val;

            clsb_vals.push_back(clsb_val);
            clb_vals.push_back(clb_val);
            cls_vals.push_back(cls_val);
        }

            TH1F *bkg_pvals_cls  = new TH1F(Form("bkg_clsvals_bin%d",i), "bkg cls p values", 50, -0.01, 1.01);
            bkg_pvals_cls->SetLineColor(1);
            bkg_pvals_cls->SetLineWidth(3);
            TH1F *bkg_pvals_clsb  = new TH1F(Form("bkg_clsbvals_bin%d",i), "bkg clsb p values", 50, -0.01, 1.01);
            bkg_pvals_clsb->SetLineColor(2);
            bkg_pvals_clsb->SetLineWidth(3);
            TH1F *bkg_pvals_clb  = new TH1F(Form("bkg_clbvals_bin%d",i), "bkg clb p values", 50, -0.01, 1.01);
            bkg_pvals_clb->SetLineColor(3);
            bkg_pvals_clb->SetLineWidth(3);
            for(int j=0; j<sampledBValues[i].size(); j++){
                bkg_pvals_cls->Fill(TMath::Min( cls_vals[j] , 1.));
                bkg_pvals_clsb->Fill(TMath::Min( clsb_vals[j] , 1.));
                bkg_pvals_clb->Fill(TMath::Min( clb_vals[j] , 1.));
            }

        if (arg->debug || arg->controlplot ){

            TCanvas *canvasdebug = newNoWarnTCanvas("canvasdebug", "canvas1", 1200, 1000);
            bkg_pvals_cls->Draw();
            bkg_pvals_clsb->Draw("same");
            bkg_pvals_clb->Draw("same");
            TLegend *leg = new TLegend(0.65,0.74,0.89,0.95);
            leg->SetHeader("p-value distributions");
            leg->SetFillColor(0);
            leg->AddEntry(bkg_pvals_cls,"CLs","L");
            leg->AddEntry(bkg_pvals_clsb,"CLs+b","L");
            leg->AddEntry(bkg_pvals_clb,"CLb","L");
            leg->Draw("same");
            savePlot(canvasdebug, TString(Form("p_values%d",i))+"_"+scanVar1);
        }

        std::vector<double> probs  = {TMath::Prob(4,1)/2., TMath::Prob(1,1)/2., 0.5, 1.-(TMath::Prob(1,1)/2.), 1.-(TMath::Prob(4,1)/2.) };
        std::vector<double> quantiles_clsb = Quantile<double>( clsb_vals, probs );
        std::vector<double> quantiles_clb = Quantile<double>( clb_vals, probs );
        std::vector<double> quantiles_cls = Quantile<double>( cls_vals, probs );

        // check
        if ( arg->debug ) {
          cout << i << endl;
          cout << "Quants: ";
          for (int k=0; k<probs.size(); k++) cout << probs[k] << " , ";
          cout << endl;
          cout << "CLb: ";
          for (int k=0; k<quantiles_clb.size(); k++) cout << quantiles_clb[k] << " , ";
          cout << endl;
          cout << "CLsb: ";
          for (int k=0; k<quantiles_clsb.size(); k++) cout << quantiles_clsb[k] << " , ";
          cout << endl;
          cout << "CLs: ";
          for (int k=0; k<quantiles_cls.size(); k++) cout << quantiles_cls[k] << " , ";
          cout << endl;
        }

        // //ideal method, but prone to fluctuations
        hCLsExp->SetBinContent   ( i, TMath::Min( quantiles_cls[2] , 1.) );
        hCLsExp->SetBinError   ( i, sqrt((1.-TMath::Min( quantiles_cls[2] , 1.))*TMath::Min( quantiles_cls[2] , 1.)/sampledBValues[i].size()) );        
        hCLsErr1Up->SetBinContent( i, TMath::Min( quantiles_cls[3] , 1.) );
        hCLsErr1Dn->SetBinContent( i, TMath::Min( quantiles_cls[1] , 1.) );
        hCLsErr2Up->SetBinContent( i, TMath::Min( quantiles_cls[4] , 1.) );
        hCLsErr2Dn->SetBinContent( i, TMath::Min( quantiles_cls[0] , 1.) );

        // //Test median error...
        // // std::cout << "bootstrapping median errors for bin " << i << std::endl;
        // std::vector<float> medians;
        // for (int m=0; m<10000; m++){
        //     TRandom3 rndg(0);
        //     std::vector<double> testsample;
        //     for(int bs_index=0; bs_index<cls_vals.size(); bs_index++){
        //         testsample.push_back(cls_vals[rndg.Integer(cls_vals.size())]);
        //     }
        //     // medians.push_back(m);
        //     medians.push_back(TMath::Min( Quantile<double>( testsample, probs )[2], 1.));
        //     // std::cout << medians[m] << std::endl;
        // }
        // boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::variance> > acc;
        // acc = for_each(medians.begin(), medians.end(), acc);
        // // cout <<hCLsExp->GetBinContent(i)<<": " <<  boost::accumulators::mean(acc) << "\t" << sqrt(boost::accumulators::variance(acc)) << endl;
        // mederr_bootstrap->SetBinContent(i , sqrt(boost::accumulators::variance(acc)));

        // // std::cout << "non-parameric median errors for bin " << i << std::endl;
        // std::sort (cls_vals.begin(), cls_vals.begin()+cls_vals.size());
        // int k =0;
        // do{
        //     k++;
        //     // std::cout << "Binomial: " << TMath::BinomialI(0.5, cls_vals.size(), cls_vals.size()/2-k) - TMath::BinomialI(0.5, cls_vals.size(), cls_vals.size()/2+k) << std::endl;
        // // }while( ROOT::Math::binomial_cdf(cls_vals.size()/2+k,0.5, cls_vals.size()) - ROOT::Math::binomial_cdf(cls_vals.size()/2-k,0.5, cls_vals.size()) <=0.68);
        // }while(TMath::BetaIncomplete(0.5,cls_vals.size()/2-k+1, cls_vals.size()-(cls_vals.size()/2-k)) - TMath::BetaIncomplete(0.5,cls_vals.size()/2+k+1, cls_vals.size()-(cls_vals.size()/2+k))<=0.68);

        // hCLsExp->SetBinError(i , (cls_vals[cls_vals.size()/2+k-1] - cls_vals[cls_vals.size()/2-k-1])/2.);

        // double mederr_asymptotic_val = sqrt( probs[2]*( 1.-probs[2] ) / ( cls_vals.size()*bkg_pvals_cls->GetBinContent( bkg_pvals_cls->FindBin( TMath::Min( quantiles_cls[2], 1. ) ) ) / ( 1.0*bkg_pvals_cls->GetEntries()*bkg_pvals_cls->GetBinWidth(2) ) ) );
        // std::vector<double> low_high_median = {0.5 - mederr_asymptotic_val, 0.5 + mederr_asymptotic_val};
        // std::vector<double> quantiles_cls_low_high_med = Quantile<double>( cls_vals, low_high_median );
        // // mederr_asymptotic->SetBinContent(i,sqrt( probs[2]*( 1.-probs[2] ) / ( cls_vals.size()*bkg_pvals_cls->GetBinContent( bkg_pvals_cls->FindBin( TMath::Min( quantiles_cls[2], 1. ) ) ) / ( 1.0*bkg_pvals_cls->GetEntries()*bkg_pvals_cls->GetBinWidth(2) ) ) ) );
        // mederr_asymptotic->SetBinContent(i, (quantiles_cls_low_high_med[1]-quantiles_cls_low_high_med[0])/2.);


        // mederr_poisson_cls->SetBinContent(i,sqrt((1.-TMath::Min( quantiles_cls[2] , 1.))*TMath::Min( quantiles_cls[2] , 1.)/sampledBValues[i].size()));
        // mederr_poisson_clsb_clb->SetBinContent(i,sqrt( (quantiles_clsb[2]*(1.-quantiles_clsb[2])/sampledSchi2Values[i].size()) + (quantiles_clb[2]*(1.-quantiles_clb[2])/sampledBValues[i].size())));
        // //...

        // CLs values in data
        int nDataAboveBkgExp = 0;
        double dataTestStat = p>0 ? TMath::ChisquareQuantile(1.-p,1) : 1.e10;
        for (int j=0; j<sampledBValues[i].size(); j++ ) {
          if ( sampledBValues[i][j] > dataTestStat ) nDataAboveBkgExp += 1;
        }
        float dataCLb    = p_clb;
        float dataCLbErr = sqrt( dataCLb * (1.-dataCLb) / sampledBValues[i].size() );
        if(dataCLb==0){
            std::cout << "!!!!!! ERROR: CL_b=0: this should only happen for really few toys! Setting to a small value. Please run more toys to get a reliable result." << std::endl;
            dataCLb=1e-9;
            dataCLbErr=1.;
        }
        if ( p/dataCLb >= 1. ) {
          hCLsFreq->SetBinContent(i, 1.);
          hCLsFreq->SetBinError  (i, 0.);
        }
        else if ( dataTestStat == 1.e10 ) {
          hCLsFreq->SetBinContent(i, hCL->GetBinContent(i) );
          hCLsFreq->SetBinError  (i, hCL->GetBinError(i) );
        }
        else if ( hCLsFreq->GetBinCenter(i) <= hCLsFreq->GetBinCenter(h_better->GetMaximumBin()) ) {
          hCLsFreq->SetBinContent(i, 1.);
          hCLsFreq->SetBinError  (i, 0.);
        }
        else {
          hCLsFreq->SetBinContent(i, p / dataCLb);
          hCLsFreq->SetBinError  (i, (p/dataCLb) * sqrt( sq( hCL->GetBinError(i) / hCL->GetBinContent(i) ) + sq( dataCLbErr / dataCLb ) ) );
        }

        if (arg->debug) {
          cout << "At scanpoint " << std::scientific << hCL->GetBinCenter(i) << ": ===== number of toys for pValue calculation: " << nbetter << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue:         " << p << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue CLs:     " << p_cls << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue CLsFreq: " << hCLsFreq->GetBinContent(i) << endl;
        }
    }

    // //Test median errors...
    // TCanvas *canvas_medianerr = newNoWarnTCanvas("canvas_medianerr", "canvas1", 1200, 1000);
    // canvas_medianerr->SetRightMargin(0.11);
    // // mederr_bootstrap->GetYaxis()->SetRangeUser(0.,0.1);
    // mederr_bootstrap->Draw("P");
    // mederr_nonpar->Draw("SAME");
    // mederr_asymptotic->Draw("SAME");
    // mederr_poisson_cls->Draw("SAME");
    // mederr_poisson_clsb_clb->Draw("SAME");
    // TLegend *leg_medianerr = new TLegend(0.5,0.74,0.89,0.95);
    // leg_medianerr->SetHeader("Median uncertainties");
    // leg_medianerr->SetFillColor(0);
    // leg_medianerr->AddEntry(mederr_bootstrap,"from bootstrapping (optimal)","PE");
    // leg_medianerr->AddEntry(mederr_nonpar, "non-parameric","L");
    // leg_medianerr->AddEntry(mederr_asymptotic, "with asymptotic formula", "L");
    // leg_medianerr->AddEntry(mederr_poisson_cls, "simple binomial error on cls dist.", "L");
    // leg_medianerr->AddEntry(mederr_poisson_clsb_clb, "(wrong) error prop. clsb/clb", "L");
    // leg_medianerr->Draw("same");
    // canvas_medianerr->SaveAs("median_error_test.png"); 
    // //...


    if ( arg->controlplot ) makeControlPlots( sampledBValues, sampledSchi2Values );
    if ( arg->controlplot ) makeControlPlotsBias( sampledBiasValues );

    if ( arg->controlplot ){

        // do the control plots for the combiner case
        // ToDo: the tayloring of those control plots does not suit the datasets case as there is no normalisation to chi2minGlobal
        // ControlPlots cp(&t);
        // if ( arg->plotid==0 || arg->plotid==1 ) cp.ctrlPlotMore(profileLH);
        // if ( arg->plotid==0 || arg->plotid==2 ) cp.ctrlPlotChi2();
        // if ( arg->plotid==0 || arg->plotid==3 ) cp.ctrlPlotNuisances();
        // if ( arg->plotid==0 || arg->plotid==4 ) cp.ctrlPlotObservables();
        // if ( arg->plotid==0 || arg->plotid==5 ) cp.ctrlPlotChi2Distribution();
        // if ( arg->plotid==0 || arg->plotid==6 ) cp.ctrlPlotChi2Parabola();
        // if ( arg->plotid==0 || arg->plotid==7 ) cp.ctrlPlotPvalue();
        // cp.saveCtrlPlots();

        TCanvas *biascanv = newNoWarnTCanvas("biascanv", "biascanv");
        biascanv->SetRightMargin(0.11);
        h_sig_bkgtoys->GetXaxis()->SetTitle("POI residual for bkg-only toys");
        h_sig_bkgtoys->GetYaxis()->SetTitle("Entries");
        h_sig_bkgtoys->GetXaxis()->SetTitleSize(0.06);
        h_sig_bkgtoys->GetYaxis()->SetTitleSize(0.06);
        h_sig_bkgtoys->GetXaxis()->SetLabelSize(0.06);
        h_sig_bkgtoys->GetYaxis()->SetLabelSize(0.06);
        h_sig_bkgtoys->SetLineWidth(2);
        h_sig_bkgtoys->SetFillColor(kBlue);
        h_sig_bkgtoys->SetFillStyle(3003);
        h_sig_bkgtoys->SetLineColor(kBlue);
        h_sig_bkgtoys->Draw();
        TLegend *leg = new TLegend(0.65,0.74,0.89,0.95);
        leg->SetHeader("Bkg-only");
        leg->SetFillColor(0);
        leg->AddEntry(h_sig_bkgtoys,"POI residual","LF");
        leg->AddEntry((TObject*)0,Form("#mu=%4.2g +/- %4.2g",h_sig_bkgtoys->GetMean(),h_sig_bkgtoys->GetMeanError()),"");
        leg->AddEntry((TObject*)0,Form("#sigma=%4.2g +/- %4.2g",h_sig_bkgtoys->GetStdDev(),h_sig_bkgtoys->GetStdDevError()),"");
        leg->Draw("same");
        savePlot(biascanv, "BiasControlPlot_bkg-only_"+scanVar1);
        hCLb->GetXaxis()->SetTitle(w->var(scanVar1)->GetTitle());
        hCLb->GetYaxis()->SetTitle("CL_{b}");
        hCLb->GetXaxis()->SetTitleSize(0.06);
        hCLb->GetYaxis()->SetTitleSize(0.06);
        hCLb->GetXaxis()->SetLabelSize(0.06);
        hCLb->GetYaxis()->SetLabelSize(0.06);
        hCLb->SetLineWidth(2);
        hCLb->GetYaxis()->SetRangeUser(0.,1.05);
        hCLb->Draw("PE");
        savePlot(biascanv, "CLb_values_"+scanVar1);
    }

    if (arg->debug || arg->controlplot) {
        
        // Bkg-only p-values distribution. assuming first scan point ~ bkg-only.
        // Should be flat. Large peaks at 0/1 indicate negative test statistics.
        TCanvas *canvas1 = newNoWarnTCanvas("canvas1", "canvas1");
        bkg_pvals->SetLineWidth(2);
        bkg_pvals->SetXTitle("bkg-only p value");
        bkg_pvals->Draw();
        savePlot(canvas1,"bkg-only_pvalues_"+scanVar1);

        // Distributions of fractions of failed fits for the scan toys and the bkg-only toys.
        // Fraction should be small and hopefully independent of the scanvariable.
        for (int i = 1; i <= h_failed->GetNbinsX(); i++) {
            double n_failed = h_failed->GetBinContent(i);
            double n_failed_bkg = h_failed_bkg->GetBinContent(i);
            double n_tot= h_tot->GetBinContent(i);
            h_failed->SetBinContent(i, n_failed/n_tot);
            h_failed->SetBinError(i, sqrt((n_failed/n_tot) * (1. - (n_failed/n_tot)) / n_tot));
            h_failed_bkg->SetBinContent(i, n_failed_bkg/n_tot);
            h_failed_bkg->SetBinError(i, sqrt((n_failed_bkg/n_tot) * (1. - (n_failed_bkg/n_tot)) / n_tot));
        }
        canvas1->SetRightMargin(0.11);
        double max_failed = max(h_failed->GetMaximum(),h_failed_bkg->GetMaximum());
        h_failed->GetYaxis()->SetRangeUser(0.,max_failed+min(max_failed,0.15));
        h_failed_bkg->GetYaxis()->SetRangeUser(0.,max_failed+min(max_failed,0.15));

        h_failed->GetXaxis()->SetTitle(w->var(scanVar1)->GetTitle());
        h_failed->GetYaxis()->SetTitle("failed toy fraction");
        h_failed->GetXaxis()->SetTitleSize(0.06);
        h_failed->GetYaxis()->SetTitleSize(0.06);
        h_failed->GetXaxis()->SetLabelSize(0.06);
        h_failed->GetYaxis()->SetLabelSize(0.06);
        h_failed->SetLineWidth(2);

        h_failed_bkg->GetXaxis()->SetTitle(w->var(scanVar1)->GetTitle());
        h_failed_bkg->GetYaxis()->SetTitle("failed toy fraction");
        h_failed_bkg->GetXaxis()->SetTitleSize(0.06);
        h_failed_bkg->GetYaxis()->SetTitleSize(0.06);
        h_failed_bkg->GetXaxis()->SetLabelSize(0.06);
        h_failed_bkg->GetYaxis()->SetLabelSize(0.06);
        h_failed_bkg->SetLineColor(kRed);
        h_failed_bkg->SetMarkerColor(kRed);
        h_failed_bkg->SetLineWidth(2);

        h_failed->Draw("PE");
        h_failed_bkg->Draw("SAMESPE");

        TLegend *leg = new TLegend(0.7,0.8,0.89,0.95);
        leg->SetHeader(("   " + std::to_string(int((double) nentries / (double)nPoints1d)) + " toys").c_str());
        leg->SetFillColorAlpha(0, 0.5);
        leg->AddEntry(h_failed,"plugin toys","PE");
        leg->AddEntry(h_failed_bkg,"bkg-only toys","PE");
        leg->Draw("same");


        // Distribution of good plugin toys
        // Values should be 1. and flat. 
        savePlot(canvas1, "failed_toys_plugin_"+scanVar1);
        TCanvas* can = newNoWarnTCanvas("can", "can");
        can->cd();
        gStyle->SetOptTitle(0);
        gStyle->SetPadTopMargin(0.05);
        gStyle->SetPadRightMargin(0.11);
        gStyle->SetPadBottomMargin(0.17);
        gStyle->SetPadLeftMargin(0.16);
        gStyle->SetLabelOffset(0.015, "X");
        gStyle->SetLabelOffset(0.015, "Y");
        h_fracGoodToys->SetXTitle(w->var(scanVar1)->GetTitle());
        h_fracGoodToys->SetYTitle("fraction of good plugin toys");
        h_fracGoodToys->Draw("PE");
        savePlot(can, "good toys_"+scanVar1);


        TCanvas *canvas = newNoWarnTCanvas("canvas", "canvas");
        canvas->Divide(2, 2);
        canvas->cd(1);
        h_all->SetXTitle(w->var(scanVar1)->GetTitle());
        h_all->SetYTitle("Valid toys");
        h_all->Draw();
        canvas->cd(2);
        h_better->SetYTitle("Better toys than #Delta#chi^{2}_{data}");
        h_better->SetXTitle(w->var(scanVar1)->GetTitle());
        h_better->Draw();
        canvas->cd(3);
        // the goodness of fit distribution -> should be smooth close to the best fit point (probably always true)
        h_gof->SetXTitle(w->var(scanVar1)->GetTitle());
        h_gof->SetYTitle("(-2*NLL(toy,free)) - (-2*NLL(data,free)) (goodness of fit)");
        h_gof->Draw();
        TLegend *leg_gof = new TLegend(0.16,0.8,0.89,0.95);
        leg_gof->SetHeader("Should be smooth close to best fit point");
        leg_gof->SetFillColorAlpha(0, 0.5);
        leg_gof->Draw("same");
        TArrow *lD = new TArrow( hCL->GetBinCenter(hCL->GetMaximumBin()),0.9*h_gof->GetMaximum(), hCL->GetBinCenter(hCL->GetMaximumBin()), h_gof->GetMinimum(), 0.15, "|>" ); 
        lD->SetLineColor(kRed);
        lD->SetLineWidth(2);
        lD->Draw("same");

        canvas->cd(4);
        h_background->SetXTitle(w->var(scanVar1)->GetTitle());
        h_background->SetYTitle("fraction of neg. test stat toys");
        h_background->SetLineColor(kBlue);
        h_background->SetMarkerColor(kBlue);
        h_background->SetLineWidth(2);
        h_background->Draw("PE");
        h_negtest_bkg->SetXTitle(w->var(scanVar1)->GetTitle());
        h_negtest_bkg->SetYTitle("fraction of neg. test stat toys");
        h_negtest_bkg->SetLineColor(kRed);
        h_negtest_bkg->SetMarkerColor(kRed);
        h_negtest_bkg->SetLineWidth(2);
        h_negtest_bkg->Draw("SAMESPE");

        TLegend *leg_neg = new TLegend(0.7,0.8,0.89,0.95);
        leg_neg->SetHeader(("   " + std::to_string(int((double) nentries / (double)nPoints1d)) + " toys").c_str());
        leg_neg->SetFillColorAlpha(0, 0.5);
        leg_neg->AddEntry(h_background,"plugin toys","PE");
        leg_neg->AddEntry(h_negtest_bkg,"bkg-only toys","PE");
        leg_neg->Draw("same");
        savePlot(canvas, "debug_plots_"+scanVar1);
    }
    // goodness-of-fit

    int iBinBestFit = hCL->GetMaximumBin();
    float nGofBetter = h_gof->GetBinContent(iBinBestFit);
    float nall = h_all->GetBinContent(iBinBestFit);
    float fitprobabilityVal = nGofBetter / nall;
    float fitprobabilityErr = sqrt(fitprobabilityVal * (1. - fitprobabilityVal) / nall);
    cout << "MethodDatasetsPluginScan::readScan1dTrees() : fit prob of best-fit point: "
         << Form("(%.1f+/-%.1f)%%", fitprobabilityVal * 100., fitprobabilityErr * 100.) << endl;
}





double MethodDatasetsPluginScan::getPValueTTestStatistic(double test_statistic_value) {
    if ( test_statistic_value >= 0) {
        // this is the normal case
        return TMath::Prob(test_statistic_value, 1);
    } else {
        if (arg->verbose) {
        cout << "MethodDatasetsPluginScan::scan1d_prob() : WARNING : Test statistic is negative, forcing it to zero" << std::endl
             << "Fit at current scan point has higher likelihood than free fit." << std::endl
             << "This should not happen except for very small underflows when the scan point is at the best fit value. " << std::endl
             << "Value of test statistic is " << test_statistic_value << std::endl
             << "An equal upwards fluctuaion corresponds to a p value of " << TMath::Prob(abs(test_statistic_value), 1) << std::endl;
        }
        // TMath::Prob will return 0 if the Argument is slightly below zero. As we are working with a float-zero we can not rely on it here:
        // TMath::Prob( 0 ) returns 1
        return 1.;
    }
}


///
/// Perform the 1d Plugin scan.
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodDatasetsPluginScan::scan1d(int nRun)
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

    TString probResName = Form("root/scan1dDatasetsProb_" + this->pdf->getName() + "_%ip" + "_" + scanVar1 + ".root", arg->npoints1d);
    TFile* probResFile = TFile::Open(probResName);
    if (!probResFile) {
        std::cout << "ERROR in MethodDatasetsPluginScan::scan1d - Prob scan result file not found in " << std::endl
                  << probResName << std::endl
                  << "Please run the prob scan before running the plugin scan. " << std::endl
                  << "The result file of the prob scan can be specified via the --probScanResult command line argument." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Define outputfile
    TString dirname = "root/scan1dDatasetsPlugin_" + this->pdf->getName() + "_" + scanVar1;
    system("mkdir -p " + dirname);
    TFile* outputFile = new TFile(Form(dirname + "/scan1dDatasetsPlugin_" + this->pdf->getName() + "_" + scanVar1 + "_run%i.root", nRun), "RECREATE");

    // Set up toy root tree
    ToyTree toyTree(this->pdf, arg);
    toyTree.init();
    toyTree.nrun = nRun;

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    RooDataSet* parsFunctionCall = new RooDataSet("parsFunctionCall", "parsFunctionCall", *w->set(pdf->getParName()));
    parsFunctionCall->add(*w->set(pdf->getParName()));

        // if CLs toys we need to keep hold of what's going on in the bkg only case
    // there is a small overhead here but it's necessary because the bkg only hypothesis
    // might not necessarily be in the scan range (although often it will be the first point)
    vector<RooAbsData*> cls_bkgOnlyToys;
    vector<TString> bkgOnlyGlobObsSnaphots;
    vector<float> chi2minGlobalBkgToysStore;    // Global fit to bkg-only toys
    vector<float> chi2minBkgBkgToysStore;       // Bkg fit to bkg-only toys
    vector<float> scanbestBkgToysStore;         // best fit point of gloabl fit to bkg-only toys
    vector<float> scanbestBkgBkgToysStore;      // best fit point of bkg fit to bkg-only toys (usually zero because bkg pdf will not depend on signal parameter)
    vector<int> covQualFreeBkgToysStore;      // covariance quality of gloabl fit to bkg-only toys
    vector<int> covQualBkgBkgToysStore;       // covariance quality of bkg fit to bkg-only toys
    vector<int> StatusFreeBkgToysStore;       // status of gloabl fit to bkg-only toys
    vector<int> StatusBkgBkgToysStore;        // status of bkg fit to bkg-only toys
    // Titus: Try importance sampling from the combination part -> works, but definitely needs improvement in precision
    int nActualToys = nToys;
    if ( arg->importance ){
        float plhPvalue = TMath::Prob(toyTree.chi2min - toyTree.chi2minGlobal,1);
        nActualToys = nToys*importance(plhPvalue);
    }
    for ( int j = 0; j < nActualToys; j++ ) {
        // std::cout << "Toy " << j << std::endl;
      // if(pdf->getBkgPdf())
      {
        Utils::setParameters(w,dataBkgFitResult); //set parameters to bkg fit so the generation always starts at the same value
        // pdf->printParameters();
        pdf->generateBkgToys(0,arg->var[0]);
        pdf->generateBkgToysGlobalObservables(0,j);
        RooAbsData* bkgOnlyToy = pdf->getBkgToyObservables();
        cls_bkgOnlyToys.push_back( (RooAbsData*)bkgOnlyToy->Clone() ); // clone required because of deleteToys() call at end of loop
        bkgOnlyGlobObsSnaphots.push_back(pdf->globalObsBkgToySnapshotName);
        pdf->setToyData( bkgOnlyToy );
        parameterToScan->setConstant(false);
        // Do a global fit to bkg-only toys
        RooFitResult *rb = loadAndFitBkg(pdf);
        assert(rb);
        pdf->setMinNllScan(pdf->minNll);
        if (pdf->getFitStatus() != 0) {
            pdf->setFitStrategy(1);
            delete rb;
            rb = loadAndFitBkg(pdf);
            pdf->setMinNllScan(pdf->minNll);
            assert(rb);

            if (pdf->getFitStatus() != 0) {
                pdf->setFitStrategy(2);
                delete rb;
                rb = loadAndFitBkg(pdf);
                assert(rb);
            }
        }
        Utils::setParameters(w,rb); // set parameters to fitresult of best fit before making refitting decision, necessary if using multipdf
        // implement physical range a la Feldman Cousins
        bool refit_necessary = false;
        if ( arg->physRanges.size()>0 ){
            for ( int j=0; j<arg->physRanges[0].size(); j++ ){
                if ( w->var( arg->physRanges[0][j].name ) ){
                    if(w->var( arg->physRanges[0][j].name )->getVal()<arg->physRanges[0][j].min){
                        if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].min << std::endl;
                        w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].min);
                        w->var( arg->physRanges[0][j].name )->setConstant(true);
                        refit_necessary= true;
                    }
                    else if (w->var( arg->physRanges[0][j].name )->getVal() > arg->physRanges[0][j].max){
                        if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].max << std::endl;
                        w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].max);
                        w->var( arg->physRanges[0][j].name )->setConstant(true);
                        refit_necessary=true;
                    }
                }
            }
        }
        // refit after having set parameters accordingly
        if(refit_necessary){
            rb = loadAndFitBkg(pdf);
            assert(rb);
            pdf->setMinNllScan(pdf->minNll);
            if (pdf->getFitStatus() != 0) {
                pdf->setFitStrategy(1);
                delete rb;
                rb = loadAndFitBkg(pdf);
                pdf->setMinNllScan(pdf->minNll);
                assert(rb);

                if (pdf->getFitStatus() != 0) {
                    pdf->setFitStrategy(2);
                    delete rb;
                    rb = loadAndFitBkg(pdf);
                    assert(rb);
                }
            }
        }


        if (std::isinf(pdf->minNll) || std::isnan(pdf->minNll)) {
            cout  << "++++ > second and a half fit gives inf/nan: "  << endl
                  << "++++ > minNll: " << pdf->minNll << endl
                  << "++++ > status: " << pdf->getFitStatus() << endl;
            pdf->setFitStatus(-99);
        }
        if (rb->edm()>1.e-3) {
            cout  << "++++ > Fit not converged: "  << endl
                  << "++++ > edm: " << rb->edm() << endl
                  << "++++ > status: " << -60 << endl;
            pdf->setFitStatus(-60);
        }
        pdf->setMinNllScan(pdf->minNll);

        // chi2minGlobalBkgToysStore.push_back( 2 * rb->minNll() );
        chi2minGlobalBkgToysStore.push_back( 2 * pdf->getMinNll() );
        if(rb->floatParsFinal().find(scanVar1)){
            scanbestBkgToysStore.push_back( ((RooRealVar*)w->set(pdf->getParName())->find(scanVar1))->getVal() );
        }
        // if the pdf does not depend on the signal parameter, set best fit value of signa l parameter for the bkg fit to 0
        else scanbestBkgToysStore.push_back(0.0);
        covQualFreeBkgToysStore.push_back(rb->covQual());
        StatusFreeBkgToysStore.push_back(pdf->getFitStatus());

        //reset parameters free from the Feldman Cousins behaviour
        if ( arg->physRanges.size()>0){
            for ( int j=0; j<arg->physRanges[0].size(); j++ ){
                if ( w->var( arg->physRanges[0][j].name ) && w->set(pdf->getParName())->find(arg->physRanges[0][j].name)){  //if somebody wants to modify a constant parameter make sure the parameter doesn't accidentally become floating...
                    w->var( arg->physRanges[0][j].name )->setConstant(false);
                }
            }
        }

        // fit the bkg-only toys with the bkg-only hypothesis
        delete rb;
        rb = pdf->fitBkg(bkgOnlyToy, arg->var[0]);
        assert(rb);
        pdf->setMinNllScan(pdf->minNll);
        if (pdf->getFitStatus() != 0) {
            pdf->setFitStrategy(1);
            delete rb;
            rb = pdf->fitBkg(bkgOnlyToy, arg->var[0]);
            pdf->setMinNllScan(pdf->minNll);
            assert(rb);

            if (pdf->getFitStatus() != 0) {
                pdf->setFitStrategy(2);
                delete rb;
                rb = pdf->fitBkg(bkgOnlyToy, arg->var[0]);
                assert(rb);
            }
        }
        if (rb->edm()>1.e-3) {
            cout  << "++++ > Bkg Fit not converged: "  << endl
                  << "++++ > edm: " << rb->edm() << endl
                  << "++++ > status: " << -60 << endl;
            pdf->setFitStatus(-60);
        }
        // chi2minBkgBkgToysStore.push_back( 2 * rb->minNll() );
        chi2minBkgBkgToysStore.push_back( 2 * pdf->getMinNllBkg() );
        if(rb->floatParsFinal().find(scanVar1)){
            std::cout << "found signal parameter in bkg fit with value " << ((RooRealVar*)w->set(pdf->getParName())->find(scanVar1))->getVal() << std::endl;
            scanbestBkgBkgToysStore.push_back( ((RooRealVar*)w->set(pdf->getParName())->find(scanVar1))->getVal() );
        }
        // if the pdf does not depend on the signal parameter, set best fit value of signal parameter for the bkg fit to 0
        else scanbestBkgBkgToysStore.push_back(0.0);
        covQualBkgBkgToysStore.push_back(rb->covQual());
        StatusBkgBkgToysStore.push_back(pdf->getFitStatus());

        delete rb;
        pdf->deleteToys();
      }
    }
    // start scan
    std::cout << "MethodDatasetsPluginScan::scan1d_plugin() : starting ... with " << nPoints1d << " scanpoints..." << std::endl;
    ProgressBar progressBar(arg, nPoints1d);
    for ( int i = 0; i < nPoints1d; i++ )
    {

                toyTree.npoint = i;

                progressBar.progress();
        // scanpoint is calculated using min, max, which are the hCL x-Axis limits set in this->initScan()
        // this uses the "scan" range, as expected
        // don't add half the bin size. try to solve this within plotting method

        float scanpoint = parameterToScan_min + (parameterToScan_max - parameterToScan_min) * (double)i / ((double)nPoints1d - 1);
        toyTree.scanpoint = scanpoint;

                if ( i==0 && scanpoint != 0 ) {
                    cout << "WARNING: For CLs option the first point in the scan should be zero, not: " << scanpoint << endl;
                    // exit(1);
                }

        if (arg->debug) cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - scanpoint in step " << i << " : " << scanpoint << endl;

        // don't scan in unphysical region
        // by default this means checking against "free" range
        if ( scanpoint < parameterToScan->getMin() || scanpoint > parameterToScan->getMax() + 2e-13 ) {
            cout << "not obvious: " << scanpoint << " < " << parameterToScan->getMin() << " and " << scanpoint << " > " << parameterToScan->getMax() + 2e-13 << endl;
            continue;
        }

        // Load the parameter values (nuisance parameters and parameter of interest) from the fit to data with fixed parameter of interest.
        this->setParevolPointByIndex(i);

        toyTree.statusScanData  = this->getParValAtIndex(i, "statusScanData");
        toyTree.chi2min         = this->getParValAtIndex(i, "chi2min");
        toyTree.covQualScanData = this->getParValAtIndex(i, "covQualScanData");

        // get the chi2 of the data
        if (this->chi2minGlobalFound) {
            toyTree.chi2minGlobal     = this->getChi2minGlobal();
        }
        else {
            cout << "FATAL in MethodDatasetsPluginScan::scan1d_plugin() - Global Minimum not set!" << endl;
            exit(EXIT_FAILURE);
        }

         toyTree.chi2minBkg     = this->getChi2minBkg();

        toyTree.storeParsPll();
        toyTree.genericProbPValue = this->getPValueTTestStatistic(toyTree.chi2min - toyTree.chi2minGlobal);

        // Titus: Try importance sampling from the combination part -> works, but definitely needs improvement in precision
        int nActualToys = nToys;
        if ( arg->importance ){
            float plhPvalue = TMath::Prob(toyTree.chi2min - toyTree.chi2minGlobal,1);
            nActualToys = nToys*importance(plhPvalue);
        }
        //Titus: Debug histogram to see the different deltachisq distributions
        TH1F histdeltachi2("histdeltachi2", "histdeltachi2", 200,0,5);
        for ( int j = 0; j < nActualToys; j++ )
        {
            if (arg->debug) cout << ">> new toy\n" << endl;
            this->pdf->setMinNllFree(0);
            this->pdf->setMinNllScan(0);

                        toyTree.ntoy = j;

            // 1. Generate toys

            // For toy generation, set all parameters (nuisance parameters and parameter of interest) to the values from the constrained
            // fit to data with fixed parameter of interest.
            // This is called the PLUGIN method.
            this->setParevolPointByIndex(i);

            // If there is a multipdf, set bestIndexScan (taken from ProbScan)
            // and this index will be used to generate toys
            if (this->pdf->isMultipdfInitialized()) {
                this->getProfileLH()->probScanTree->GetEntry(i);
                int index = this->getProfileLH()->probScanTree->bestIndexScanData;
                this->pdf->setBestIndexScan(index);
            }
            this->pdf->generateToys(); // this is generating the toy dataset
            this->pdf->generateToysGlobalObservables(); // this is generating the toy global observables and saves globalObs in snapshot

            //
            // 2. Fit to toys with parameter of interest fixed to scanpoint
            //
            if (arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - perform scan toy fit" << endl;

            // set parameters to constrained data scan fit result again
            this->setParevolPointByIndex(i);

            // fixed parameter of interest
            parameterToScan->setConstant(true);
            this->pdf->setFitStrategy(0);
            RooFitResult* r   = this->loadAndFit(this->pdf);
            assert(r);
            pdf->setMinNllScan(pdf->minNll);

            this->setAndPrintFitStatusFreeToys(toyTree);

            if (pdf->getFitStatus() != 0) {
                pdf->setFitStrategy(1);
                delete r;
                r = this->loadAndFit(this->pdf);
                pdf->setMinNllScan(pdf->minNll);
                assert(r);

                this->setAndPrintFitStatusFreeToys(toyTree);

                if (pdf->getFitStatus() != 0) {
                    pdf->setFitStrategy(2);
                    delete r;
                    r = this->loadAndFit(this->pdf);
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


            // toyTree.chi2minToy          = 2 * r->minNll(); // 2*r->minNll(); //2*r->minNll();
            toyTree.chi2minToy          = 2 * pdf->getMinNll(); // 2*r->minNll(); //2*r->minNll();
            toyTree.chi2minToyPDF       = 2 * pdf->getMinNllScan();
            toyTree.covQualScan         = r->covQual();
            toyTree.statusScan          = r->status();
            toyTree.statusScanPDF       = pdf->getFitStatus(); //r->status();
            toyTree.storeParsScan();

            pdf->deleteNLL();

            RooDataSet* parsAfterScanFit = new RooDataSet("parsAfterScanFit", "parsAfterScanFit", *w->set(pdf->getParName()));
            parsAfterScanFit->add(*w->set(pdf->getParName()));

            //
            // 2.5 Fit to bkg only toys with parameter of interest fixed to scanpoint
            //
            if (arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - perform scan toy fit to background" << endl;

            // set parameters to constrained data scan fit result again
            this->setParevolPointByIndex(i);

            // fixed parameter of interest
            parameterToScan->setConstant(true);
            this->pdf->setFitStrategy(0);
            // temporarily store our current toy here so we can put it back in a minute
            RooAbsData *tempData = (RooAbsData*)this->pdf->getToyObservables();
            // now get our background only toy (to fit under this hypothesis)
            RooAbsData *bkgToy = (RooAbsData*)cls_bkgOnlyToys[j];
            if (arg->debug) cout << "Setting background toy as data " << bkgToy << endl;
            this->pdf->setBkgToyData( bkgToy );
            this->pdf->setGlobalObsSnapshotBkgToy( bkgOnlyGlobObsSnaphots[j] );

            RooFitResult* rb   = this->loadAndFitBkg(this->pdf);
            assert(rb);
            pdf->setMinNllScan(pdf->minNll);

            this->setAndPrintFitStatusFreeToys(toyTree);

            if (pdf->getFitStatus() != 0) {
                pdf->setFitStrategy(1);
                delete rb;
                rb = this->loadAndFitBkg(this->pdf);
                pdf->setMinNllScan(pdf->minNll);
                assert(rb);

                this->setAndPrintFitStatusFreeToys(toyTree);

                if (pdf->getFitStatus() != 0) {
                    pdf->setFitStrategy(2);
                    delete rb;
                    rb = this->loadAndFitBkg(this->pdf);
                    assert(rb);
                }
            }

            if (std::isinf(pdf->minNll) || std::isnan(pdf->minNll)) {
                cout  << "++++ > second and a half fit gives inf/nan: "  << endl
                      << "++++ > minNll: " << pdf->minNll << endl
                      << "++++ > status: " << pdf->getFitStatus() << endl;
                pdf->setFitStatus(-99);
            }
            pdf->setMinNllScan(pdf->minNll);


            // toyTree.chi2minBkgToy          = 2 * rb->minNll(); // 2*r->minNll(); //2*r->minNll();
            toyTree.chi2minBkgToy          = 2 * pdf->getMinNll(); // 2*r->minNll(); //2*r->minNll();
            toyTree.chi2minBkgToyPDF       = 2 * pdf->getMinNllScan();
            toyTree.covQualScanBkg         = rb->covQual(); // 2*r->minNll(); //2*r->minNll();
            toyTree.statusScanBkg          = pdf->getFitStatus();

            pdf->deleteNLL();

            //
            // 3. Fit to toys with free parameter of interest
            //
            if (arg->debug)cout << "DEBUG in MethodDatasetsPluginScan::scan1d_plugin() - perform free toy fit" << endl;
            // Use parameters from the scanfit to data

            this->setParevolPointByIndex(i);

            // free parameter of interest
            parameterToScan->setConstant(false);
            //setLimit(w, scanVar1, "free");
            // w->var(scanVar1)->removeRange();

            // set dataset back
            if (arg->debug) cout << "Setting toy back as data " << tempData << endl;
            this->pdf->setToyData( tempData );
            // restore MinNllScan to value from 2. (not take from 2.5) for more correct error messages
            pdf->setMinNllScan(toyTree.chi2minToy/2.);

            // Fit
            pdf->setFitStrategy(0);
            RooFitResult* r1  = this->loadAndFit(this->pdf);
            assert(r1);
            pdf->setMinNllFree(pdf->minNll);
            // toyTree.chi2minGlobalToy = 2 * r1->minNll();
            toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();

            if (! std::isfinite(pdf->getMinNllFree())) {
                cout << "----> nan/inf flag detected " << endl;
                cout << "----> fit status: " << pdf->getFitStatus() << endl;
                pdf->setFitStatus(-99);
            }

            bool negTestStat = toyTree.chi2minToy - toyTree.chi2minGlobalToy < 0;

            this->setAndPrintFitStatusConstrainedToys(toyTree);


            if (pdf->getFitStatus() != 0 || negTestStat ) {

                pdf->setFitStrategy(1);

                if (arg->verbose) cout << "----> refit with strategy: 1" << endl;
                delete r1;
                r1  = this->loadAndFit(this->pdf);
                assert(r1);
                pdf->setMinNllFree(pdf->minNll);
                // toyTree.chi2minGlobalToy = 2 * r1->minNll();
                toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();
                if (! std::isfinite(pdf->getMinNllFree())) {
                    cout << "----> nan/inf flag detected " << endl;
                    cout << "----> fit status: " << pdf->getFitStatus() << endl;
                    pdf->setFitStatus(-99);
                }
                negTestStat = toyTree.chi2minToy - toyTree.chi2minGlobalToy < 0;

                this->setAndPrintFitStatusConstrainedToys(toyTree);

                if (pdf->getFitStatus() != 0 || negTestStat ) {

                    pdf->setFitStrategy(2);

                    if (arg->verbose) cout << "----> refit with strategy: 2" << endl;
                    delete r1;
                    r1  = this->loadAndFit(this->pdf);
                    assert(r1);
                    pdf->setMinNllFree(pdf->minNll);
                    // toyTree.chi2minGlobalToy = 2 * r1->minNll();
                    toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();
                    if (! std::isfinite(pdf->getMinNllFree())) {
                        cout << "----> nan/inf flag detected " << endl;
                        cout << "----> fit status: " << pdf->getFitStatus() << endl;
                        pdf->setFitStatus(-99);
                    }
                    if (r1->edm()>1.e-3) {
                        cout << "----> too large edm " << endl;
                        cout << "----> edm: " << r1->edm() << endl;
                        pdf->setFitStatus(-60);
                    }
                    this->setAndPrintFitStatusConstrainedToys(toyTree);

                    if ( (toyTree.chi2minToy - toyTree.chi2minGlobalToy) < 0) {
                        cout << "+++++ > still negative test statistic after whole procedure!! " << endl;
                        cout << "+++++ > try to fit with different starting values" << endl;
                        cout << "+++++ > dChi2: " << toyTree.chi2minToy - toyTree.chi2minGlobalToy << endl;
                        cout << "+++++ > dChi2PDF: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl;
                        Utils::setParameters(this->pdf->getWorkspace(), pdf->getParName(), parsAfterScanFit->get(0));
                        // if (parameterToScan->getVal() < 1e-13) parameterToScan->setVal(0.67e-12); //what do we gain from this?
                        parameterToScan->setConstant(false);
                        pdf->deleteNLL();
                        RooFitResult* r_tmp = this->loadAndFit(this->pdf);
                        assert(r_tmp);
                        if (r_tmp->status() == 0 && r_tmp->minNll() < r1->minNll() && r_tmp->minNll() > -1e27) {
                            pdf->setMinNllFree(pdf->minNll);
                            cout << "+++++ > Improvement found in extra fit: Nll before: " << r1->minNll()
                                 << " after: " << r_tmp->minNll() << endl;
                            delete r1;
                            r1 = r_tmp;
                            cout << "+++++ > new minNll value: " << r1->minNll() << endl;
                        }
                        else {
                            // set back parameter value to last fit value
                            cout << "+++++ > no Improvement found, reset ws par value to last fit result" << endl;
                            parameterToScan->setVal(static_cast<RooRealVar*>(r1->floatParsFinal().find(parameterToScan->GetName()))->getVal());
                            delete r_tmp;
                        }
                    };
                    if (arg->debug) {
                        cout  << "===== > compare free fit result with pdf parameters: " << endl;
                        cout  << "===== > minNLL for fitResult: " << r1->minNll() << endl
                              << "===== > minNLL for pdfResult: " << pdf->getMinNllFree() << endl
                              << "===== > status for pdfResult: " << pdf->getFitStatus() << endl
                              << "===== > status for fitResult: " << r1->status() << endl;
                    }
                }
            }

            Utils::setParameters(w,r1); //  set parameters to fitresult of best fit before making refitting decision, necessary if using multipdf
            // implement physical range a la Feldman Cousins
            bool refit_necessary = false;
            std::map<TString, double> boundary_vals;
            if ( arg->physRanges.size()>0 ){
                for ( int j=0; j<arg->physRanges[0].size(); j++ ){
                    if ( w->var( arg->physRanges[0][j].name ) ){
                        if(w->var( arg->physRanges[0][j].name )->getVal()<arg->physRanges[0][j].min){
                            if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].min << std::endl;
                            w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].min);
                            w->var( arg->physRanges[0][j].name )->setConstant(true);
                            boundary_vals[arg->physRanges[0][j].name] = arg->physRanges[0][j].min;
                            refit_necessary=true;
                        }
                        else if (w->var( arg->physRanges[0][j].name )->getVal() > arg->physRanges[0][j].max){
                            if(arg->debug) std::cout << "MethodDatasetsPluginScan::scan1d()::fit " << arg->physRanges[0][j].name <<"=" << w->var( arg->physRanges[0][j].name )->getVal() <<" out of physics range, fixing to " << arg->physRanges[0][j].max << std::endl;
                            w->var( arg->physRanges[0][j].name )->setVal(arg->physRanges[0][j].max);
                            w->var( arg->physRanges[0][j].name )->setConstant(true);
                            boundary_vals[arg->physRanges[0][j].name] = arg->physRanges[0][j].max;
                            refit_necessary=true;
                        }
                    }
                }
            }
            // refit after having set parameters accordingly
            //Fit
            if(refit_necessary){
                pdf->setFitStrategy(0);
                r1  = this->loadAndFit(this->pdf);
                assert(r1);
                pdf->setMinNllFree(pdf->minNll);
                // toyTree.chi2minGlobalToy = 2 * r1->minNll();
                toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();

                if (! std::isfinite(pdf->getMinNllFree())) {
                    cout << "----> nan/inf flag detected " << endl;
                    cout << "----> fit status: " << pdf->getFitStatus() << endl;
                    pdf->setFitStatus(-99);
                }

                negTestStat = toyTree.chi2minToy - toyTree.chi2minGlobalToy < 0;

                this->setAndPrintFitStatusConstrainedToys(toyTree);


                if (pdf->getFitStatus() != 0 || negTestStat ) {

                    pdf->setFitStrategy(1);

                    if (arg->verbose) cout << "----> refit with strategy: 1" << endl;
                    delete r1;
                    r1  = this->loadAndFit(this->pdf);
                    assert(r1);
                    pdf->setMinNllFree(pdf->minNll);
                    // toyTree.chi2minGlobalToy = 2 * r1->minNll();
                    toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();
                    if (! std::isfinite(pdf->getMinNllFree())) {
                        cout << "----> nan/inf flag detected " << endl;
                        cout << "----> fit status: " << pdf->getFitStatus() << endl;
                        pdf->setFitStatus(-99);
                    }
                    negTestStat = toyTree.chi2minToy - toyTree.chi2minGlobalToy < 0;

                    this->setAndPrintFitStatusConstrainedToys(toyTree);

                    if (pdf->getFitStatus() != 0 || negTestStat ) {

                        pdf->setFitStrategy(2);

                        if (arg->verbose) cout << "----> refit with strategy: 2" << endl;
                        delete r1;
                        r1  = this->loadAndFit(this->pdf);
                        assert(r1);
                        pdf->setMinNllFree(pdf->minNll);
                        // toyTree.chi2minGlobalToy = 2 * r1->minNll();
                        toyTree.chi2minGlobalToy = 2 * pdf->getMinNllFree();
                        if (! std::isfinite(pdf->getMinNllFree())) {
                            cout << "----> nan/inf flag detected " << endl;
                            cout << "----> fit status: " << pdf->getFitStatus() << endl;
                            pdf->setFitStatus(-99);
                        }
                        if (r1->edm()>1.e-3) {
                            cout << "----> too large edm " << endl;
                            cout << "----> edm: " << r1->edm() << endl;
                            pdf->setFitStatus(-60);
                        }
                        this->setAndPrintFitStatusConstrainedToys(toyTree);

                        if ( (toyTree.chi2minToy - toyTree.chi2minGlobalToy) < 0) {
                            cout << "+++++ > still negative test statistic after whole procedure!! " << endl;
                            cout << "+++++ > try to fit with different starting values" << endl;
                            cout << "+++++ > dChi2: " << toyTree.chi2minToy - toyTree.chi2minGlobalToy << endl;
                            cout << "+++++ > dChi2PDF: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl;
                            Utils::setParameters(this->pdf->getWorkspace(), pdf->getParName(), parsAfterScanFit->get(0));
                            // but need to keep the parameters fixed to boundary:
                            for(auto element : boundary_vals){
                                w->var(element.first)->setVal(element.second);
                            }
                            pdf->deleteNLL();
                            RooFitResult* r_tmp = this->loadAndFit(this->pdf);
                            assert(r_tmp);
                            if (r_tmp->status() == 0 && r_tmp->minNll() < r1->minNll() && r_tmp->minNll() > -1e27) {
                                pdf->setMinNllFree(pdf->minNll);
                                cout << "+++++ > Improvement found in extra fit: Nll before: " << r1->minNll()
                                     << " after: " << r_tmp->minNll() << endl;
                                delete r1;
                                r1 = r_tmp;
                                cout << "+++++ > new minNll value: " << r1->minNll() << endl;
                            }
                            else {
                                // set back parameter value to last fit value
                                cout << "+++++ > no Improvement found";
                                if(!parameterToScan->isConstant()){
                                    std::cout << ", reset ws par value to last fit result";
                                    parameterToScan->setVal(static_cast<RooRealVar*>(r1->floatParsFinal().find(parameterToScan->GetName()))->getVal());
                                }
                                std::cout << std::endl;
                                delete r_tmp;
                            }
                        };
                        if (arg->debug) {
                            cout  << "===== > compare free fit result with pdf parameters: " << endl;
                            cout  << "===== > minNLL for fitResult: " << r1->minNll() << endl
                                  << "===== > minNLL for pdfResult: " << pdf->getMinNllFree() << endl
                                  << "===== > status for pdfResult: " << pdf->getFitStatus() << endl
                                  << "===== > status for fitResult: " << r1->status() << endl;
                        }
                    }
                }
            }

            //reset parameters free from the Feldman Cousins behaviour
            if ( arg->physRanges.size()>0){
                for ( int j=0; j<arg->physRanges[0].size(); j++ ){
                    if ( w->var( arg->physRanges[0][j].name ) && w->set(pdf->getParName())->find(arg->physRanges[0][j].name)){  //if somebody wants to modify a constant parameter make sure the parameter doesn't accidentally become floating...
                        w->var( arg->physRanges[0][j].name )->setConstant(false);
                    }
                }
            }

            // set the limit back again
            // setLimit(w, scanVar1, "scan");

            // toyTree.chi2minGlobalToy    = 2 * r1->minNll(); //2*r1->minNll();
            toyTree.chi2minGlobalToy    = 2 * pdf->getMinNllFree(); //2*r1->minNll();
            toyTree.chi2minGlobalToyPDF = 2 * pdf->getMinNllFree(); //2*r1->minNll();
            toyTree.statusFreePDF       = pdf->getFitStatus(); //r1->status();
            toyTree.statusFree          = r1->status();
            toyTree.covQualFree         = r1->covQual();
            toyTree.scanbest            = ((RooRealVar*)w->set(pdf->getParName())->find(scanVar1))->getVal();
            toyTree.storeParsFree();
            pdf->deleteNLL();

            assert( chi2minGlobalBkgToysStore.size() == nToys );
            assert( scanbestBkgToysStore.size() == nToys );
            assert( covQualFreeBkgToysStore.size() == nToys );
            assert( StatusFreeBkgToysStore.size() == nToys );

            assert( chi2minBkgBkgToysStore.size() == nToys );
            assert( scanbestBkgBkgToysStore.size() == nToys );
            assert( covQualBkgBkgToysStore.size() == nToys );
            assert( StatusBkgBkgToysStore.size() == nToys );
            //}
            toyTree.chi2minGlobalBkgToy = chi2minGlobalBkgToysStore[j];
            toyTree.scanbestBkg         = scanbestBkgToysStore[j];
            toyTree.covQualFreeBkg      = covQualFreeBkgToysStore[j];
            toyTree.statusFreeBkg       = StatusFreeBkgToysStore[j];

            toyTree.chi2minBkgBkgToy    = chi2minBkgBkgToysStore[j];
            toyTree.scanbestBkgfitBkg   = scanbestBkgBkgToysStore[j];
            toyTree.covQualBkgBkg       = covQualBkgBkgToysStore[j];
            toyTree.statusBkgBkg        = StatusBkgBkgToysStore[j];

            if (arg->debug) {
                cout << "#### > Fit summary: " << endl;
                cout  << "#### > free fit status: " << toyTree.statusFree << " vs pdf: " << toyTree.statusFreePDF << endl
                      << "#### > scan fit status: " << toyTree.statusScan << " vs pdf: " << toyTree.statusScanPDF << endl
                      << "#### > free min nll: " << toyTree.chi2minGlobalToy << " vs pdf: " << toyTree.chi2minGlobalToyPDF << endl
                      << "#### > scan min nll: " << toyTree.chi2minToy << " vs pdf: " << toyTree.chi2minToyPDF << endl
                      << "#### > dChi2 fitresult: " << toyTree.chi2minToy - toyTree.chi2minGlobalToy << endl
                      << "#### > dChi2 pdfresult: " << toyTree.chi2minToyPDF - toyTree.chi2minGlobalToyPDF << endl;
                cout  << std::setprecision(6);

                if (toyTree.chi2minToy - toyTree.chi2minGlobalToy > 20 && (toyTree.statusFree == 0 && toyTree.statusScan == 0)
                        && toyTree.chi2minToy > -1e27 && toyTree.chi2minGlobalToy > -1e27) {
                    cout << std::setw(30) << std::setfill('-') << ">>> HIGH test stat value!! print fit results with fit strategy: " << pdf->getFitStrategy() << std::setfill(' ') << endl;
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

            //Titus:Debug fill deltachisq hist
            if (arg->debug) histdeltachi2.Fill(toyTree.chi2minToy-toyTree.chi2minGlobalToy);

            toyTree.fill();
            //remove dataset and pointers
            delete parsAfterScanFit;
            delete r;
            delete r1;
            delete rb;
            pdf->deleteToys();


        } // End of toys loop
        // reset
        setParameters(w, pdf->getParName(), parsFunctionCall->get(0));
        //delete result;

        //setParameters(w, pdf->getObsName(), obsDataset->get(0));

        //Titus Draw debug histdeltachisq
        if (arg->debug)
        {
            TCanvas histplot("histplot", "Delta chi2 toys", 1024, 786);
            histdeltachi2.Draw();
            string plotstring = "plots/pdf/deltachi2_"+to_string(i)+".pdf";
            histplot.SaveAs(plotstring.c_str());
        }

    } // End of npoints loop
    toyTree.writeToFile();
    outputFile->Close();
    delete parsFunctionCall;
    if (arg->verbose) std::cout << "DEBUG::MethodDatasetsPluginScan::scan1d total: s+b fits: "<< pdf->nsbfits << " (optimal: "<< nToys*(3*nPoints1d+1)+1 << "), bkg-only fits: " << pdf->nbkgfits << " (optimal "<< nToys+2<<")" << std::endl;
    return 0;
}



void MethodDatasetsPluginScan::drawDebugPlots(int runMin, int runMax, TString fileNameBaseIn) {
    int nFilesRead, nFilesMissing;
    TChain* c = this->readFiles(runMin, runMax, nFilesRead, nFilesMissing, fileNameBaseIn);
    //ToyTree t(this->pdf, c);
    //t.open();
    cout << "does it take long?" << endl;

    TString cut = "scanpoint == 0 && statusScan == 0 && statusFree == 0 && abs(chi2minToy)<300e3 && abs(chi2minGlobalToy)<300e3";
    TString isphysical = "(chi2minToy-chi2minGlobalToy)>=0";
    TCanvas* can = new TCanvas("can", "DChi2Nominal", 1024, 786);
    TCanvas* can1 = new TCanvas("can1", "BR_{Bd}", 1024, 786);
    TCanvas* can3 = new TCanvas("can3", "Chi2distr", 1024, 786);

    TCanvas* can2 = new TCanvas("can2", "DChi2False", 1024, 786);
    can->cd();
    chain->Draw("chi2minToy-chi2minGlobalToy", cut + "&&" + isphysical + " && abs(chi2minToy-chi2minGlobalToy)<1e2", "norm");
    can1->cd();
    chain->Draw("BR_{Bd}_free", cut + "&&" + isphysical, "norm");
    can2->cd();
    chain->Draw("chi2minToy-chi2minGlobalToy", "!(" + cut + "&&" + isphysical + ") && abs(chi2minToy-chi2minGlobalToy)<1e2", "norm");
    can3->cd();
    c->Draw("chi2minToy", cut, "norm");
    c->Draw("chi2minGlobalToy", cut, "normSAME");
    //cout << "draw takes a while" << endl;
};


///
/// Assumption: root file is given to the scanner which only has toy at a specific scanpoint, not necessary!
///
void MethodDatasetsPluginScan::performBootstrapTest(int nSamples, const TString& ext) {
    TRandom3 rndm;
    TH1F* hist = new TH1F("h", "h", 800, 1e-4, 0.008);
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
    for (int i = 0; i < t.GetEntries(); i++) {
        t.GetEntry(i);
        if (i == 0) {
            q_data = t.chi2min - this->chi2minGlobal;
            cout << "Test stat for data: " << q_data << endl;
        }
        if (!(t.statusScan == 0 && t.statusFree == 0 && fabs(t.chi2minToy) < 1e27
                && fabs(t.chi2minGlobalToy) < 1e27 && t.scanpoint == 0))
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

        q.push_back(t.chi2minToy - t.chi2minGlobalToy);

    }
    cout  << "INFO in MethodDatasetsPluginScan::performBootstrapTest - Tree loop finished" << endl;
    cout  << "- start BootstrapTest with " << nSamples
          << " Samples and " << numberOfToys << " Toys each" << endl;
    cout  << " Total number failed: " << totFailed << endl;

    for (int i = 0; i < nSamples; ++i) {
        int nSelected   = 0;
        double nbetter  = 0;
        for (int j = 0; j < numberOfToys; j++) {

            int rndmInt = -1;
            do {
                rndmInt = rndm.Integer(numberOfToys);
            } while (std::find(failed.begin(), failed.end(), rndmInt) != failed.end());

            if ( (q[rndmInt]) > q_data ) nbetter += 1;
        }
        double p = nbetter / numberOfToys;
        bootstrapPVals.push_back(p);
        hist->Fill(p);
        if (i % 100 == 0) cout << i << " Samples from " << nSamples << " done. p Value: " << p << " with " << nbetter << " Toys of " << numberOfToys << " total" << endl;
    }
    TCanvas* c = new TCanvas("c", "c", 1024, 768);
    hist->SetLineColor(kRed + 2);
    hist->SetLineWidth(2);
    hist->Fit("gaus");
    hist->Draw();

    c->SaveAs(Form("plots/root/" + name + "_bootStrap_%i_samples_with_%i_toys_" + ext + ".root", nSamples, numberOfToys));
    c->SaveAs(Form("plots/C/" + name + "_bootStrap_%i_samples_with_%i_toys_" + ext + ".C", nSamples, numberOfToys));
    c->SaveAs(Form("plots/pdf/" + name + "_bootStrap_%i_samples_with_%i_toys_" + ext + ".pdf", nSamples, numberOfToys));
    c->SaveAs(Form("plots/png/" + name + "_bootStrap_%i_samples_with_%i_toys_" + ext + ".png", nSamples, numberOfToys));

    return;

};

void MethodDatasetsPluginScan::printDebug(const RooFitResult& r) {
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


RooSlimFitResult* MethodDatasetsPluginScan::getParevolPoint(float scanpoint) {
    std::cout << "ERROR: not implemented for MethodDatasetsPluginScan, use setParevolPointByIndex() instad" << std::endl;
    exit(EXIT_FAILURE);
}


///
/// Load the param. values from the data-fit at a certain scan point
///
void MethodDatasetsPluginScan::setParevolPointByIndex(int index) {



    this->getProfileLH()->probScanTree->t->GetEntry(index);
    RooArgSet* pars          = (RooArgSet*)this->pdf->getWorkspace()->set(pdf->getParName());

    //\todo: make sure this is checked during pdf init, do not check again here
    if (!pars) {
        cout << "MethodDatasetsPluginScan::setParevolPointByIndex(int index) : ERROR : no parameter set found in workspace!" << endl;
        exit(EXIT_FAILURE);
    }

    TIterator* it = pars->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ) {
        TString parName     = p->GetName();
        TLeaf* parLeaf      = (TLeaf*) this->getProfileLH()->probScanTree->t->GetLeaf(parName + "_scan");
        if (!parLeaf) {
            cout << "MethodDatasetsPluginScan::setParevolPointByIndex(int index) : ERROR : no var (" << parName
                 << ") found in PLH scan file!" << endl;
            exit(EXIT_FAILURE);
        }
        float scanParVal    = parLeaf->GetValue();
        p->setVal(scanParVal);
    }
}



void MethodDatasetsPluginScan::setAndPrintFitStatusConstrainedToys(const ToyTree& toyTree) {

    if (pdf->getMinNllScan() != 0 && (pdf->getMinNllFree() > pdf->getMinNllScan())) {
        // create unique failureflag
        switch (pdf->getFitStatus())
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

    bool negTestStat = toyTree.chi2minToy - toyTree.chi2minGlobalToy < 0;

    if ( (pdf->getFitStatus() != 0 || negTestStat ) && arg->debug ) {
        cout  << "----> problem in current fit: going to refit with strategy " << pdf->getFitStrategy() << " , summary: " << endl
              << "----> NLL value: " << std::setprecision(9) << pdf->getMinNllFree() << endl
              << "----> fit status: " << pdf->getFitStatus() << endl
              << "----> dChi2: " << (toyTree.chi2minToy - toyTree.chi2minGlobalToy) << endl
              << "----> dChi2PDF: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl;

        switch (pdf->getFitStatus()) {
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
                  << "----> dChi2: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                  << "----> scan fit min nll:" << pdf->getMinNllScan() << endl
                  << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
            break;
        case -13:
            cout  << "----> free fit has status 0 but creates a negative test statistic" << endl
                  << "----> dChi2: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                  << "----> scan fit min nll:" << pdf->getMinNllScan() << endl
                  << "----> free fit min nll:" << pdf->getMinNllFree() << endl;
            break;
        case -12:
            cout  << "----> free fit has status 1 and creates a negative test statistic" << endl
                  << "----> dChi2: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
                  << "----> scan fit min nll:" << pdf->getMinNllScan() << endl
                  << "----> free fit min nll:" << pdf->getMinNllFree() << endl;

            break;
        case -33:
            cout  << "----> free fit has status -1 and creates a negative test statistic" << endl
                  << "----> dChi2: " << 2 * (pdf->getMinNllScan() - pdf->getMinNllFree()) << endl
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


void MethodDatasetsPluginScan::setAndPrintFitStatusFreeToys(const ToyTree& toyTree) {

    if (! std::isfinite(pdf->getMinNllScan())) {
        if ( arg->debug) {
          cout << "----> nan/inf flag detected " << endl;
          cout << "----> fit status: " << pdf->getFitStatus() << endl;
        }
        pdf->setFitStatus(-99);
    }

    if (pdf->getFitStatus() != 0 && arg->debug) {
        cout  << "----> problem in current fit: going to refit with strategy 1, summary: " << endl
              << "----> NLL value: " << std::setprecision(9) << pdf->minNll << endl
              << "----> fit status: " << pdf->getFitStatus() << endl;
        switch (pdf->getFitStatus()) {
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

void MethodDatasetsPluginScan::makeControlPlots(map<int, vector<double> > bVals, map<int, vector<double> > sbVals)
{
    // the quantiles of the CLb distribution (for expected CLs)
    std::vector<double> probs  = { TMath::Prob(4,1), TMath::Prob(1,1), 0.5, 1.-TMath::Prob(1,1), 1.-TMath::Prob(4,1) };
    std::vector<double> clb_vals  = { 1.-TMath::Prob(4,1), 1.-TMath::Prob(1,1), 0.5, TMath::Prob(1,1), TMath::Prob(4,1) };

    for ( int i=1; i<= hCLs->GetNbinsX(); i++ ) {

        std::vector<double> quantiles = Quantile<double>( bVals[i], probs );
        std::vector<double> clsb_vals;
        for (int k=0; k<quantiles.size(); k++ ){
            clsb_vals.push_back( getVectorFracAboveValue( sbVals[i], quantiles[k] ) );
        }
        TCanvas *c = newNoWarnTCanvas( Form("q%d",i), Form("q%d",i) );
        double max = *(std::max_element( bVals[i].begin(), bVals[i].end() ) );
        TH1F *hb = new TH1F( Form("hb%d",i), "hbq", 50,0, max );
        TH1F *hsb = new TH1F( Form("hsb%d",i), "hsbq", 50,0, max );
        // fixing the range for teststat plots for private plots (DONT COMMIT THIS UNCOMMENTED)
        // TH1F *hb = new TH1F( Form("hb%d",i), "hbq", 50,0, 5 );
        // TH1F *hsb = new TH1F( Form("hsb%d",i), "hsbq", 50,0, 5 );

        for ( int j=0; j<bVals[i].size(); j++ ) hb->Fill( bVals[i][j] );
        for ( int j=0; j<sbVals[i].size(); j++ ) hsb->Fill( sbVals[i][j] );

        // double dataVal = TMath::ChisquareQuantile( 1.-hCL->GetBinContent(i),1 );
        double dataVal = hChi2min->GetBinContent(i);
        // std::cout << "CLb alternative: " << getVectorFracAboveValue( bVals[i], dataVal) << std::endl;
        TArrow *lD = new TArrow( dataVal, 0.6*hsb->GetMaximum(), dataVal, 0., 0.15, "|>" );

        vector<TLine*> qLs;
        for ( int k=0; k<quantiles.size(); k++ ) {
            qLs.push_back( new TLine( quantiles[k], 0, quantiles[k], 0.8*hsb->GetMaximum() ) );
        }
        TLatex *lat = new TLatex();
        lat->SetTextColor(kRed);
        lat->SetTextSize(0.6*lat->GetTextSize());
        lat->SetTextAlign(22);

        hsb->GetXaxis()->SetTitle("Test Statistic Value");
        hsb->GetYaxis()->SetTitle("Entries");
        hsb->GetXaxis()->SetTitleSize(0.06);
        hsb->GetYaxis()->SetTitleSize(0.06);
        hsb->GetXaxis()->SetLabelSize(0.06);
        hsb->GetYaxis()->SetLabelSize(0.06);
        hsb->SetLineWidth(2);
        hb->SetLineWidth(2);
        hsb->SetFillColor(kBlue);
        hb->SetFillColor(kRed);
        hsb->SetFillStyle(3003);
        hb->SetFillStyle(3004);
        hb->SetLineColor(kRed);
        hsb->SetLineColor(kBlue);

        //TGraph *gb = Utils::smoothHist(hb, 0);
        //TGraph *gsb = Utils::smoothHist(hsb, 1);

        //gb->SetLineColor(kRed+1);
        //gb->SetLineWidth(4);
        //gsb->SetLineColor(kBlue+1);
        //gsb->SetLineWidth(4);

        hsb->Draw();
        hb->Draw("same");
        //gb->Draw("Lsame");
        //gsb->Draw("Lsame");

        qLs[0]->SetLineWidth(2);
        qLs[0]->SetLineStyle(kDashed);
        qLs[4]->SetLineWidth(2);
        qLs[4]->SetLineStyle(kDashed);
        qLs[1]->SetLineWidth(3);
        qLs[3]->SetLineWidth(3);
        qLs[2]->SetLineWidth(5);

        for ( int k=0; k<quantiles.size(); k++ ){
            qLs[k]->SetLineColor(kRed);
            qLs[k]->Draw("same");
        }
        lat->DrawLatex( quantiles[0], hsb->GetMaximum(), "-2#sigma" );
        lat->DrawLatex( quantiles[1], hsb->GetMaximum(), "-1#sigma" );
        lat->DrawLatex( quantiles[2], hsb->GetMaximum(), "<B>" );
        lat->DrawLatex( quantiles[3], hsb->GetMaximum(), "+1#sigma" );
        lat->DrawLatex( quantiles[4], hsb->GetMaximum(), "+2#sigma" );

        lD->SetLineColor(kBlack);
        lD->SetLineWidth(5);
        lD->Draw("same");

        TLegend *leg = new TLegend(0.74,0.54,0.94,0.7);
        leg->SetHeader(Form("p=%4.2g",hCLs->GetBinCenter(i)));
        leg->SetFillColor(0);
        leg->AddEntry(hb,"B-only Toys","LF");
        leg->AddEntry(hsb,"S+B Toys","LF");
        leg->AddEntry(lD,"Data","L");
        leg->Draw("same");
        c->SetLogy();
        c->SetRightMargin(0.11);
        savePlot(c,TString(Form("cls_testStatControlPlot_p%d",i))+"_"+scanVar1);
    }

    TCanvas *c = newNoWarnTCanvas( "cls_ctr", "CLs Control" );
    hCLsFreq->SetLineColor(kBlack);
    hCLsFreq->SetLineWidth(3);
    hCLsExp->SetLineColor(kRed);
    hCLsExp->SetLineWidth(3);

    hCLsErr1Up->SetLineColor(kBlue+2);
    hCLsErr1Up->SetLineWidth(2);
    hCLsErr1Dn->SetLineColor(kBlue+2);
    hCLsErr1Dn->SetLineWidth(2);

    hCLsErr2Up->SetLineColor(kBlue+2);
    hCLsErr2Up->SetLineWidth(2);
    hCLsErr2Up->SetLineStyle(kDashed);
    hCLsErr2Dn->SetLineColor(kBlue+2);
    hCLsErr2Dn->SetLineWidth(2);
    hCLsErr2Dn->SetLineStyle(kDashed);

    hCLsFreq->GetXaxis()->SetTitle("POI");
    hCLsFreq->GetYaxis()->SetTitle("Raw CLs");
    hCLsFreq->GetXaxis()->SetTitleSize(0.06);
    hCLsFreq->GetYaxis()->SetTitleSize(0.06);
    hCLsFreq->GetXaxis()->SetLabelSize(0.06);
    hCLsFreq->GetYaxis()->SetLabelSize(0.06);

    hCLsFreq->Draw("L");
    hCLsErr2Up->Draw("Lsame");
    hCLsErr2Dn->Draw("Lsame");
    hCLsErr1Up->Draw("Lsame");
    hCLsErr1Dn->Draw("Lsame");
    hCLsExp->Draw("Lsame");
    hCLsFreq->Draw("Lsame");

    savePlot(c, "cls_ControlPlot_"+scanVar1);
}

void MethodDatasetsPluginScan::makeControlPlotsBias(map<int, vector<double> > biasVals)
{
    for ( int i=1; i<= hCLs->GetNbinsX(); i++ ) {

        TCanvas *c = newNoWarnTCanvas( Form("q%d",i), Form("q%d",i));
        c->SetRightMargin(0.11);
        double range_max = *(std::max_element( biasVals[i].begin(), biasVals[i].end() ) );
        double range_min = *(std::min_element( biasVals[i].begin(), biasVals[i].end() ) );
        TH1F *hsig = new TH1F( Form("hsig%d",i), "hsig", 50,range_min+(range_max-range_min)*0.001, range_max-(range_max-range_min)*0.001); //this way we loose a little stats in the tails, but make sure the overflow bins are not used.
        for(int j=0;j<biasVals[i].size();j++){
                hsig->Fill(biasVals[i][j]);
        }

        TFitResultPtr fitresult = hsig->Fit("gaus","LSQ","",range_min, range_max);
        hsig->GetXaxis()->SetTitle("POI residual #hat{#alpha} #minus #alpha_{0}");
        hsig->GetYaxis()->SetTitle("Entries");
        hsig->GetXaxis()->SetTitleSize(0.06);
        hsig->GetYaxis()->SetTitleSize(0.06);
        hsig->GetXaxis()->SetLabelSize(0.06);
        hsig->GetYaxis()->SetLabelSize(0.06);
        hsig->SetLineWidth(2);
        hsig->SetFillColor(kBlue);
        hsig->SetFillStyle(3003);
        hsig->SetLineColor(kBlue);

        //TGraph *gsb = Utils::smoothHist(hsig, 1);

        //gsb->SetLineColor(kBlue+1);
        //gsb->SetLineWidth(4);

        hsig->Draw();
        //gb->Draw("Lsame");
        //gsb->Draw("Lsame");

        TLegend *leg = new TLegend(0.65,0.74,0.89,0.95);
        leg->SetHeader(Form("p=%4.2g",hCLs->GetBinCenter(i)));
        leg->SetFillColor(0);
        leg->AddEntry(hsig,"POI residual","LF");
        leg->AddEntry((TObject*)0,Form("#mu=%4.2g +/- %4.2g",fitresult->Parameter(1),fitresult->ParError(1)),"");
        leg->AddEntry((TObject*)0,Form("#sigma=%4.2g +/- %4.2g",fitresult->Parameter(2),fitresult->ParError(2)),"");
        leg->Draw("same");
        savePlot(c,TString(Form("BiasControlPlot_p%d",i))+"_"+scanVar1);
    }
    return;
}
