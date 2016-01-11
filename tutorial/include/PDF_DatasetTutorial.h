#ifndef PDF_DatasetTutorial_h
#define PDF_DatasetTutorial_h

#include "PDF_Datasets_Abs.h"
 

class PDF_DatasetTutorial : public PDF_Datasets_Abs
{
public:
  PDF_DatasetTutorial(RooWorkspace* w);
  RooFitResult* fit(bool fitToys = kTRUE);
  void          generateToys(int SeedShift = 0) override;
  void          generateToysGlobalObservables(bool useConstrPdf = true, int SeedShift = 0) override;
  inline void   setNToys(int nToys){toysToGenerate = nToys;};
  inline int    getNToys(){return toysToGenerate;};
  ~PDF_DatasetTutorial();

  protected: 
    int     toysToGenerate; //> number of toys to generate
  private:
    bool    drawFitsDebug;  //> for visualizing toys and fit results, only changeable in the code
};

#endif