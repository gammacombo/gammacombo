#ifndef ParameterCache_h
#define ParameterCache_h

#include "Utils.h"

#include <TString.h>

#include <fstream>
#include <map>
#include <vector>

class Combiner;
class MethodAbsScan;
class OptParser;
class RooSlimFitResult;

class ParameterCache {

 public:
  ParameterCache(const OptParser* arg = nullptr);

  void cacheParameters(MethodAbsScan* scanner, TString fileName);
  bool loadPoints(TString fileName);
  void printFitResultToOutStream(std::ofstream& out, const RooSlimFitResult* slimFitResult) const;
  void printPoint() const;
  int getNPoints() const;
  void setPoint(Combiner* cmb, int i);
  void setPoint(MethodAbsScan* scanner, int i);
  std::vector<TString> getFixedNames(std::vector<Utils::FixPar> fixPar) const;

  std::vector<std::map<TString, double>> startingValues;

 private:
  bool m_parametersLoaded = false;
  const OptParser* m_arg = nullptr;
};

#endif
