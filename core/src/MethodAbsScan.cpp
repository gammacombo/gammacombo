/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#include "MethodAbsScan.h"

MethodAbsScan::MethodAbsScan()
    : rndm()
{
    methodName = "Abs";
    drawFilled = true;
};


MethodAbsScan::MethodAbsScan(Combiner *c):
    MethodAbsScan(c->getArg())
// C++11 onwards, one can delegate constructors,
// but then, there can be no other initializers
{
    combiner = c;
    w = c->getWorkspace();
    name = c->getName();
    title = c->getTitle();
    pdfName = "pdf_"+combiner->getPdfName();
    obsName = "obs_"+combiner->getPdfName();
    parsName = "par_"+combiner->getPdfName();
    thName = "th_"+combiner->getPdfName();

    // check workspace content
    if ( !w->pdf(pdfName) ) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << pdfName  << endl; exit(1); }
    if ( !w->set(obsName) ) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << obsName  << endl; exit(1); }
    if ( !w->set(parsName)) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << parsName << endl; exit(1); }
    if ( !w->set(thName)  ) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << thName   << endl; exit(1); }
}


    // constructor without combiner, this is atm still needed for the datasets stuff
    MethodAbsScan::MethodAbsScan(OptParser* opt):
    rndm(),
    methodName("Abs"),
    combiner(nullptr),
    w(nullptr),
    arg(opt),
    scanVar1(opt->var[0]),
    verbose(opt->verbose),
    drawSolution(0),
    nPoints1d(opt->npoints1d),
    nPoints2dx(opt->npoints2dx),
    nPoints2dy(opt->npoints2dy),
    pvalueCorrectorSet(false),
    chi2minGlobal(0.0),
    chi2minBkg(0.0),
    chi2minGlobalFound(false),
    lineStyle(0),
    lineColor(kBlue-8),
    lineWidth(2),
    textColor(kBlack),
    fillStyle(1001),
    fillColor(kBlue-8),
    hCL(0),
    hCLs(0),
    hCLsFreq(0),
    hCLsExp(0),
    hCLsErr1Up(0),
    hCLsErr1Dn(0),
    hCLsErr2Up(0),
    hCLsErr2Dn(0),
    hCL2d(0),
    hCLs2d(0),
    hChi2min(0),
    hChi2min2d(0),
    obsDataset(nullptr),
    startPars(0),
    globalMin(0),
    nWarnings(0),
    drawFilled(true),
    m_xrangeset(false),
    m_yrangeset(false),
    m_initialized(false)
    {
        if ( opt->var.size()>1 ) scanVar2 = opt->var[1];
        if( opt->CL.size()>0 ){
            for ( auto level: opt->CL ) {
                ConfidenceLevels.push_back(level/100.);
            }
        }
        else{
            ConfidenceLevels.push_back(0.6827); // 1sigma
            ConfidenceLevels.push_back(0.9545); // 2sigma
            ConfidenceLevels.push_back(0.9973); // 3sigma
        }
    }

MethodAbsScan::~MethodAbsScan()
{
    for ( int i=0; i<allResults.size(); i++ ){
        if ( allResults[i] ) delete allResults[i];
    }
    if ( hCL ) delete hCL;
    if ( hCLs ) delete hCLs;
    if ( hCLsFreq ) delete hCLsFreq;
    if ( hCLsExp ) delete hCLsExp;
    if ( hCLsErr1Up ) delete hCLsErr1Up;
    if ( hCLsErr1Dn ) delete hCLsErr1Dn;
    if ( hCLsErr2Up ) delete hCLsErr2Up;
    if ( hCLsErr2Dn ) delete hCLsErr2Dn;
    if ( hCLs2d ) delete hCLs2d;
    if ( hCL2d ) delete hCL2d;
    if ( hChi2min ) delete hChi2min;
    if ( hChi2min2d ) delete hChi2min2d;
    if ( obsDataset ) delete obsDataset;
    if ( startPars ) delete startPars;
    if ( globalMin ) delete globalMin;
}

///
/// Try to find global mininum of the PDF.
/// Despite its name this often finds a local minimum. It's merely
/// used as a starting point. When the scans stumbles upon a better
/// minimum, we'll keep that one.
///
/// Resets parameters to the values they had at function call.
/// Save the RooFitResult of the global minimum (or whatever minimum it found...)
/// into the globalMin member.
///
/// \param force If set to true it fits again, even it the fit was already run before.
///
void MethodAbsScan::doInitialFit(bool force)
{
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    if ( arg->debug ){
        cout << "\n============================================================" << endl;
        cout << "MethodAbsScan::doInitialFit() : MAKE FIRST FIT ..." << endl;
        cout << "MethodAbsScan::doInitialFit() : PDF " << pdfName << endl << endl;
    }
    if ( !force && chi2minGlobalFound ){
        if ( arg->debug ){
            cout << "MethodAbsScan::doInitialFit() : Already found previously: chi2minGlobal = " << chi2minGlobal << endl;
            cout << "\n============================================================" << endl;
        }
        return;
    }

    // Save parameter values that were active at function call.
    if ( startPars ) delete startPars;
    startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
    startPars->add(*w->set(parsName));

    // load parameter range
    combiner->loadParameterLimits();

    fixParameters(w, obsName);     ///< fix observables
    floatParameters(w, parsName);  ///< physics parameters need to be floating to find global minimum

    // load again the parameter values that were specified on the command line -
    // loading a set of start parameters might have changed them
    combiner->setParametersConstant();

    // fix parameters we decided to keep constant (to keep a parameter constant
    // add them manually to the workspace set 'const')
    fixParameters(w, "const");

    // check choice of start parameters
    float nsigma = 10.;
    PullPlotter p(this);
    if ( p.hasPullsAboveNsigma(nsigma) ){
        cout << "MethodAbsScan::doInitialFit() : WARNING : Chosen start parameter values result in pulls larger\n"
        "                                WARNING : than " << nsigma << " sigma. Check the values in your\n"
        "                                WARNING : ParametersAbs class!\n"
        "Offending pulls:" << endl;
        p.printPulls(nsigma);
        cout << endl;
    }

    // print init parameters
    if ( arg->debug ){
        cout << "MethodAbsScan::doInitialFit() : init parameters:" << endl;
        w->set(parsName)->Print("v");
        cout << "MethodAbsScan::doInitialFit() : init pulls:" << endl;
        p.printPulls(0.);
        cout << "MethodAbsScan::doInitialFit() : PDF evaluated at init parameters: ";
        cout << w->pdf(pdfName)->getVal() << endl;
        RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*w->pdf(pdfName)));
        cout << "MethodAbsScan::doInitialFit() : Chi2 at init parameters: ";
        cout << ll.getVal() << endl;
    }

    int quiet = arg->debug ? 1 : -1;
    RooFitResult* r = fitToMinBringBackAngles(w->pdf(pdfName), true, quiet);
    // RooFitResult* r = fitToMin(w->pdf(pdfName), true, quiet);
    if ( arg->debug ) r->Print("v");
    // globalMin = new RooSlimFitResult(r);
    globalMin = r;
    chi2minGlobal = globalMin->minNll();
    chi2minGlobalFound = true;

    // reset parameters to their values at function call
    setParameters(w, parsName, startPars->get(0));

    if ( arg->debug ) cout << "============================================================\n" << endl;
    RooMsgService::instance().setGlobalKillBelow(INFO);
}

///
/// Set the global minimum manually.
///
void MethodAbsScan::setChi2minGlobal(double x)
{
    chi2minGlobalFound = true;
    chi2minGlobal = x;
}


