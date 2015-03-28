/**
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-29
 */
#include <cmath>
#include <iostream>

#include <TH2.h>
#include <RooArgSet.h>

#include "RooBinned2DBicubicBase.h"

template<class BASE>
RooBinned2DBicubicBase<BASE>::BinSizeException::~BinSizeException() throw ()
{ }

template<class BASE>
const char* RooBinned2DBicubicBase<BASE>::BinSizeException::what() const throw ()
{ return "Variable-sized bins not supported!"; }

template<class BASE>
RooBinned2DBicubicBase<BASE>::~RooBinned2DBicubicBase()
{ }

template<class BASE>
RooBinned2DBicubicBase<BASE>::RooBinned2DBicubicBase(
	const RooBinned2DBicubicBase<BASE>& other,
	const char* name) :
    BASE(other, name),
    x("x", this, other.x), y("y", this, other.y),
    nBinsX(other.nBinsX), nBinsY(other.nBinsY),
    binSizeX(other.binSizeX), binSizeY(other.binSizeY),
    xmin(other.xmin), xmax(other.xmax),
    ymin(other.ymin), ymax(other.ymax),
    coeffs(other.coeffs)
{ }

template<class BASE>
RooBinned2DBicubicBase<BASE>& RooBinned2DBicubicBase<BASE>::operator=(
	const RooBinned2DBicubicBase<BASE>& other)
{
    if (&other == this) return *this;
    BASE::operator=(other);
    x = RooRealProxy("x", this, other.x);
    y = RooRealProxy("y", this, other.y);
    nBinsX = other.nBinsX;
    nBinsY = other.nBinsY;
    binSizeX = other.binSizeX;
    binSizeY = other.binSizeY;
    xmin = other.xmin;
    xmax = other.xmax;
    ymin = other.ymin;
    ymax = other.ymax;
    coeffs = other.coeffs;
    return *this;
}

template<class BASE>
inline double RooBinned2DBicubicBase<BASE>::histcont(
	const TH2& h, int xbin, int ybin) const
{
    return (0 <= xbin && xbin < nBinsX - 1 &&
	    0 <= ybin && ybin < nBinsY - 1) ?
	h.GetBinContent(1 + xbin, 1 + ybin) : 0.;
}

template<class BASE>
inline double RooBinned2DBicubicBase<BASE>::dhistdx(
	const TH2& h, int xbin, int ybin) const
{
    return 0.5 * (histcont(h, xbin + 1, ybin) -
	    histcont(h, xbin - 1, ybin));
}

template<class BASE>
inline double RooBinned2DBicubicBase<BASE>::dhistdy(
	const TH2& h, int xbin, int ybin) const
{
    return 0.5 * (histcont(h, xbin, ybin + 1) -
	    histcont(h, xbin, ybin - 1));
}

template<class BASE>
inline double RooBinned2DBicubicBase<BASE>::d2histdxdy(
	const TH2& h, int xbin, int ybin) const
{
    return 0.5 * (histcont(h, xbin - 1, ybin - 1) -
	    histcont(h, xbin + 1, ybin - 1) +
	    histcont(h, xbin + 1, ybin + 1) -
	    histcont(h, xbin - 1, ybin + 1));
}

