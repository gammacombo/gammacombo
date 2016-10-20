/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#include "MethodAbsScan.h"

///
/// 'Default Constructor'
/// Introduced so that inherited classes do not have to call an
/// explicit constructor
///
	MethodAbsScan::MethodAbsScan()
: rndm()
{
	exit(1);
	methodName = "Abs";
	drawFilled = true;
};

	MethodAbsScan::MethodAbsScan(Combiner *c)
: rndm()
{
	combiner = c;
	methodName = "Abs";
	w = combiner->getWorkspace();
	name = combiner->getName();
	title = combiner->getTitle();
	arg = combiner->getArg();
	scanVar1 = arg->var[0];
	if ( arg->var.size()>1 ) scanVar2 = arg->var[1];
	verbose = arg->verbose;
	drawSolution = 0;
	nPoints1d  = arg->npoints1d;
	nPoints2dx = arg->npoints2dx;
	nPoints2dy = arg->npoints2dy;
	pvalueCorrectorSet = false;
	pdfName  = "pdf_"+combiner->getPdfName();
	obsName  = "obs_"+combiner->getPdfName();
	parsName = "par_"+combiner->getPdfName();
	thName   = "th_"+combiner->getPdfName();
  toysName = "toy_"+combiner->getPdfName();
	chi2minGlobal = 0.0;
	chi2minGlobalFound = false;
	lineStyle = 0;
	lineColor = kBlue-8;
	textColor = kBlack;
  fillStyle = 1001;
	hCL = 0;
	hCL2d = 0;
	hChi2min = 0;
	hChi2min2d = 0;
	obsDataset = 0;
	startPars = 0;
	globalMin = 0;
	nWarnings = 0;
	drawFilled = true;
	m_xrangeset = false;
	m_yrangeset = false;
	m_initialized = false;

	// check workspace content
	if ( !w->pdf(pdfName) ) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << pdfName  << endl; exit(1); }
	if ( !w->set(obsName) ) { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << obsName << endl; exit(1); }
	if ( !w->set(parsName) ){ cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << parsName << endl; exit(1); }
	if ( !w->set(thName) )  { cout << "MethodAbsScan::MethodAbsScan() : ERROR : not found in workspace : " << thName << endl; exit(1); }
}

