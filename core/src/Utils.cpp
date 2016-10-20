/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "Utils.h"

int Utils::countFitBringBackAngle;      ///< counts how many times an angle needed to be brought back
int Utils::countAllFitBringBackAngle;   ///< counts how many times fitBringBackAngle() was called

///
/// Fit PDF to minimum.
/// \param pdf The PDF.
/// \param thorough Activate Hesse and Minos
/// \param printLevel -1 = no output, 1 verbose output
///
RooFitResult* Utils::fitToMin(RooAbsPdf *pdf, bool thorough, int printLevel)
{
	RooMsgService::instance().setGlobalKillBelow(ERROR);

	RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*pdf));
	bool quiet = printLevel<0;
	RooMinuit m(ll);
	if (quiet){
		m.setPrintLevel(-2);
		m.setNoWarn();
	}
	else m.setPrintLevel(1);
	// if (quiet) m.setLogFile();
	m.setErrorLevel(1.0);
	m.setStrategy(2);
	m.setProfile(0); // 1 enables migrad timer
	unsigned long long start = rdtsc();
	int status = m.migrad();
	// m.simplex();
	// m.migrad();
	// m.simplex();
	if (thorough){
		m.hesse();
		// MINOS seems to fail in more complicated scenarios
		// Can't just use m.minos() because there's a cout that cannot be turned off (root 5-34-03).
		// It's not there when we run minos only on selected parameters- so select them all!
		// //m.minos();
		// RooArgSet floatingPars;
		// TIterator* it = pdf->getVariables()->createIterator();
		// while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		//   if ( !p->isConstant() ) floatingPars.add(*p);
		// }
		// delete it;
		// m.minos(floatingPars);
		// IMPROVE doesn't really improve much
		// //m.improve();
	}
	unsigned long long stop = rdtsc();
	if (!quiet) std::printf("Fit took %llu clock cycles.\n", stop - start);
	RooFitResult *r = m.save();
	// if (!quiet) r->Print("v");
	RooMsgService::instance().setGlobalKillBelow(INFO);
	return r;
}

///
/// Return an equivalent angle between 0 and 2pi.
/// \param angle Angle that is possibly smaller than 0 or larger than 2pi.
///
double Utils::bringBackAngle(double angle)
{
	double val = fmod(angle, 2.*TMath::Pi());
	if ( val<0.0 ) val = val + 2.*TMath::Pi();
	return val;
}

///
/// Compute the difference between 2 angles. This will never be
/// larger than pi because they wrap around!
///
/// \param angle1 - first angle
/// \param angle2 - second angle
/// \return difference
///
double Utils::angularDifference(double angle1, double angle2)
{
	float angleSmaller = max(bringBackAngle(angle1), bringBackAngle(angle2));
	float angleLarger = min(bringBackAngle(angle1), bringBackAngle(angle2));
	float diff1 = angleLarger-angleSmaller;
	float diff2 = (2.*TMath::Pi()-angleLarger) + angleSmaller;
	return min(diff1, diff2);
}

///
/// Fit a pdf to the minimum, but keep angular parameters in a range of
/// [0,2pi]. If after an initial fit, a parameter has walked outside this
/// interval, add multiples of 2pi to bring it back. Then, refit.
/// All variables that have unit 'rad' are taken to be angles.
///
RooFitResult* Utils::fitToMinBringBackAngles(RooAbsPdf *pdf, bool thorough, int printLevel)
{
	countAllFitBringBackAngle++;
	RooFitResult* r = fitToMin(pdf, thorough, printLevel);
	bool refit = false;
	TIterator* it = r->floatParsFinal().createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		if ( ! isAngle(p) ) continue;
		if ( p->getVal()<0.0 || p->getVal()>2.*TMath::Pi() ){
			RooArgSet *pdfPars = pdf->getParameters(RooArgSet());
			RooRealVar *pdfPar = (RooRealVar*)pdfPars->find(p->GetName());
			pdfPar->setVal(bringBackAngle(p->getVal()));
			refit = true;
			delete pdfPars;
		}
	}
	if ( refit ){
		countFitBringBackAngle++;
		delete r;
		r = fitToMin(pdf, thorough, printLevel);
	}
	delete it;
	return r;
}

