#include "CLIntervalMaker.h"

	CLIntervalMaker::CLIntervalMaker(OptParser *arg, const TH1F &pvalues)
: _pvalues(pvalues)
{
	assert(arg);
	_arg = arg;
	// for ( int i=1; i<pvalues.GetNbinsX(); i++ )
	// 	cout << i << " " << pvalues.GetBinCenter(i) << " " << pvalues.GetBinContent(i) << endl;
}

CLIntervalMaker::~CLIntervalMaker()
{}

///
/// Add a maximum (e.g. found by the Prob method) to calculate the
/// confidence intervals around it.
///
/// \param value - parameter value at the maximum
/// \param method - details on how this maximum was found
///
void CLIntervalMaker::provideMorePreciseMaximum(float value, TString method)
{
	// cout << "CLIntervalMaker::provideMorePreciseMaximum() : " << value << endl;
	float level = 1.; // accept this many bin sizes deviation
	for ( int i=0; i<_clintervals1sigma.size(); i++ ){
		if ( fabs(_clintervals1sigma[i].central - value) < level*_pvalues.GetBinWidth(1) ){
			_clintervals1sigma[i].central = value;
			_clintervals1sigma[i].centralmethod = method;
			_clintervals1sigma[i].pvalueAtCentral = _pvalues.GetBinContent(valueToBin(value));
		}
	}
	for ( int i=0; i<_clintervals2sigma.size(); i++ ){
		if ( fabs(_clintervals2sigma[i].central - value) < level*_pvalues.GetBinWidth(1) ){
			_clintervals2sigma[i].central = value;
			_clintervals2sigma[i].centralmethod = method;
			_clintervals2sigma[i].pvalueAtCentral = _pvalues.GetBinContent(valueToBin(value));
		}
	}
}

///
/// Finds the maxima of the _pvalue histogram and fills
/// _clintervals1sigma and _clintervals2sigma. Only accepts
/// maxima that are not similar to existing ones, that were, e.g.,
/// set by addMaximum().
///
/// \param pValueThreshold - ignore maxima under this pvalue threshold
/// 				to reject low statistics plugin crap
///
void CLIntervalMaker::findMaxima(float pValueThreshold)
{
	for ( int i=2; i<_pvalues.GetNbinsX()-1; i++ ){
		if ( _pvalues.GetBinContent(i-1) < _pvalues.GetBinContent(i)
				&& _pvalues.GetBinContent(i)   > _pvalues.GetBinContent(i+1) ){
			if ( _pvalues.GetBinContent(i) > pValueThreshold ){
				CLInterval cli;
				cli.centralmethod = "max bin";
				// cli.centralmethod = Form("maximum bin above p=%.2f",pValueThreshold);
				cli.central = _pvalues.GetBinCenter(i);
				cli.pvalueAtCentral = _pvalues.GetBinContent(i);
				_clintervals1sigma.push_back(cli); // push_back stores copies
				_clintervals2sigma.push_back(cli);
			}
		}
	}
}
// void CLIntervalMaker::findMaxima(float pValueThreshold)
// {
// 	for ( int i=3; i<_pvalues.GetNbinsX()-2; i++ ){
// 		if ( _pvalues.GetBinContent(i-1) < _pvalues.GetBinContent(i)
// 			&& _pvalues.GetBinContent(i)   > _pvalues.GetBinContent(i+1)
// 			&& _pvalues.GetBinContent(i-2) < _pvalues.GetBinContent(i-1)
// 			&& _pvalues.GetBinContent(i+1) > _pvalues.GetBinContent(i+2)
// 			){
// 			if ( _pvalues.GetBinContent(i) > pValueThreshold ){
// 				CLInterval cli;
// 				cli.centralmethod = "max bin";
// 				// cli.centralmethod = Form("maximum bin above p=%.2f",pValueThreshold);
// 				cli.central = _pvalues.GetBinCenter(i);
// 				cli.pvalueAtCentral = _pvalues.GetBinContent(i);
// 				_clintervals1sigma.push_back(cli); // push_back stores copies
// 				_clintervals2sigma.push_back(cli);
// 			}
// 		}
// 	}
// }


