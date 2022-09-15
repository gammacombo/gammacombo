/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef ProgressBar_h
#define ProgressBar_h

#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

///
/// Class showing a progress bar.
///
class ProgressBar
{
public:

    ProgressBar(OptParser *arg, unsigned int n);
    ~ProgressBar();

    void progress();
    void skipSteps(unsigned int n);
    
private:
    
    void progressBar();
    void progressPercentage();

    OptParser* _arg;  ///< command line arguments
    unsigned int _n;  ///< maximum number of steps, "100%"
    unsigned int _x;  ///< current step, "78%"
    int _width;       ///< width of the progress bar
    int _resolution;  ///< update the display this many times
    bool _batch;      ///< display progress in a log-file compatible way
};

#endif
