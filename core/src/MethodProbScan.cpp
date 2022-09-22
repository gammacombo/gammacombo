/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */
#include "TSystem.h"
#include "MethodProbScan.h"

MethodProbScan::MethodProbScan(Combiner *comb)
: MethodAbsScan(comb)
{
    methodName = "Prob";
    scanDisableDragMode = false;
    nScansDone = 0;
}


MethodProbScan::MethodProbScan(OptParser* opt)
: MethodAbsScan(opt)
{
    methodName = "Prob";
    scanDisableDragMode = false;
    nScansDone = 0;
}

///
/// Default constructor
///
MethodProbScan::MethodProbScan()
{
    methodName = "Prob";
    scanDisableDragMode = false;
    nScansDone = 0;
}

MethodProbScan::~MethodProbScan()
{
}

///
/// Perform 1d Prob scan.
///
/// - Scan range defined through limit "scan".
/// - Will fill the hCL histogram with the 1-CL curve.
/// - Start at a scan value that is in the middle of the allowed
///   range, preferably a solution, and scan up and down from there.
/// - use the "probforce" command line flag to enable force minimum finding
///
/// \param fast This will scan each scanpoint only once.
/// \param reverse This will scan in reverse direction.
///   When using the drag mode, this can sometimes make a difference.
/// \return status: 2 = new global minimum found, 1 = error
///
int MethodProbScan::scan1d(bool fast, bool reverse, bool quiet)
{
    if ( arg->debug ) cout << "MethodProbScan::scan1d() : starting ... " << endl;
    nScansDone++;

    // The "improve" method doesn't need multiple scans.
    if ( arg->probforce || arg->probimprove ) fast = true;
    if ( arg->probforce ) scanDisableDragMode = true;

    // Save parameter values that were active at function call.
    if ( startPars ) delete startPars;
    startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
    startPars->add(*w->set(parsName));

    // // start scan from global minimum (not always a good idea as we need to set from other places as well)
    // setParameters(w, parsName, globalMin);

    // load scan parameter and scan range
    setLimit(w, scanVar1, "scan");
    RooRealVar *par = w->var(scanVar1);
    assert(par);
    float min = hCL->GetXaxis()->GetXmin();
    float max = hCL->GetXaxis()->GetXmax();
    if ( fabs(par->getMin()-min)>1e-6 || fabs(par->getMax()-max)>1e-6 ){
        cout << "MethodProbScan::scan1d() : WARNING : Scan range was changed after initScan()" << endl;
        cout << "                           was called so the old range will be used." << endl;
    }
    if ( arg->verbose ){
        cout << "\nProb configuration:" << endl;
        cout << "  combination : " << title << endl;
        cout << "  scan variable : " << scanVar1 << endl;
        cout << "  scan range : " << min << " ... " << max << endl;
        cout << "  scan steps : " << nPoints1d << endl;
        cout << "  fast mode : " << fast << endl;
        cout << endl;
    }

    // Set limit to all parameters.
    combiner->loadParameterLimits();

    // fix scan parameter
    par->setConstant(true);

    // j =
    // 0 : start value -> upper limit
    // 1 : upper limit -> start value
    // 2 : start value -> lower limit
    // 3 : lower limit -> start value
    float startValue = par->getVal();
    bool scanUp;

    // for the status bar
    float nTotalSteps = nPoints1d;
    nTotalSteps *= fast ? 1 : 2;
    float nStep = 0;
    float printFreq = nTotalSteps>15 ? 10 : nTotalSteps;

    // Report on the smallest new minimum we come across while scanning.
    // Sometimes the scan doesn't find the minimum
    // that was found before. Warn if this happens.
    double bestMinOld = chi2minGlobal;
    double bestMinFoundInScan = 100.;

    for ( int jj=0; jj<4; jj++ )
    {
        int j = jj;
        if ( reverse ) switch(jj)
        {
            case 0: j = 2; break;
            case 1: j = 3; break;
            case 2: j = 0; break;
            case 3: j = 1; break;
        }

        float scanStart, scanStop;
        switch(j)
        {
            case 0:
                // UP
                setParameters(w, parsName, startPars->get(0));
                scanStart = startValue;
                scanStop  = par->getMax();
                scanUp = true;
                break;
            case 1:
                // DOWN
                scanStart = par->getMax();
                scanStop  = startValue;
                scanUp = false;
                break;
            case 2:
                // DOWN
                setParameters(w, parsName, startPars->get(0));
                scanStart = startValue;
                scanStop  = par->getMin();
                scanUp = false;
                break;
            case 3:
                // UP
                scanStart = par->getMin();
                scanStop  = startValue;
                scanUp = true;
                break;
        }

        if ( fast && ( j==1 || j==3 ) ) continue;

        for ( int i=0; i<nPoints1d; i++ )
        {
            float scanvalue;
            if ( scanUp )
            {
                scanvalue = min + (max-min)*(double)i/(double)nPoints1d + hCL->GetBinWidth(1)/2.;
                if ( scanvalue < scanStart ) continue;
                if ( scanvalue > scanStop ) break;
            }
            else
            {
                scanvalue = max - (max-min)*(double)(i+1)/(double)nPoints1d + hCL->GetBinWidth(1)/2.;
                if ( scanvalue > scanStart ) continue;
                if ( scanvalue < scanStop ) break;
            }

            // disable drag mode
            // (the improve method doesn't work with drag mode as parameter run
            // at their limits)
            if ( scanDisableDragMode ) setParameters(w, parsName, startPars->get(0));

            // set the parameter of interest to the scan point
            par->setVal(scanvalue);

            // don't scan in unphysical region
            if ( scanvalue < par->getMin() || scanvalue > par->getMax() ) continue;

            // status bar
            if ( (((int)nStep % (int)(nTotalSteps/printFreq)) == 0))
                if (!quiet) cout << "MethodProbScan::scan1d() : scanning " << (float)nStep/(float)nTotalSteps*100. << "%   \r" << flush;

            // fit!
            RooFitResult *fr = 0;
            if ( arg->probforce )         fr = fitToMinForce(w, combiner->getPdfName());
            else if ( arg->probimprove )  fr = fitToMinImprove(w, combiner->getPdfName());
            else                          fr = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
            double chi2minScan = fr->minNll();
            if ( std::isinf(chi2minScan) ) chi2minScan=1e4; // else the toys in PDF_testConstraint don't work
            RooSlimFitResult *r = new RooSlimFitResult(fr); // try to save memory by using the slim fit result
            delete fr;
            allResults.push_back(r);
            bestMinFoundInScan = TMath::Min((double)chi2minScan, (double)bestMinFoundInScan);

            TString warningChi2Neg;
            if ( chi2minScan < 0 ){
                float newChi2minScan = chi2minGlobal + 25.; // 5sigma more than best point
                warningChi2Neg = "MethodProbScan::scan1d() : WARNING : " + title;
                warningChi2Neg += TString(Form(" chi2 negative for scan point %i: %f",i,chi2minScan));
                warningChi2Neg += " setting to: " + TString(Form("%f",newChi2minScan));
                //cout << warningChi2Neg << "\r" << flush;
                cout << warningChi2Neg << endl;
                chi2minScan = newChi2minScan;
            }

            // If we find a minimum smaller than the old "global" minimum, this means that all
            // previous 1-CL values are too high.
            if ( chi2minScan<chi2minGlobal ){
                if ( arg->verbose ) cout << "MethodProbScan::scan1d() : WARNING : '" << title << "' new global minimum found! "
                                                                        << " chi2minScan=" << chi2minScan << endl;
                chi2minGlobal = chi2minScan;
                // recompute previous 1-CL values
                for ( int k=1; k<=hCL->GetNbinsX(); k++ ){
                    hCL->SetBinContent(k, TMath::Prob(hChi2min->GetBinContent(k)-chi2minGlobal, 1));
                }
            }

            double deltaChi2 = chi2minScan - chi2minGlobal;
            double oneMinusCL = TMath::Prob(deltaChi2, 1);
            double deltaChi2Bkg = TMath::Max(chi2minScan - hChi2min->GetBinContent(1), 0.0);
            if(i==0) deltaChi2Bkg = 0.0;
            double oneMinusCLBkg = TMath::Prob(deltaChi2Bkg, 1);
            hCLs->SetBinContent(hCLs->FindBin(scanvalue), oneMinusCLBkg);

            if(i==0) chi2minBkg = chi2minScan;

            // Save the 1-CL value and the corresponding fit result.
            // But only if better than before!
            if ( hCL->GetBinContent(hCL->FindBin(scanvalue)) <= oneMinusCL ){
                hCL->SetBinContent(hCL->FindBin(scanvalue), oneMinusCL);
                hChi2min->SetBinContent(hCL->FindBin(scanvalue), chi2minScan);
                int iRes = hCL->FindBin(scanvalue)-1;
                curveResults[iRes] = r;
            }
            nStep++;
        }
    }
    cout << "MethodProbScan::scan1d() : scan done.           " << endl;

    if ( bestMinFoundInScan-bestMinOld > 0.01 ){
        cout << "MethodProbScan::scan1d() : WARNING: Scan didn't find similar minimum to what was found before!" << endl;
        cout << "MethodProbScan::scan1d() :          Too strict parameter limits? Too coarse scan steps? Didn't load global minimum?" << endl;
        cout << "MethodProbScan::scan1d() :          chi2 bestMinFoundInScan=" << bestMinFoundInScan << ", bestMinOld=" << bestMinOld << endl;
    }

    // attempt to correct for undercoverage
    if (pvalueCorrectorSet) {
        for ( int k=1; k<=hCL->GetNbinsX(); k++ ){
            double pvalueProb = hCL->GetBinContent(k);
            pvalueProb = pvalueCorrector->transform(pvalueProb);
            hCL->SetBinContent(k, pvalueProb);
        }
    }

    setParameters(w, parsName, startPars->get(0));
    saveSolutions();
    if (arg->confirmsols) confirmSolutions();

    if ( (bestMinFoundInScan-bestMinOld)/bestMinOld > 0.01 ) return 1;
    return 0;
}

