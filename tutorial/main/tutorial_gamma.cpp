/**
 * Author: Matthew Kenzie <matthew.kenzie@cern.ch>
 * Date: December 2020
 *
 **/

#include "GammaComboEngine.h"

#include "PDF_GGSZ.h"
#include "PDF_GGSZ_cartesian.h"

int main(int argc, char* argv[])
{
	GammaComboEngine gc("gamma_ggsz", argc, argv);

	///////////////////////////////////////////////////
	//
	// define PDFs
	//
	///////////////////////////////////////////////////

	gc.addPdf(1, new PDF_GGSZ          ("belle","belle","belle"),   "GGSZ measurement in (r,d,g)");
	gc.addPdf(2, new PDF_GGSZ_cartesian("belle","belle","belle"),   "GGSZ measurement in (x,y)");

  ///////////////////////////////////////////////////
	//
	// Define combinations
	//
	///////////////////////////////////////////////////

	gc.newCombiner(0, "empty", "empty");

	///////////////////////////////////////////////////
	//
	// Run
	//
	///////////////////////////////////////////////////

	gc.run();

  return 0;
}