template<class BASE>
RooBinned2DBicubicBase<BASE>::RooBinned2DBicubicBase(
	const char* name, const char* title, const TH2& h,
	RooAbsReal& xvar, RooAbsReal& yvar) :
    BASE(name, title),
    x("x", "x", this, xvar), y("y", "y", this, yvar),
    nBinsX(1 + h.GetNbinsX()), nBinsY(1 + h.GetNbinsY()),
    binSizeX(h.GetXaxis()->GetBinWidth(1)),
    binSizeY(h.GetYaxis()->GetBinWidth(1)),
    xmin(h.GetXaxis()->GetBinCenter(1) - binSizeX),
    xmax(h.GetXaxis()->GetBinCenter(nBinsX - 1) + binSizeX),
    ymin(h.GetYaxis()->GetBinCenter(1) - binSizeY),
    ymax(h.GetYaxis()->GetBinCenter(nBinsY - 1) + binSizeY),
    coeffs(CoeffRecLen * nBinsX * nBinsY)
{
    const TAxis *xaxis = h.GetXaxis(), *yaxis = h.GetYaxis();
    // verify that all bins have same size
    for (int i = 1; i < nBinsX; ++i) {
	if (std::abs(xaxis->GetBinWidth(i) / binSizeX - 1.) > 1e-9)
	    throw BinSizeException();
    }
    for (int i = 1; i < nBinsY; ++i) {
	if (std::abs(yaxis->GetBinWidth(i) / binSizeY - 1.) > 1e-9)
	    throw BinSizeException();
    }
    // ok, go through histogram to precalculate the interpolation coefficients
    // in rectangles between bin centres
    //
    // for that purpose, we map each of those rectangles to the unit square
    for (int j = -1; j < nBinsY - 1; ++j) {
	for (int i = -1; i < nBinsX - 1; ++i) {
	    const double rhs[NCoeff] = {
		// function values in bin centres
		histcont(h, i, j),
		histcont(h, i + 1, j),
		histcont(h, i, j + 1),
		histcont(h, i + 1, j + 1),
		// df/dx in bin centres (finite difference approximation)
		dhistdx(h, i, j),
		dhistdx(h, i + 1, j),
		dhistdx(h, i, j + 1),
		dhistdx(h, i + 1, j + 1),
		// df/dy in bin centres (finite difference approximation)
		dhistdy(h, i, j),
		dhistdy(h, i + 1, j),
		dhistdy(h, i, j + 1),
		dhistdy(h, i + 1, j + 1),
		// d^2f/dxdy in bin centres (finite difference approximation)
		d2histdxdy(h, i, j),
		d2histdxdy(h, i + 1, j),
		d2histdxdy(h, i, j + 1),
		d2histdxdy(h, i + 1, j + 1)
	    };
	    // work out solution - strange array placement is due to the fact
	    // that terms with x/y to high powers can be small, so they should
	    // be added up first during evaluation to avoid cancellation
	    // issues; at the same time you want to access them in order to
	    // not confuse the CPU cache, so they're stored back to front
	    //
	    // a_00 ... a_30
	    coeff(1 + i, 1 + j, 15) = rhs[0];
	    coeff(1 + i, 1 + j, 14) = rhs[4];
	    coeff(1 + i, 1 + j, 13) =
		3. * (-rhs[0] + rhs[1]) - 2. * rhs[4] - rhs[5];
	    coeff(1 + i, 1 + j, 12) =
		2. * (rhs[0] - rhs[1]) + rhs[4] + rhs[5];
	    // a_31 ... a_31
	    coeff(1 + i, 1 + j, 11) = rhs[8];
	    coeff(1 + i, 1 + j, 10) = rhs[12];
	    coeff(1 + i, 1 + j, 9) =
		3. * (-rhs[8] + rhs[9]) - 2. * rhs[12] - rhs[13];
	    coeff(1 + i, 1 + j, 8) =
		2. * (rhs[8] - rhs[9]) + rhs[12] + rhs[13];
	    // a_02 ... a_32
	    coeff(1 + i, 1 + j, 7) =
		3. * (-rhs[0] + rhs[2]) - 2. * rhs[8] - rhs[10];
	    coeff(1 + i, 1 + j, 6) =
		3. * (-rhs[4] + rhs[6]) - 2. * rhs[12] - rhs[14];
	    coeff(1 + i, 1 + j, 5) =
		9. * (rhs[0] - rhs[1] - rhs[2] + rhs[3]) +
		6. * (rhs[4] - rhs[6] + rhs[8] - rhs[9]) + 4. * rhs[12] +
		3. * (rhs[5] - rhs[7] + rhs[10] - rhs[11]) +
		2. * (rhs[13] + rhs[14]) + rhs[15];
	    coeff(1 + i, 1 + j, 4) =
		6. * (-rhs[0] + rhs[1] + rhs[2] - rhs[3]) +
		4. * (-rhs[8] + rhs[9]) +
		3. * (-rhs[4] - rhs[5] + rhs[6] + rhs[7]) +
		2. * (-rhs[10] + rhs[11] - rhs[12] - rhs[13]) -
		rhs[14] - rhs[15];
	    // a_03 ... a_33
	    coeff(1 + i, 1 + j, 3) =
		2. * (rhs[0] - rhs[2]) + rhs[8] + rhs[10];
	    coeff(1 + i, 1 + j, 2) =
		2. * (rhs[4] - rhs[6]) + rhs[12] + rhs[14];
	    coeff(1 + i, 1 + j, 1) =
		6. * (-rhs[0] + rhs[1] + rhs[2] - rhs[3]) +
		4. * (-rhs[4] + rhs[6]) +
		3. * (-rhs[8] + rhs[9] - rhs[10] + rhs[11]) +
		2. * (- rhs[5] + rhs[7] - rhs[12] - rhs[14]) -
		rhs[13] - rhs[15];
	    coeff(1 + i, 1 + j, 0) =
		4. * (rhs[0] - rhs[1] - rhs[2] + rhs[3]) +
		2. * (rhs[4] + rhs[5] - rhs[6] - rhs[7] +
			rhs[8] - rhs[9] + rhs[10] - rhs[11]) +
		rhs[12] + rhs[13] + rhs[14] + rhs[15];
	    // coeff(1 + i, 1 + j, 17) contains integral of function over the
	    // square in "unit square coordinates", i.e. neglecting bin widths
	    // this is done to help speed up calculations of 2D integrals
	    double sum = 0.;
	    for (int k = 0; k < NCoeff; ++k)
		sum += coeff(1 + i, 1 + j, k) /
		    double((4 - (k % 4)) * (4 - (k / 4)));
	    coeff(1 + i, 1 + j, NCoeff) = sum;
	}
    }
}