///
/// Find the global minimum in a more thorough way.
/// First fit with external start parameters, then
/// for each parameter that starts with "d" or "r" (typically angles and ratios):
///   - at upper scan range, rest at start parameters
///   - at lower scan range, rest at start parameters
/// This amounts to a maximum of 1+2^n fits, where n is the number
/// of parameters to be varied.
///
/// \param w Workspace holding the pdf.
/// \param name Name of the pdf without leading "pdf_".
/// \param forceVariables Apply the force method for these variables only. Format
/// "var1,var2,var3," (list must end with comma). Default is to apply for all angles,
/// all ratios except rD_k3pi and rD_kpi, and the k3pi coherence factor.
///
RooFitResult* Utils::fitToMinForce(RooWorkspace *w, TString name, TString forceVariables)
{
	bool debug = true;

	TString parsName = "par_"+name;
	TString obsName  = "obs_"+name;
	TString pdfName  = "pdf_"+name;
	RooFitResult *r = 0;
	int printlevel = -1;
	RooMsgService::instance().setGlobalKillBelow(ERROR);

	// save start parameters
	if ( !w->set(parsName) ){
		cout << "MethodProbScan::scan2d() : ERROR : parsName not found: " << parsName << endl;
		exit(1);
	}
	RooDataSet *startPars = new RooDataSet("startParsForce", "startParsForce", *w->set(parsName));
	startPars->add(*w->set(parsName));

	// set up parameters and ranges
	RooArgList *varyPars = new RooArgList();
	TIterator* it = w->set(parsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() )
	{
		if ( p->isConstant() ) continue;
		if ( forceVariables=="" && ( false
					|| TString(p->GetName()).BeginsWith("d") ///< use these variables
					// || TString(p->GetName()).BeginsWith("r")
					|| TString(p->GetName()).BeginsWith("k")
					|| TString(p->GetName()) == "g"
					) && ! (
						TString(p->GetName()) == "rD_k3pi"  ///< don't use these
						|| TString(p->GetName()) == "rD_kpi"
						// || TString(p->GetName()) == "dD_kpi"
						|| TString(p->GetName()) == "d_dk"
						|| TString(p->GetName()) == "d_dsk"
						))
		{
			varyPars->add(*p);
		}
		else if ( forceVariables.Contains(TString(p->GetName())+",") )
		{
			varyPars->add(*p);
		}
	}
	delete it;
	int nPars = varyPars->getSize();
	if ( debug ) cout << "Utils::fitToMinForce() : nPars = " << nPars << " => " << pow(2.,nPars) << " fits" << endl;
	if ( debug ) cout << "Utils::fitToMinForce() : varying ";
	if ( debug ) varyPars->Print();

	//////////

	r = fitToMinBringBackAngles(w->pdf(pdfName), false, printlevel);

	//////////

	int nErrors = 0;

	// We define a binary mask where each bit corresponds
	// to parameter at max or at min.
	for ( int i=0; i<pow(2.,nPars); i++ )
	{
		if ( debug ) cout << "Utils::fitToMinForce() : fit " << i << "        \r" << flush;
		setParameters(w, parsName, startPars->get(0));

		for ( int ip=0; ip<nPars; ip++ )
		{
			RooRealVar *p = (RooRealVar*)varyPars->at(ip);
			float oldMin = p->getMin();
			float oldMax = p->getMax();
			setLimit(w, p->GetName(), "force");
			if ( i/(int)pow(2.,ip) % 2==0 ) { p->setVal(p->getMin()); }
			if ( i/(int)pow(2.,ip) % 2==1 ) { p->setVal(p->getMax()); }
			p->setRange(oldMin, oldMax);
		}

		// check if start parameters are sensible, skip if they're not
		double startParChi2 = getChi2(w->pdf(pdfName));
		if ( startParChi2>2000 ){
			nErrors += 1;
			continue;
		}

		// refit
		RooFitResult *r2 = fitToMinBringBackAngles(w->pdf(pdfName), false, printlevel);

		// In case the initial fit failed, accept the second one.
		// If both failed, still select the second one and hope the
		// next fit succeeds.
		if ( !(r->edm()<1 && r->covQual()==3) ){
			delete r;
			r = r2;
		}
		else if ( r2->edm()<1 && r2->covQual()==3 && r2->minNll()<r->minNll() ){
			// better minimum found!
			delete r;
			r = r2;
		}
		else{
			delete r2;
		}
	}

	if ( debug ) cout << endl;
	if ( debug ) cout << "Utils::fitToMinForce() : nErrors = " << nErrors << endl;

	RooMsgService::instance().setGlobalKillBelow(INFO);

	// (re)set to best parameters
	setParameters(w, parsName, r);

	delete startPars;
	return r;
}

