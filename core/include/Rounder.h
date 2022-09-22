/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Class to round confidence intervals and central values.
 *
 **/

#ifndef Rounder_h
#define Rounder_h

#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class Rounder
{
public:

    Rounder(OptParser *arg, float cllo, float clhi, float central);
    ~Rounder();
    
    int   getNsubdigits();
    float CLlo();
    float CLhi();
    float central();
    float errNeg();
    float errPos();
    
private:
      
    OptParser *arg;  ///< command line arguments
    float m_cllo;
    float m_clhi;
    float m_central;
};

#endif
