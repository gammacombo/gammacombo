#ifndef PDF_DatasetCustom_h
#define PDF_DatasetCustom_h

#include "PDF_Datasets.h"
 

class PDF_DatasetCustom : public PDF_Datasets
{
public:
  PDF_DatasetCustom(RooWorkspace* w);
  RooFitResult* fit(RooDataSet* dataToFit);
  RooFitResult* fitBkg(RooDataSet* dataToFit);
  void  generateToys(int SeedShift = 0) override;
  void  generateBkgToys(int SeedShift = 0) override;
  void  plotting(std::string plotString, RooDataSet* data, int count=0, bool isToy=0);
  ~PDF_DatasetCustom();

  std::string massVarName =""; 
  std::string plotDir ="plots/pdf/";
  bool sanity = false; 

  private:
  int counter =0;
  int counterToy =0;
  int counterBG =0;
  int counterSB =0;
};

#endif
