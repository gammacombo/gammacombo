/**
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Abstract class to define the (nuisance) parameters.
 * Project: Tutorial
 *
 **/

#ifndef ParametersTutorial_h
#define ParametersTutorial_h

#include "ParametersAbs.h"

class ParametersTutorial : public ParametersAbs
{
public:
    ParametersTutorial();
    
protected:
    void defineParameters();
};

#endif
