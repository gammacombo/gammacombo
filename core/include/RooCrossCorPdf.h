#ifndef ROOCROSSCORPDF
#define ROOCROSSCORPDF

#include "RooAbsPdf.h"
//#include "RooRealProxy.h"
//#include "Rtypes.h"
#include "TMatrixDSym.h"
#include "RooListProxy.h"
#include "TVectorD.h"


class RooRealVar;

class  RooCrossCorPdf : public RooAbsPdf {


public:
  RooCrossCorPdf(const char *name, const char *title,
	const RooArgList& th, const RooArgList& obs, const TMatrixDSym& invcov, int nObsPdf1);
  RooCrossCorPdf(const  RooCrossCorPdf& other, const char* name = 0);
  virtual TObject* clone(const char* newname) const { return new  RooCrossCorPdf(*this,newname); }
  inline virtual ~ RooCrossCorPdf() {}


protected:
  RooListProxy _th ;
  RooListProxy _obs ;
  TMatrixDSym _invcov ;
  int _nObsPdf1;
  Double_t evaluate() const;


private:
  ClassDef( RooCrossCorPdf,2) //  RooCrossCorPdf function PDF
};

#endif
