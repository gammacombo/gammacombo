/**
 * Gamma Combination
 * Author: Matthew Kenzie matthew.kenzie@cern.ch
 * Date: Apr 2015
 *
 **/

#ifndef BatchScriptWriter_h
#define BatchScriptWriter_h

#include <TString.h>

#include <string>
#include <vector>

class Combiner;
class FileNameBuilder;
class OptParser;
class PDF_Abs;

class BatchScriptWriter {
 public:
  BatchScriptWriter(int argc, char* argv[]);

  void writeScripts(const OptParser* arg, std::vector<Combiner*>* cmb, const FileNameBuilder* fnb);
  void writeScripts_datasets(const OptParser* arg, PDF_Abs* pdf);
  void writeScript(TString fname, TString outfloc, int jobn, const OptParser* arg);
  void writeCondorScript(TString fname, const OptParser* arg);
  std::string exec;
  std::string subpkg;
};

#endif
