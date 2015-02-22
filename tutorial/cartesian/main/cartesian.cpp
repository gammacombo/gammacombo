/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Oct 2014
 *
 **/

#include <stdlib.h>
#include "GammaComboEngine.h"
#include "PDF_rb.h"
#include "PDF_Cartesian.h"

using namespace std;
using namespace RooFit;
using namespace Utils;

int main(int argc, char* argv[])
{
	GammaComboEngine gc("cartesian", argc, argv);

	// define PDFs
	gc.addPdf(1, new PDF_Cartesian("year2014","year2014","year2014"),   "Cartesian");
	gc.addPdf(2, new PDF_rb("year2013","year2013","year2013"),   "rb");

	// Define combinations
	gc.newCombiner(0, "empty", "empty");
	gc.newCombiner(1, "cartesian1", "Cartesian",		1);
	gc.newCombiner(2, "cartesian2", "rb",           	2);
	gc.newCombiner(3, "cartesian3", "Cartesian & rb",	1,2);

	// Run
	gc.run();
}