///
/// Checks if a bin is in a CL interval: it is, when it's pvalue
/// is above the provided threshold.
///
/// \param binid - bin id of _pvalues histogram
/// \param pvalue - the p value threshold
/// \return - true, if bin is in interval
///
bool CLIntervalMaker::isInInterval(int binid, float pvalue) const
{
	return _pvalues.GetBinContent(binid) > pvalue;
}

///
/// Stores a confidence interval found by findRawIntervalsForCentralValues().
///
/// \param binidHi - bin id of _pvalues histogram
/// \param binidLo - bin id of _pvalues histogram
/// \param pvalue - p-value threshold, accept points into interval if their p-value is above
/// \param clis - vector of confidence intervals, usually _clintervals1sigma or _clintervals2sigma
///
void CLIntervalMaker::storeRawInterval(int binidLo, int binidHi, float pvalue, vector<CLInterval> &clis)
{
	CLInterval c;
	c.pvalue = pvalue;
	// use the histogram border for non-closed intervals,
	// else the bin center
	if ( binidLo==1 ){
		c.min = _pvalues.GetXaxis()->GetXmin();
		c.minmethod = "border";
		c.minclosed = false;
	}
	else {
		c.min = binToValue(binidLo);
		c.minmethod = "raw";
		c.minclosed = true;
	}
	if ( binidHi==_pvalues.GetNbinsX() ){
		c.max = _pvalues.GetXaxis()->GetXmax();
		c.maxmethod = "border";
		c.maxclosed = false;
	}
	else {
		c.max = binToValue(binidHi);
		c.maxmethod = "raw";
		c.maxclosed = true;
	}
	c.central = c.min+(c.max-c.min)/2.;
	c.centralmethod = "raw mid";
	c.pvalueAtCentral = _pvalues.GetBinContent(_pvalues.GetXaxis()->FindBin(c.central));
	clis.push_back(c);
}

///
/// Finds the raw interval boundaries corresponding to the provided p-value
/// The raw intervals are just connected sections where all bins of the p-value
/// histogram _pvalues lie above the given threshold.
///
/// \param pvalue - pvalue of the intervals to find
/// \param clis - saves intervals into this vector of confidence intervals, usually _clintervals1sigma or _clintervals2sigma
///
void CLIntervalMaker::findRawIntervals(float pvalue, vector<CLInterval> &clis)
{
	bool intervalIsOpened = false;
	int intervalBinLo = 1;
	int intervalBinHi = _pvalues.GetNbinsX();

	// check if we start with an opened interval
	if ( isInInterval(1, pvalue) ) intervalIsOpened = true;

	// loop over the pvalue histogram and find the intervals
	for ( int j=1; j<=_pvalues.GetNbinsX(); j++ ){
		if ( intervalIsOpened ){
			if ( isInInterval(j, pvalue) && j!=_pvalues.GetNbinsX() ) continue; // right border will close!
			intervalBinHi = j;
			intervalIsOpened = false;
			storeRawInterval(intervalBinLo, intervalBinHi, pvalue, clis);
		}
		else{
			if ( !isInInterval(j, pvalue) ) continue;
			intervalBinLo = j;
			intervalIsOpened = true;
		}
	}
}

