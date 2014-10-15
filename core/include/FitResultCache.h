/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef FitResultCache_h
#define FitResultCache_h

#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

///
/// Helper class for the scan methods. Store several parameter points 
/// that are needed at various points at the scan. The points are handled
/// in the format of RooFitResults, hence the name. We need
/// - the parameters at a function call so they can be restored once the
///    function is done
/// - a round robin database to hold the N last fit results, so that in
///    the plugin scans we can refit multiple times with varying start
///    parameters
///
class FitResultCache
{
public:

	FitResultCache(OptParser *arg, int roundrobinsize=4);
	~FitResultCache();

	void storeParsAtFunctionCall(const RooArgSet* set);
	void storeParsAtGlobalMin(const RooArgSet* set);
	void storeParsRoundRobin(const RooArgSet* set);
	void initRoundRobinDB(const RooArgSet* set);
	const RooArgSet* getRoundRobinNminus(int n);
	const inline RooArgSet* getParsAtFunctionCall(){assert(_parsAtFunctionCall); return _parsAtFunctionCall->get(0);};
	const inline RooArgSet* getParsAtGlobalMin(){assert(_parsAtGlobalMin); return _parsAtGlobalMin->get(0);};	

private:
      
	OptParser* _arg;                   	///< command line arguments
	int _roundrobinsize;								///< size of the round robin database
	int _roundrobinid;									///< id of currently active round robin cell
	RooDataSet* _parsAtFunctionCall;
	RooDataSet* _parsAtGlobalMin;
	vector<RooDataSet*> _parsRoundRobin;

};

#endif
