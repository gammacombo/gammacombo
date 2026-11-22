#include <CLIntervalPrinter.h>

#include <OptParser.h>
#include <Rounder.h>
#include <Utils.h>

#include <TString.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

CLIntervalPrinter::CLIntervalPrinter(const OptParser* arg, TString name, TString var, TString unit, TString method,
                                     int CLsType)
    : _arg(arg), _name(name), _var(var), _unit(unit), _method(method), _clstype(CLsType) {
  assert(arg);
}

/**
 * Set the intervals to be printed.
 *
 * If more vectors of intervals are added, the set of all intervals will be printed in order
 * (as defined in CLInterval.h).
 *
 * @param intervals Vector of confidence intervals, each one corresponding to one solution.
 */
void CLIntervalPrinter::addIntervals(std::vector<CLInterval>& intervals) {
  for (const auto& i : intervals) _intervals.emplace(i);
}

/**
 * Set the intervals to be printed.
 *
 * @param intervals Vector of confidence intervals, each one corresponding to one solution.
 */
void CLIntervalPrinter::addIntervals(const std::vector<std::vector<std::unique_ptr<CLInterval>>>& intervals) {
  for (const auto& ints : intervals) {
    for (const auto& i : ints) {
      if (!i) continue;
      _intervals.emplace(CLInterval(*i));
    }
  }
}

/**
 * Print all valid intervals.
 */
void CLIntervalPrinter::print() const {
  for (auto interval : _intervals) {
    CLInterval i = interval;

    std::string unit{_unit};
    if (convertToDegrees) {
      using Utils::RadToDeg;
      i.central = RadToDeg(i.central);
      i.min = RadToDeg(i.min);
      i.max = RadToDeg(i.max);
      i.minerr = RadToDeg(i.minerr);
      i.maxerr = RadToDeg(i.maxerr);
      unit = "Deg";
    }

    Rounder rounder(_arg, i.min, i.max, i.central);
    const auto CLlo = rounder.CLlo();
    const auto CLhi = rounder.CLhi();
    const auto central = rounder.central();
    const auto errNeg = rounder.errNeg();
    const auto errPos = rounder.errPos();
    const auto cl = 1. - i.pvalue;
    const std::string unit_str = unit.empty() ? "" : std::format(", [{}]", unit);
    const auto float_format = std::format("{{:-7.{:d}f}}", rounder.getNsubdigits());
    const auto format_str =
        std::format("{{:s}} = [{0}, {0}] ({0} -{0} + {0}) @{{:4.3f}}CL{{:s}}, {{:s}}", float_format);
    std::cout << std::vformat(format_str,
                              std::make_format_args(_var, CLlo, CLhi, central, errNeg, errPos, cl, unit_str, _method));

    // TODO remove the following code from quickhack stage once we have switched to the CLIntervalMaker mechanism
    // to get more useful information on the CL intervals
    if (_clstype == 1) std::cout << " Simplified CL_s";
    if (_clstype == 2) std::cout << " Standard CL_s";

    if (_arg->isQuickhack(8) && _arg->verbose) {
      std::cout << std::format(", central: {:>7s}, interval: [{:>6s}, {:>6s}], p(central): {:4.3f}",
                               std::string(i.centralmethod), std::string(i.minmethod), std::string(i.maxmethod),
                               i.pvalueAtCentral);
    }
    const auto float_format2 = std::format("{{:8.{:d}f}}", rounder.getNsubdigits() + 1);
    const auto format_str2 = std::format(", border errs: [{0}, {0}]", float_format2);
    std::cout << std::vformat(format_str2, std::make_format_args(i.minerr, i.maxerr)) << std::endl;
  }
}

/**
 * Save all valid intervals to a Python file.
 */
void CLIntervalPrinter::savePython() const {
  const std::string dirname = "plots/cl";
  const std::string clstype_str = (_clstype == 0) ? "" : "_CLs" + std::to_string(_clstype);
  const std::string ofname = dirname + "/clintervals_" + _name + "_" + _var + "_" + _method + clstype_str + ".py";
  if (_arg->verbose) std::cout << "CLIntervalPrinter::save() : saving " << ofname << std::endl;
  std::string cmd = "mkdir -p " + dirname;
  system(cmd.c_str());
  std::ofstream outf;
  outf.open(ofname);
  outf << "# Confidence Intervals\n"
       << "intervals = {" << std::endl;

  double previousCL = -1.;

  for (const auto& interval : _intervals) {
    CLInterval i = interval;

    std::string unit{_unit};
    if (convertToDegrees) {
      using Utils::RadToDeg;
      i.central = RadToDeg(i.central);
      i.min = RadToDeg(i.min);
      i.max = RadToDeg(i.max);
      i.minerr = RadToDeg(i.minerr);
      i.maxerr = RadToDeg(i.maxerr);
      unit = "Deg";
    }

    double thisCL = 1. - i.pvalue;
    if (previousCL != thisCL) {
      if (previousCL != -1.) outf << "  ],\n";
      outf << std::format("  '{:.4f}' : [\n", thisCL);
    }

    Rounder rounder(_arg, i.min, i.max, i.central);
    const auto float_format = std::format("{{:.{:d}f}}", rounder.getNsubdigits());
    const auto float_format2 = std::format("{{:.{:d}f}}", rounder.getNsubdigits() + 1);
    const auto format_str =
        std::format("    {{{{'var': '{{:s}}', 'min': '{0}', 'max': '{0}', 'central': '{0}', 'neg': '{0}', "
                    "'pos': '{0}', 'minerr': '{1}', 'maxerr': {1}:, 'cl': '{{:.4f}}', 'unit': '{{:s}}', "
                    "'method': '{{:s}}'}}}},\n",
                    float_format, float_format2);
    const auto CLlo = rounder.CLlo();
    const auto CLhi = rounder.CLhi();
    const auto central = rounder.central();
    const auto errNeg = rounder.errNeg();
    const auto errPos = rounder.errPos();
    outf << std::vformat(format_str, std::make_format_args(_var, CLlo, CLhi, central, errNeg, errPos, i.minerr,
                                                           i.maxerr, thisCL, unit, _method));
    previousCL = thisCL;
  }
  if (previousCL != -1.) { outf << "  ]\n"; }
  outf << "}" << std::endl;
  outf.close();
}