///
/// Find the interval boundaries corresponding to the central values
/// already saved in _clintervals1sigma or _clintervals2sigma, and to
/// the number of sigmas given. Saves the result into the intevals in
/// _clintervals1sigma or _clintervals2sigma.
///
/// \param pvalue - pvalue of the intervals to find
/// \param clis - list of confidence intervals holding the central value
///
void CLIntervalMaker::findRawIntervalsForCentralValues(float pvalue, vector<CLInterval> &clis)
{
	for ( int i=0; i<clis.size(); i++ ){
		if ( clis[i].pvalueAtCentral<pvalue ) continue; // skip central values that will not going to be included in an interval at this pvalue
		clis[i].pvalue = pvalue;
		clis[i].minmethod = "bins";
		clis[i].maxmethod = "bins";
		int centralValueBin = valueToBin(clis[i].central);

		// find lower interval bound
		clis[i].min = _pvalues.GetXaxis()->GetXmin();
		for ( int j=centralValueBin; j>0; j-- ){
			if ( _pvalues.GetBinContent(j) < pvalue ){
				clis[i].min = _pvalues.GetBinCenter(j);
				break;
			}
		}

		// find upper interval bound
		clis[i].max = _pvalues.GetXaxis()->GetXmax();
		for ( int j=centralValueBin; j<_pvalues.GetNbinsX(); j++ ){
			if ( _pvalues.GetBinContent(j) < pvalue ){
				clis[i].max = _pvalues.GetBinCenter(j);
				break;
			}
		}

		// check if both boundaries were found
		clis[i].minclosed = clis[i].min != _pvalues.GetXaxis()->GetXmin();
		clis[i].maxclosed = clis[i].max != _pvalues.GetXaxis()->GetXmax();
	}
}

///
/// Remove bad intervals, where findRawIntervalsForCentralValues() couldn't find
/// interval boundaries corresponding to the central value and pvalue
/// given.
///
void CLIntervalMaker::removeBadIntervals()
{
	vector<CLInterval> _clintervals1sigmaTmp;
	vector<CLInterval> _clintervals2sigmaTmp;
	for ( int i=0; i<_clintervals1sigma.size(); i++ ){
		if ( _clintervals1sigma[i].pvalue>0 ) _clintervals1sigmaTmp.push_back(_clintervals1sigma[i]);
	}
	for ( int i=0; i<_clintervals2sigma.size(); i++ ){
		if ( _clintervals2sigma[i].pvalue>0 ) _clintervals2sigmaTmp.push_back(_clintervals2sigma[i]);
	}
	_clintervals1sigma = _clintervals1sigmaTmp;
	_clintervals2sigma = _clintervals2sigmaTmp;
}

///
/// Check if the two bins i and (i+1) of the _pvalues histogram
/// are on the same side of y.
///
/// \param i - bin
/// \param y - y value
/// \return - true if i and i+1 are on the same side
///
bool CLIntervalMaker::binsOnSameSide(int i, float y) const
{
	return (  (_pvalues.GetBinContent(i)>y && _pvalues.GetBinContent(i+1)>y)
	       || (_pvalues.GetBinContent(i)<y && _pvalues.GetBinContent(i+1)<y) );
}

///
/// Tries to find a bin i such that the two bins i and (i+1) are not on the
/// same side of y. If i does already satisfy the requirement, i is returned.
/// Else also the left and right neighbor of i are checked, and returned if
/// they satisfy the requirement.
///
/// \param i - bin ID of _pvalues histogram
/// \param y - y value
/// \return - bin ID of _pvalues histogram
///
int CLIntervalMaker::checkNeighboringBins(int i, float y) const
{
	if ( !binsOnSameSide(i, y) ) return i;
	if ( 1 < i+1 && i+1 <= _pvalues.GetNbinsX()-1 && !binsOnSameSide(i+1, y) ) return i+1;
	if ( 1 < i-1 && i-1 <= _pvalues.GetNbinsX()-1 && !binsOnSameSide(i-1, y) ) return i-1;
	cout << "CLIntervalMaker::checkNeighboringBins() : WARNING : ";
	cout << "no direct neighbor of bin " << i << " lies on different sides of y=" << y;
	cout << " Using bin " << i << "." << endl;
	return i;
}

