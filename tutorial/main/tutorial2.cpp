/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Oct 2014
 *
 **/

#include <stdlib.h>
#include "GammaComboEngine.h"

#include "PDF_Circle.h"
#include "PDF_Gaus.h"
#include "PDF_Gaus2d.h"

using namespace std;
using namespace RooFit;
using namespace Utils;

int main(int argc, char* argv[])
{
	GammaComboEngine gc("tutorial", argc, argv);

	///////////////////////////////////////////////////
	//
	// define PDFs
	//
	///////////////////////////////////////////////////

	gc.addPdf(1, new PDF_Gaus(lumi1fb,lumi1fb,lumi1fb),   "1D Gaussian (A)");
	gc.addPdf(2, new PDF_Gaus(lumi2fb,lumi2fb,lumi2fb),   "1D Gaussian (B)");
	gc.addPdf(3, new PDF_Gaus2d(lumi1fb,lumi1fb,lumi1fb), "2D Gaussian (A,B)");
	gc.addPdf(4, new PDF_Circle(lumi1fb,lumi1fb,lumi1fb), "circle (A,B)");

	///////////////////////////////////////////////////
	//
	// Define combinations
	//
	///////////////////////////////////////////////////

	gc.newCombiner(0, "empty", "empty");
	gc.newCombiner(1, "tutorial_1", "Gaus 1",           1);
	gc.newCombiner(2, "tutorial_2", "Gaus 2",           2);  
	gc.newCombiner(3, "tutorial_3", "Gaus 1 & Gaus 2",  1,2);
	gc.newCombiner(4, "tutorial_4", "2D Gaus",          3);
	gc.newCombiner(5, "tutorial_5", "2D Gaus + Gaus 1", 2,3);
	gc.newCombiner(6, "tutorial_6", "Circle",           4);
	gc.newCombiner(7, "tutorial_7", "2D Gaus + Circle", 3,4);

	///////////////////////////////////////////////////
	//
	// Run
	//
	///////////////////////////////////////////////////

	gc.run();
}
