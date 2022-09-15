/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef CLInterval_h
#define CLInterval_h

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "TString.h"

using namespace std;

///
/// Class that represents a confidence interval
///
class CLInterval
{
    public:

        CLInterval();
        CLInterval(const CLInterval &other);
        ~CLInterval();
        void print();

        float pvalue;           // pvalue corresponding to this interval
        float pvalueAtCentral;  // pvalue at the central value
        float min;              // lower interval border
        float max;              // upper interval border
        float central;          // central value
        bool    minclosed;      // true if the interval was not closed by limited scan range
        bool    maxclosed;      // true if the interval was not closed by limited scan range
        TString minmethod;      // details on the algorithm that found this interval
        TString maxmethod;      // details on the algorithm that found this interval
        TString centralmethod;  // details on the algorithm that found the central value
};

#endif
