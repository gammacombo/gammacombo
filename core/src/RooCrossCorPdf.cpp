#include "RooFit.h"
#include <iostream>
#include <math.h>

#include "RooCrossCorPdf.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"



RooCrossCorPdf::RooCrossCorPdf(const char *name, const char *title,
		const RooArgList& th, const RooArgList& obs, const TMatrixDSym& invcov, int nObsPdf1 ) :

	RooAbsPdf(name, title),
	_th("th","parameters",this,kTRUE,kFALSE),
	_obs("obs","observables",this,kTRUE,kFALSE),
	_invcov(invcov),
	_nObsPdf1(nObsPdf1)
{
	_th.add(th) ;
	_obs.add(obs) ;
}


RooCrossCorPdf::RooCrossCorPdf(const RooCrossCorPdf& other, const char* name) :
	RooAbsPdf(other, name), 
	_th("th",this,other._th), 
	_obs("obs",this,other._obs),
	_invcov(other._invcov),
	_nObsPdf1(other._nObsPdf1)
{
}


Double_t RooCrossCorPdf::evaluate() const 
{
	//return 1.;
	// Represent observables and theory as vector
	TVectorD obs(_obs.getSize()) ;
	TVectorD th(_th.getSize()) ;
	for (int i=0 ; i<_obs.getSize() ; i++) {
		obs[i] = ((RooAbsReal*)_obs.at(i))->getVal() ;
		th[i] = ((RooAbsReal*)_th.at(i))->getVal() ;
	}

	double ret = 0.0;
	for ( int i=0; i<_obs.getSize(); i++ )
		for ( int j=i; j<_obs.getSize(); j++ ){
			if ( i< _nObsPdf1 && j< _nObsPdf1 ) continue;
			if ( i>=_nObsPdf1 && j>=_nObsPdf1 ) continue;
			if ( _invcov[i][j]==0 ) continue;
			ret += _invcov[i][j] * (obs[i]-th[i]) * (obs[j]-th[j]);
		}
	return exp(-ret);
}

