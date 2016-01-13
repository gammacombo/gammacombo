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
#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ParametersAbs
{
public:
  inline              ParametersAbs(){};
  inline virtual      ~ParametersAbs(){};
  Parameter*          var(TString name);
  RooRealVar*         get(TString name);
  Parameter*          newParameter(TString name);
  Parameter::Range    range(float min, float max);

protected:
  vector<Parameter*>  m_parameters;
  inline virtual void defineParameters(){};
};

#endif