void MethodAbsScan::initScan()
{
    if ( arg->debug ) cout << "MethodAbsScan::initScan() : initializing ..." << endl;
    if ( m_initialized ) {
        cout << "MethodAbsScan::initScan() : already initialized." << endl;
        exit(1);
    }

    // Init the 1-CL histograms. Range is taken from the scan range defined in
    // the ParameterAbs class (and derived ones), unless the --scanrange command
    // line argument is set.
    RooRealVar *par1 = w->var(scanVar1);
    if ( !par1 ){
        if ( arg->debug ) cout << "MethodAbsScan::initScan() : ";
        cout << "ERROR : No such scan parameter: " << scanVar1 << endl;
        cout << "        Choose an existing one using: --var par" << endl << endl;
        cout << "  Available parameters:" << endl;
        cout << "  ---------------------" << endl << endl;
        for ( int i=0; i<combiner->getParameterNames().size(); i++ ){
            cout << "    " << combiner->getParameterNames()[i] << endl;
        }
        cout << endl;
        exit(1);
    }
    if ( !m_xrangeset && arg->scanrangeMin != arg->scanrangeMax ){
        setXscanRange(arg->scanrangeMin,arg->scanrangeMax);
    }
    setLimit(w, scanVar1, "scan");
    float min1 = par1->getMin();
    float max1 = par1->getMax();
    hCL = new TH1F("hCL"+getUniqueRootName(), "hCL"+pdfName, nPoints1d, min1, max1);
    if ( hChi2min ) delete hChi2min;
    hChi2min = new TH1F("hChi2min"+getUniqueRootName(), "hChi2min"+pdfName, nPoints1d, min1, max1);
    if (hCLs) delete hCLs;
    hCLs = new TH1F("hCLs" + getUniqueRootName(), "hCLs" + pdfName, nPoints1d, min1, max1);
    if (hCLs) delete hCLsFreq;
    hCLsFreq = new TH1F("hCLsFreq" + getUniqueRootName(), "hCLsFreq" + pdfName, nPoints1d, min1, max1);
    if (hCLsExp) delete hCLsExp;
    hCLsExp = new TH1F("hCLsExp" + getUniqueRootName(), "hCLsExp" + pdfName, nPoints1d, min1, max1);
    if (hCLsErr1Up) delete hCLsErr1Up;
    hCLsErr1Up = new TH1F("hCLsErr1Up" + getUniqueRootName(), "hCLsErr1Up" + pdfName, nPoints1d, min1, max1);
    if (hCLsErr1Dn) delete hCLsErr1Dn;
    hCLsErr1Dn = new TH1F("hCLsErr1Dn" + getUniqueRootName(), "hCLsErr1Dn" + pdfName, nPoints1d, min1, max1);
    if (hCLsErr2Up) delete hCLsErr2Up;
    hCLsErr2Up = new TH1F("hCLsErr2Up" + getUniqueRootName(), "hCLsErr2Up" + pdfName, nPoints1d, min1, max1);
    if (hCLsErr2Dn) delete hCLsErr2Dn;
    hCLsErr2Dn = new TH1F("hCLsErr2Dn" + getUniqueRootName(), "hCLsErr2Dn" + pdfName, nPoints1d, min1, max1);


    // fill the chi2 histogram with very unlikely values such
    // that inside scan1d() the if clauses work correctly
    for ( int i=1; i<=nPoints1d; i++ ) hChi2min->SetBinContent(i,1e6);

    if ( scanVar2!="" )
    {
        RooRealVar *par2 = w->var(scanVar2);
        if ( !par2 ){
            if ( arg->debug ) cout << "MethodAbsScan::initScan() : ";
            cout << "ERROR : No such scan parameter: " << scanVar2 << endl;
            cout << "        Choose an existing one using: --var par" << endl << endl;
            cout << "  Available parameters:" << endl;
            cout << "  ---------------------" << endl << endl;
            for ( int i=0; i<combiner->getParameterNames().size(); i++ ){
                cout << "    " << combiner->getParameterNames()[i] << endl;
            }
            cout << endl;
            exit(1);
        }
        if ( !m_yrangeset && arg->scanrangeyMin != arg->scanrangeyMax ){
            setYscanRange(arg->scanrangeyMin,arg->scanrangeyMax);
        }
        setLimit(w, scanVar2, "scan");
        float min2 = par2->getMin();
        float max2 = par2->getMax();
    if (hCL2d) delete hCL2d;
        hCL2d      = new TH2F("hCL2d"+getUniqueRootName(),      "hCL2d"+pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
    if (hChi2min2d) delete hChi2min2d;
        hChi2min2d = new TH2F("hChi2min2d"+getUniqueRootName(), "hChi2min",      nPoints2dx, min1, max1, nPoints2dy, min2, max2);
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

    // global minimum
    doInitialFit();

    // turn off some messages
    RooMsgService::instance().setStreamStatus(0,kFALSE);
    RooMsgService::instance().setStreamStatus(1,kFALSE);
    m_initialized = true;
}

///
/// Save this scanner to a root file placed into plots/scanner.
/// It contains the 1-CL histograms and the solutions.
///
void MethodAbsScan::saveScanner(TString fName)
{
    if ( fName=="" ){
        FileNameBuilder fb(arg);
        fName = fb.getFileNameScanner(this);
    }
    if ( arg->debug ) cout << "MethodAbsScan::saveScanner() : saving scanner: " << fName << endl;
        TFile f(fName, "recreate");
        // save 1-CL histograms
        if ( scanVar2!="" ) {
        hCL2d->Write("hCL");
        if (hCLs2d) hCLs2d->Write("hCLs");
    }
    else {
        hCL->Write("hCL");
        if (hCLs) hCLs->Write("hCLs");
        if (hCLsFreq) hCLsFreq->Write("hCLsFreq");
        if (hCLsExp) hCLsExp->Write("hCLsExp");
        if (hCLsErr1Up) hCLsErr1Up->Write("hCLsErr1Up");
        if (hCLsErr1Dn) hCLsErr1Dn->Write("hCLsErr1Dn");
        if (hCLsErr2Up) hCLsErr2Up->Write("hCLsErr2Up");
        if (hCLsErr2Dn) hCLsErr2Dn->Write("hCLsErr2Dn");
    }
    // save chi2 histograms
    if ( scanVar2!="" ) hChi2min2d->Write("hChi2min");
    else hChi2min->Write("hChi2min");
    // save solutions
    for ( int i=0; i<solutions.size(); i++ ){
        f.WriteObject(solutions[i], Form("sol%i",i));
    }
}

///
/// Save a scanner from plots/scanner.
/// It contains the 1-CL histograms and the solutions.
///
bool MethodAbsScan::loadScanner(TString fName)
{
    if ( fName=="" ){
        FileNameBuilder fb(arg);
        fName = fb.getFileNameScanner(this);
    }
    if ( arg->debug ) cout << "MethodAbsScan::loadScanner() : ";
    cout << "loading scanner: " << fName << endl;
    if ( !FileExists(fName) ){
        cout << "MethodAbsScan::loadScanner() : ERROR : file not found: " << fName << endl;
        cout << "                               Run first without the '-a plot' option to produce the missing file." << endl;
        exit(1);
    }
    TFile *f = new TFile(fName, "ro"); // don't delete this later else the objects die
    // load 1-CL histograms
    TObject *obj = f->Get("hCL");
    if ( obj==0 ){
        cout << "MethodAbsScan::loadScanner() : ERROR : 'hCL' not found in root file " << fName << endl;
        exit(1);
    }
    if ( scanVar2!="" ) {
        hCL2d = (TH2F*)obj;
        hCL2d->SetName("hCL2d"+getUniqueRootName());
    }
    else {
        hCL = (TH1F*)obj;
        hCL->SetName("hCL"+getUniqueRootName());
    }
    // load chi2 histograms
    obj = f->Get("hChi2min");
    if ( obj==0 ){
        cout << "MethodAbsScan::loadScanner() : ERROR : 'hChi2min' not found in root file " << fName << endl;
        // exit(1);
        // return false;
    }
    if ( scanVar2!="" ) {
        hChi2min2d = (TH2F*)obj;
        hChi2min2d->SetName("hChi2min2d"+getUniqueRootName());
    }
    else {
        hChi2min = (TH1F*)obj;
        hChi2min->SetName("hChi2min"+getUniqueRootName());
    }
    // load CLs histograms
    if ( std::find( arg->cls.begin(), arg->cls.end(), 1 ) != arg->cls.end() ) {
        obj = f->Get("hCLs");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLs' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        if ( scanVar2!="" ) {
            hCLs2d = (TH2F*)obj;
            hCLs2d->SetName("hCLs2d"+getUniqueRootName());
        }
        else {
            hCLs = (TH1F*)obj;
            hCLs->SetName("hCLs"+getUniqueRootName());
        }
    }
    // load CLs histograms
    bool lookForMixedCLs = std::find( arg->cls.begin(), arg->cls.end(), 2 ) != arg->cls.end() && !methodName.Contains("Prob");
    if ( lookForMixedCLs ) {
        obj = f->Get("hCLsFreq");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsFreq' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsFreq = (TH1F*)obj;
            hCLsFreq->SetName("hCLsFreq"+getUniqueRootName());
        }
        obj = f->Get("hCLsExp");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsExp' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsExp = (TH1F*)obj;
            hCLsExp->SetName("hCLsExp"+getUniqueRootName());
        }
        obj = f->Get("hCLsErr1Up");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr1Up' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsErr1Up = (TH1F*)obj;
            hCLsErr1Up->SetName("hCLsErr1Up"+getUniqueRootName());
        }
        obj = f->Get("hCLsErr1Dn");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr1Dn' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsErr1Dn = (TH1F*)obj;
            hCLsErr1Dn->SetName("hCLsErr1Dn"+getUniqueRootName());
        }
        obj = f->Get("hCLsErr2Up");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr2Up' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsErr2Up = (TH1F*)obj;
            hCLsErr2Up->SetName("hCLsErr2Up"+getUniqueRootName());
        }
        obj = f->Get("hCLsErr2Dn");
        if ( obj==0 ){
            cout << "MethodAbsScan::loadScanner() : WARNING : 'hCLsErr2Dn' not found in root file - you can ignore this if you're not running in dataset mode " << fName << endl;
        }
        else if ( scanVar2=="" ){
            hCLsErr2Dn = (TH1F*)obj;
            hCLsErr2Dn->SetName("hCLsErr2Dn"+getUniqueRootName());
        }
    }
    // load solutions: try the first one hundred
    solutions.clear();
    int nSol = 100;
    for ( int i=0; i<nSol; i++ ){
        RooSlimFitResult *r = (RooSlimFitResult*)f->Get(Form("sol%i",i));
        if ( !r ) break;
        solutions.push_back(r);
    }
    if ( f->Get(Form("sol%i",nSol)) ){
        cout << "MethodAbsScan::loadScanner() : WARNING : Only the first 100 solutions read from: " << fName << endl;
    }

    return true;
}


