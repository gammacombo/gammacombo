#include "CLIntervalPrinter.h"

CLIntervalPrinter::CLIntervalPrinter(OptParser *arg, TString name, TString var, TString unit, TString method, int CLsType)
{
    assert(arg);
    _arg = arg;
    _name = name;
    _var = var;
    _unit = unit;
    _method = method;
    _clstype = CLsType;
    _degrees= false;
}

CLIntervalPrinter::~CLIntervalPrinter()
{}

///
/// Set the intervals. If more vectors of intervals are added, each of them will
/// be printed in order.
///
/// \param intervals - vector of confidence intervals, each one corresponding to one solution
///
void CLIntervalPrinter::addIntervals(vector<CLInterval> &intervals)
{
    _intervals.push_back(intervals);
}

///
/// Helper function to sort the intervals according to their
/// lower boundary.
///
bool CLIntervalPrinter::compareByMin(const CLInterval &a, const CLInterval &b)
{
    return a.min < b.min;
}


void CLIntervalPrinter::print()
{
    for ( int k=0; k<_intervals.size(); k++ ){
        // sort the intervals
        vector<CLInterval> sortedIntervals(_intervals[k]);
        sort(sortedIntervals.begin(), sortedIntervals.end(), compareByMin);
        for ( int j=0; j<sortedIntervals.size(); j++ ){
            CLInterval i = sortedIntervals[j];

            // convert to degrees if necessary
            if ( _degrees ){
                i.central = RadToDeg(i.central);
                i.min = RadToDeg(i.min);
                i.max = RadToDeg(i.max);
                _unit = "Deg";
            }

            Rounder myRounder(_arg, i.min, i.max, i.central);
            int d = myRounder.getNsubdigits();
            printf("%s = [%7.*f, %7.*f] (%7.*f -%7.*f +%7.*f) @%3.2fCL",
                    _var.Data(),
                    d, myRounder.CLlo(), d, myRounder.CLhi(),
                    d, myRounder.central(),
                    d, myRounder.errNeg(), d, myRounder.errPos(),
                    1.-i.pvalue);
            if ( _unit!="" ) cout << ", ["<<_unit<<"]";
            // \todo remove the following code from quickhack stage once we have switched
            // to the CLIntervalMaker mechanism to get more useful information
            // on the CL intervals
            cout << ", " << _method;
            if (_clstype==1) cout << " Simplified CL_s";
            if (_clstype==2) cout << " Standard CL_s";

            if ( _arg->isQuickhack(8) ){
                if ( _arg->verbose ){
                    cout << Form(", central: %-7s", i.centralmethod.Data());
                    cout << Form(", interval: [%-6s, %-6s]", i.minmethod.Data(), i.maxmethod.Data());
                    cout << ", p(central): " << i.pvalueAtCentral;
                }
            }
            cout << endl;
        }
        cout << endl;
    }
}


void CLIntervalPrinter::savePython()
{
    TString dirname = "plots/cl";
    TString ofname;
    if (_clstype==0) ofname = dirname+"/clintervals_"+_name+"_"+_var+"_"+_method+".py";
    else ofname = dirname+"/clintervals_"+_name+"_"+_var+"_"+_method+"_CLs"+std::to_string(_clstype)+".py";
    if ( _arg->verbose ) cout << "CLIntervalPrinter::save() : saving " << ofname << endl;
    system("mkdir -p "+dirname);
    ofstream outf;
    outf.open(ofname);
    outf << "# Confidence Intervals" << endl;
    outf << "intervals = {" << endl;

    float previousCL = -1.0;

    for ( int k=0; k<_intervals.size(); k++ )
    for ( int j=0; j<_intervals[k].size(); j++ )
    {
        CLInterval i = _intervals[k][j];

        // convert to degrees if necessary
        if ( _degrees ){
            i.central = RadToDeg(i.central);
            i.min = RadToDeg(i.min);
            i.max = RadToDeg(i.max);
            _unit = "Deg";
        }

        Rounder myRounder(_arg, i.min, i.max, i.central);
        int d = myRounder.getNsubdigits();

        float thisCL = 1.-i.pvalue;
        if ( previousCL!=thisCL ){
            if ( previousCL!=-1) outf << "  ]," << endl;
            outf << Form("  '%.2f' : [", thisCL) << endl;
        }

        outf << Form("    {'var':'%s', 'min':'%.*f', 'max':'%.*f', 'central':'%.*f', "
                "'neg':'%.*f', 'pos':'%.*f', 'cl':'%.2f', 'unit':'%s', 'method':'%s'},\n",
                _var.Data(),
                d, myRounder.CLlo(), d, myRounder.CLhi(),
                d, myRounder.central(),
                d, myRounder.errNeg(), d, myRounder.errPos(),
                thisCL,
                _unit.Data(),
                _method.Data());
        previousCL = thisCL;
    }
    if(previousCL!=-1){
        outf << "  ]" << endl;
    }
    outf << "}" << endl;
    outf.close();
}
