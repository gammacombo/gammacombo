#include <iostream>
#include <math.h>

#include "RooDoubleCB.h"
#include "RooRealConstant.h"
#include "RooRealVar.h"

using namespace RooFit;

ClassImp(RooDoubleCB);

    RooDoubleCB::RooDoubleCB(){}

RooDoubleCB::RooDoubleCB(const char *name, const char *title, RooAbsReal &_x,
                         RooAbsReal &_mean, RooAbsReal &_width,
                         RooAbsReal &_alpha1, RooAbsReal &_n1,
                         RooAbsReal &_alpha2, RooAbsReal &_n2)
    : RooAbsPdf(name, title), x("x", "x", this, _x),
      mean("mean", "mean", this, _mean), width("width", "width", this, _width),
      alpha1("alpha1", "alpha1", this, _alpha1), n1("n1", "n1", this, _n1),
      alpha2("alpha2", "alpha2", this, _alpha2), n2("n2", "n2", this, _n2) {};

RooDoubleCB::RooDoubleCB(const RooDoubleCB &other, const char *name)
    : RooAbsPdf(other, name), x("x", this, other.x),
      mean("mean", this, other.mean), width("width", this, other.width),
      alpha1("alpha1", this, other.alpha1), n1("n1", this, other.n1),
      alpha2("alpha2", this, other.alpha2), n2("n2", this, other.n2) {};

double RooDoubleCB::evaluate() const {

  double A1 = pow(n1 / fabs(alpha1), n1) * exp(-alpha1 * alpha1 / 2);
  double A2 = pow(n2 / fabs(alpha2), n2) * exp(-alpha2 * alpha2 / 2);
  double B1 = n1 / fabs(alpha1) - fabs(alpha1);
  double B2 = n2 / fabs(alpha2) - fabs(alpha2);

  if ((x - mean) / width >= -alpha1 && (x - mean) / width <= alpha2) {

    return exp(-(x - mean) * (x - mean) / (2 * width * width));

  }

  else if ((x - mean) / width < -alpha1) {

    return A1 * pow(B1 - (x - mean) / width, -n1);

  }

  else if ((x - mean) / width > alpha2) {

    return A2 * pow(B2 + (x - mean) / width, -n2);

  }

  else {

    std::cout << "ERROR evaluating range..." << std::endl;
    std::cout << "x: " << x << std::endl;
    std::cout << "mean: " << mean << std::endl;
    std::cout << "width: " << width << std::endl;
    std::cout << "alpha1: " << alpha1 << std::endl;
    std::cout << "alpha2: " << alpha2 << std::endl;

    return 99;
  }
}