int MethodAbsScan::scan1d()
{
    cout << "MethodAbsScan::scan1d() : not implemented." << endl;
    return 0;
}


int MethodAbsScan::scan2d()
{
    cout << "MethodAbsScan::scan2d() : not implemented." << endl;
    return 0;
}

///
/// Find an interpolated x value near a certain bin position of a histogram that is the
/// best estimate for h(x)=y. Interpolates by means of a straight line between two
/// known points.
/// \param h the histogram to be interpolated
/// \param i interpolate around this bin. Must be a bin such that i and i+1 are above and below val
/// \param y the y position we want to find the interpolated x for
/// \param val Return value: interpolated x position
///
void MethodAbsScan::interpolateSimple(TH1F* h, int i, float y, float &val)
{
    // std::cout << "MethodAbsScan::interpolateSimple(): i=" << i << " y=" << y << std::endl;
    if ( !( 1 <= i && i <= h->GetNbinsX()-1 ) ) return;
    float p1x = h->GetBinCenter(i);
    float p1y = h->GetBinContent(i);
    float p2x = h->GetBinCenter(i+1);
    float p2y = h->GetBinContent(i+1);
    val = p2x + (y-p2y)/(p1y-p2y)*(p1x-p2x);
}

///
/// Solve a quadratic equation by means of a modified pq formula:
/// @f[x^2 + \frac{p_1}{p_2} x + \frac{p_0-y}{p2} = 0@f]
///
float MethodAbsScan::pq(float p0, float p1, float p2, float y, int whichSol)
{
    if ( whichSol == 0 ) return -p1/2./p2 + sqrt( sq(p1/2./p2) - (p0-y)/p2 );
    else                 return -p1/2./p2 - sqrt( sq(p1/2./p2) - (p0-y)/p2 );
}

///
/// Find an interpolated x value near a certain bin position of a histogram that is the
/// best estimate for h(x)=y. Interpolates by means of fitting a second grade polynomial
/// to up to five adjacent points. Because that's giving us two solutions, we use the central
/// value and knowledge about if it is supposed to be an upper or lower boundary to pick
/// one.
/// \param h - the histogram to be interpolated
/// \param i - interpolate around this bin. Must be a bin such that i and i+1 are above and below val
/// \param y - the y position we want to find the interpolated x for
/// \param central - Central value of the solution we're trying to get the CL interval for.
/// \param upper - Set to true if we're computing an upper interval boundary.
/// \param val - Return value: interpolated x position
/// \param err - Return value: estimated interpolation error
/// \return true, if inpterpolation was performed, false, if conditions were not met
///
bool MethodAbsScan::interpolate(TH1F* h, int i, float y, float central, bool upper, float &val, float &err)
{
    // cout << "MethodAbsScan::interpolate(): i=" << i << " y=" << y << " central=" << central << endl;
    if ( i > h->GetNbinsX()-2 ) return false;
    if ( i < 3 ) return false;

    // if method Prob, don't interpolate (no proper error estimate)
    if(methodName.Contains("Prob")){
        for (int k=0;k<h->GetNbinsX();k++){
            h->SetBinError(k+1,0.);
        }
    }

    // compute pol2 fit interpolation
    TGraphErrors *g = new TGraphErrors(3);
    g->SetPoint(0, h->GetBinCenter(i-1), h->GetBinContent(i-1));
    g->SetPointError(0, h->GetBinWidth(i-1)/2., h->GetBinError(i-1));
    g->SetPoint(1, h->GetBinCenter(i), h->GetBinContent(i));
    g->SetPointError(1, h->GetBinWidth(i)/2., h->GetBinError(i));
    g->SetPoint(2, h->GetBinCenter(i+1), h->GetBinContent(i+1));
    g->SetPointError(2, h->GetBinWidth(i+1)/2., h->GetBinError(i+1));

    // see if we can add a 4th and 5th point
    if ( (h->GetBinContent(i-2) - h->GetBinError(i-2) < h->GetBinContent(i-1) + h->GetBinError(i-1) && h->GetBinContent(i-1) < h->GetBinContent(i))
            || (h->GetBinContent(i-2) + h->GetBinError(i-2) > h->GetBinContent(i-1) - h->GetBinError(i-1) && h->GetBinContent(i-1) > h->GetBinContent(i)) )
    {
        if((upper && h->FindBin(central)<i-2)||!upper) // don't use for upper limit calculation if point is equal or below central value
        {
            // add to the beginning
            TGraphErrors *gNew = new TGraphErrors(g->GetN()+1);
            gNew->SetPoint(0, h->GetBinCenter(i-2), h->GetBinContent(i-2));
            gNew->SetPointError(0, h->GetBinWidth(i-2)/2., h->GetBinError(i-2));
            Double_t pointx, pointy;
            Double_t pointxerr, pointyerr;
            for ( int i=0; i<g->GetN(); i++)
            {
                g->GetPoint(i, pointx, pointy);
                pointxerr = g->GetErrorX(i);
                pointyerr = g->GetErrorY(i);
                gNew->SetPoint(i+1, pointx, pointy);
                gNew->SetPointError(i+1, pointxerr, pointyerr);
            }
            delete g;
            g = gNew;
        }
    }

    if ( (h->GetBinContent(i+2) - h->GetBinError(i+2) < h->GetBinContent(i+1) + h->GetBinError(i+1)&& h->GetBinContent(i+1) < h->GetBinContent(i))
            || (h->GetBinContent(i+2) + h->GetBinError(i+2)> h->GetBinContent(i+1) - h->GetBinError(i+1)&& h->GetBinContent(i+1) > h->GetBinContent(i)) )
    {
        if((!upper && h->FindBin(central)>i+2)|| upper) // don't use for lower limit calculation if point is equal or above central value
        {
            // add to the end
            g->Set(g->GetN()+1);
            g->SetPoint(g->GetN()-1, h->GetBinCenter(i+2), h->GetBinContent(i+2));
            g->SetPointError(g->GetN()-1, h->GetBinWidth(i+2)/2., h->GetBinError(i+2));
        }
    }


    TF1 *f1 = new TF1("f1", "[0]+[1]*(x-[2])", h->GetBinCenter(i-2), h->GetBinCenter(i+2));
    TF1 *f2 = new TF1("f2", "[0]+[1]*(x-[3])+[2]*(x-[3])**2", h->GetBinCenter(i-2), h->GetBinCenter(i+2));
    f1->FixParameter(2,h->GetBinCenter(i));
    f2->FixParameter(3,h->GetBinCenter(i));

    f1->SetParameter(1,(h->GetBinContent(i+1)-h->GetBinContent(i))/h->GetBinWidth(i));
    g->Fit("f1", "q");    // fit linear to get decent start parameters
    f2->SetParameter(0,f1->GetParameter(0));
    f2->SetParameter(1,f1->GetParameter(1));
    g->Fit("f2", "qf+");  // refit with minuit to get more correct errors (TGraph fit errors bug)
    double p[3], e[3];
    // for ( int ii=0; ii<3; ii++ )
    // {
    //  p[ii] = f2->GetParameter(ii);
    //  e[ii] = f2->GetParError(ii);
    // }
    p[0]= f2->GetParameter(2)*(f2->GetParameter(3)*f2->GetParameter(3))-f2->GetParameter(1)*f2->GetParameter(3)+f2->GetParameter(0);
    p[1]= f2->GetParameter(1)-2*f2->GetParameter(2)*f2->GetParameter(3);
    p[2]= f2->GetParameter(2);

    double sol0 = pq(p[0], p[1], p[2], y, 0);
    double sol1 = pq(p[0], p[1], p[2], y, 1);
    // cout << upper << " ";
    // printf("%f %f %f\n", central, sol0, sol1);

    // std::cout << central << "\t" << sol0 << "\t" <<sol1 << std::endl;

    // debug: show fitted 1-CL histogram
    if ( arg->controlplot)
    {
      TString debugTitle = methodName + Form(" y=%.2f ",y);
      debugTitle += upper?Form("%f upper",central):Form("%f lower",central);
      TCanvas *c = newNoWarnTCanvas(getUniqueRootName(), debugTitle);
      g->SetMarkerStyle(3);
      g->SetHistogram(h);
      h->Draw();
      g->Draw("p");
      f2->Draw("SAME");
      savePlot(c,TString(name+"_"+scanVar1+"_boundary_interpolation_"+methodName+"_"+TString(h->GetName())+"_"+std::to_string(y)));
    }



    if((h->GetBinCenter(i-2) > sol0 || sol0 > h->GetBinCenter(i+2)) && (h->GetBinCenter(i-2) > sol1 || sol1 > h->GetBinCenter(i+2)))
    {
        if(arg->verbose || arg->debug){
            cout << "MethodAbsScan::interpolate(): Quadratic interpolation out of bounds [" << h->GetBinCenter(i-2) <<", " << h->GetBinCenter(i+2) << "]:"<< std::endl;
            std::cout << "Solutions are "<< central << "(free fit result)\t" << sol0 << "(bound solution 0) \t" <<sol1 << "(bound solution 1)." << std::endl;
        }
        return false;
    }
    else if(sol0!=sol0 || sol1!=sol1){
        if(arg->verbose || arg->debug){
            cout << "MethodAbsScan::interpolate(): Quadratic interpolation leads to NaN:"<< std::endl;
            std::cout << "Solutions are "<< central << "(free fit result)\t" << sol0 << "(bound solution 0) \t" <<sol1 << "(bound solution 1)." << std::endl;
        }
        return false;
    }


    int useSol = 0;

    if ( (sol0<central && sol1>central) || (sol1<central && sol0>central) )
    {
        if ( upper )
        {
            if ( sol0<sol1 ) useSol = 1;
            else             useSol = 0;
        }
        else
        {
            if ( sol0<sol1 ) useSol = 0;
            else             useSol = 1;
        }
    }
    else
    {
        if ( fabs(h->GetBinCenter(i)-sol0) < fabs(h->GetBinCenter(i)-sol1) ) useSol = 0;
        else useSol = 1;
    }

    if ( useSol==0 ) val = sol0;
    else             val = sol1;

    // try error propagation: sth is wrong in the formulae
    // float err0 = TMath::Max(sq(val-pq(p[0]+e[0], p[1], p[2], y, useSol)), sq(val-pq(p[0]-e[0], p[1], p[2], y, useSol)));
    // float err1 = TMath::Max(sq(val-pq(p[0], p[1]+e[1], p[2], y, useSol)), sq(val-pq(p[0], p[1]-e[1], p[2], y, useSol)));
    // float err2 = TMath::Max(sq(val-pq(p[0], p[1], p[2]+e[2], y, useSol)), sq(val-pq(p[0], p[1], p[2]-e[2], y, useSol)));
    // err = sqrt(err0+err1+err2);
    // printf("%f %f %f\n", val, pq(p[0]+e[0], p[1], p[2], y, useSol), pq(p[0]-e[0], p[1], p[2], y, useSol));
    // printf("%f %f %f\n", val, pq(p[0], p[1]+e[1], p[2], y, useSol), pq(p[0], p[1]-e[1], p[2], y, useSol));
    // printf("%f %f %f\n", val, pq(p[0], p[1], p[2]+e[2], y, useSol), pq(p[0], p[1], p[2]-e[2], y, useSol));
    err = 0.0;
    return true;
}

