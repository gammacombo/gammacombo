/**
 * \author Till Moritz Karbach, moritz.karbach@cern.ch
 * \date August 2014
 * \brief Class that can make new darker or lighter colors.
 *
 **/

#ifndef ColorBuilder_h
#define ColorBuilder_h

#include <TROOT.h>
#include <TObjArray.h>
#include <TColor.h>

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
