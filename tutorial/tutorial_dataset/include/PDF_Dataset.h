#ifndef PDF_Dataset_h
#define PDF_Dataset_h

#include "PDF_Generic_Abs.h"
 
using namespace RooFit;
using namespace std;
using namespace Utils;

class PDF_Dataset : public PDF_Generic_Abs
{
public:
  PDF_Dataset(RooWorkspace* w, OptParser* opt);
  RooFitResult* fit(bool fitToys = kTRUE);
  void          generateToys(int SeedShift = 0);
  inline void   setNToys(int nToys){toysToGenerate = nToys;};
  inline int    getNToys(){return toysToGenerate;};
  ~PDF_Dataset();

  protected: 
    int     toysToGenerate; //> number of toys to generate
  private:
    bool    drawFitsDebug;  //> for visualizing toys and fit results, only changeable in the code
};

#endif