///
/// Calculate the CL intervals from the CL curve. Start from
/// known local minima and scan upwards and downwards to find the interval
/// boundaries. Then scan again from the boundaries of the scan range to
/// cover the case where an CL interval is not closed yet at the boundary.
/// Use a fit-based interpolation (interpolate()) if we have more than 25 bins,
/// else revert to a straight line interpolation (interpolateSimple()).
///
void MethodAbsScan::calcCLintervals(int CLsType, bool calc_expected, bool quiet)
{
    TH1F *histogramCL = this->getHCL();
    // calc CL intervals with CLs method
    if (CLsType==1 && this->getHCLs())
    {
        histogramCL =this->getHCLs();
    }
    else if (CLsType==2 && this->getHCLsFreq())
    {
        histogramCL = this->getHCLsFreq();
    }
    if (CLsType==2 && calc_expected && hCLsExp)
    {
        histogramCL = hCLsExp;
        std::cout << "Determine expected upper limit:" << std::endl;
    }

    if(CLsType!=0){
        std::cout<< "Confidence Intervals for CLs method "<< CLsType << ":" << std::endl;
    }
    if ( arg->isQuickhack(8) ){
        // \todo Switch to the new CLIntervalMaker mechanism. It can be activated
        // already using --qh 8, but it really is in beta stage still
        // \todo add user specific CL interval
        cout << "\nMethodAbsScan::calcCLintervals() : USING NEW CLIntervalMaker for " << name << endl << endl;
        CLIntervalMaker clm(arg, *histogramCL);
        clm.findMaxima(0.04); // ignore maxima under pvalue=0.04
        for ( int iSol=0; iSol<solutions.size(); iSol++ ){
            float sol = getScanVar1Solution(iSol);
            clm.provideMorePreciseMaximum(sol, "max PLH");
        }
        clm.calcCLintervals();
        // print
        TString unit = w->var(scanVar1)->getUnit();
        CLIntervalPrinter clp(arg, name, scanVar1, unit, methodName);
        if(calc_expected){
            clp = CLIntervalPrinter(arg, name, scanVar1, unit, methodName+TString("_expected_standardCLs"));
        }
        clp.setDegrees(isAngle(w->var(scanVar1)));
        clp.addIntervals(clm.getClintervals1sigma());
        clp.addIntervals(clm.getClintervals2sigma());
        clp.print();
        cout << endl;
    }

    if(solutions.empty()){
     cout << "MethodAbsScan::calcCLintervals() : Solutions vector empty. "
          << "Using simple method with  linear splines."<<endl;
        this->calcCLintervalsSimple(CLsType, calc_expected);
        return;
    }
    else if((CLsType==1||CLsType==2) && !this->getHCLs()) {
        cout<<"Using simple method with  linear splines."<<endl;
        this->calcCLintervalsSimple(CLsType, calc_expected);
    }

    if ( arg->debug ) cout << "MethodAbsScan::calcCLintervals() : ";
    if (!quiet) cout << "CONFIDENCE INTERVALS for combination " << name << endl << endl;

    clintervals1sigma.clear(); // clear, else calling this function twice doesn't work
    clintervals2sigma.clear();
    clintervals3sigma.clear();
    clintervalsuser.clear();
    int n = histogramCL->GetNbinsX();
    RooRealVar* par = w->var(scanVar1);

    for ( int iSol=0; iSol<solutions.size(); iSol++ )
    {
        const int NumOfCL = ConfidenceLevels.size();
        std::vector<float> CLhi(NumOfCL, 0.0);
        std::vector<float> CLhiErr(NumOfCL, 0.0);
        std::vector<float> CLlo(NumOfCL, 0.0);
        std::vector<float> CLloErr(NumOfCL, 0.0);

        for ( int c=0; c<NumOfCL; c++ )
        {
            CLlo[c] = histogramCL->GetXaxis()->GetXmin();
            CLhi[c] = histogramCL->GetXaxis()->GetXmax();
            float y = 1.- ConfidenceLevels[c];
            float sol = getScanVar1Solution(iSol);
            int sBin = histogramCL->FindBin(sol);
            if (arg->debug) std::cout << "solution bin: " << sBin << std::endl;
            if(histogramCL->IsBinOverflow(sBin)||histogramCL->IsBinUnderflow(sBin)){
                std::cout << "MethodAbsScan::calcCLintervals(): WARNING: no solution in scanrange found, will use lowest bin!" << std::endl;
                sBin=1;
            }

            // find lower interval bound
            for ( int i=sBin; i>0; i-- ){
                if ( histogramCL->GetBinContent(i) < y ){
                    if ( n>25 ){
                        bool check = interpolate(histogramCL, i, y, sol, false, CLlo[c], CLloErr[c]);
                        if(!check && (arg->verbose ||arg->debug)) cout << "MethodAbsScan::calcCLintervals(): Using linear interpolation." << endl;
                        if ( !check || CLlo[c]!=CLlo[c] ) interpolateSimple(histogramCL, i, y, CLlo[c]);
                    }
                    else{
                        cout << "MethodAbsScan::calcCLintervals(): Low number of scan points. Using linear interpolation." << endl;
                        interpolateSimple(histogramCL, i, y, CLlo[c]);
                    }
                    break;
                }
            }

            // find upper interval bound
            for ( int i=sBin; i<n; i++ ){
                if ( histogramCL->GetBinContent(i) < y ){
                    if ( n>25 ){
                        bool check = interpolate(histogramCL, i-1, y, sol, true, CLhi[c], CLhiErr[c]);
                        if(!check && (arg->verbose ||arg->debug)) cout << "MethodAbsScan::calcCLintervals(): Using linear interpolation." << endl;
                        if (!check || CLhi[c]!=CLhi[c] ) interpolateSimple(histogramCL, i-1, y, CLhi[c]);
                    }
                    else{
                        cout << "MethodAbsScan::calcCLintervals(): Low number of scan points. Using linear interpolation." << endl;
                        interpolateSimple(histogramCL, i-1, y, CLhi[c]);
                    }
                    break;
                }
            }

            // save interval if solution is contained in it
            // /todo save clintervals properly dynamically
            if ( histogramCL->GetBinContent(sBin)>y )
            {
                CLInterval cli;
                cli.pvalue = 1.-ConfidenceLevels[c];
                cli.min = CLlo[c];
                cli.max = CLhi[c];
                cli.central = sol;
                if ( c==0 ) clintervals1sigma.push_back(cli);
                if ( c==1 ) clintervals2sigma.push_back(cli);
                if ( c==2 ) clintervals3sigma.push_back(cli);
                if ( c==3 ) clintervalsuser.push_back(cli);
            }
        }
    }

    // compute largest 1sigma interval
    if ( arg->largest ){
        int size = clintervals1sigma.size();
        for ( int k=0; k<size; k++ ){
            CLInterval i;
            i.central = clintervals1sigma[k].central;
            i.pvalue = clintervals1sigma[k].pvalue;
            i.minmethod = "largest";
            i.maxmethod = "largest";
            i.min = clintervals1sigma[0].min;
            for ( int j=0; j<clintervals1sigma.size(); j++ ) i.min = TMath::Min(i.min, clintervals1sigma[j].min);
            i.max = clintervals1sigma[0].max;
            for ( int j=0; j<clintervals1sigma.size(); j++ ) i.max = TMath::Max(i.max, clintervals1sigma[j].max);
            clintervals1sigma.push_back(i);
        }
    }
    if (!quiet) printCLintervals(CLsType, calc_expected);

    //
    // scan again from the histogram boundaries
    //
    // \todo: something is buggy here
    for ( int iBoundary=0; iBoundary<2; iBoundary++ )
    {
        const int NumOfCL = ConfidenceLevels.size();
        std::vector<float> CLhi(NumOfCL, 0.0);
        std::vector<float> CLhiErr(NumOfCL, 0.0);
        std::vector<float> CLlo(NumOfCL, 0.0);
        std::vector<float> CLloErr(NumOfCL, 0.0);

        for ( int c=0; c<NumOfCL; c++ )
        {
            CLlo[c] = histogramCL->GetXaxis()->GetXmin();
            CLhi[c] = histogramCL->GetXaxis()->GetXmax();
            float y = 1.-ConfidenceLevels[c];

            if ( iBoundary==1 )
            {
                // find lower interval bound
                if ( histogramCL->GetBinContent(n)<y ) continue;  ///< skip if p-value is too low at boundary
                for ( int i=n; i>0; i-- )
                {
                    if ( histogramCL->GetBinContent(i) > y )
                    {
                        if ( n>25 ) interpolate(histogramCL, i, y, histogramCL->GetXaxis()->GetXmax(), false, CLlo[c], CLloErr[c]);
                        else        interpolateSimple(histogramCL, i, y, CLlo[c]);
                        break;
                    }
                }
            }
            else
            {
                // find upper interval bound
                if ( histogramCL->GetBinContent(1)<y ) continue;  ///< skip if p-value is too low at boundary
                for ( int i=1; i<n; i++ )
                {
                    if ( histogramCL->GetBinContent(i) > y )
                    {
                        if ( n>25 ) interpolate(histogramCL, i-1, y, histogramCL->GetXaxis()->GetXmin(), true, CLhi[c], CLhiErr[c]);
                        else        interpolateSimple(histogramCL, i-1, y, CLhi[c]);
                        break;
                    }
                }
            }

            // convert to degrees if necessary
            TString unit = par->getUnit();
            if ( isAngle(par) ){
                CLlo[c] = RadToDeg(CLlo[c]);
                CLloErr[c] = RadToDeg(CLloErr[c]);
                CLhi[c] = RadToDeg(CLhi[c]);
                CLhiErr[c] = RadToDeg(CLhiErr[c]);
                unit = "Deg";
            }

            int pErr = 2;
            if ( arg && arg->digits>0 ) pErr = arg->digits;
            if (CLsType==1 && this->getHCLs()) cout << "Simplified CL_s: ";
            if (CLsType==2 && this->getHCLsFreq()) cout << "Standard CL_s: ";
            printf("\n%s = [%7.*f, %7.*f] @%3.2fCL",
                    par->GetName(),
                    pErr, CLlo[c], pErr, CLhi[c],
                    ConfidenceLevels[c]);
            if ( CLloErr[c]!=0 || CLhiErr[c]!=0 ) printf(", accuracy = [%1.5f, %1.5f]", CLloErr[c], CLhiErr[c]);
            if ( unit!="" ) cout << ", ["<<unit<<"]";
            cout << ", " << methodName << " (boundary scan)" << endl;
        }
    }

    cout << endl;

    // Print fit chi2 etc. (not done for datasets running)
    if ( !combiner ) return;
    if ( !combiner->isCombined() ) return;
    double chi2 = this->getSolution(0)->minNll();
    int nObs = combiner->getObservables()->getSize();
    int nPar = combiner->getParameters()->getSize();
    double prob = TMath::Prob(chi2,nObs-nPar);
    cout << "Fit quality: chi2/(nObs-nPar) = " << Form("%.2f",chi2) << "/(" << nObs << "-" << nPar << "), P = " << Form("%4.1f%%",prob*100.) << endl; cout << endl;
}

