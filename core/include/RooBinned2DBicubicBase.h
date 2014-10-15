/**
 * @file RooBinned2DBicubicBase.h
 *
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-29
 */
#ifndef _ROOBINNED2DBICUBICBASE
#define _ROOBINNED2DBICUBICBASE

#include <exception>

#include <RooAbsReal.h>
#include <RooAbsPdf.h>
#include <RooRealProxy.h>

#include "SharedArray.h"

class TH2;
class RooArgSet;

/** @brief smoothly interpolate a TH2 for use in RooFit
 *
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-30
 *
 * Smoothly interpolate between bin midpoints of a 2D histogram with
 * equal-sized bins
 *
 * The interpolation function coincides with the saved histogram values in bin
 * midpoints; a cubic interpolation polynomial is used. Bins must all have the
 * same size. The interpolation polynomial in a "cell" between four bin
 * midpoints is:
 *
 * \f[ p(x, y) = \sum_{i=0}^3 \sum_{j=0}^3 a_{ij} x^i y^j \f]
 *
 * The coefficients \f$a_{ij}\f$ are determined by the requirement that
 * function value, first derivatives and the mixed second derivative must agree
 * with that of the binned function at the four bin midpoints at the edges of
 * the cell. Symmetric finite differences are used to approximate these
 * derivatives.
 *
 * For each cell, the coefficients are determined at constuction time. The
 * object also keeps a cache of 2D integrals over complete cells, such that 2D
 * integrations can be done analytically in a reasonably short amount of time.
 */
template <class BASE>
class RooBinned2DBicubicBase : public BASE
{
    private:
	/// length of coefficient record in array
	enum { NCoeff = 16, CoeffRecLen = 17 };
	/// exception to throw in case of variable-sized bins
	class BinSizeException : public std::exception
        {
	    public:
		/// constructor
		BinSizeException() throw () { }
		/// destructor
		virtual ~BinSizeException() throw ();
		/// description
		virtual const char* what() const throw ();
	};

    public:
	/// constructor for ROOT I/O (ROOT does not care)
	RooBinned2DBicubicBase() : coeffs(0) { }
	/// constructor from histogram
	RooBinned2DBicubicBase(
		const char* name, const char* title, const TH2& h,
		RooAbsReal& xvar, RooAbsReal& yvar);
	/// copy constructor
	RooBinned2DBicubicBase(
		const RooBinned2DBicubicBase<BASE>& other, const
		char* name = 0);
	/// assignment operator
	RooBinned2DBicubicBase<BASE>& operator=(
		const RooBinned2DBicubicBase<BASE>& other);
	/// clone method
	virtual RooBinned2DBicubicBase<BASE>* clone(
		const char* newname = 0) const;

	/// destructor
	virtual ~RooBinned2DBicubicBase();

	/// evaluation of function
	virtual Double_t evaluate() const;
	/// advertise analytical integrals
	virtual Int_t getAnalyticalIntegral(
		RooArgSet& allVars, RooArgSet& integVars,
		const char* rangeName = 0) const;
	/// evaluate advertised analytical integral
        virtual Double_t analyticalIntegral(
		Int_t code, const char* rangeName = 0) const;


    private:
	/// proxy for RooAbsReals
	RooRealProxy x, y;
	/// number of bins
	int nBinsX, nBinsY;
	/// bin size in x and y directions
	double binSizeX, binSizeY;
	/// x and y range
	double xmin, xmax, ymin, ymax;
	/// coefficients of interpolation polynomials
	SharedArray<double> coeffs;

	/// helper to deal with TH2 bin contents
	double histcont(const TH2& h, int xbin, int ybin) const;
	/// d/dx finite differences of histogram
	double dhistdx(const TH2& h, int xbin, int ybin) const;
	/// d/dy finite differences of histogram
	double dhistdy(const TH2& h, int xbin, int ybin) const;
	/// d^2/dydx finite differences of histogram
	double d2histdxdy(const TH2& h, int xbin, int ybin) const;

	/// const convenience access to base class
	inline const BASE& base() const
	{ return *reinterpret_cast<const BASE*>(this); }
	/// convenience access to base class
	inline BASE& base()
	{ return *reinterpret_cast<BASE*>(this); }

	/// const access to coefficients
	inline SharedArray<double>::RWProxy coeff(
		int binx, int biny, int coeff) const
	{ return coeffs[coeff + CoeffRecLen * (binx + nBinsX * biny)]; }
	/// access to coefficients
	inline SharedArray<double>::RWProxy coeff(int binx, int biny, int coeff)
	{ return coeffs[coeff + CoeffRecLen * (binx + nBinsX * biny)]; }

	/// evaluate at given point
	double eval(double x, double y) const;
	/// evaluate integral over x at given y from (x1, y) to (x2, y)
	double evalX(double x1, double x2, double y) const;
	/// evaluate integral over y at given x from (x, y1) to (x, y2)
	double evalY(double x, double y1, double y2) const;
	/// evaluate integral over x and y from (x1, y1) to (x2, y2)
	double evalXY(double x1, double x2, double y1, double y2) const;

	ClassDef(RooBinned2DBicubicBase, 1);
};

// help genreflex
#ifdef __GCCXML__
template class RooBinned2DBicubicBase<RooAbsReal>;
template class RooBinned2DBicubicBase<RooAbsPdf>;
#endif // __GCCXML__

typedef RooBinned2DBicubicBase<RooAbsReal> RooBinned2DBicubic;
typedef RooBinned2DBicubicBase<RooAbsPdf> RooBinned2DBicubicPdf;

#endif // _ROOBINNED2DBICUBICBASE

// vim: ft=cpp:sw=4:tw=78