///
/// Find an interpolated x value near a certain bin position of a histogram that is the
/// best estimate for h(x)=y. Interpolates by means of a straight line between two
/// known points.
///
/// \param h - the histogram to be interpolated
/// \param i - interpolate around this bin. Must be a bin such that i and i+1 are above and below val
/// \param y - the y position we want to find the interpolated x for
/// \param val - Return value: interpolated x position
/// \return true if successful
///
bool CLIntervalMaker::interpolateLine(const TH1F* h, int i, float y, float &val) const
{
	// cout << "CLIntervalMaker::interpolateLine(): i=" << i << " y=" << y << endl;
	if ( !( 1 <= i && i <= h->GetNbinsX()-1 ) ) return false;
	if ( binsOnSameSide(i, y) ) {
		cout << "CLIntervalMaker::interpolateLine() : ERROR : bins i and i+1 on same side of y" << endl;
		return false;
	}
	float p1x = h->GetBinCenter(i);
	float p1y = h->GetBinContent(i);
	float p2x = h->GetBinCenter(i+1);
	float p2y = h->GetBinContent(i+1);
	val = p2x + (y-p2y)/(p1y-p2y)*(p1x-p2x);
	return true;
}

///
/// Improve the intervals through an interpolation with a straight line.
///
/// \param clis - list of confidence intervals holding the central value and min and max boundaries
///
void CLIntervalMaker::improveIntervalsLine(vector<CLInterval> &clis) const
{
	for ( int i=0; i<clis.size(); i++ ){
		bool wasImproved;
		float newMin, newMax;
		int binMin, binMax;
		// improve lower boundary
		if ( clis[i].minclosed ){
			binMin = checkNeighboringBins(valueToBin(clis[i].min), clis[i].pvalue);
			wasImproved = interpolateLine(&_pvalues, binMin, clis[i].pvalue, newMin);
			if ( wasImproved ){
				clis[i].minmethod = "line";
				clis[i].min = newMin;
			}
		}
		// improve upper boundary
		if ( clis[i].maxclosed ){
			binMax = checkNeighboringBins(valueToBin(clis[i].max), clis[i].pvalue);
			wasImproved = interpolateLine(&_pvalues, binMax, clis[i].pvalue, newMax);
			if ( wasImproved ){
				clis[i].maxmethod = "line";
				clis[i].max = newMax;
			}
		}
	}
}

///
/// Improve the intervals through an fit with a pol2.
///
/// \param clis - list of confidence intervals holding the central value and min and max boundaries
///
void CLIntervalMaker::improveIntervalsPol2fit(vector<CLInterval> &clis) const
{
	for ( int i=0; i<clis.size(); i++ ){
		bool wasImproved;
		float newMin, newMax, newMinErr, newMaxErr;
		int binMin, binMax;
		// improve lower boundary
		if ( clis[i].minclosed ){
			binMin = checkNeighboringBins(valueToBin(clis[i].min), clis[i].pvalue);
			wasImproved = interpolatePol2fit(&_pvalues, binMin, clis[i].pvalue, clis[i].central, false, newMin, newMinErr);
			if ( wasImproved ){
				clis[i].minmethod = "pol2";
				clis[i].min = newMin;
			}
		}
		// improve upper boundary
		if ( clis[i].maxclosed ){
			binMax = checkNeighboringBins(valueToBin(clis[i].max), clis[i].pvalue);
			wasImproved = interpolatePol2fit(&_pvalues, binMax, clis[i].pvalue, clis[i].central, true, newMax, newMaxErr);
			if ( wasImproved ){
				clis[i].maxmethod = "pol2";
				clis[i].max = newMax;
			}
		}
	}
}

///
/// Solve a quadratic equation by means of a modified pq formula:
/// @f[x^2 + \frac{p_1}{p_2} x + \frac{p_0-y}{p2} = 0@f]
///
float CLIntervalMaker::pq(float p0, float p1, float p2, float y, int whichSol) const
{
	if ( whichSol == 0 ) return -p1/2./p2 + sqrt( sq(p1/2./p2) - (p0-y)/p2 );
	else                 return -p1/2./p2 - sqrt( sq(p1/2./p2) - (p0-y)/p2 );
}