template<class BASE>
RooBinned2DBicubicBase<BASE>* RooBinned2DBicubicBase<BASE>::clone(
	const char* newname) const
{ return new RooBinned2DBicubicBase<BASE>(*this, newname); }

template<class BASE>
Double_t RooBinned2DBicubicBase<BASE>::evaluate() const
{ return eval(x, y); }

template <class BASE>
Int_t RooBinned2DBicubicBase<BASE>::getAnalyticalIntegral(
	RooArgSet& allVars, RooArgSet& integVars,
	const char* /* rangeName */) const
{
    int retVal = 0;
    if (allVars.find(x.arg())) {
	integVars.add(x.arg());
	retVal += 1;
    }
    if (allVars.find(y.arg())) {
	integVars.add(y.arg());
	retVal += 2;
    }
    return retVal;
}

template <class BASE>
Double_t RooBinned2DBicubicBase<BASE>::analyticalIntegral(
	Int_t code, const char* rangeName) const
{
    switch (code) {
	case 1:
	    return evalX(x.min(rangeName), x.max(rangeName), y);
	case 2:
	    return evalY(x, y.min(rangeName), y.max(rangeName));
	case 3:
	    return evalXY(x.min(rangeName), x.max(rangeName),
		    y.min(rangeName), y.max(rangeName));
    };
    coutE(Integration) << base().GetName() <<
	": Unknown integration code " << code << " supplied!" << std::endl;
    return -1.;
}

