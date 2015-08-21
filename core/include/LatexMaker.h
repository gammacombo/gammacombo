#ifndef LatexMaker_h
#define LatexMaker_h

#include <iostream>
#include <fstream>
#include <vector>

#include "TString.h"
#include "TMatrixDSym.h"

#include "RooArgList.h"

#include "PDF_Abs.h"

class LatexMaker {

  public:

    LatexMaker( TString cName, PDF_Abs *_pdf );
    ~LatexMaker();

    void writeFile();
    void writeCorrMatrix( ofstream& file, TMatrixDSym mat, RooArgList *observables, std::vector<TString> labels);

    TString outfname;
    PDF_Abs *pdf;

};

#endif