///
/// Print CL intervals.
///
void MethodAbsScan::printCLintervals(int CLsType, bool calc_expected)
{
    TString unit = w->var(scanVar1)->getUnit();
    CLIntervalPrinter clp(arg, name, scanVar1, unit, methodName, CLsType);
    if(calc_expected){
        clp = CLIntervalPrinter(arg, name, scanVar1, unit, methodName+TString("_expected_standardCLs"));
    }
    clp.setDegrees(isAngle(w->var(scanVar1)));
    clp.addIntervals(clintervals1sigma);
    clp.addIntervals(clintervals2sigma);
    clp.addIntervals(clintervals3sigma);
    clp.addIntervals(clintervalsuser);
    clp.print();
    clp.savePython();
    cout << endl;

    // print solutions not contained in the 1sigma and 2sigma intervals
    for ( int i=0; i<solutions.size(); i++ )
    {
        float sol = getScanVar1Solution(i);
        bool cont=false;
        for ( int j=0; j<clintervals1sigma.size(); j++ ) if ( clintervals1sigma[j].min<sol && sol<clintervals1sigma[j].max ) cont=true;
        for ( int j=0; j<clintervals2sigma.size(); j++ ) if ( clintervals2sigma[j].min<sol && sol<clintervals2sigma[j].max ) cont=true;
        for ( int j=0; j<clintervals3sigma.size(); j++ ) if ( clintervals3sigma[j].min<sol && sol<clintervals3sigma[j].max ) cont=true;
        for ( int j=0; j<clintervalsuser.size(); j++ ) if ( clintervalsuser[j].min<sol && sol<clintervalsuser[j].max ) cont=true;
        if ( cont==true ) continue;
        if ( w->var(scanVar1)->getUnit()==TString("Rad") ) sol = RadToDeg(sol);
        int d = arg->digits;
        if ( d<=0 ) d = 3;
        printf("%s = %7.*f", w->var(scanVar1)->GetName(), d, sol);
        if ( unit!="" ) cout << " ["<<unit<<"]";
        cout << endl;
    }
}
///
/// Get the CL interval that includes the best-fit value.
/// \param sigma 1,2
///
CLInterval MethodAbsScan::getCLintervalCentral(int sigma, bool quiet)
{
    return getCLinterval(0,sigma,quiet);
}

///
/// Get the CL interval that includes the best-fit value.
/// \param sigma 1,2
///
CLInterval MethodAbsScan::getCLinterval(int iSol, int sigma, bool quiet)
{
    if ( clintervals1sigma.size()==0 ) calcCLintervals(0,false,quiet);
    if ( clintervals1sigma.size()==0 ){
        // no constraint at 1sigma, return full range.
        assert(hCL);
        CLInterval i;
        i.pvalue  = 0.;
        i.central = -1e10;
        i.min     = hCL->GetXaxis()->GetXmin();
        i.max     = hCL->GetXaxis()->GetXmax();
        return i;
    }

    vector<CLInterval> intervals;
    if ( sigma==1 ) intervals = clintervals1sigma;
    else if ( sigma==2 ) intervals = clintervals2sigma;
    else if ( sigma==3) intervals = clintervals3sigma;
    else{
        cout << "MethodAbsScan::getCLintervalCentral() : ERROR : no such CL intervals! sigma=" << sigma << endl;
        exit(1);
    }

    if ( iSol >= intervals.size() ) {
        cout << "MethodAbsScan::getCLinterval() : ERROR : no solution with id " << iSol << endl;
        exit(1);
    }

    // compute largest interval
    if ( arg->largest ){
        CLInterval i;
        i.pvalue = intervals[iSol].pvalue;
        i.min = intervals[iSol].min;
        for ( int j=0; j<intervals.size(); j++ ) i.min = TMath::Min(i.min, intervals[j].min);
        i.max = intervals[iSol].max;
        for ( int j=0; j<intervals.size(); j++ ) i.max = TMath::Max(i.max, intervals[j].max);
        return i;
    }

    return intervals[iSol];
}


float MethodAbsScan::getCL(double val)
{
    return 1.-hCL->Interpolate(val);
}


void MethodAbsScan::plotOn(OneMinusClPlotAbs *plot, int CLsType)
{
    plot->addScanner(this, CLsType);
}


RooRealVar* MethodAbsScan::getScanVar1()
{
    return w->var(scanVar1);
}

TString MethodAbsScan::getScanVar1Name()
{
    return scanVar1;
}

RooRealVar* MethodAbsScan::getScanVar2()
{
    return w->var(scanVar2);
}

TString MethodAbsScan::getScanVar2Name()
{
    return scanVar2;
}


void MethodAbsScan::print()
{
    cout << "MethodAbsScan::print() : Method: " << methodName;
    cout << ", Scanner: " << name << endl;
    w->set(parsName)->Print("v");
}

