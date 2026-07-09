/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef CLInterval_h
#define CLInterval_h

#include <TString.h>

#include <compare>
#include <limits>

/**
 * Class that represents a confidence interval.
 *
 * Intervals are ordered based on minmethod, maxmethod, centralmethod, decreasing p-value, increasing min, increasing
 * max (p-value at maximum and central value should then be uniquely identified by the other characteristics of the
 * intervals).
 */
struct CLInterval {
  bool operator==(const CLInterval& rhs) const noexcept;
  std::strong_ordering operator<=>(const CLInterval& rhs) const noexcept;

  bool checkPrecision(double precRel, bool returnOnNaN = true) const;

  /// Print the details of the CL interval.
  void print() const;

  /// P-value that defines the interval.
  double pvalue = std::numeric_limits<double>::quiet_NaN();
  /// P-value at the central value.
  double pvalueAtCentral = std::numeric_limits<double>::quiet_NaN();
  /// Lower interval border.
  double min = std::numeric_limits<double>::quiet_NaN();
  /// Upper interval border.
  double max = std::numeric_limits<double>::quiet_NaN();
  /// Error on lower interval border.
  double minerr = std::numeric_limits<double>::quiet_NaN();
  /// Error on upper interval border.
  double maxerr = std::numeric_limits<double>::quiet_NaN();
  /// Central value.
  double central = std::numeric_limits<double>::quiet_NaN();
  /// True if the interval was not closed by limited scan range.
  bool minclosed = false;
  /// True if the interval was not closed by limited scan range.
  bool maxclosed = false;
  /// Identifier of the algorithm that found the lower border of the interval.
  TString minmethod = "n/a";
  /// Identifier of the algorithm that found the upper border of the interval.
  TString maxmethod = "n/a";
  /// Identifier of the algorithm that found the central value of the interval.
  TString centralmethod = "n/a";
};

#endif
