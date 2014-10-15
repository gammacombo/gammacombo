#include "CLIntervalPrinter.h"

CLIntervalPrinter::CLIntervalPrinter(OptParser *arg, TString name, TString var, TString unit, TString method)
{
	assert(arg);
	_arg = arg;
	_name = name;
	_var = var;
	_unit = unit;
	_method = method;
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


void CLIntervalPrinter::print()
{
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
		printf("%s = [%7.*f, %7.*f] (%7.*f -%7.*f +%7.*f) @%3.2fCL",
				_var.Data(),
				d, myRounder.CLlo(), d, myRounder.CLhi(),
				d, myRounder.central(), 
				d, myRounder.errNeg(), d, myRounder.errPos(),
				1.-i.pvalue);
		if ( _unit!="" ) cout << ", ["<<_unit<<"]";
		if ( !i.closed ){
			cout << " (not closed)";
		}
		cout << ", " << _method;
		if ( _arg->verbose ){
			cout << ", central: " << i.centralmethod;
			cout << ", interval: [" << i.minmethod << ", " << i.maxmethod << "]";
			cout << ", p(central): " << i.pvalueAtCentral;		
		}
		cout << endl;
	}
} 


void CLIntervalPrinter::savePython()
{
	TString dirname = "plots/cl";
        TString ofname = dirname+"/clintervals_"+_name+"_"+_var+"_"+_method+".py";
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
	outf << "  ]" << endl;
	outf << "}" << endl;
	outf.close();
} 
