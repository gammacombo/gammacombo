/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

#ifndef ROO_DOUBLECB
#define ROO_DOUBLECB

#include "RooAbsPdf.h"
#include "RooRealProxy.h"

class RooRealVar;

class RooAbsReal;

class RooDoubleCB : public RooAbsPdf {

public:
  RooDoubleCB();

  RooDoubleCB(const char *name, const char *title, RooAbsReal &_x,
              RooAbsReal &_mean, RooAbsReal &_width, RooAbsReal &_alpha1,
              RooAbsReal &_n1, RooAbsReal &_alpha2, RooAbsReal &_n2);

  RooDoubleCB(const RooDoubleCB &other, const char *name = 0);

  virtual TObject *clone(const char *newname) const {

    return new RooDoubleCB(*this, newname);
  }

  inline virtual ~RooDoubleCB() {}

protected:
  RooRealProxy x;

  RooRealProxy mean;

  RooRealProxy width;

  RooRealProxy alpha1;

  RooRealProxy n1;

  RooRealProxy alpha2;

  RooRealProxy n2;

  Double_t evaluate() const;

  // private:

  ClassDef(RooDoubleCB, 1)
};

#endif