///
/// Fit to the minimum using an improve method.
/// The idea is to kill known minima. For this, we add a multivariate
/// Gaussian to the chi2 at the position of the minimum. Ideally it should
/// be a parabola shaped function, but the Gaussian is readily available.
/// A true parabola like in the original IMPROVE algorithm doesn't work
/// because apparently it affects the other regions of
/// the chi2 too much. There are two parameters to tune: the width of the
/// Gaussian, which is controlled by the error definition of the first fit,
/// and the height of the Gaussian, which is set to 16 (=4 sigma).
/// Three fits are performed: 1) initial fit, 2) fit with improved FCN,
/// 3) fit of the non-improved FCN using the result from step 2 as the start
/// parameters.
/// So far it is only available for the Prob method, via the probimprove
/// command line flag.
///
RooFitResult* Utils::fitToMinImprove(RooWorkspace *w, TString name)
{
	TString parsName = "par_"+name;
	TString obsName  = "obs_"+name;
	TString pdfName  = "pdf_"+name;
	int printlevel = -1;
	RooMsgService::instance().setGlobalKillBelow(ERROR);

	// step 1: find a minimum to start with
	RooFitResult *r1 = 0;
	{
		RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*w->pdf(pdfName)));
		// RooFitResult* r1 = fitToMin(&ll, printlevel);
		RooMinuit m(ll);
		m.setPrintLevel(-2);
		m.setNoWarn();
		m.setErrorLevel(4.0); ///< define 2 sigma errors. This will make the hesse PDF 2 sigma wide!
		int status = m.migrad();
		r1 = m.save();
		// if ( 102<RadToDeg(w->var("g")->getVal())&&RadToDeg(w->var("g")->getVal())<103 )
		// {
		//   cout << "step 1" << endl;
		//   r1->Print("v");
		//   gStyle->SetPalette(1);
		//   float xmin = 0.;
		//   float xmax = 3.14;
		//   float ymin = 0.;
		//   float ymax = 0.2;
		//   TH2F* histo = new TH2F("histo", "histo", 100, xmin, xmax, 100, ymin, ymax);
		//   for ( int ix=0; ix<100; ix++ )
		//   for ( int iy=0; iy<100; iy++ )
		//   {
		//     float x = xmin + (xmax-xmin)*(double)ix/(double)100;
		//     float y = ymin + (ymax-ymin)*(double)iy/(double)100;
		//     w->var("d_dk")->setVal(x);
		//     w->var("r_dk")->setVal(y);
		//     histo->SetBinContent(ix+1,iy+1,ll.getVal());
		//   }
		//   newNoWarnTCanvas("c2");
		//   histo->GetZaxis()->SetRangeUser(0,20);
		//   histo->Draw("colz");
		//   setParameters(w, parsName, r1);
		// }
	}

	// step 2: build and fit the improved fcn
	RooFitResult *r2 = 0;
	{
		// create a Hesse PDF, import both PDFs into a new workspace,
		// so that their parameters are linked
		RooAbsPdf* hessePdf = r1->createHessePdf(*w->set(parsName));
		if ( !hessePdf ) return r1;
		// RooWorkspace* wImprove = (RooWorkspace*)w->Clone();
		RooWorkspace* wImprove = new RooWorkspace();
		wImprove->import(*w->pdf(pdfName));
		wImprove->import(*hessePdf);
		hessePdf = wImprove->pdf(hessePdf->GetName());
		RooAbsPdf* fullPdf = wImprove->pdf(pdfName);

		RooFormulaVar ll("ll", "ll", "-2*log(@0) +16*@1", RooArgSet(*fullPdf, *hessePdf));
		// RooFitResult *r2 = fitToMin(&ll, printlevel);
		RooMinuit m(ll);
		m.setPrintLevel(-2);
		m.setNoWarn();
		m.setErrorLevel(1.0);
		int status = m.migrad();
		r2 = m.save();

		// if ( 102<RadToDeg(w->var("g")->getVal())&&RadToDeg(w->var("g")->getVal())<103 )
		// {
		//   cout << "step 3" << endl;
		//   r2->Print("v");
		//
		//   gStyle->SetPalette(1);
		//   float xmin = 0.;
		//   float xmax = 3.14;
		//   float ymin = 0.;
		//   float ymax = 0.2;
		//   TH2F* histo = new TH2F("histo", "histo", 100, xmin, xmax, 100, ymin, ymax);
		//   for ( int ix=0; ix<100; ix++ )
		//   for ( int iy=0; iy<100; iy++ )
		//   {
		//     float x = xmin + (xmax-xmin)*(double)ix/(double)100;
		//     float y = ymin + (ymax-ymin)*(double)iy/(double)100;
		//     wImprove->var("d_dk")->setVal(x);
		//     wImprove->var("r_dk")->setVal(y);
		//     histo->SetBinContent(ix+1,iy+1,ll.getVal());
		//   }
		//   newNoWarnTCanvas("c7");
		//   histo->GetZaxis()->SetRangeUser(0,20);
		//   histo->Draw("colz");
		//   // setParameters(wImprove, parsName, r1);
		// }
		delete wImprove;
	}

	// step 3: use the fit result of the improved fit as
	// start parameters for the nominal fcn
	RooFitResult* r3;
	{
		setParameters(w, parsName, r2);
		RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*w->pdf(pdfName)));
		RooMinuit m(ll);
		m.setPrintLevel(-2);
		m.setNoWarn();
		m.setErrorLevel(1.0);
		int status = m.migrad();
		r3 = m.save();
		// if ( 102<RadToDeg(w->var("g")->getVal())&&RadToDeg(w->var("g")->getVal())<103 )
		// {
		//   cout << "step 3" << endl;
		//   r3->Print("v");
		// }
	}

	// step 5: chose better minimum
	// cout << r1->minNll() << " " << r3->minNll() << endl;
	RooFitResult* r = 0;
	if ( r1->minNll()<r3->minNll() )
	{
		delete r3;
		r = r1;
	}
	else
	{
		delete r1;
		r = r3;
		// cout << "Utils::fitToMinImprove() : improved fit is better!" << endl;
	}

	RooMsgService::instance().setGlobalKillBelow(INFO);

	// set to best parameters
	setParameters(w, parsName, r);
	return r;
}


double Utils::getChi2(RooAbsPdf *pdf)
{
	RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*pdf));
	return ll.getVal();
}

//
// Randomize all parameters of a set defined in a given
// workspace.
//
void Utils::randomizeParameters(RooWorkspace* w, TString setname)
{
	TIterator* it = w->set(setname)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		if ( p->isConstant() ) continue;
		// sample from uniform distribution
		p->randomize();
	}
}

