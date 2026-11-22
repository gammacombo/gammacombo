#ifndef CLIntervalPrinter_h
#define CLIntervalPrinter_h

#include "CLInterval.h"

#include <TString.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

class OptParser;

///
/// Class that prints CL intervals and saves them to disk.
///
class CLIntervalPrinter {
 public:
  CLIntervalPrinter(const OptParser* arg, TString name, TString var, TString unit, TString method, int CLsType = 0);

  void addIntervals(std::vector<CLInterval>& intervals);
  void addIntervals(const std::vector<std::vector<std::unique_ptr<CLInterval>>>& intervals);

  void print() const;
  void savePython() const;
  inline void setDegrees(bool yesno = true) { convertToDegrees = yesno; };

 private:
  const OptParser* _arg = nullptr;  ///< Command line arguments
  std::string _name;                ///< Name of combination
  std::string _var;                 ///< Name of scan variable
  std::string _unit;                ///< Unit of scan variable
  std::string _method;              ///< Method name (e.g. Prob)
  bool convertToDegrees = false;    ///< Convert values into degrees
  std::set<CLInterval> _intervals;  ///< Container of intervals sorted according to default less (@see CLInterval).
  int _clstype = 0;                 ///< Type of CLs intervals, 0 means no CLs method
};

#endif