///
/// Find an interpolated x value near a certain bin position of a histogram that is the
/// best estimate for h(x)=y. Interpolates by means of fitting a second grade polynomial
/// to up to five adjacent points. Because that's giving us two solutions, we use the central
/// value and knowledge about if it is supposed to be an upper or lower boundary to pick
/// one.
///
/// \param h - the histogram to be interpolated
/// \param i - interpolate around this bin. Must be a bin such that i and i+1 are above and below val
/// \param y - the y position we want to find the interpolated x for
/// \param central - Central value of the solution we're trying to get the CL interval for.
/// \param upper - Set to true if we're computing an upper interval boundary.
/// \param val - Return value: interpolated x position
/// \param err - Return value: estimated interpolation error
/// \return true, if inpterpolation was performed, false, if conditions were not met
///
bool CLIntervalMaker::interpolatePol2fit(const TH1F* h, int i, float y, float central, bool upper,
		float &val, float &err) const
{
	// cout << "CLIntervalMaker::interpolatePol2fit(): i=" << i << " y=" << y << " central=" << central << endl;
	// check if too close to border so we don't have enough bins to fit
	if ( !( 2 <= i && i <= h->GetNbinsX()-1 ) ) return false;
	// check if all necessary bins are on the same side. They are not always,
	// if e.g. in a plugin there's statistical fluctuations
	if ( binsOnSameSide(i-1, y) && binsOnSameSide(i, y) ){
		//cout << "CLIntervalMaker::interpolatePol2fit() : ERROR : bins i-1, i, and i+1 on same side of y" << endl;
		return false;
	}

	// create a TGraph that we can fit
	TGraph *g = new TGraph(3);
	g->SetPoint(0, h->GetBinCenter(i-1), h->GetBinContent(i-1));
	g->SetPoint(1, h->GetBinCenter(i),   h->GetBinContent(i));
	g->SetPoint(2, h->GetBinCenter(i+1), h->GetBinContent(i+1));

	// see if we can add a 4th and 5th point:
	if ( 3 <= i && i <= h->GetNbinsX()-2 ){
		// add a point to the beginning
		if ( (h->GetBinContent(i-2) < h->GetBinContent(i-1) && h->GetBinContent(i-1) < h->GetBinContent(i))
		  || (h->GetBinContent(i-2) > h->GetBinContent(i-1) && h->GetBinContent(i-1) > h->GetBinContent(i)) ){
			TGraph *gNew = new TGraph(g->GetN()+1);
			gNew->SetPoint(0, h->GetBinCenter(i-2), h->GetBinContent(i-2));
			Double_t pointx, pointy;
			for ( int i=0; i<g->GetN(); i++ ){
				g->GetPoint(i, pointx, pointy);
				gNew->SetPoint(i+1, pointx, pointy);
			}
			delete g;
			g = gNew;
		}
		// add a point to the end
		if ( (h->GetBinContent(i+2) < h->GetBinContent(i+1) && h->GetBinContent(i+1) < h->GetBinContent(i))
		  || (h->GetBinContent(i+2) > h->GetBinContent(i+1) && h->GetBinContent(i+1) > h->GetBinContent(i)) ){
			g->Set(g->GetN()+1);
			g->SetPoint(g->GetN()-1, h->GetBinCenter(i+2), h->GetBinContent(i+2));
		}
	}

	// debug: show fitted 1-CL histogram
	// if ( y<0.1 )
	// // if ( methodName == TString("Plugin") && y<0.1 )
	// {
	//   // TString debugTitle = methodName + Form(" y=%.2f ",y);
	//   // debugTitle += upper?Form("%f upper",central):Form("%f lower",central);
	// 		TString debugTitle = "honk";
	//   TCanvas *c = newNoWarnTCanvas(getUniqueRootName(), debugTitle);
	//   g->SetMarkerStyle(3);
	//   g->SetHistogram(const_cast<TH1F*>(h));
	//   const_cast<TH1F*>(h)->Draw();
	//   g->DrawClone("p");
	// }

	// fit
	TF1 *f1 = new TF1("f1", "pol2", h->GetBinCenter(i-2), h->GetBinCenter(i+2));
	g->Fit("f1", "q");    // fit linear to get decent start parameters
	g->Fit("f1", "qf+");  // refit with minuit to get more correct errors (TGraph fit errors bug)
	float p[3], e[3];
	for ( int ii=0; ii<3; ii++ ){
		p[ii] = f1->GetParameter(ii);
		e[ii] = f1->GetParError(ii);
	}

	// get solution by solving the pol2 for x
	float sol0 = pq(p[0], p[1], p[2], y, 0);
	float sol1 = pq(p[0], p[1], p[2], y, 1);

	// decide which of both solutions to use based on the position of
	// the central value
	// \todo we probably don't need the central value to decide which solution
	// to use. Just take the one closest to the original bin! Can't think of a
	// situation where this should fail. So we don't need the central value in
	// this method at all, to be removed.
	int useSol = 0;
	//if ( (sol0<central && sol1>central) || (sol1<central && sol0>central) ){
		//if ( upper ){
			//if ( sol0<sol1 ) useSol = 1;
			//else             useSol = 0;
		//}
		//else{
			//if ( sol0<sol1 ) useSol = 0;
			//else             useSol = 1;
		//}
	//}
	//else{
		if ( fabs(h->GetBinCenter(i)-sol0) < fabs(h->GetBinCenter(i)-sol1) ) useSol = 0;
		else useSol = 1;
	//}
	if ( useSol==0 ) val = sol0;
	else             val = sol1;

	// try error propagation: sth is wrong in the formulae
	// float err0 = TMath::Max(sq(val-pq(p[0]+e[0], p[1], p[2], y, useSol)), sq(val-pq(p[0]-e[0], p[1], p[2], y, useSol)));
	// float err1 = TMath::Max(sq(val-pq(p[0], p[1]+e[1], p[2], y, useSol)), sq(val-pq(p[0], p[1]-e[1], p[2], y, useSol)));
	// float err2 = TMath::Max(sq(val-pq(p[0], p[1], p[2]+e[2], y, useSol)), sq(val-pq(p[0], p[1], p[2]-e[2], y, useSol)));
	// err = sqrt(err0+err1+err2);
	// printf("%f %f %f\n", val, pq(p[0]+e[0], p[1], p[2], y, useSol), pq(p[0]-e[0], p[1], p[2], y, useSol));
	// printf("%f %f %f\n", val, pq(p[0], p[1]+e[1], p[2], y, useSol), pq(p[0], p[1]-e[1], p[2], y, useSol));
	// printf("%f %f %f\n", val, pq(p[0], p[1], p[2]+e[2], y, useSol), pq(p[0], p[1], p[2]-e[2], y, useSol));
	err = 0.0;
	delete g;
	return true;
}


