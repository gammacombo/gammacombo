/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef PDF_Gaus2d_h
#define PDF_Gaus2d_h

#include "PDF_Abs.h"
#include "ParametersTutorial.h"

using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_Gaus2d : public PDF_Abs
{
public:
  PDF_Gaus2d(config cObs=lumi1fb, config cErr=lumi1fb, config cCor=lumi1fb);
  ~PDF_Gaus2d();
  void          buildPdf();
  void          initObservables();
  virtual void  initParameters();
  virtual void  initRelations();
  void          setCorrelations(config c);
  void          setObservables(config c);
  void          setUncertainties(config c);
};

#endif
