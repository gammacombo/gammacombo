/**
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: April 2013
 *
 * Class representing a (nuisance) parameter.
 *
 **/

#ifndef Parameter_h
#define Parameter_h

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "TString.h"

using namespace std;

class Parameter
{
public:
                  Parameter();
  inline virtual  ~Parameter(){};
  inline void     setVal(double v){startvalue=v;};
  void            Print();

  struct Range { float min; float max; };

  TString name;
  TString title;
  TString unit;
  float startvalue;
  Range phys;
  Range scan;
  Range force;
  Range bboos;
  Range free;
};

#endif
