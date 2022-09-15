/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef CLIntervalPrinter_h
#define CLIntervalPrinter_h

#include "OptParser.h"
#include "Rounder.h"
#include "Utils.h"
#include "CLInterval.h"
#include <string>

using namespace std;
using namespace Utils;

///
/// Class that prints CL intervals and saves them to disk.
///
class CLIntervalPrinter
{
public:

    CLIntervalPrinter(OptParser *arg, TString name, TString var, TString unit, TString method, int CLsType=0);
    ~CLIntervalPrinter();

    void        print();
    void        savePython();
    inline void setDegrees(bool yesno=true){_degrees=yesno;};
    void        addIntervals(vector<CLInterval> &intervals);

private:

    static bool compareByMin(const CLInterval &a, const CLInterval &b);

    OptParser *_arg;        ///< command line arguments
    TString _name;          ///< name of combination
    TString _var;           ///< name of scan variable
    TString _unit;          ///< unit of scan variable
    TString _method;        ///< method name (e.g. Prob)
    bool _degrees;          ///< convert values into degrees
    vector< vector<CLInterval> > _intervals;    ///< container of intervals
    int _clstype;           ///< Type of CLs intervals, 0 means no CLs method
};

#endif
