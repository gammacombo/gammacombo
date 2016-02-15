#ifndef PDF_DatasetTutorial_h
#define PDF_DatasetTutorial_h

#include "PDF_Datasets_Abs.h"
 

class PDF_DatasetTutorial : public PDF_Datasets_Abs
{
public:
  PDF_DatasetTutorial(RooWorkspace* w);
  RooFitResult* fit(bool fitToys = kTRUE) override;
  void          generateToys(int SeedShift = 0) override;
  ~PDF_DatasetTutorial();

  private:
    bool    drawFitsDebug;  //> for visualizing toys and fit results, only changeable in the code
};

#endif