#include "FitResultCache.h"

FitResultCache::FitResultCache(OptParser *arg, int roundrobinsize)
{
  assert(arg);
  _arg = arg;
	_roundrobinsize = roundrobinsize;
	_parsAtFunctionCall = 0;
	_parsAtGlobalMin = 0;
	_roundrobinid = 0;
	for ( int i=0; i<_roundrobinsize; i++ ) _parsRoundRobin.push_back(0);
}


FitResultCache::~FitResultCache()
{
	if ( _parsAtFunctionCall ) delete _parsAtFunctionCall;
	if ( _parsAtGlobalMin ) delete _parsAtGlobalMin;
	for ( int i=0; i<_parsRoundRobin.size(); i++ ){
		if ( _parsRoundRobin[i] ) delete _parsRoundRobin[i];
	}
}

///
/// Store the parameters held by set. Can only be called once
/// per instance of FitResultCache.
///
/// \param set - the set of parameters to be saved
///
void FitResultCache::storeParsAtFunctionCall(const RooArgSet* set)
{
	if ( _parsAtFunctionCall ){
		cout << "FitResultCache::storeParsAtFunctionCall() : ERROR : "
			"Trying to overwrite the parameters at funciton call. Exit." << endl;
		exit(1);
	}
	assert(set);
	_parsAtFunctionCall = new RooDataSet("parsAtFunctionCall", "parsAtFunctionCall", *set);
	_parsAtFunctionCall->add(*set);
}

///
/// Store the parameters held by set. Can only be called once
/// per instance of FitResultCache.
///
/// \param set - the set of parameters to be saved
///
void FitResultCache::storeParsAtGlobalMin(const RooArgSet* set)
{
	assert(set);
	if ( _parsAtGlobalMin ) delete _parsAtGlobalMin;
	_parsAtGlobalMin = new RooDataSet("parsAtGlobalMin", "parsAtGlobalMin", *set);
	_parsAtGlobalMin->add(*set);
}

///
/// Store the parameters held by set into the round robin database.
///
/// \param set - the set of parameters to be saved
///
void FitResultCache::storeParsRoundRobin(const RooArgSet* set)
{
	assert(set);
	_roundrobinid++;
	if ( _roundrobinid>=_roundrobinsize ) _roundrobinid = 0;
	if ( _parsRoundRobin[_roundrobinid] ) delete _parsRoundRobin[_roundrobinid];
	_parsRoundRobin[_roundrobinid] = new RooDataSet("parsAtFunctionCall", "parsAtFunctionCall", *set);
	_parsRoundRobin[_roundrobinid]->add(*set);
}

///
/// Initialize all round robin slots to the parameters
/// contained in set.
///
/// \param set - the set of parameters to be saved
///
void FitResultCache::initRoundRobinDB(const RooArgSet* set)
{
	for ( int i=0; i<_parsRoundRobin.size(); i++ ){ 
		storeParsRoundRobin(set);
	}
}

///
/// Get an entry from the round robin database.
/// Ownership stays with FitResultCache.
/// 
/// \param n - the point we want to get, 0 is the most recent one
///
const RooArgSet* FitResultCache::getRoundRobinNminus(int n)
{
	int id = _roundrobinid-n;
	if ( id<0 ) id += _parsRoundRobin.size();
	if ( id < 0 || id >=_parsRoundRobin.size() || _parsRoundRobin[id]==0 ){
		cout << "FitResultCache::getRoundRobinNminus() : ERROR : "
			"Trying to access a round robin point that doesn't exist: id=" << id << ". Exit." << endl;
		exit(1);
	}
	return _parsRoundRobin[id]->get(0);
}
