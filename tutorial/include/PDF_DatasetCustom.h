#ifndef PDF_DatasetCustom_h
#define PDF_DatasetCustom_h

#include "PDF_Datasets.h"
 

class PDF_DatasetCustom : public PDF_Datasets
{
public:
  PDF_DatasetCustom(RooWorkspace* w);
  RooFitResult* fitBkg(RooDataSet* dataToFit);
  void          generateBkgToys(int SeedShift = 0) override;
  ~PDF_DatasetCustom();

  private:
};

#endif