int CLIntervalMaker::valueToBin(float val) const
{
	return _pvalues.GetXaxis()->FindBin(val);
}


float	CLIntervalMaker::binToValue(int bin) const
{
	return _pvalues.GetBinCenter(bin);
}


void CLIntervalMaker::print()
{
	CLIntervalPrinter clp(_arg, "test", "var", "", "CLMaker's print()");
	clp.setDegrees(false);
	clp.addIntervals(_clintervals1sigma);
	clp.addIntervals(_clintervals2sigma);
	clp.print();
	cout << endl;
}

///
/// Calculate the CL intervals.
///
void CLIntervalMaker::calcCLintervals()
{
	// _clintervals1sigma.clear();
	// _clintervals2sigma.clear();
	// findMaxima(0.04); // ignore maxima under pvalue=0.04
	// print();

	// cout << "findRawIntervalsForCentralValues()" << endl;
	findRawIntervalsForCentralValues(1.-0.6827, _clintervals1sigma);
	findRawIntervalsForCentralValues(1.-0.9545, _clintervals2sigma);
	// print();

	// cout << "removeBadIntervals()" << endl;
	removeBadIntervals();
	// print();

	// find raw intervals independently from any central values
	findRawIntervals(1.-0.6827, _clintervals1sigma);
	findRawIntervals(1.-0.9545, _clintervals2sigma);

	// cout << "improveIntervalsLine()" << endl;
	improveIntervalsLine(_clintervals1sigma);
	improveIntervalsLine(_clintervals2sigma);
	// print();

	// cout << "improveIntervalsPol2fit()" << endl;
	improveIntervalsPol2fit(_clintervals1sigma);
	improveIntervalsPol2fit(_clintervals2sigma);
	//print();
}
