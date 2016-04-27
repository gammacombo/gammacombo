#ifndef PDF_DatasetTutorial_h
#define PDF_DatasetTutorial_h

#include "PDF_Datasets.h"
 

class PDF_DatasetTutorial : public PDF_Datasets
{
public:
  PDF_DatasetTutorial(RooWorkspace* w);
  RooFitResult* fit(RooDataSet* dataToFit);
  void          generateToys(int SeedShift = 0) override;
  ~PDF_DatasetTutorial();

  private:
    bool    drawFitsDebug;  //> for visualizing toys and fit results, only changeable in the code
};

#endif