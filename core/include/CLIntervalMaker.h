/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef CLIntervalMaker_h
#define CLIntervalMaker_h

#include "TF1.h"
#include "OptParser.h"
#include "Utils.h"
#include "CLInterval.h"
#include "CLIntervalPrinter.h"

using namespace std;
using namespace Utils;

///
/// Class that makes CL intervals from 1-CL histograms.
///
class CLIntervalMaker
{
    public:

        CLIntervalMaker(OptParser *arg, const TH1F &pvalues);
        ~CLIntervalMaker();
        void   calcCLintervals();
        void   findMaxima(float pValueThreshold);
        inline vector<CLInterval>& getClintervals1sigma(){return _clintervals1sigma;};
        inline vector<CLInterval>& getClintervals2sigma(){return _clintervals2sigma;};
        void   print();
        void   provideMorePreciseMaximum(float value, TString method);

    private:

        int   checkNeighboringBins(int i, float y) const;
        bool  binsOnSameSide(int i, float y) const;
        float binToValue(int bin) const;
        void  findRawIntervals(float pvalue, vector<CLInterval> &clis);
        void  findRawIntervalsForCentralValues(float pvalue, vector<CLInterval> &clis);
        bool  interpolateLine(const TH1F* h, int i, float y, float &val) const;
        bool  interpolatePol2fit(const TH1F* h, int i, float y, float central, bool upper, float &val, float &err) const;
        bool  isInInterval(int binid, float pvalue) const;
        void  improveIntervalsLine(vector<CLInterval> &clis) const;
        void  improveIntervalsPol2fit(vector<CLInterval> &clis) const;
        float pq(float p0, float p1, float p2, float y, int whichSol) const;
        void  removeBadIntervals();
        bool  similarMaximumExists(float value) const;
        void  storeRawInterval(int binidLo, int binidHi, float pvalue, vector<CLInterval> &clis);
        int   valueToBin(float val) const;

        OptParser*          _arg;               ///< command line arguments
        const TH1F&         _pvalues;           ///< the pvalue histogram
        vector<CLInterval> _clintervals1sigma;  ///< 1 sigma intervals
        vector<CLInterval> _clintervals2sigma;  ///< 2 sigma intervals
};

#endif
