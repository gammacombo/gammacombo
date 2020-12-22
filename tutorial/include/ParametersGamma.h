/**
 * Abstract class to define the (nuisance) parameters:
 * Gamma Combination
 *
 **/

#ifndef ParametersGamma_h
#define ParametersGamma_h

#include "ParametersAbs.h"
#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ParametersGamma : public ParametersAbs
{
public:
    ParametersGamma();
    inline ~ParametersGamma(){};

protected:
    void defineParameters();
};

#endif
