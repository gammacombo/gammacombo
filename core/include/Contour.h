/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Feb 2015
 *
 * Class holding a 2D contour consisting of several TGraphs.
 *
 **/

#ifndef Contour_h
#define Contour_h

#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class Contour
{
	public:

		Contour(OptParser *arg, TList *listOfGraphs);
		~Contour();
		void               Draw();
		inline void        setSigma(int s){m_sigma=s;};

	private:

		OptParser*         m_arg;         ///< command line arguments
		vector<TGraph*>    m_contours;     ///< container for the several disjoint subcontours
		int                m_sigma;       ///< sigma level of the contour
};

#endif