///
/// Make a 1d plot of the NLL in var
///
void MethodAbsScan::plot1d(TString var)
{
    cout << "MethodAbsScan::plot1d() : Method: " << methodName;
    cout << ", Scanner: " << name << endl;

    //   RooRealVar* vx = w->var(var);
    //   assert(vx);
    // setLimit(w, var, "plot");
    //
    //   // cout << "MethodAbsScan::plot1d() : loading global minimum ..." << endl;
    //   // if ( !globalMin ){ cout << "MethodAbsScan::plot1d() : no global minimum. Call doInitialFit() first!" << endl; exit(1); }
    //   // setParameters(w, parsName, globalMinP);
    //   // print();
    //
    //   RooNLLVar nll("nll", "nll", *(w->pdf(pdfName)), *(w->data(dataName))) ;
    //
    //   TString plotName = "plot1d_"+name+"_"+var;
    //   TCanvas *c1 = newNoWarnTCanvas();
    //   RooPlot *frame = vx->frame();
    //   // w->pdf(pdfName)->plotOn(frame);
    //   nll.plotOn(frame);
    //   frame->Draw();
    //
    //   savePlot(c1, plotName);
}

///
/// Make a 2d plot of the PDF in varx and vary.
///
void MethodAbsScan::plot2d(TString varx, TString vary)
{
    cout << "MethodAbsScan::plot2d() : Method: " << methodName;
    cout << ", scanner: " << name << endl;

    RooRealVar* vx = w->var(varx);
    RooRealVar* vy = w->var(vary);
    assert(vx);
    assert(vy);
    setLimit(w, varx, "plot");
    setLimit(w, vary, "plot");

    cout << "MethodAbsScan::plot2d() : loading global minimum ..." << endl;
    if ( !globalMin ){ cout << "MethodAbsScan::plot2d() : no global minimum. Call doInitialFit() first!" << endl; exit(1); }

    setParameters(w, parsName, globalMin);
    print();

    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadRightMargin(0.15);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadLeftMargin(0.14);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPalette(1);

    TString plotName = "plot2d_"+name+"_"+varx+"_"+vary;
    TCanvas *c1 = newNoWarnTCanvas(plotName, plotName);
    TH1* h = w->pdf(pdfName)->createHistogram(plotName, *vx, YVar(*vy));
    h->Draw("colz");

    savePlot(c1, plotName + arg->plotext );
}

///
/// Load the values at a specific minimum into
/// the workspace. This way we can use it for
/// goodness of fit, start points, etc.
/// \param i Index of the solution, i=0 corresponds to the best one.
///
bool MethodAbsScan::loadSolution(int i)
{
    if ( arg->debug ) cout << "MethodAbsScan::loadSolution() : loading solution " << i << endl;
    if ( i<0 || i >= solutions.size() ){
        cout << "MethodAbsScan::loadSolution() : ERROR : solution ID out of range." << endl;
        return false;
    }
    RooArgSet *tmp = new RooArgSet();
    tmp->add(solutions[i]->floatParsFinal());
    tmp->add(solutions[i]->constPars());
    setParameters(w, parsName, tmp);
    delete tmp;
    return true;
}

///
/// Load the values given by an (external) fit result.
///
void MethodAbsScan::loadParameters(RooSlimFitResult *r)
{
    if ( arg->debug ) cout << "MethodAbsScan::loadParameters() : loading a RooSlimFitResult " << endl;
    RooArgSet *tmp = new RooArgSet();
    tmp->add(r->floatParsFinal());
    tmp->add(r->constPars());
    setParameters(w, parsName, tmp);
    delete tmp;
}

///
/// Print local minima solutions.
///
void MethodAbsScan::printLocalMinima()
{
    TDatime date; // lets also print the current date
    if ( arg->debug ){
        cout << "MethodAbsScan::printLocalMinima() : LOCAL MINIMA for " << title << endl;
        cout << endl;
    }
    for ( int i=0; i<solutions.size(); i++ ){
        cout << "SOLUTION " << i << ":\n" << endl;
        cout << "  combination: " << name << endl;
        cout << "  title:       " << title << endl;
        cout << "  date:        " << date.AsString() << endl;
        solutions[i]->Print(arg->verbose, arg->printcor);
    }
}

///
/// Save local minima solutions.
///
void MethodAbsScan::saveLocalMinima(TString fName)
{
    TDatime date; // lets also print the current date
    if ( arg->debug ){
        cout << "MethodAbsScan::saveLocalMinima() : LOCAL MINIMA for " << title << endl;
        cout << endl;
    }
  ofstream outfile;
  outfile.open(fName.Data());

    for ( int i=0; i<solutions.size(); i++ ){
        outfile << "\%SOLUTION " << i << ":\n" << endl;
        outfile << "\%  combination: " << name << endl;
        outfile << "\%  title:       " << title << endl;
        outfile << "\%  date:        " << date.AsString() << endl;
        solutions[i]->SaveLatex(outfile, arg->verbose, arg->printcor);
    }
    outfile.close();
}

///
/// Get value of scan parameter at a certain solution.
/// \param iVar - Index of scan variable, 1 or 2.
/// \param iSol - Index of solution. 0 corresponds to the best one,
///               indices increase in order of chi2.
/// \return central value of the solution
/// \return -999 no solutions available
/// \return -99 solution not found
/// \return -9999 no such variable
///
float MethodAbsScan::getScanVarSolution(int iVar, int iSol)
{
    if ( solutions.size()==0 ){
        return -999;
    }
    if ( iSol >= solutions.size() ){
        cout << "MethodAbsScan::getScanVarSolution() : ERROR : no solution with id " << iSol << endl;
        return -99.;
    }
    RooSlimFitResult *r = getSolution(iSol);
    assert(r);
    TString varName;
    if      ( iVar==1 ) varName = getScanVar1Name();
    else if ( iVar==2 ) varName = getScanVar2Name();
    else{
        cout << "MethodAbsScan::getScanVarSolution() : WARNING : no such variable " << iVar << endl;
        return -9999.;
    }
    if ( r->isConfirmed() ){
        return r->getFloatParFinalVal(varName);
    }
    else{
        if ( nWarnings==0 ) cout << "MethodAbsScan::getScanVarSolution() : WARNING : Using unconfirmed solution." << endl;
        nWarnings+=1;
        return r->getConstParVal(varName);
    }
}

///
/// Get value of scan parameter 1 a certain solution.
/// \param iSol Index of solution. 0 corresponds to the best one,
/// indices increase in order of chi2.
///
float MethodAbsScan::getScanVar1Solution(int iSol)
{
    return getScanVarSolution(1, iSol);
}

///
/// Get value of scan parameter 2 a certain solution
/// (only meaningful for 2d scan).
/// \param iSol Index of solution. 0 corresponds to the best one,
/// indices increase in order of chi2.
///
float MethodAbsScan::getScanVar2Solution(int iSol)
{
    return getScanVarSolution(2, iSol);
}

///
/// Sort solutions in order of increasing chi2.
///
void MethodAbsScan::sortSolutions()
{
    if ( arg->debug ) cout << "MethodAbsScan::sortSolutions() : sorting solutions ..." << endl;
    vector<RooSlimFitResult*> solutionsUnSorted = solutions;
    vector<RooSlimFitResult*> tmp;
    solutions = tmp;
    int nSolutions = solutionsUnSorted.size();
    for ( int i=0; i<nSolutions; i++ ){
        float min = solutionsUnSorted[0]->minNll();
        int iMin = 0;
        for ( int i=0; i<solutionsUnSorted.size(); i++ ){
            if ( solutionsUnSorted[i]->minNll() < min ){
                min = solutionsUnSorted[i]->minNll();
                iMin = i;
            }
        }
        solutions.push_back(solutionsUnSorted[iMin]);
        solutionsUnSorted.erase(solutionsUnSorted.begin()+iMin);
    }
    if ( arg->debug ) cout << "MethodAbsScan::sortSolutions() : solutions sorted: " << solutions.size() << endl;
}

