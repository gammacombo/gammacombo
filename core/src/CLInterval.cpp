#include "CLInterval.h"

CLInterval::CLInterval()
{
	pvalue = -1.;
	pvalueAtCentral = -1.;
	min = -1.;
	max = -1.;
	central = -1.;
	minclosed = false;
	maxclosed = false;
	minmethod = "n/a";
	maxmethod = "n/a";
	centralmethod = "n/a";
}

CLInterval::CLInterval(const CLInterval &other)
{
	pvalue = other.pvalue;
	pvalueAtCentral = other.pvalueAtCentral;
	min = other.min;
	max = other.max;
	central = other.central;
	minclosed = other.minclosed;
	maxclosed = other.maxclosed;
	minmethod = other.minmethod;
	maxmethod = other.maxmethod;
	centralmethod = other.centralmethod;
}

CLInterval::~CLInterval()
{}

void CLInterval::print()
{
	cout << "pvalue=" << pvalue
		<< " pvalueAtCentral=" << pvalueAtCentral
		<< " min=" << min
		<< " max=" << max
		<< " central=" << central
		<< " minclosed=" << minclosed
		<< " maxclosed=" << maxclosed
		<< " minmethod=" << minmethod
		<< " maxmethod=" << maxmethod
		<< " centralmethod=" << centralmethod
		<< endl;
}
