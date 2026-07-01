/**
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Abstract class to define the (nuisance) parameters.
 *
 **/

#ifndef ParametersAbs_h
#define ParametersAbs_h

#include "Parameter.h"

#include <TString.h>

#include <vector>

class RooRealVar;

class ParametersAbs {
 public:
  virtual ~ParametersAbs() = default;

  Parameter* var(TString name);
  RooRealVar* get(TString name);
  Parameter* newParameter(TString name);

  Parameter::Range range(double min, double max);

 protected:
  std::vector<Parameter*> m_parameters;
  inline virtual void defineParameters() {};
};

#endif
