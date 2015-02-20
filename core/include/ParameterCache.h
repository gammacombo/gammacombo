#ifndef ParameterCache_h
#define ParameterCache_h

#include <iostream>
#include <vector>
#include <algorithm>

#include "Combiner.h"
#include "FileNameBuilder.h"
#include "MethodAbsScan.h"
#include "RooAbsArg.h"
#include "RooSlimFitResult.h"
#include "TDatime.h"
#include "TIterator.h"
#include "TString.h"
#include "Utils.h"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/lexical_cast.hpp"

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
		OptParser* m_arg;
		TString m_basename;
		FileNameBuilder* m_fnamebuilder;
};

#endif

