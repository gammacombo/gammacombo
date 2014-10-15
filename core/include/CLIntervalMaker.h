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
	void	calcCLintervals();
	void	findMaxima(float pValueThreshold);
	void	print();
	void 	provideMorePreciseMaximum(float value, TString method);

private:
	
	int		checkNeighboringBins(int i, float y) const;
	bool	binsOnSameSide(int i, float y) const;
	float	binToValue(int bin) const;
	bool	similarMaximumExists(float value) const;
	void 	findRawIntervals(float pvalue, vector<CLInterval> &clis) const;
	bool	interpolateLine(const TH1F* h, int i, float y, float &val) const;
	bool	interpolatePol2fit(const TH1F* h, int i, float y, float central, bool upper, float &val, float &err) const;
	void 	improveIntervalsLine(vector<CLInterval> &clis) const;
	void 	improveIntervalsPol2fit(vector<CLInterval> &clis) const;
	void	removeBadIntervals();
	int		valueToBin(float val) const;
	float pq(float p0, float p1, float p2, float y, int whichSol) const;
      
	OptParser*			_arg;		///< command line arguments
	const TH1F&			_pvalues; ///< the pvalue histogram
  vector<CLInterval> _clintervals1sigma;           ///< 1 sigma intervals
  vector<CLInterval> _clintervals2sigma;           ///< 2 sigma intervals
};

#endif
