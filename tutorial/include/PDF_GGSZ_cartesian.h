/**
 * Author: Matthew Kenzie <matthew.kenzie@cern.ch>
 * Date: December 2020
 *
 **/

#ifndef PDF_GGSZ_cartesian_h
#define PDF_GGSZ_cartesian_h

#include "PDF_Abs.h"
#include "ParametersGamma.h"
#include "PDF_GGSZ.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_GGSZ_cartesian : public PDF_GGSZ
{
public:
  PDF_GGSZ_cartesian(TString cObs, TString cErr, TString cCor);
  ~PDF_GGSZ_cartesian();
  virtual void  initParameters();
  virtual void  initRelations();

};

#endif