///
/// Refit all possible solutions with the scan parameter left
/// free to confirm the solutions. We will reject solutions as
/// fake if the free fit using them as the starting point will
/// move too far away. Or, if their Delta chi2 value is above 25.
///
void MethodAbsScan::confirmSolutions()
{
    if ( arg->debug ) cout << "MethodAbsScan::confirmSolutions() : Confirming solutions ..." << endl;
    FitResultCache frCache(arg);
    frCache.storeParsAtFunctionCall(w->set(parsName));

    vector<RooSlimFitResult*> confirmedSolutions;
    RooRealVar *par1 = w->var(scanVar1);
    RooRealVar *par2 = w->var(scanVar2);
    if ( par1 ) par1->setConstant(false);
    if ( par2 ) par2->setConstant(false);
    for ( int i=0; i<solutions.size(); i++){
        bool ok = loadSolution(i);
        if ( !ok ) continue;
        if ( arg->debug ){
            cout << "MethodAbsScan::confirmSolutions() : solution " << i;
            cout << " " << par1->GetName() << "=" << par1->getVal();
            if ( par2 ) cout << " " << par2->GetName() << "=" << par2->getVal();
            cout << endl;
        }

        // refit the solution
        // true uses thorough fit with HESSE, -1 silences output
        RooFitResult *r = fitToMinBringBackAngles(w->pdf(pdfName), true, -1);

        // Check scan parameter shift.
        // We'll allow for a shift equivalent to 3 step sizes.
        // Express the scan step size in terms of sigmas of the fitted parameters.
        float allowedSigma;
        if ( arg->var.size()==1 ){
            // 1d scan
            float par1stepsize = (par1->getMax("scan")-par1->getMin("scan"))/arg->npoints1d;
            RooRealVar* par1New = (RooRealVar*)r->floatParsFinal().find(par1->GetName());
            float par1stepsizeInSigma = par1New->getError()>0 ? par1stepsize/par1New->getError() : 0.2;
            allowedSigma = 3.*par1stepsizeInSigma;
        }
        else if ( arg->var.size()==2 ){
            // 2d scan
            float par1stepsize = (par1->getMax("scan")-par1->getMin("scan"))/arg->npoints2dx;
            float par2stepsize = (par2->getMax("scan")-par2->getMin("scan"))/arg->npoints2dy;
            RooRealVar* par1New = (RooRealVar*)r->floatParsFinal().find(par1->GetName());
            RooRealVar* par2New = (RooRealVar*)r->floatParsFinal().find(par2->GetName());
            float par1stepsizeInSigma = par1New->getError()>0 ? par1stepsize/par1New->getError() : 1.;
            float par2stepsizeInSigma = par2New->getError()>0 ? par2stepsize/par2New->getError() : 1.;
            allowedSigma = TMath::Max(3.*par1stepsizeInSigma,3.*par2stepsizeInSigma);
        }

        TIterator* it = 0;
        // Warn if a parameter is close to its limit
        it = r->floatParsFinal().createIterator();
        while ( RooRealVar* p = (RooRealVar*)it->Next() ){
            if ( p->getMax() - p->getVal() < p->getError()
                    || p->getVal() - p->getMin() < p->getError() ){
                cout << "\nMethodAbsScan::confirmSolutions() : WARNING : " << p->GetName() << " is close to its limit!" << endl;
                cout << "                                  : ";
                p->Print();
                cout << endl;
            }
        }
        delete it;

        // check migration of the parameters
        RooArgList listOld = solutions[i]->floatParsFinal();
        listOld.add(solutions[i]->constPars());
        RooArgList listNew = r->floatParsFinal();
        listNew.add(r->constPars());
        it = w->set(parsName)->createIterator();
        bool isConfirmed = true;
        TString rejectReason = "";
        while ( RooRealVar* p = (RooRealVar*)it->Next() )
        {
            RooRealVar* pOld = (RooRealVar*)listOld.find(p->GetName());
            RooRealVar* pNew = (RooRealVar*)listNew.find(p->GetName());
            if ( !pOld && !pNew ){
                cout << "MethodAbsScan::confirmSolutions() : ERROR : parameter not found: " << p->GetName() << endl;
                continue;
            }
            if ( pNew->getError()>0 ){
                float shift = fabs(pOld->getVal()-pNew->getVal());
                if ( isAngle(pOld) ) shift = angularDifference(pOld->getVal(), pNew->getVal());
                if ( shift/pNew->getError() > allowedSigma ){
                    if ( arg->debug ){
                        cout << "MethodAbsScan::confirmSolutions() : solution " << i << ", too large parameter shift:" << endl;
                        pOld->Print();
                        pNew->Print();
                    }
                    isConfirmed = false;
                    rejectReason = TString("too large shift in ") + pNew->GetName();
                }
            }
        }
        if ( r->minNll()-chi2minGlobal > 25 ){
            cout << "MethodAbsScan::confirmSolutions() : WARNING : local minimum has DeltaChi2>25." << endl;
            isConfirmed = false;
            rejectReason = Form("too large chi2: DeltaChi2>25 - chi2minGlobal: %e and confirmed NLL: %e", chi2minGlobal, r->minNll()) ;
        }
        if ( isConfirmed ){
            if ( arg->debug ) cout << "MethodAbsScan::confirmSolutions() : solution " << i << " accepted." << endl;
            RooSlimFitResult *sr = new RooSlimFitResult(r, true); // true saves correlation matrix
            sr->setConfirmed(true);
            confirmedSolutions.push_back(sr);
            delete r;
        }
        else{
            cout << "MethodAbsScan::confirmSolutions() : WARNING : solution " << i << " rejected "
                                     "(" << rejectReason << ")" << endl;
        }
    }
    // do NOT delete the old solutions! They are still in allResults and curveResults.
    solutions = confirmedSolutions;
    sortSolutions();
    if ( arg->debug ) printLocalMinima();
    removeDuplicateSolutions();
    // reset parameters
    setParameters(w, parsName, frCache.getParsAtFunctionCall());
}

///
/// Remove duplicate solutions from the common solutions storage
/// ('solutions' vector). Duplicate solutions can occur when two
/// unconfirmed solutions converge to the same true local minimum
/// when refitted by confirmSolutions().
///
/// No solutions will be removed if --qh 9 is given.
/// \todo upgrade the quickhack to a proper option
///
void MethodAbsScan::removeDuplicateSolutions()
{
    if ( arg->isQuickhack(9) ) return;
    vector<RooSlimFitResult*> solutionsNoDup;
    for ( int i=0; i<solutions.size(); i++ ){
        bool found = false;
        for ( int j=i+1; j<solutions.size(); j++ ){
            if ( compareSolutions(solutions[i],solutions[j]) ) found = true;
            if ( found==true ) continue;
        }
        if ( !found ) solutionsNoDup.push_back(solutions[i]);
        else{
            if ( arg->debug ) cout << "MethodAbsScan::removeDuplicateSolutions() : removing duplicate solution " << i << endl;
        }
    }
    if ( solutions.size()!=solutionsNoDup.size() ){
        cout << endl;
        if ( arg->debug ) cout << "MethodAbsScan::removeDuplicateSolutions() : ";
        cout << "INFO : some equivalent solutions were removed. In case of 2D scans" << endl;
        cout << "       many equivalent solutions may lay on a contour of constant chi2, in" << endl;
        cout << "       that case removing them is perhaps not desired. You can keep all solutions" << endl;
        cout << "       using --qh 9\n" << endl;
    }
    solutions = solutionsNoDup;
}

///
/// Compare two solutions.
/// \param r1 First solution
/// \param r2 Second solution
/// \return true, if both are equal inside a certain margin
///
bool MethodAbsScan::compareSolutions(RooSlimFitResult* r1, RooSlimFitResult* r2)
{
    // compare chi2
    if ( fabs(r1->minNll()-r2->minNll())>0.05 ) return false;
    // construct parameter lists
    RooArgList list1 = r1->floatParsFinal(); list1.add(r1->constPars());
    RooArgList list2 = r2->floatParsFinal(); list2.add(r2->constPars());
    // compare each parameter
    TIterator* it = w->set(parsName)->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() )
    {
        RooRealVar* p1 = (RooRealVar*)list1.find(p->GetName());
        RooRealVar* p2 = (RooRealVar*)list2.find(p->GetName());
        if ( !p1 && !p2 ){
            cout << "MethodAbsScan::compareSolutions() : ERROR : parameter not found: " << p->GetName() << endl;
            continue;
        }
        // We accept two parameters to be equal if they agree within 0.1 sigma.
        float sigma1 = p1->getError()>0 ? p1->getError() : p1->getVal()/10.;
        float sigma2 = p2->getError()>0 ? p2->getError() : p2->getVal()/10.;
        if ( fabs(p1->getVal()-p2->getVal())/(sqrt(sq(sigma1)+sq(sigma2))) > 0.1 ) return false;
    }
    return true;
}

///
/// Return a solution corresponding to a minimum of the profile
/// likelihoood.
/// \param i Index of the solution, they are orderd after increasing chi2,
///         i=0 is that with the smallest chi2.
///
RooSlimFitResult* MethodAbsScan::getSolution(int i)
{
    if ( i >= solutions.size() ){
        cout << Form("MethodAbsScan::getSolution() : ERROR : No solution with id %i.",i) << endl;
        return 0;
    }
    return solutions[i];
}

///
/// Helper function to copy over solutions from another
/// scanner. Clears the solutions vector and sets the one
/// given.
///
void MethodAbsScan::setSolutions(vector<RooSlimFitResult*> s)
{
    solutions.clear();
    for ( int i=0; i<s.size(); i++ ){
        solutions.push_back(s[i]);
    }
}


int MethodAbsScan::getDrawSolution()
{
    return drawSolution;
}

///
/// Make a pull plot of observables corresponding
/// to the given solution.
///
void MethodAbsScan::plotPulls(int nSolution)
{
    PullPlotter p(this);
    p.loadParsFromSolution(nSolution);
    p.savePulls();
    p.plotPulls();
}

void MethodAbsScan::setXscanRange(float min, float max)
{
    if ( min==max ) return;
    RooRealVar *par1 = w->var(scanVar1);
    assert(par1);
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    par1->setRange("scan", min, max);
    RooMsgService::instance().setGlobalKillBelow(INFO);
    if(arg->debug) std::cout << "DEBUG in MethodAbsScan::setXscanRange(): setting range for " << scanVar1 << ": " << min << ": " << max << std::endl;
    m_xrangeset = true;
}