///
/// Modify the CL histograms according to the defined test statistics
///\return status 0 ->potentially can encode debug information here
///
int MethodProbScan::computeCLvalues(){
    std::cout << "Computing CL values based on test statistic decision" << std::endl;
    std::cout << "Using "<< arg->teststatistic <<"-sided test statistic" << std::endl;

    if ( !globalMin && !this->getSolution() ) {
        std::cout << "Could not find a solution so can't redefine the test statistics appropriately" << std::endl;
        return 1;
    }


    float bestfitpoint;
    float bestfitpointerr;

    if ( globalMin ) {
        bestfitpoint    = ((RooRealVar*)globalMin->floatParsFinal().find(scanVar1))->getVal();
        bestfitpointerr = ((RooRealVar*)globalMin->floatParsFinal().find(scanVar1))->getError();
    }

    if(this->getSolution()){
        if (this->getSolution()->floatParsFinal().find(scanVar1)) {
            bestfitpoint    = ((RooRealVar*)this->getSolution()->floatParsFinal().find(scanVar1))->getVal();
            bestfitpointerr = ((RooRealVar*)this->getSolution()->floatParsFinal().find(scanVar1))->getError();
        }
    }

    for (int k=1; k<=hCL->GetNbinsX(); k++){
        float scanvalue=hChi2min->GetBinCenter( k );
        float teststat_measured = hChi2min->GetBinContent( k ) - chi2minGlobal;
        float CLb = 1. - (normal_cdf(TMath::Sqrt(teststat_measured) + ((scanvalue - 0.)/bestfitpointerr)) + normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.)/bestfitpointerr)) - 1.);
        if (arg->teststatistic ==1){ // use one-sided test statistic
            teststat_measured = bestfitpoint <= scanvalue ? teststat_measured : 0.; // if mu < muhat then q_mu = 0
            hCL->SetBinContent(k,1. - normal_cdf(TMath::Sqrt(teststat_measured)));
            // if (scanvalue < bestfitpoint) hCL->SetBinContent(k,1.0);    // should not be here, but looks ugly if solution is drawn as p=1
            CLb = 1. - normal_cdf(TMath::Sqrt(teststat_measured) - ((scanvalue - 0.)/bestfitpointerr));
        }
        if ( arg->cls.size()>0) hCLs->SetBinContent(k,min(1.,hCL->GetBinContent(k)/CLb));
        // std::cout << "MethodProbScan::" << k << "\t" << hCL->GetBinContent(k) << "\t" << CLb << "\t" << hCLs->GetBinContent(k) <<std::endl;

    }

    return 0;
}


