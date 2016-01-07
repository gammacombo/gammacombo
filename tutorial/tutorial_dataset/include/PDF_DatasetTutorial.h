#ifndef PDF_DatasetTutorial_h
#define PDF_DatasetTutorial_h

#include "PDF_DatasetTutorials_Abs.h"
 
using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_DatasetTutorial : public PDF_DatasetTutorials_Abs
{
public:
  PDF_DatasetTutorial(RooWorkspace* w, OptParser* opt);
  RooFitResult* fit(bool fitToys = kTRUE);
  void          generateToys(int SeedShift = 0);
  inline void   setNToys(int nToys){toysToGenerate = nToys;};
  inline int    getNToys(){return toysToGenerate;};
  ~PDF_DatasetTutorial();

  protected: 
    int     toysToGenerate; //> number of toys to generate
  private:
    bool    drawFitsDebug;  //> for visualizing toys and fit results, only changeable in the code
};

#endif