MethodAbsScan::~MethodAbsScan()
{
	for ( int i=0; i<allResults.size(); i++ ){
		if ( allResults[i] ) delete allResults[i];
	}
	if ( hCL ) delete hCL;
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

	fixParameters(w, obsName);      ///< fix observables
	floatParameters(w, parsName);   ///< physics parameters need to be floating to find global minimum

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
		hCL2d      = new TH2F("hCL2d"+getUniqueRootName(),      "hCL2d"+pdfName, nPoints2dx, min1, max1, nPoints2dy, min2, max2);
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
	if ( scanVar2!="" ) hCL2d->Write("hCL");
	else hCL->Write("hCL");
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
	// cout << "MethodAbsScan::interpolateSimple(): i=" << i << " y=" << y << endl;
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

	// compute pol2 fit interpolation
	TGraph *g = new TGraph(3);
	g->SetPoint(0, h->GetBinCenter(i-1), h->GetBinContent(i-1));
	g->SetPoint(1, h->GetBinCenter(i),   h->GetBinContent(i));
	g->SetPoint(2, h->GetBinCenter(i+1), h->GetBinContent(i+1));

	// see if we can add a 4th and 5th point
	if ( (h->GetBinContent(i-2) < h->GetBinContent(i-1) && h->GetBinContent(i-1) < h->GetBinContent(i))
			|| (h->GetBinContent(i-2) > h->GetBinContent(i-1) && h->GetBinContent(i-1) > h->GetBinContent(i)) )
	{
		// add to the beginning
		TGraph *gNew = new TGraph(g->GetN()+1);
		gNew->SetPoint(0, h->GetBinCenter(i-2), h->GetBinContent(i-2));
		Double_t pointx, pointy;
		for ( int i=0; i<g->GetN(); i++)
		{
			g->GetPoint(i, pointx, pointy);
			gNew->SetPoint(i+1, pointx, pointy);
		}
		delete g;
		g = gNew;
	}

	if ( (h->GetBinContent(i+2) < h->GetBinContent(i+1) && h->GetBinContent(i+1) < h->GetBinContent(i))
			|| (h->GetBinContent(i+2) > h->GetBinContent(i+1) && h->GetBinContent(i+1) > h->GetBinContent(i)) )
	{
		// add to the end
		g->Set(g->GetN()+1);
		g->SetPoint(g->GetN()-1, h->GetBinCenter(i+2), h->GetBinContent(i+2));
	}

	// debug: show fitted 1-CL histogram
	// if ( y>0.1 )
	// if ( methodName == TString("Plugin") && y<0.1 )
	// {
	//   TString debugTitle = methodName + Form(" y=%.2f ",y);
	//   debugTitle += upper?Form("%f upper",central):Form("%f lower",central);
	//   TCanvas *c = newNoWarnTCanvas(getUniqueRootName(), debugTitle);
	//   g->SetMarkerStyle(3);
	//   g->SetHistogram(h);
	//   h->Draw();
	//   g->Draw("p");
	// }

	TF1 *f1 = new TF1("f1", "pol2", h->GetBinCenter(i-2), h->GetBinCenter(i+2));
	g->Fit("f1", "q");    // fit linear to get decent start parameters
	g->Fit("f1", "qf+");  // refit with minuit to get more correct errors (TGraph fit errors bug)
	float p[3], e[3];
	for ( int ii=0; ii<3; ii++ )
	{
		p[ii] = f1->GetParameter(ii);
		e[ii] = f1->GetParError(ii);
	}

	float sol0 = pq(p[0], p[1], p[2], y, 0);
	float sol1 = pq(p[0], p[1], p[2], y, 1);
	// cout << upper << " ";
	// printf("%f %f %f\n", central, sol0, sol1);
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
void MethodAbsScan::calcCLintervals()
{
	if ( arg->isQuickhack(8) ){
		// \todo Switch to the new CLIntervalMaker mechanism. It can be activated
		// already using --qh 8, but it really is in beta stage still
		cout << "\nMethodAbsScan::calcCLintervals() : USING NEW CLIntervalMaker for " << name << endl << endl;
		CLIntervalMaker clm(arg, *hCL);
		clm.findMaxima(0.04); // ignore maxima under pvalue=0.04
		for ( int iSol=0; iSol<solutions.size(); iSol++ ){
			float sol = getScanVar1Solution(iSol);
			clm.provideMorePreciseMaximum(sol, "max PLH");
		}
		clm.calcCLintervals();
		// print
		TString unit = w->var(scanVar1)->getUnit();
		CLIntervalPrinter clp(arg, name, scanVar1, unit, methodName);
		clp.setDegrees(isAngle(w->var(scanVar1)));
		clp.addIntervals(clm.getClintervals1sigma());
		clp.addIntervals(clm.getClintervals2sigma());
		clp.print();
		cout << endl;
	}

	cout << endl;
	if ( arg->debug ) cout << "MethodAbsScan::calcCLintervals() : ";
	cout << "CONFIDENCE INTERVALS for combination " << name << endl << endl;
	clintervals1sigma.clear(); // clear, else calling this function twice doesn't work
	clintervals2sigma.clear();
  clintervals3sigma.clear();
	int n = hCL->GetNbinsX();
	RooRealVar* par = w->var(scanVar1);

	for ( int iSol=0; iSol<solutions.size(); iSol++ )
	{
		float CL[3]      = {0.6827, 0.9545, 0.9973 };
		float CLhi[3]    = {0.0};
		float CLhiErr[3] = {0.0};
		float CLlo[3]    = {0.0};
		float CLloErr[3] = {0.0};

		for ( int c=0; c<3; c++ )
		{
			CLlo[c] = hCL->GetXaxis()->GetXmin();
			CLhi[c] = hCL->GetXaxis()->GetXmax();
			float y = 1.-CL[c];
			float sol = getScanVar1Solution(iSol);
			int sBin = hCL->FindBin(sol);

			// find lower interval bound
			for ( int i=sBin; i>0; i-- ){
				if ( hCL->GetBinContent(i) < y ){
					if ( n>25 ){
						bool check = interpolate(hCL, i, y, sol, false, CLlo[c], CLloErr[c]);
						if ( !check || CLlo[c]!=CLlo[c] ) interpolateSimple(hCL, i, y, CLlo[c]);
					}
					else{
						interpolateSimple(hCL, i, y, CLlo[c]);
					}
					break;
				}
			}

			// find upper interval bound
			for ( int i=sBin; i<n; i++ ){
				if ( hCL->GetBinContent(i) < y ){
					if ( n>25 ){
						bool check = interpolate(hCL, i-1, y, sol, true, CLhi[c], CLhiErr[c]);
						if ( CLhi[c]!=CLhi[c] ) interpolateSimple(hCL, i-1, y, CLhi[c]);
					}
					else{
						interpolateSimple(hCL, i-1, y, CLhi[c]);
					}
					break;
				}
			}

			// save interval if solution is contained in it
			if ( hCL->GetBinContent(sBin)>y )
			{
				CLInterval cli;
				cli.pvalue = 1.-CL[c];
				cli.min = CLlo[c];
				cli.max = CLhi[c];
				cli.central = sol;
				if ( c==0 ) clintervals1sigma.push_back(cli);
				if ( c==1 ) clintervals2sigma.push_back(cli);
				if ( c==2 ) clintervals3sigma.push_back(cli);
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

	printCLintervals();

	//
	// scan again from the histogram boundaries
	//
	for ( int iBoundary=0; iBoundary<2; iBoundary++ )
	{
		float CL[2]      = {0.6827, 0.9545};
		float CLhi[2]    = {0.0};
		float CLhiErr[2] = {0.0};
		float CLlo[2]    = {0.0};
		float CLloErr[2] = {0.0};

		for ( int c=0; c<2; c++ )
		{
			CLlo[c] = hCL->GetXaxis()->GetXmin();
			CLhi[c] = hCL->GetXaxis()->GetXmax();
			float y = 1.-CL[c];

			if ( iBoundary==1 )
			{
				// find lower interval bound
				if ( hCL->GetBinContent(n)<y ) continue;  ///< skip if p-value is too low at boundary
				for ( int i=n; i>0; i-- )
				{
					if ( hCL->GetBinContent(i) > y )
					{
						if ( n>25 ) interpolate(hCL, i, y, hCL->GetXaxis()->GetXmax(), false, CLlo[c], CLloErr[c]);
						else        interpolateSimple(hCL, i, y, CLlo[c]);
						break;
					}
				}
			}
			else
			{
				// find upper interval bound
				if ( hCL->GetBinContent(1)<y ) continue;  ///< skip if p-value is too low at boundary
				for ( int i=1; i<n; i++ )
				{
					if ( hCL->GetBinContent(i) > y )
					{
						if ( n>25 ) interpolate(hCL, i-1, y, hCL->GetXaxis()->GetXmin(), true, CLhi[c], CLhiErr[c]);
						else        interpolateSimple(hCL, i-1, y, CLhi[c]);
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
			printf("\n%s = [%7.*f, %7.*f] @%3.2fCL",
					par->GetName(),
					pErr, CLlo[c], pErr, CLhi[c],
					CL[c]);
			if ( CLloErr[c]!=0 || CLhiErr[c]!=0 ) printf(", accuracy = [%1.5f, %1.5f]", CLloErr[c], CLhiErr[c]);
			if ( unit!="" ) cout << ", ["<<unit<<"]";
			cout << ", " << methodName << " (boundary scan)" << endl;
		}
	}

	cout << endl;
}

///
/// Print CL intervals.
///
void MethodAbsScan::printCLintervals()
{
	TString unit = w->var(scanVar1)->getUnit();
	CLIntervalPrinter clp(arg, name, scanVar1, unit, methodName);
	clp.setDegrees(isAngle(w->var(scanVar1)));
	clp.addIntervals(clintervals1sigma);
	clp.addIntervals(clintervals2sigma);
	clp.addIntervals(clintervals3sigma);
	clp.print();
	clp.savePython();
	cout << endl;

	// print solutions not contained in the 1sigma and 2sigma intervals
	for ( int i=0; i<getNSolutions(); i++ )
	{
		float sol = getScanVar1Solution(i);
		bool cont=false;
		for ( int j=0; j<clintervals1sigma.size(); j++ ) if ( clintervals1sigma[j].min<sol && sol<clintervals1sigma[j].max ) cont=true;
		for ( int j=0; j<clintervals2sigma.size(); j++ ) if ( clintervals2sigma[j].min<sol && sol<clintervals2sigma[j].max ) cont=true;
		for ( int j=0; j<clintervals3sigma.size(); j++ ) if ( clintervals3sigma[j].min<sol && sol<clintervals3sigma[j].max ) cont=true;
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
CLInterval MethodAbsScan::getCLintervalCentral(int sigma)
{
	if ( clintervals1sigma.size()==0 ) calcCLintervals();
	if ( clintervals1sigma.size()==0 ){
		// no constraint at 1sigma, return full range.
		assert(hCL);
		CLInterval i;
		i.pvalue 	= 0.0;
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

	// compute largest interval
	if ( arg->largest ){
		CLInterval i;
		i.pvalue = intervals[0].pvalue;
		i.min = intervals[0].min;
		for ( int j=0; j<intervals.size(); j++ ) i.min = TMath::Min(i.min, intervals[j].min);
		i.max = intervals[0].max;
		for ( int j=0; j<intervals.size(); j++ ) i.max = TMath::Max(i.max, intervals[j].max);
		return i;
	}

	// the first entry corresponds to the central value!
	return intervals[0];
}


float MethodAbsScan::getCL(double val)
{
	return 1.-hCL->Interpolate(val);
}


void MethodAbsScan::plotOn(OneMinusClPlotAbs *plot)
{
	plot->addScanner(this);
}


RooRealVar* MethodAbsScan::getScanVar1()
{
	return w->var(scanVar1);
}

TString	MethodAbsScan::getScanVar1Name()
{
	return scanVar1;
}

RooRealVar* MethodAbsScan::getScanVar2()
{
	return w->var(scanVar2);
}

TString	MethodAbsScan::getScanVar2Name()
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