///
/// Delete a pointer if it is not included in
/// the curveResults2d vector. Also removes it
/// from the allResults vector by setting the entry
/// to 0.
/// \return true if r was deleted, or if it is 0
///
bool MethodProbScan::deleteIfNotInCurveResults2d(RooSlimFitResult *r)
{
    if ( r==0 ) return true;
    bool del = true;
    for ( int j=0; j<hCL2d->GetNbinsX(); j++ )
        for ( int k=0; k<hCL2d->GetNbinsY(); k++ ){
            if ( r==curveResults2d[j][k] ){
                del = false;
                break;
            }
        }
    if ( del ){
        delete r;
        // remove also from allResults vector
        for ( int j=0; j<allResults.size(); j++ ){
            if ( r==allResults[j] ) allResults[j] = 0;
        }
    }
    return del;
}

///
/// Compute coordinates that can be used as coordinates
/// of that fit result, that we want to use as start parameters
/// in the drag mode. The i,j coordinates spiral out. This function
/// computes coordinates from inner turns of the spiral.
/// If the inner turn doesn't exist, the start coordinates are set
/// as the result, and false is returned.
/// \param iStart center of the spiral
/// \param jStart center of the spiral
/// \param i current coordinates on the outmost turn of the spiral
/// \param j current coordinates on the outmost turn of the spiral
/// \param iResult resulting coordinates of an inner turn
/// \param jResult resulting coordinates of an inner turn
/// \param nTurn number of inner turn to jump to, 1=next-to-outer turn, 2=second-next, etc.
///
bool MethodProbScan::computeInnerTurnCoords(const int iStart, const int jStart,
        const int i, const int j, int &iResult, int &jResult, int nTurn)
{
    // compute bin coordinates of start parameters: connect center of
    // the spiral to the scan point with a straight line, go back by sqrt(2)
    // units, take bin this ends us in
    iResult = iStart;
    jResult = jStart;
    if ( sq(i-iStart)+sq(j-jStart)>0 )
    {
        iResult = round((float)i - float(nTurn)*1.41 * float(i-iStart)/sqrt(sq(i-iStart)+sq(j-jStart)));
        jResult = round((float)j - float(nTurn)*1.41 * float(j-jStart)/sqrt(sq(i-iStart)+sq(j-jStart)));
    }
    if ( iResult-1>=curveResults2d.size() )    iResult = iStart;
    if ( jResult-1>=curveResults2d[0].size() ) jResult = jStart;
    // check result
    if ( iResult-1>=curveResults2d.size() || jResult-1>=curveResults2d[0].size()
            || iResult-1<0 || jResult-1<0 )
    {
        cout << "MethodProbScan::computeInnerTurnCoords() : ERROR : resulting coordinates out of range! "
                                                            << iResult-1 << " " << jResult-1 << endl;
    }
    if ( iResult==iStart && jResult==jStart ) return false;
    return true;
}


