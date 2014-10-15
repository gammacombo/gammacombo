/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: March 2014
 *
 * Class to print structures into *.dot files that can be
 * proceesed by graphviz. For example the Measurement/parameter
 * structure of the combiners.
 *
 **/

#ifndef Graphviz_h
#define Graphviz_h

#include "Combiner.h"
#include "OptParser.h"
#include "Utils.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Utils;

class Graphviz
{
public:

  Graphviz(OptParser *arg);
  ~Graphviz();
  
  void       printCombiner(Combiner* cmb);
  void       printCombinerLayer(Combiner* cmb);
    
private:
  
  TString   graphvizString(TString s);
  // TString   graphvizString(string s);
  bool      isDmixingParameter(TString s);
  ofstream& openFile(TString name);
  OptParser *arg;                     ///< command line arguments
};

#endif
