/**
 * Author: Matthew Kenzie <matthew.kenzie@cern.ch>
 * Date: December 2020
 *
 **/

#ifndef PDF_GGSZ_h
#define PDF_GGSZ_h

#include "PDF_Abs.h"
#include "ParametersGamma.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_GGSZ : public PDF_Abs
{
public:
  PDF_GGSZ(TString cObs, TString cErr, TString cCor);
  ~PDF_GGSZ();
  void          buildPdf();
  void          initObservables();
  virtual void  initParameters();
  virtual void  initRelations();
  void          setCorrelations(TString c);
  void          setObservables(TString c);
  void          setUncertainties(TString c);

protected:
  ParametersAbs*  p;        // keeps definions of the fit parameters of the project
};

#endif