void MethodProbScan::sanityChecks()
{
    if ( !w->set(parsName) ){
        cout << "MethodProbScan::sanityChecks() : ERROR : parsName not found: " << parsName << endl;
        exit(1);
    }
    if ( !w->var(scanVar1) ){
        cout << "MethodProbScan::sanityChecks() : ERROR : scanVar1 not found: " << scanVar1 << endl;
        exit(1);
    }
    if ( !w->var(scanVar2) ){
        cout << "MethodProbScan::sanityChecks() : ERROR : scanVar2 not found: " << scanVar2 << endl;
        exit(1);
    }
}

///
/// Perform a 2d Prob scan.
/// Scan range defined through limit "scan".
/// Fills the hCL2d histogram with the 1-CL curve.
/// Saves all encountered fit results to allResults.
/// Saves the fit results that make it into the 1-CL curve into curveResults2d.
/// Scan strategy: Spiral out!
///
int MethodProbScan::scan2d()
{
    if ( arg->debug ) cout << "MethodProbScan::scan2d() : starting ..." << endl;
    nScansDone++;
    sanityChecks();
    if ( startPars ) delete startPars;

    // Define whether the 2d contours in hCL are "1D sigma" (ndof=1) or "2D sigma" (ndof=2).
    // Leave this at 1 for now, as the "2D sigma" contours are computed from hChi2min2d, not hCL.
    int ndof = 1;

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

    // store start parameters so we can reset them later
    startPars = new RooDataSet("startPars", "startPars", *w->set(parsName));
    startPars->add(*w->set(parsName));

    // // start scan from global minimum (not always a good idea as we need to set from other places as well)
    // setParameters(w, parsName, globalMin);

    // Define scan parameters and scan range:
    RooRealVar *par1 = w->var(scanVar1);
    RooRealVar *par2 = w->var(scanVar2);

    // Set limit to all parameters.
    combiner->loadParameterLimits();

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
    TH2F *hDbgChi2min2d = histHardCopy(hChi2min2d, firstScanDone, true, TString(hChi2min2d->GetName())+TString("_Dbg"));
    hDbgChi2min2d->SetTitle(Form("#Delta#chi^{2} for scan %i, %s",nScansDone,title.Data()));
    if ( firstScanDone ) hDbgChi2min2d->GetZaxis()->SetRangeUser(hChi2min2dMin,hChi2min2dMin+81);
    hDbgChi2min2d->GetXaxis()->SetTitle(par1->GetTitle());
    hDbgChi2min2d->GetYaxis()->SetTitle(par2->GetTitle());
    hDbgChi2min2d->GetZaxis()->SetTitle("#Delta#chi^{2}");
    TH2F *hDbgStart = histHardCopy(hChi2min2d, false, true, TString(hChi2min2d->GetName())+TString("_DbgSt"));

    // start coordinates
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
                    cout << Form("MethodProbScan::scan2d() : scanning %3.0f%%", (float)nSteps/(float)nTotalSteps*100.)
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
                if ( !arg->probforce ) fr = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
                else                   fr = fitToMinForce(w, combiner->getPdfName());
                double chi2minScan = fr->minNll();
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
                if ( chi2minScan > -500 && chi2minScan<chi2minGlobal ){
                    // warn only if there was a significant improvement
                    if ( arg->debug || chi2minScan<chi2minGlobal-1e-2 ){
                        if ( arg->verbose ) cout << "MethodProbScan::scan2d() : WARNING : '" << title << "' new global minimum found! chi2minGlobal="
                                                            << chi2minGlobal << " chi2minScan=" << chi2minScan << endl;
                    }
                    chi2minGlobal = chi2minScan;
                    // recompute previous 1-CL values
                    for ( int k=1; k<=hCL2d->GetNbinsX(); k++ )
                        for ( int l=1; l<=hCL2d->GetNbinsY(); l++ ){
                            hCL2d->SetBinContent(k, l, TMath::Prob(hChi2min2d->GetBinContent(k,l)-chi2minGlobal, ndof));
                        }
                }

                double deltaChi2 = chi2minScan - chi2minGlobal;
                double oneMinusCL = TMath::Prob(deltaChi2, ndof);

                // Save the 1-CL value. But only if better than before!
                if ( hCL2d->GetBinContent(i, j) < oneMinusCL ){
                    hCL2d->SetBinContent(i, j, oneMinusCL);
                    hChi2min2d->SetBinContent(i, j, chi2minScan);
                    hDbgChi2min2d->SetBinContent(i, j, chi2minScan);
                    curveResults2d[i-1][j-1] = r;
                }

                // draw/update histograms - doing only every nth update
        // depending on value of updateFreq
        // saves a lot of time for small combinations
                if ( ( arg->interactive && ((int)nSteps % arg->updateFreq == 0) ) || nSteps==nTotalSteps ){
                    hDbgChi2min2d->Draw("colz");
                    hDbgStart->Draw("boxsame");
                    startpointmark->Draw();
                    cDbg->Update();
          cDbg->Modified();
          gSystem->ProcessEvents();
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
    cout << "MethodProbScan::scan2d() : scan done.            " << endl;
    if ( arg->debug ){
        cout << "MethodProbScan::scan2d() : full scan time:             "; tScan.Print();
        cout << "MethodProbScan::scan2d() : - fitting:                  "; tFit.Print();
        cout << "MethodProbScan::scan2d() : - create RooSlimFitResults: "; tSlimResult.Print();
        cout << "MethodProbScan::scan2d() : - memory management:        "; tMemory.Print();
    }
    setParameters(w, parsName, startPars->get(0));
    saveSolutions2d();
    if ( arg->debug ) printLocalMinima();
    if ( arg->confirmsols ) confirmSolutions();

    // clean all fit results that didn't make it into the final result
    for ( int i=0; i<allResults.size(); i++ ){
        deleteIfNotInCurveResults2d(allResults[i]);
    }

    if ( bestMinFoundInScan-bestMinOld > 0.1 )
    {
        cout << "MethodProbScan::scan2d() : WARNING: Scan didn't find minimum that was found before!" << endl;
        cout << "MethodProbScan::scan2d() :          Are you using too strict parameter limits?" << endl;
        cout << "MethodProbScan::scan2d() :          min chi2 found in scan: " << bestMinFoundInScan << ", old min chi2: " << bestMinOld << endl;
        return 1;
    }

  // cleanup
  if (hDbgChi2min2d) delete hDbgChi2min2d;
  if (hDbgStart) delete hDbgStart;

    return 0;
}


///
/// Find the RooFitResults corresponding to all local
/// minima from the curveResults vector
/// and save them into the solutions vector.
/// It will be sorted for minChi2. Index 0
/// will correspond to the least chi2.
///
void MethodProbScan::saveSolutions()
{
    if ( arg->debug ) cout << "MethodProbScan::saveSolutions() : searching for minima in hChi2min ..." << endl;

    // delete old solutions if any
    vector<RooSlimFitResult*> tmp;
    solutions = tmp;

    // loop over chi2 histogram to locate local maxima
    for ( int i=2; i<hChi2min->GetNbinsX()-1; i++ )
    {
        bool oneBinMax = hChi2min->GetBinContent(i-1) > hChi2min->GetBinContent(i)
            && hChi2min->GetBinContent(i+1) > hChi2min->GetBinContent(i);
        bool twoBinMax = hChi2min->GetBinContent(i-1) > hChi2min->GetBinContent(i)
            && hChi2min->GetBinContent(i  )== hChi2min->GetBinContent(i+1)
            && hChi2min->GetBinContent(i+2) > hChi2min->GetBinContent(i+1);
        if ( !(oneBinMax || twoBinMax )) continue;

        // loop over fit results to find those that produced it
        for ( int j=0; j<curveResults.size(); j++ ){
            if ( !curveResults[j] ){
                if ( arg->debug ) cout << "MethodProbScan::saveSolutions() : WARNING : empty solution at index " << j << endl;
                continue;
            }

            if ( hChi2min->FindBin(curveResults[j]->getConstParVal(scanVar1))==i ){
                if ( arg->debug ){
                    cout << "MethodProbScan::saveSolutions() : saving solution " << j << ":" << endl;
                    curveResults[j]->Print();
                }
                solutions.push_back(curveResults[j]);
            }
        }
    }

    if ( solutions.size()==0 ){
        cout << "MethodProbScan::saveSolutions() : ERROR : No solutions found." << endl;
    }

    sortSolutions();
}

///
/// Find the RooFitResults corresponding to all local
/// minima from the curveResults2d vector
/// and save them into the solutions vector.
/// It will be sorted for minChi2. Index 0
/// will correspond to the least chi2.
///
/// We use a brute force minimum finding over the histogram,
/// as it is not large: at each bin, inpsect the 8 surrounding
/// ones.
///
void MethodProbScan::saveSolutions2d()
{
    if ( arg->debug ) cout << "MethodProbScan::saveSolutions2d() : searching for minima in hChi2min2d ..." << endl;

    // delete old solutions if any
    for ( int j=0; j<solutions.size(); j++ ) if ( solutions[j] ) delete solutions[j];
    solutions.clear();

    // loop over chi2 histogram to locate local minima
    for ( int i=2; i<hChi2min2d->GetNbinsX(); i++ )
    {
        for ( int j=2; j<hChi2min2d->GetNbinsY(); j++ )
        {
            if ( !(hChi2min2d->GetBinContent(i-1,   j) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i+1,   j) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i,   j-1) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i,   j+1) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i-1, j-1) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i+1, j+1) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i+1, j-1) > hChi2min2d->GetBinContent(i, j)
                        && hChi2min2d->GetBinContent(i-1, j+1) > hChi2min2d->GetBinContent(i, j)
                  )) continue;

            RooSlimFitResult *r = curveResults2d[i-1][j-1]; // -1 because it starts counting at 0, but histograms at 1
            if (!r){
                cout << "MethodProbScan::saveSolutions2d() : ERROR : No corresponding RooFitResult found! Skipping (i,j)="
                                                             << Form("(%i,%i)",i,j) << endl;
                continue;
            }
            if ( arg->debug ) cout << "MethodProbScan::saveSolutions2d() : saving solution of bin " << Form("(%i,%i)",i,j) << " ..." << endl;
            solutions.push_back((RooSlimFitResult*)curveResults2d[i-1][j-1]->Clone());
        }
    }

    if ( solutions.size()==0 ){
        cout << "MethodProbScan::saveSolutions2d() : WARNING : No solutions found in 2D scan!" << endl;
        cout << endl;
        cout << "  This can happen when a solution is too close" << endl;
        cout << "  to the plot boundary. In this case, either change" << endl;
        cout << "  the scan range using --scanrange and --scanrangey," << endl;
        cout << "  or increase the number of scan points, using" << endl;
        cout << "  --npoints or --npoints2dx, --npoints2dy" << endl;
        cout << endl;
        return;
    }
    sortSolutions();
}


///
/// Get the chi2 value of the profile likelihood at a given
/// scan point. Requires that the scan was performed before
/// by scan1d().
///
float MethodProbScan::getChi2min(float scanpoint)
{
    assert(hChi2min);
    int iBin = hChi2min->FindBin(scanpoint);
    return hChi2min->GetBinContent(iBin);
}