//
// Randomize all parameters according to a Gaussian centered on their
// best fit value with a width of their uncertainty
//
void Utils::randomizeParametersGaussian(RooWorkspace* w, TString setname, RooSlimFitResult *r)
{
  RooArgList list = r->floatParsFinal();
  TIterator* it = list.createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ) {
    RooRealVar *var = (RooRealVar*)w->var( p->GetName() );
    if ( w->set(setname) ) {
      if ( ! w->set(setname)->find( p->GetName() ) ) continue;
    }
    if ( p->isConstant() ) {
      var->setVal( p->getVal() );
    }
    else {
      double randnumb = RooRandom::randomGenerator()->Gaus( p->getVal(), p->getError() );
      // make sure it's in range (if not then throw again)
      while ( randnumb < var->getMin() || randnumb > var->getMax() ) {
        randnumb = RooRandom::randomGenerator()->Gaus( p->getVal(), p->getError() );
      }
      var->setVal( randnumb );
    }
  }
}

//
// Randomize all parameters according to a flat distribution within
// some number of sigma of the best fit value
//
void Utils::randomizeParametersUniform(RooWorkspace* w, TString setname, RooSlimFitResult *r, double sigmaRange)
{
  RooArgList list = r->floatParsFinal();
  TIterator* it = list.createIterator();
  while ( RooRealVar* p = (RooRealVar*)it->Next() ) {
    RooRealVar *var = (RooRealVar*)w->var( p->GetName() );
    if ( p->isConstant() ) {
      var->setVal( p->getVal() );
    }
    else {
      double randnumb = RooRandom::randomGenerator()->Uniform( p->getVal()-sigmaRange*p->getError(), p->getVal()+sigmaRange*p->getError() );
      // make sure it's in range (if not then throw again)
      while ( randnumb < var->getMin() || randnumb > var->getMax() ) {
        randnumb = RooRandom::randomGenerator()->Gaus( p->getVal(), p->getError() );
      }
      var->setVal( randnumb );
    }
  }
}

///
/// Set each parameter in workspace to the values found
/// in the fit result
///
void Utils::setParameters(RooWorkspace* w, RooFitResult* values){
	RooArgList list = values->floatParsFinal();
	list.add(values->constPars());
	TIterator* it = list.createIterator();
	while(RooRealVar* p = (RooRealVar*)it->Next()){
		RooRealVar* var = dynamic_cast<RooRealVar*>(w->allVars().find(p->GetName()));
		if(!(var)){
			std::cout <<  "WARNING in Utils::setParameters(RooWorkspace,RooFitResult) -- no Var found with name "
				<< p->GetName() << " in Workspace!" << endl;
		}
		else{
			var->setVal(p->getVal());
		}
	}
	return;
};

///
/// Set each parameter in setMe to the value found in values.
/// Do nothing if parameter is not found in values.
///
void Utils::setParameters(const RooAbsCollection* setMe, const RooAbsCollection* values)
{
	TIterator* it = setMe->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		RooRealVar *var = (RooRealVar*)values->find(p->GetName());
		if ( var ) p->setVal(var->getVal());
	}
	delete it;
}

///
/// Set each floating parameter in setMe to the value found in values.
/// Do nothing if parameter is not found in values.
///
void Utils::setParametersFloating(const RooAbsCollection* setMe, const RooAbsCollection* values)
{
	TIterator* it = setMe->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		if ( p->isConstant() ) continue;
		RooRealVar *var = (RooRealVar*)values->find(p->GetName());
		if ( var ) p->setVal(var->getVal());
	}
	delete it;
}

///
/// Set each parameter in the named set parname inside workspace w
/// to the value found in set. Do nothing if a parameter is present
/// in the parname set, but not found in set.
/// \param w workspace containing a parameter set of name parname
/// \param parname Name of the parameter set containing the "destination" parameters.
/// \param set parameter set holding the "from" parameters.
///
void Utils::setParameters(RooWorkspace* w, TString parname, const RooAbsCollection* set)
{
	if ( !w->set(parname) ){
		cout << "Utils::setParameters() : ERROR : set not found in workspace: " << parname << endl;
		assert(0);
	}
	setParameters(w->set(parname), set);
}

///
/// Set each floating parameter in the named set parname inside workspace w
/// to the value found in set. Do nothing if a parameter is present
/// in the parname set, but not found in set.
///
/// \param w workspace containing a parameter set of name parname
/// \param parname Name of the parameter set containing the "destination" parameters.
/// \param set parameter set holding the "from" parameters.
///
void Utils::setParametersFloating(RooWorkspace* w, TString parname, const RooAbsCollection* set)
{
	setParametersFloating(w->set(parname), set);
}

///
/// Set each parameter in the named set parname inside workspace w
/// to the value found in the final set of floating (or floating and constant)
/// fit parameters in r.
///
/// \param w - workspace containing a parameter set of name parname
/// \param parname - Name of the parameter set containing the "destination" parameters.
/// \param r - a fit result holding the parameter values to be set
/// \param constAndFloat - If set to true, parameter values will be copied from both
///                        constant and floating fit parameters in the RooFitResult.
///                        Default is false.
///
void Utils::setParameters(RooWorkspace* w, TString parname, RooFitResult* r, bool constAndFloat)
{
	if ( constAndFloat ){
		RooArgList list = r->floatParsFinal();
		list.add(r->constPars());
		setParameters(w, parname, &list);
		return;
	}
	setParameters(w, parname, &(r->floatParsFinal()));
}

