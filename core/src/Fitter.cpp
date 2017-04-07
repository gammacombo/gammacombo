#include "Fitter.h"

Fitter::Fitter(OptParser *arg, RooWorkspace *w, TString name)
{
  this->w = w;
  this->name = name;
  this->arg = arg;
  
  startparsFirstFit = 0;
  startparsSecondFit = 0;
  nFit1Best = 0;
  nFit2Best = 0;
  pdfName  = "pdf_"+name;
  obsName  = "obs_"+name;
  parsName = "par_"+name;
  theResult = 0;
}

Fitter::~Fitter()
{}

///
/// Perform two fits, each time using different start parameters,
/// retain the smallest chi2. Note: To debug the start paramter
/// issues, remember that fitToMinBringBackAngles() may perform a
/// second fit, which then uses the start parameters of the first.
/// This will show up in the RooFitResult.
///
void Fitter::fitTwice(){
  // first fit
	setParametersFloating(w, parsName, startparsFirstFit);
  RooFitResult *r1 = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
  bool f1failed = !(r1->edm()<1 && r1->covQual()==3);
  
  // second fit
  setParametersFloating(w, parsName, startparsSecondFit);
  RooFitResult *r2 = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
  bool f2failed = !(r2->edm()<1 && r2->covQual()==3);
  
  if ( f1failed && f2failed )
  {
    theResult = r1;
    delete r2;
  }  
  else if ( f1failed )
  {
    nFit2Best++;
    theResult = r2;
    delete r1;
  }
  else if ( f2failed )
  {
    nFit1Best++;
    theResult = r1;
    delete r2;
  }
  else if ( r1->minNll() < r2->minNll() )
  {
    nFit1Best++;
    theResult = r1;
    delete r2;
  }
  else
  {
    nFit2Best++;
    theResult = r2;
    delete r1;
  }
  
  setParametersFloating(w, parsName, theResult);
}

///
/// Force minimum finding. Will use the start parameters set by
/// setStartparsFirstFit().
///
void Fitter::fitForce()
{
  setParametersFloating(w, parsName, startparsFirstFit);
  theResult = fitToMinForce(w, name);
  setParametersFloating(w, parsName, theResult);
}

///
/// Returns minimum chi2 value obtained by fit().
/// \return min chi2; 1e6 if fit wasn't performed yet
///
float Fitter::getChi2()
{
  if (!theResult) assert(0);
  if (theResult->minNll()<-10) return -10;  ///< else we have many entries at -1e27 in the ToyTree
  return theResult->minNll();
}

///
/// Check the fit result. We require a good covariance
/// matrix and a reasonable EDM for a status "ok".
/// \return Status code: 0=ok, 1=error, -1=fit() didn't run
///
int Fitter::getStatus()
{
  if ( !theResult ) return -1;
  if ( theResult->floatParsFinal().getSize()==0 ) return 0;
  if ( theResult->edm()<1 && theResult->status()==0 && theResult->covQual()==3 ) return 0;
  // theResult->Print("v");
  return 1;
}

///
/// Run the fit. Select a fit method here.
///
void Fitter::fit()
{
  if ( theResult ) delete theResult;
  if ( arg->scanforce ) fitForce();
  else fitTwice();
}

void Fitter::print()
{
  cout << "Fitter: nFit1Best=" << nFit1Best << " nFit2Best=" << nFit2Best << endl;
}