template<class BASE>
double RooBinned2DBicubicBase<BASE>::eval(double x, double y) const
{
    if (x != x || y != y) return 0.;
    if (x <= xmin || x >= xmax || y <= ymin || y >= ymax) return 0.;
    // find the bin in question
    const int binx = int(double(nBinsX) * (x - xmin) / (xmax - xmin));
    const int biny = int(double(nBinsY) * (y - ymin) / (ymax - ymin));
    // get low edge of bin
    const double xlo = double(nBinsX - binx) / double(nBinsX) * xmin +
	double(binx) / double(nBinsX) * xmax;
    const double ylo = double(nBinsY - biny) / double(nBinsY) * ymin +
	double(biny) / double(nBinsY) * ymax;
    // normalise to coordinates in unit sqare
    const double hx = (x - xlo) / binSizeX;
    const double hy = (y - ylo) / binSizeY;
    // monomials
    const double hxton[4] = { hx * hx * hx, hx * hx, hx, 1. };
    const double hyton[4] = { hy * hy * hy, hy * hy, hy, 1. };
    // sum up
    double retVal = 0.;
    for (int k = 0; k < NCoeff; ++k)
	retVal += coeff(binx, biny, k) * hxton[k % 4] * hyton[k / 4];

    return retVal;
}

template<class BASE>
double RooBinned2DBicubicBase<BASE>::evalX(double x1, double x2, double y) const
{
    if (x1 != x1 || x2 != x2 || y != y) return 0.;
    // find the bin in question
    const int biny = int(double(nBinsY) * (y - ymin) / (ymax - ymin));
    // get low edge of bin
    const double ylo = double(nBinsY - biny) / double(nBinsY) * ymin +
	double(biny) / double(nBinsY) * ymax;
    // normalise to coordinates in unit sqare
    const double hy = (y - ylo) / binSizeY;
    // monomials
    const double hyton[4] = { hy * hy * hy, hy * hy, hy, 1. };
    // integral
    double sum = 0.;
    for (int binx = 0; binx < nBinsX; ++binx) {
	// get low/high edge of bin
	const double xhi = double(nBinsX - binx - 1) / double(nBinsX) * xmin +
	    double(binx + 1) / double(nBinsX) * xmax;
	if (xhi < x1) continue;
	const double xlo = double(nBinsX - binx) / double(nBinsX) * xmin +
	    double(binx) / double(nBinsX) * xmax;
	if (xlo > x2) break;
	// work out integration range
	const double a = ((xlo > x1) ? 0. : (x1 - xlo)) / binSizeX;
	const double b = ((xhi < x2) ? binSizeX : (x2 - xlo)) / binSizeX;
	// integrated monomials
	const double hxton[4] = { 0.25 * (b * b * b * b - a * a * a * a),
	    (b * b * b - a * a * a) / 3., 0.5 * (b * b - a * a), b - a };
	double lsum = 0.;
	for (int k = 0; k < NCoeff; ++k)
	    lsum += coeff(binx, biny, k) * hxton[k % 4] * hyton[k / 4];
	sum += lsum;
    }
    // move from unit square coordinates to user coordinates
    return sum * binSizeX;
}

template<class BASE>
double RooBinned2DBicubicBase<BASE>::evalY(double x, double y1, double y2) const
{
    if (x != x || y1 != y1 || y2 != y2) return 0.;
    // find the bin in question
    const int binx = int(double(nBinsX) * (x - xmin) / (xmax - xmin));
    // get low edge of bin
    const double xlo = double(nBinsX - binx) / double(nBinsX) * xmin +
	double(binx) / double(nBinsX) * xmax;
    // normalise to coordinates in unit sqare
    const double hx = (x - xlo) / binSizeX;
    // monomials
    const double hxton[4] = { hx * hx * hx, hx * hx, hx, 1. };
    // integral
    double sum = 0.;
    for (int biny = 0; biny < nBinsY; ++biny) {
	// get low/high edge of bin
	const double yhi = double(nBinsY - biny - 1) / double(nBinsY) * ymin +
	    double(biny + 1) / double(nBinsY) * ymax;
	if (yhi < y1) continue;
	const double ylo = double(nBinsY - biny) / double(nBinsY) * ymin +
	    double(biny) / double(nBinsY) * ymax;
	if (ylo > y2) break;
	// work out integration range
	const double a = ((ylo > y1) ? 0. : (y1 - ylo)) / binSizeY;
	const double b = ((yhi < y2) ? binSizeY : (y2 - ylo)) / binSizeY;
	// integrated monomials
	const double hyton[4] = { 0.25 * (b * b * b * b - a * a * a * a),
	    (b * b * b - a * a * a) / 3., 0.5 * (b * b - a * a), b - a };
	double lsum = 0.;
	for (int k = 0; k < NCoeff; ++k)
	    lsum += coeff(binx, biny, k) * hxton[k % 4] * hyton[k / 4];
	sum += lsum;
    }
    // move from unit square coordinates to user coordinates
    return sum * binSizeY;
}