void Utils::setParameters(RooWorkspace* w, TString parname, RooSlimFitResult* r, bool constAndFloat)
{
	if ( constAndFloat ){
		RooArgList list = r->floatParsFinal();
		list.add(r->constPars());
		setParameters(w, parname, &list);
		return;
	}
	setParameters(w, parname, &(r->floatParsFinal()));
}

///
/// Set each floating parameter in the named set parname inside workspace w
/// to the value found in the final set of floating fit parameters
/// in r.
///
void Utils::setParametersFloating(RooWorkspace* w, TString parname, RooFitResult* r)
{
	setParametersFloating(w, parname, &(r->floatParsFinal()));
}

///
/// Set each parameter in the named set parname inside workspace w
/// to the value found in the first row of the provided dataset.
///
void Utils::setParameters(RooWorkspace* w, TString parname, RooDataSet* d)
{
	setParameters(w, parname, d->get(0));
}

///
/// Set each floating parameter in the named set parname inside workspace w
/// to the value found in the first row of the provided dataset.
///
void Utils::setParametersFloating(RooWorkspace* w, TString parname, RooDataSet* d)
{
	setParametersFloating(w, parname, d->get(0));
}

///
/// Fix each parameter in the named set "parname" inside workspace "w".
///
void Utils::fixParameters(RooWorkspace* w, TString parname)
{
	if ( w->set(parname) ) fixParameters(w->set(parname));
}

void Utils::fixParameters(const RooAbsCollection* set)
{
	TIterator* it = set->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		p->setConstant(true);
	}
}

///
/// Float each parameter in the named set "parname" inside workspace "w".
///
void Utils::floatParameters(RooWorkspace* w, TString parname)
{
	TIterator* it = w->set(parname)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		p->setConstant(false);
	}
}

void Utils::floatParameters(const RooAbsCollection* set)
{
	TIterator* it = set->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		p->setConstant(false);
	}
}

///
/// Load a named parameter range for a certain parameter.
///
/// \param v - The parameter which will get the limit set.
/// \param limitname - Name of the limit to set.
///
void Utils::setLimit(RooRealVar* v, TString limitname)
{
	RooMsgService::instance().setGlobalKillBelow(ERROR);
	v->setRange(v->getMin(limitname), v->getMax(limitname));
	RooMsgService::instance().setGlobalKillBelow(INFO);
}

///
/// Load a named parameter range for a certain parameter,
/// which is found inside a workspace.
///
/// \param w - The workspace holding the parameter.
/// \param parname - The name of the parameter.
/// \param limitname - Name of the limit to set.
///
void Utils::setLimit(RooWorkspace* w, TString parname, TString limitname)
{
	RooMsgService::instance().setGlobalKillBelow(ERROR);
	w->var(parname)->setRange(w->var(parname)->getMin(limitname), w->var(parname)->getMax(limitname));
	RooMsgService::instance().setGlobalKillBelow(INFO);
}

///
/// Load a named parameter range for a list of parameters.
///
/// \param set - The list holding the parameters.
/// \param limitname - Name of the limit to set.
///
void Utils::setLimit(const RooAbsCollection* set, TString limitname)
{
	RooMsgService::instance().setGlobalKillBelow(ERROR);
	TIterator* it = set->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		p->setRange(p->getMin(limitname), p->getMax(limitname));
	}
	RooMsgService::instance().setGlobalKillBelow(INFO);
}

///
/// Build a full correlation matrix by
/// filling diagonal elements with unity
/// and symmetrizing.
///
void Utils::buildCorMatrix(TMatrixDSym &cor)
{
	// fill diagonals
	for ( int i=0; i<cor.GetNcols(); i++ )
	{
		cor[i][i] = 1.;
	}

	// symmetrize
	for ( int i=0; i<cor.GetNcols(); i++ )
		for ( int j=0; j<cor.GetNcols(); j++ )
		{
			if ( cor[i][j]!=0.0 && cor[j][i]==0.0 ) cor[j][i] = cor[i][j];
			if ( cor[i][j]==0.0 && cor[j][i]!=0.0 ) cor[i][j] = cor[j][i];
		}
}

///
/// Build a covariance matrix
/// from a correlation matrix and error vectors.
///
TMatrixDSym* Utils::buildCovMatrix(TMatrixDSym &cor, float *err)
{
	int n = cor.GetNcols();
	TMatrixDSym cov(n);
	for ( int i=0; i<n; i++ )
		for ( int j=0; j<n; j++ )
		{
			cov[i][j] = err[i] * cor[i][j] * err[j];
		}
	return new TMatrixDSym(cov);
}

///
/// Build a covariance matrix
/// from a correlation matrix and error vectors.
///
TMatrixDSym* Utils::buildCovMatrix(TMatrixDSym &cor, vector<double> &err)
{
	int n = cor.GetNcols();
	TMatrixDSym cov(n);
	for ( int i=0; i<n; i++ )
		for ( int j=0; j<n; j++ ){
			cov[i][j] = err[i] * cor[i][j] * err[j];
		}
	return new TMatrixDSym(cov);
}

