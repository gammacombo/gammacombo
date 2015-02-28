/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Feb 2015
 *
 * Class holding the 1-N sigma confidence contours.
 *
 **/

#ifndef ConfidenceContours_h
#define ConfidenceContours_h

#include "TRoot.h"
#include "OptParser.h"
#include "Contour.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ConfidenceContours
{
	public:

		ConfidenceContours(OptParser *arg);
		~ConfidenceContours();
		void                     computeContours(TH2F* hist, histogramType type);
		void                     Draw();

	private:

		TH2F*                    addBoundaryBins(TH2F* hist);
		void                     transformChi2valleyToHill(TH2F* hist,float offset);
		OptParser*               m_arg;      ///< command line arguments
		vector<Contour*>         m_contours; ///< container for the 1,...,N sigma contours
};

#endif
