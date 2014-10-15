/**
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Abstract class to define the (nuisance) parameters:
 * Gamma Combination
 *
 **/

#ifndef ParametersTutorial_h
#define ParametersTutorial_h

#include "ParametersAbs.h"
#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ParametersTutorial : public ParametersAbs
{
public:
    ParametersTutorial();
    inline ~ParametersTutorial(){};
    
protected:
    void defineParameters();
};

#endif