///
/// Merge two named sets of variables inside a RooWorkspace.
/// Duplicate variables will only be contained once.
///
void Utils::mergeNamedSets(RooWorkspace *w, TString mergedSet, TString set1, TString set2)
{
	// 1. fill all variables into a vector
	vector<string> varsAll;
	TIterator* it = w->set(set1)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) varsAll.push_back(p->GetName());
	delete it;
	it = w->set(set2)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) varsAll.push_back(p->GetName());
	delete it;

	// 2. remove duplicates
	sort(varsAll.begin(), varsAll.end());
	vector<string> vars;
	vars.push_back(varsAll[0]);
	string previous = varsAll[0];
	for ( int i=1; i<varsAll.size(); i++ ){
		if ( previous==varsAll[i] ) continue;
		vars.push_back(varsAll[i]);
		previous=varsAll[i];
	}

	// 3. make new, combined set on the workspace
	TString varsCommaList = "";
	for ( int i=0; i<vars.size(); i++ ){
		varsCommaList.Append(vars[i]);
		if ( i<vars.size()-1 ) varsCommaList.Append(",");
	}
	w->defineSet(mergedSet, varsCommaList);
}

/*
 * doesn't work with 3GB files.
 *
 */
bool Utils::FileExists( TString strFilename )
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;
	// Attempt to get the file attributes
	intStat = stat( strFilename, &stFileInfo );
	if(intStat == 0) {
		// We were able to get the file attributes
		// so the file obviously exists.
		blnReturn = true;
	} else {
		// We were not able to get the file attributes.
		// This may mean that we don't have permission to
		// access the folder which contains this file. If you
		// need to do that level of checking, lookup the
		// return values of stat which will give you
		// more details on why stat failed.
		blnReturn = false;
	}
	return(blnReturn);
}


void Utils::savePlot(TCanvas *c1, TString name)
{
	cout << "saving plot (pdf and other formats) to: plots/pdf/"+name+".pdf" << endl;
	gErrorIgnoreLevel = kWarning;
	c1->Print("plots/png/"+name+".png");
	c1->Print("plots/pdf/"+name+".pdf");
	c1->Print("plots/eps/"+name+".eps");
	c1->Print("plots/root/"+name+".root");
	c1->Print("plots/C/"+name+".C");
	gErrorIgnoreLevel = kInfo;
}

///
/// Round a number to a certain number of
/// decimal points.
///
float Utils::Round(double value, int digits)
{
	return TString(Form("%.*f", digits, value)).Atof();
}

///
/// Compute number of digits needed behind the decimal
/// point to achieve a certain number of significant digits.
/// The result can be used in the printf() function.
/// Examples:
///   23.243: returns 0 with sigdigits=2
///   2.2634: returns 1 with sigdigits=2
///
int Utils::calcNsubdigits(double value, int sigdigits)
{
	if ( value==0 ) return 1;
	double myvalue = value;
	int count = 0;
	for (; fabs(value)<pow(10.,sigdigits-1); value *= 10)
	{
		if ( count==10 ) break;
		count++;
	}

	// do it again to catch a rounding issue: 9.97 at 2 digit
	// precision gives count=1 above, but we want count=0 to
	// get "10" instead of "10.0"
	value = TString(Form("%.*f",count,myvalue)).Atof();
	if ( value==0 ) return 1;
	count = 0;
	for (; fabs(value)<pow(10.,sigdigits-1); value *= 10)
	{
		if ( count==10 ) break;
		count++;
	}
	return count;
}

///
/// Converts a RooDataSet to a TTree which then can be
/// browsed.
///
TTree* Utils::convertRooDatasetToTTree(RooDataSet *d)
{
	// set up the TTree based on the content of the first
	// row of the dataset
	map<string,float> variables;  ///< the proxy variables
	TTree* t = new TTree("tree", "tree");
	TIterator* it = d->get(0)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		variables.insert(pair<string,float>(p->GetName(),p->getVal()));
		t->Branch(p->GetName(), &variables[p->GetName()], TString(p->GetName())+"/F");
	}
	delete it;

	// loop over the dataset, filling the tree
	int nEntries = d->sumEntries();
	for ( int i=0; i<nEntries; i++ ){
		it = d->get(i)->createIterator();
		while ( RooRealVar* p = (RooRealVar*)it->Next() ){
			variables[p->GetName()] = p->getVal();
		}
		delete it;
		t->Fill();
	}

	return t;
}

///
/// Creates a fresh, independent copy of the input histogram.
/// We cannot use Root's Clone() or the like, because that
/// crap always copies Draw options and whatnot.
///
/// \param h - the input histogram
/// \param copyContent - true: also copy content. false: initialize with zeroes
/// \param uniqueName - true: append a unique string to the histogram name
/// \return a new histogram. Caller assumes ownership.
///
TH1F* Utils::histHardCopy(const TH1F* h, bool copyContent, bool uniqueName)
{
	TString name = h->GetTitle();
	if ( uniqueName ) name += getUniqueRootName();
	TH1F* hNew = new TH1F(name, h->GetTitle(),
			h->GetNbinsX(),
			h->GetXaxis()->GetXmin(),
			h->GetXaxis()->GetXmax());
	for ( int l=1; l<=h->GetNbinsX(); l++ ){
		if ( copyContent ) hNew->SetBinContent(l, h->GetBinContent(l));
		else hNew->SetBinContent(l, 0);
	}
	return hNew;
}

