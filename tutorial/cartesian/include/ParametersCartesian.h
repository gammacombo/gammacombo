/**
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Abstract class to define the (nuisance) parameters:
 * Gamma Combination
 *
 **/

#ifndef ParametersCartesian_h
#define ParametersCartesian_h

#include "ParametersAbs.h"

class ParametersCartesian : public ParametersAbs
{
public:
    ParametersCartesian();
    inline ~ParametersCartesian(){};
    
protected:
    void defineParameters();
};

#endif
