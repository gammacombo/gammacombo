/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef FileNameBuilder_h
#define FileNameBuilder_h

#include <TString.h>

#include <vector>

class Combiner;
class MethodAbsScan;
class OptParser;

///
/// Class that defines the file names of various files being
/// written across the project.
///
class FileNameBuilder {
 public:
  FileNameBuilder(const OptParser* arg, TString name = "gammacombo");

  TString getBaseName() const;
  TString getCombinerFileName(const Combiner* c) const;
  TString getCombinerFileName(const MethodAbsScan* s) const;
  TString getFileBaseName(const Combiner* c) const;
  TString getFileBaseName(const MethodAbsScan* s) const;
  TString getFileNameScanner(const MethodAbsScan* s) const;
  TString getFileNameSolution(const MethodAbsScan* s) const;
  TString getFileNamePar(const Combiner* c) const;
  TString getFileNamePar(const MethodAbsScan* s) const;
  TString getFileNamePlot(const std::vector<Combiner*>& cmb) const;
  TString getFileNamePlotSingle(const std::vector<Combiner*>& cmb, int cId) const;
  TString getFileNameStartPar(const Combiner* c) const;
  TString getFileNameStartPar(const MethodAbsScan* s) const;
  TString getFileNameAsimovPar(const Combiner* c) const;
  TString getFileNameAsimovPar(const MethodAbsScan* s) const;
  TString getAsimovCombinerNameAddition(int id) const;
  TString getPluginNameAddition() const;
  TString getPluginOnlyNameAddition() const;
  TString getPreliminaryNameAddition() const;
  TString getCLsNameAddition() const;

 private:
  const OptParser* m_arg = nullptr;  ///< command line arguments
  TString m_basename;                ///< the base name, e.g. "gammacombo"
  TString m_asimov = "Asimov";       ///< literal naming Asimov combiners
};

#endif
