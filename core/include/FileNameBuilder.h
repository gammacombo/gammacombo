/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef FileNameBuilder_h
#define FileNameBuilder_h

#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

// forward declarations - include headers only in
// cpp file to deal with circular dependencies
class MethodAbsScan;
class GammaComboEngine;
class Combiner;

///
/// Class that defines the file names of various files being
/// written across the project.
///
class FileNameBuilder
{
public:

	FileNameBuilder(OptParser *arg, TString name="gammacombo");
	~FileNameBuilder();
    
	TString getFileBaseName(const GammaComboEngine *gc);
	TString getFileBaseName(const Combiner *c);
  TString getFileNameScanner(const MethodAbsScan *c);

private:
      
	OptParser *_arg;                    ///< command line arguments
	TString _basename;									///< the base name, e.g. "gammacombo"
};

#endif
