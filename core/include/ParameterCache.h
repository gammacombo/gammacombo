#ifndef ParameterCache_h
#define ParameterCache_h

#include <iostream>
#include <vector>
#include <algorithm>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/lexical_cast.hpp"
#include "TString.h"
#include "RooSlimFitResult.h"
#include "TIterator.h"
#include "TDatime.h"
#include "RooAbsArg.h"

#include "Combiner.h"
#include "MethodAbsScan.h"
#include "Utils.h"

class ParameterCache {

	public:

		ParameterCache(OptParser* arg, TString basename="");
		~ParameterCache();

		void cacheParameters(MethodAbsScan *scanner);
		bool loadPoints(TString fileName="default");
		void printFitResultToOutStream(ofstream &out, RooSlimFitResult *slimFitResult);
		void printPoint();
		int getNPoints();
		TString getDefaultFileName();
		void setPoint(Combiner* cmb, int i);
		void setPoint(MethodAbsScan *scanner, int i);
		std::vector<TString> getFixedNames(std::vector<Utils::FixPar> fixPar);
		std::vector<std::map<TString,double> > startingValues;

	private:
		
		TString getFullPath(TString basename);
		bool parametersLoaded;
		OptParser* arg;
		TString _basename;
};

#endif

