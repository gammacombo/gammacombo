/**
 * \brief Tools to handle TGraphs
 * \author Till Moritz Karbach, moritz.karbach@cern.ch
 * \date March 2015
 *
 **/

#ifndef TGraphTools_h
#define TGraphTools_h

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TGraphSmooth.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class TGraphTools
{
    public:

        TGraph* addPointToGraphAtFirstMatchingX(TGraph* g, float xNew, float yNew);
};

#endif
