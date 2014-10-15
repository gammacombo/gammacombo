/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2014
 *
 **/

#ifndef ColorBuilder_h
#define ColorBuilder_h

#include <TROOT.h>
#include <TObjArray.h>
#include <TColor.h>

///
/// Class that can make new darker or lighter colors.
///
class ColorBuilder
{
	public:

		ColorBuilder();
		~ColorBuilder();

		int             darkcolor(int n);
		int             darklightcolor(int n, float scale);
		int             lightcolor(int n);

};

#endif