///
/// Creates a fresh, independent copy of the input histogram.
/// 2d version of TH1F* Utils::histHardCopy().
///
TH2F* Utils::histHardCopy(const TH2F* h, bool copyContent, bool uniqueName)
{
	TString name = h->GetTitle();
	if ( uniqueName ) name += getUniqueRootName();
	TH2F* hNew = new TH2F(name, h->GetTitle(),
			h->GetNbinsX(),
			h->GetXaxis()->GetXmin(),
			h->GetXaxis()->GetXmax(),
			h->GetNbinsY(),
			h->GetYaxis()->GetXmin(),
			h->GetYaxis()->GetXmax());
	for ( int k=1; k<=h->GetNbinsX(); k++ )
		for ( int l=1; l<=h->GetNbinsY(); l++ ){
			if ( copyContent ) hNew->SetBinContent(k, l, h->GetBinContent(k,l));
			else hNew->SetBinContent(k, l, 0);
		}
	return hNew;
}

///
/// Check if a matrix is positive definite.
///
bool Utils::isPosDef(TMatrixDSym* c)
{
	TMatrixDSymEigen eigen(*c);
	TVectorD eigenvalues = eigen.GetEigenValues();
	Double_t minEigenVal = eigenvalues[0];
	for ( int k=0; k<c->GetNcols(); k++ ) minEigenVal = TMath::Min(minEigenVal, eigenvalues[k]);
	if ( minEigenVal<0 )
	{
		cout << "isPosDef() : ERROR : Matrix not pos. def." << endl;
		return 0;
	}
	return 1;
}

///
/// Check if a RooRealVar is an angle.
///
bool Utils::isAngle(RooRealVar* v)
{
	return v->getUnit()==TString("Rad") || v->getUnit()==TString("rad");
}

///
/// function filling a RooArgList with parameters within a Workspace
/// parameter names given by an vector with TStrings
///
void Utils::fillArgList(RooArgList* list, RooWorkspace* w, std::vector<TString> names){
	for (std::vector<TString>::iterator it = names.begin() ; it != names.end(); ++it){
		if( ! list->add(*w->var(*it), kTRUE) ){ //> add silent
			std::cout << "WARNING: Utils::fillArgList - Var either already in ArgList: "
				<< list->GetName() << " or the List does own its vars" << endl;
		};
	}
}

///
/// Fills vector with floating pars names
///
void Utils::getParameters(const RooFitResult &result, std::vector<TString> &names){
	RooArgList pars           = result.floatParsFinal();
	TIterator * it            = pars.createIterator();
	while(RooRealVar* p = (RooRealVar*) it->Next()){
		names.push_back(TString(p->GetName()));
	}
};

///
/// finds a variable within a RooArgSet with a specific name sub-string
/// return all variables containing the sub-string in a std::vector
///
std::vector<TString> Utils::getParsWithName(const TString& subString, const RooArgSet& set){
	std::string vars = set.contentsString();
	std::vector<std::string>  _names;
	std::vector<TString>      _results;
	boost::split(_names,vars,boost::is_any_of(","));
	for( std::string str : _names){
		std::size_t found = str.find(subString);
		if(found != std::string::npos){
			_results.push_back(TString(str));
		}
	}
	return _results;
}
///
/// searches for unphysical values in a fit result, fills problematic var names in vectors
/// returns true if everything is within boundaries
///
bool Utils::checkBoundary(const RooSlimFitResult& r, std::vector<TString> lowProb, std::vector<TString> highProb){
	// make sure vectors are empty
	lowProb.clear(); highProb.clear();
	RooArgList floats = r.floatParsFinal();
	TIterator* floatIt = floats.createIterator();
	while(RooRealVar* var = (RooRealVar*)floatIt->Next()){
		//check lower bound
		if(var->getVal() < var->getMin("phys")){
			lowProb.push_back(var->GetName());
		}
		// check higher bound
		if( var->getVal() > var->getMax("phys")){
			highProb.push_back(var->GetName());
		}
	}
	return (lowProb.size() != 0 || highProb.size() != 0) ? false : true;
}

void Utils::setParsConstToBound(RooWorkspace* w, std::vector<TString> namesLow, std::vector<TString> namesHigh){
	setParsConstToBound(w, namesLow, true);
	setParsConstToBound(w, namesHigh, false);
};


void Utils::setParsConstToBound(RooWorkspace* w, std::vector<TString> names, bool low){
	if(names.size() == 0) return;
	for(auto& n : names){
		double_t valueToSet = (low)? w->var(n)->getMin("phys") : w->var(n)->getMax("phys");
		w->var(n)->setVal(valueToSet);
		w->var(n)->setConstant(kTRUE);
	}
}
void Utils::setParametersFloating(RooWorkspace* w, std::vector<TString> names, std::vector<TString> names2){
	setParametersFloating(w, names);
	setParametersFloating(w, names2);
};
void Utils::setParametersFloating(RooWorkspace* w, std::vector<TString> names){
	if(names.size() == 0) return;
	for(auto& n : names){
		w->var(n)->setConstant(kFALSE);
	}
};

