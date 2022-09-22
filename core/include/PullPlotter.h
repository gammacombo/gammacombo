/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2013
 *
 * Class to plot pulls.
 *
 **/

#ifndef PullPlotter_h
#define PullPlotter_h

#include "Utils.h"
#include "MethodAbsScan.h"
#include "OptParser.h"
#include "TPaveText.h"

class MethodAbsScan;

using namespace std;
using namespace Utils;

class PullPlotter
{
public:

    PullPlotter(MethodAbsScan *cmb);
    ~PullPlotter();

    bool hasPullsAboveNsigma(float nsigma);
    void loadParsFromSolution(int n);
    void savePulls();
    void plotPulls();
    void printPulls(float aboveNsigma = -1.);

private:

    void defineOrder();
    void plotPullsCanvas(vector<TString>& observables, int currentid, int maxid, int nObs);

    MethodAbsScan *cmb;       // the scanner to plot pulls for
    OptParser *arg;           // command line arguments
    vector<TString> obsOrder; // contains observable names in the desired plot order
    int nSolution;            // index of the solution wrt which the pulls are computed
};

#endif