void MethodAbsScan::setYscanRange(float min, float max)
{
    if ( min==max ) return;
    RooRealVar *par2 = w->var(scanVar2);
    assert(par2);
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    par2->setRange("scan", min, max);
    RooMsgService::instance().setGlobalKillBelow(INFO);
    m_yrangeset = true;
}



void MethodAbsScan::calcCLintervalsSimple(int CLsType, bool calc_expected)
{
    clintervals1sigma.clear();
    clintervals2sigma.clear();
    clintervalsuser.clear();
    // double levels[3] = {0.6827, 0.9545, CLuser};

    TH1F *histogramCL = this->hCL;
    if (this->hCLs && CLsType==1)
    {
        histogramCL = this->hCLs;
    }
    else if (this->hCLsFreq && CLsType==2)
    {
        histogramCL = this->hCLsFreq;
    }
    if (CLsType==2 && calc_expected && hCLsExp)
    {
        histogramCL = hCLsExp;
    }
    if(CLsType==0 || (this->hCLs && CLsType==1) || (this->hCLsFreq && CLsType==2))
    {
        for (int c=0;c<3;c++){
            const std::pair<double, double> borders = getBorders(TGraph(histogramCL), ConfidenceLevels[c]);
            CLInterval cli;
            cli.pvalue = 1. - ConfidenceLevels[c];
            cli.min = borders.first;
            cli.max = borders.second;
            cli.central = -1;
            if ( c==0 ) clintervals1sigma.push_back(cli);
            if ( c==1 ) clintervals2sigma.push_back(cli);
            if ( c==2 ) clintervalsuser.push_back(cli);
            if (CLsType==1) std::cout << "Simplified CL_s ";
            if (CLsType==2) std::cout << "Standard CL_s";
            std::cout<<"borders at "<<ConfidenceLevels[c]<<"    [ "<<borders.first<<" : "<<borders.second<<"]";
            cout << ", " << methodName << " (simple boundary scan)" << endl;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    //// Add a hacky calculation of the CL_s intervals
    //// \todo: Do it properly from the very start by introducing a bkg model and propagate it to the entire framework.
    /// TODO: I think this can now disappear

    if ( (!this->hCLs && CLsType==1) || (!this->hCLsFreq && CLsType==2) )
    {
        std::cout << "**************************************************************************************************************************************" << std::endl;
        std::cout << "WARNING: hCLs is empty! Will calculate CLs intervals by normalising the p values to the p value of the first bin." << std::endl;
        std::cout << "WARNING: This is only an approximate solution and MIGHT EVEN BE WRONG, if the first bin does not represent the background expectation!" << std::endl;
        std::cout << "**************************************************************************************************************************************" << std::endl;
        clintervals1sigma.clear();
        clintervals2sigma.clear();
        clintervalsuser.clear();

        for (int c=0;c<3;c++){
            const std::pair<double, double> borders_CLs = getBorders_CLs(TGraph(histogramCL), ConfidenceLevels[c]);
            CLInterval cli;
            cli.pvalue = 1. - ConfidenceLevels[c];
            cli.min = borders_CLs.first;
            cli.max = borders_CLs.second;
            cli.central = -1;
            if ( c==0 ) clintervals1sigma.push_back(cli);
            if ( c==1 ) clintervals2sigma.push_back(cli);
            if ( c==2 ) clintervalsuser.push_back(cli);
            std::cout<<"CL_s borders at "<<ConfidenceLevels[c]<<"  [ "<<borders_CLs.first<<" : "<<borders_CLs.second<<"]";
            cout << ", " << methodName << " (simple boundary scan)" << endl;
        }
    }
}

/*!
\brief determines the borders of the confidence interval by linear or qubic interpolation.
\param graph The graph holding the p-values.
\param confidence_level The confidence level at which the interval is to be determined.
\param qubic Optional parameter. False by default. If true, qubic interpolation is used.
*/
const std::pair<double, double> MethodAbsScan::getBorders(const TGraph& graph, const double confidence_level, bool qubic){

    const double p_val = 1 - confidence_level;
    TSpline* splines = nullptr;
    if(qubic) splines = new TSpline3();


    double min_edge = graph.GetX()[0];
    // will never return smaller edge than min_edge
    double max_edge = graph.GetX()[graph.GetN()-1];
    // will never return higher edge than max_edge
    int scan_steps = 1000;
    double lower_edge = min_edge;
    double upper_edge = max_edge;

    for(double point = min_edge; point < max_edge; point+= (max_edge-min_edge)/scan_steps){

     if(graph.Eval(point, splines)>p_val){
            lower_edge = point;
            break;
        }
    }
    for(double point = max_edge; point > min_edge; point-= (max_edge-min_edge)/scan_steps){
        if(graph.Eval(point, splines)>p_val){
            upper_edge = point;
            break;
        }
    }
    return std::pair<double, double>(lower_edge,upper_edge);
}

////////////////////////////////////////////////////////////////////////////////////////////
//// Do a hacky calculation of the CL_s intervals, where essentially the pValue is normalized to the pValue with n_sig=0.
//// Let's first assume that the parameter of interest is ALWAYS a parameter correlated with n_sig, so that parameter=0 means n_sig=0.
//// Therefore the pValue(CL_s) is given by the ratio of the pValue at scanpointand the pValue of the lowest bin.
//// \todo Do it properly from the very start by introducing a bkg model and propagate it to the entire framework.

const std::pair<double, double> MethodAbsScan::getBorders_CLs(const TGraph& graph, const double confidence_level, bool qubic){

    const double p_val = 1 - confidence_level;
    TSpline* splines = nullptr;
    if(qubic) splines = new TSpline3();


    double min_edge = graph.GetX()[0];
    // will never return smaller edge than min_edge
    double max_edge = graph.GetX()[graph.GetN()-1];
    // will never return higher edge than max_edge
    int scan_steps = 1000;
    double lower_edge = min_edge;
    double upper_edge = max_edge;

    for(double point = min_edge; point < max_edge; point+= (max_edge-min_edge)/scan_steps){

        //for CL_s normalize pVal to the pVal at 0 (which has to be the background model)
     if(graph.Eval(point, splines)/graph.Eval(min_edge, splines)>p_val){
            lower_edge = point;
            break;
        }
    }
    for(double point = max_edge; point > min_edge; point-= (max_edge-min_edge)/scan_steps){

        //for CL_s normalize pVal to the pVal at 0 (which has to be the background model)
        if(graph.Eval(point, splines)/graph.Eval(min_edge, splines)>p_val){
            upper_edge = point;
            break;
        }
    }
    return std::pair<double, double>(lower_edge,upper_edge);
}

bool MethodAbsScan::checkCLs()
{
    if (!hCLsExp || !hCLsErr1Up || !hCLsErr1Dn || !hCLsErr2Up || !hCLsErr2Dn){
        std::cout << "ERROR: ***************************************************" << std::endl;
        std::cout << "ERROR: MethodAbsScan::checkCLs() : No CLs plot available!!" << std::endl;
        std::cout << "ERROR: ***************************************************" << std::endl;
        return false;
    }
    assert( hCLsExp->GetNbinsX() == hCLsErr1Up->GetNbinsX() );
    assert( hCLsExp->GetNbinsX() == hCLsErr2Up->GetNbinsX() );
    assert( hCLsExp->GetNbinsX() == hCLsErr1Dn->GetNbinsX() );
    assert( hCLsExp->GetNbinsX() == hCLsErr2Dn->GetNbinsX() );

    // correct for low stats in the lower error
    for ( int i=1; i<=hCLsExp->GetNbinsX(); i++ ) {
        if ( hCLsErr1Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i) ) {
            hCLsErr1Dn->SetBinContent(i, hCLsExp->GetBinContent(i) - ( hCLsErr1Up->GetBinContent(i)-hCLsErr1Dn->GetBinContent(i) )/2. );
        }
        if ( hCLsErr1Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i) ) {
            hCLsErr1Dn->SetBinContent(i, hCLsExp->GetBinContent(i) - ( hCLsErr1Up->GetBinContent(i) - hCLsExp->GetBinContent(i) ) );
        }
        if ( ((hCLsExp->GetBinContent(i) - hCLsErr1Dn->GetBinContent(i))/hCLsExp->GetBinContent(i))<0.05 ) {
            hCLsErr1Dn->SetBinContent(i, hCLsExp->GetBinContent(i) - ( hCLsErr1Up->GetBinContent(i)-hCLsErr1Dn->GetBinContent(i) )/2. );
        }
        if ( hCLsErr2Dn->GetBinContent(i) >= hCLsExp->GetBinContent(i) ) {
            hCLsErr2Dn->SetBinContent(i, hCLsExp->GetBinContent(i) - ( hCLsErr2Up->GetBinContent(i)-hCLsErr2Dn->GetBinContent(i) )/2. );
        }
        if ( hCLsErr2Dn->GetBinContent(i) >= hCLsErr1Dn->GetBinContent(i) ) {
            hCLsErr2Dn->SetBinContent(i, hCLsExp->GetBinContent(i) - ( hCLsExp->GetBinContent(i)-hCLsErr1Dn->GetBinContent(i))*2. );
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//// end of CL_s part
////////////////////////////////////////////////////////////////////////////////////////////////////