///
/// Debug tools: print the content of a vector to stdout.
///
void Utils::dump_vector(const std::vector<int>& l) {
	for ( std::vector<int>::const_iterator it = l.begin(); it != l.end(); it++ ) {
		cout << *it << endl;
	}
}
void Utils::dump_vector(const std::vector<float>& l) {
	for ( std::vector<float>::const_iterator it = l.begin(); it != l.end(); it++ ) {
		cout << *it << endl;
	}
}
void Utils::dump_matrix(const std::vector<std::vector<int> >& l) {
	for ( int ix=0; ix<l.size(); ix++ ) {
		for ( int iy=0; iy<l[0].size(); iy++ ) {
			cout << printf("%5i",l[ix][iy]) << " ";
		}
		cout << endl;
	}
}

///
/// Debug tools: print the content of a 2d map to stdout.
///
void Utils::dump_map(const std::map<int, std::vector<int> >& map) {
	for ( std::map<int,std::vector<int> >::const_iterator it = map.begin(); it != map.end(); it++) {
		cout << "Key: " << it->first << endl;
		cout << "Values" << endl;
		dump_vector(it->second);
	}
}

std::vector<std::vector<int> > Utils::transpose(std::vector<std::vector<int> >& v)
{
	std::vector<std::vector<int> > newVector;
	int oldNx = v.size();
	if ( oldNx==0 ){
		cout << "Utils::transpose() : ERROR : x dimension is 0" << endl;
		return newVector;
	}
	int oldNy = v[0].size();
	if ( oldNy==0 ){
		cout << "Utils::transpose() : ERROR : y dimension is 0" << endl;
		return newVector;
	}
	// check if rectangular
	for ( int j=1; j<oldNx; j++ ){
		if ( v[j].size()!=oldNy ){
			cout << "Utils::transpose() : ERROR : vector not rectangular" << endl;
			return newVector;
		}
	}
	// transpose
	for ( int iy=0; iy<oldNy; iy++ ){
		std::vector<int> tmp;
		for ( int ix=0; ix<oldNx; ix++ ){
			tmp.push_back(v[ix][iy]);
		}
		newVector.push_back(tmp);
	}
	return newVector;
}

TCanvas* Utils::newNoWarnTCanvas(TString name, TString title, int width, int height)
{
	gErrorIgnoreLevel=kError;
	TCanvas* c = new TCanvas(name, title, width, height);
	gErrorIgnoreLevel=kInfo;
	return c;
}

TCanvas* Utils::newNoWarnTCanvas(TString name, TString title, int x, int y, int width, int height)
{
	gErrorIgnoreLevel=kError;
	TCanvas* c = new TCanvas(name, title, x, y, width, height);
	gErrorIgnoreLevel=kInfo;
	return c;
}

void Utils::HFAGLabel(const TString& label, Double_t xpos, Double_t ypos, Double_t scale)
{
  TVirtualPad* thePad;

  if ((thePad = TVirtualPad::Pad()) == 0) return;

  UInt_t pad_width(thePad->XtoPixel(thePad->GetX2()));
  UInt_t pad_height(thePad->YtoPixel(thePad->GetY1()));

  Double_t ysiz_pixel(25);
  Double_t ysiz(Double_t(ysiz_pixel)/Double_t(pad_height));
  Double_t xsiz(4.2*ysiz*Double_t(pad_height)/Double_t(pad_width));

  Double_t x1, x2, y1, y2;
  xsiz = scale*xsiz;
  ysiz = scale*ysiz;

  if (xpos >= 0) {
    x1 = xpos;
    x2 = xpos + xsiz;
  } else {
    x1 = 1 + xpos - xsiz;
    x2 = 1 + xpos;
  }

  if (ypos >= 0) {
    y1 = ypos+0.9*ysiz;
    y2 = ypos+0.9*ysiz + ysiz;
  } else {
    y1 = 1 + ypos - ysiz;
    y2 = 1 + ypos;
  }

  TPaveText *tbox1 = new TPaveText(x1, y1, x2, y2, "BRNDC");
  // tbox1->SetLineColor(1);
  // tbox1->SetLineStyle(1);
  // tbox1->SetLineWidth(2);
  tbox1->SetFillColor(kBlack);
  tbox1->SetFillStyle(1001);
  // tbox1->SetBorderSize(1);
  tbox1->SetShadowColor(kWhite);
  tbox1->SetTextColor(kWhite);
  tbox1->SetTextFont(76);
  tbox1->SetTextSize(24*scale);
  tbox1->SetTextAlign(22); //center-adjusted and vertically centered
  tbox1->AddText(TString("HFAG"));
  tbox1->Draw();
  //
  TPaveText *tbox2 = new TPaveText(x1, y1-0.9*ysiz, x2, y2-ysiz, "BRNDC");
  // tbox2->SetLineColor(1);
  // tbox2->SetLineStyle(1);
  // tbox2->SetLineWidth(2);
  tbox2->SetFillColor(kWhite);
  tbox2->SetFillStyle(1001);
  // tbox2->SetBorderSize(1);
  tbox2->SetShadowColor(kWhite);
  tbox2->SetTextColor(kBlack);
  tbox2->SetTextFont(76);
  tbox2->SetTextSize(18*scale);
  tbox2->SetTextAlign(22); //center-adjusted and vertically centered
  tbox2->AddText(label);
  tbox2->Draw();
  return;

}

