/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef Combiner_h
#define Combiner_h

#include "PDF_Abs.h"
#include "OptParser.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class Combiner
{
public:
  Combiner(OptParser *arg, TString title);
  Combiner(OptParser *arg, TString name, TString title);
  ~Combiner();

  void                     addPdf(PDF_Abs *p);
  void                     addPdf(PDF_Abs *p1, PDF_Abs *p2);
  void                     addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3);
  void                     addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4);
  void                     addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5);
  void                     addPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5, PDF_Abs *p6);
  void                     adjustPhysRange(TString varName, float min, float max);
  Combiner*                Clone(TString name, TString title);
  void                     combine();
  void                     fixParameter(TString var, float value);
	void										 fixParameters(TString vars);
  void                     delPdf(PDF_Abs *p);
  void                     delPdf(PDF_Abs *p1, PDF_Abs *p2);
	void                     delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3);
	void                     delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4);
	void                     delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5);
	void                     delPdf(PDF_Abs *p1, PDF_Abs *p2, PDF_Abs *p3, PDF_Abs *p4, PDF_Abs *p5, PDF_Abs *p6);
  inline OptParser*        getArg(){return arg;};
  const RooArgSet*         getParameters();
  vector<string>&          getParameterNames();
  PDF_Abs*                 getPdfProvidingObservable(TString obsname);
  const RooArgSet*         getObservables();
  vector<string>&	   getObservableNames();
  inline TString           getTitle() const {return title;};
  inline TString           getName() const {return name;};
  inline TString           getPdfName() const {return pdfName;};    ///< Returns name of combined pdf. Call combine() first.
  inline TString           getParsName() const {return parsName;};  ///< Returns name of combined parameter set. Call combine() first.
  inline TString           getObsName() const {return obsName;};    ///< Returns name of combined observables set. Call combine() first.
  RooAbsPdf*               getPdf();
  inline vector<PDF_Abs*>& getPdfs(){return pdfs;};
  inline RooWorkspace*     getWorkspace(){return w;};
  inline bool              isCombined() const {return _isCombined;}
	void    								 loadParameterLimits();
  void                     print();
  void                     replacePdf(PDF_Abs *from, PDF_Abs *to);
	void										 setName(TString name);
	void										 setObservablesToToyValues();
	void										 setParametersConstant(); // helper function for combine()
  inline void              setTitle(TString title){this->title=title;};
	inline vector<FixPar> 	 getConstVars(){return constVars;};

private:
  vector<PDF_Abs*>        pdfs;        // holds all pdfs to be combined
  TString                 title;       // title of the combination, used in plots
  TString                 name;        // name of the combination, used to refer to it and as part of file names
  TString                 pdfName;     // Name of combined pdf. Call combine() first.
  TString                 parsName;    // Name of combined parameter set. Call combine() first.
  TString                 obsName;     // Name of combined observables set. Call combine() first.
  OptParser*              arg;         // command line arguments
  RooWorkspace*           w;           // holds all input pdfs, parameters, and observables, as well as the combination
  vector<string>          pdfNames;    // hold all unique names of the pdfs to be combined
  bool                    _isCombined; // make sure we'll only combine once - else all PDFs get double counted!
  vector<FixPar>         	constVars;   // hold variables that will be set constant (filled by fixParameter())
};

#endif