template<class BASE>
double RooBinned2DBicubicBase<BASE>::evalXY(
	double x1, double x2, double y1, double y2) const
{
    if (x1 != x1 || y1 != y1) return 0.;
    if (x2 != x2 || y2 != y2) return 0.;
    // integral
    double sum = 0.;
    for (int biny = 0; biny < nBinsY; ++biny) {
	// get low/high edge of bin
	const double yhi = double(nBinsY - biny - 1) / double(nBinsY) * ymin +
	    double(biny + 1) / double(nBinsY) * ymax;
	if (yhi < y1) continue;
	const double ylo = double(nBinsY - biny) / double(nBinsY) * ymin +
	    double(biny) / double(nBinsY) * ymax;
	if (ylo > y2) break;
	// work out integration range
	const double ay = ((ylo > y1) ? 0. : (y1 - ylo)) / binSizeY;
	const double by = ((yhi < y2) ? binSizeY : (y2 - ylo)) / binSizeY;
	const bool yFullyContained = std::abs(by - ay - 1.0) < 1e-15;
	// integrated monomials
	const double hyton[4] = {
	    0.25 * (by * by * by * by - ay * ay * ay * ay),
	    (by * by * by - ay * ay * ay) / 3., 0.5 * (by * by - ay * ay),
	    by - ay };
	for (int binx = 0; binx < nBinsX; ++binx) {
	    // get low/high edge of bin
	    const double xhi = double(nBinsX - binx - 1) / double(nBinsX) * xmin +
		double(binx + 1) / double(nBinsX) * xmax;
	    if (xhi < x1) continue;
	    const double xlo = double(nBinsX - binx) / double(nBinsX) * xmin +
		double(binx) / double(nBinsX) * xmax;
	    if (xlo > x2) break;
	    // work out integration range
	    const double ax = ((xlo > x1) ? 0. : (x1 - xlo)) / binSizeX;
	    const double bx = ((xhi < x2) ? binSizeX : (x2 - xlo)) / binSizeX;
	    const bool xFullyContained = std::abs(bx - ax - 1.0) < 1e-15;
	    if (xFullyContained && yFullyContained) {
		// for fully contained bins, we have cached the integral
		sum += coeff(binx, biny, NCoeff);
		continue;
	    }
	    // integrated monomials
	    const double hxton[4] = {
		0.25 * (bx * bx * bx * bx - ax * ax * ax * ax),
		(bx * bx * bx - ax * ax * ax) / 3., 0.5 * (bx * bx - ax * ax),
		bx - ax };
	    // integrate over bin

	    double lsum = 0.;
	    for (int k = 0; k < NCoeff; ++k)
		lsum += coeff(binx, biny, k) * hxton[k % 4] * hyton[k / 4];
	    sum += lsum;
	}
    }
    // move from unit square coordinates to user coordinates
    return sum * binSizeX * binSizeY;
}

// instantiate the templates we want to have
#ifndef __GCCXML__
template class RooBinned2DBicubicBase<RooAbsReal>;
template class RooBinned2DBicubicBase<RooAbsPdf>;
#endif // __GCCXML__

// vim: ft=cpp:sw=4:tw=78
