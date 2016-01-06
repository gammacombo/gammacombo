/**
 * Gamma Combination
 * Author: Maximilian Schlupp, max.schlupp@cern.ch
 * Date: December 2013
 *
 * Abstract class for handling generic PDFs
 * The RooArgLists for Observables and Parameters must be 
 * provided by an external workspace. Either there are 
 * named sets within the workspace or define the variables
 * for each set manually.
 * The user should inherit from this class. 
 * The user has to implement the PDFs methods: fit(), generateToys() 
 *  
 **/

#ifndef PDF_Datasets_Abs_h
#define PDF_Datasets_Abs_h

#include "PDF_Abs.h"

class PDF_Datasets_Abs : public PDF_Abs
{
public:
  PDF_Datasets_Abs(RooWorkspace* w, int nObs, OptParser* opt);
  ~PDF_Datasets_Abs();
  void                  deleteConstraints(){if(globVals) delete globVals;};
  void                  deleteNLL(){if(_NLL){delete _NLL; _NLL=NULL;}};

  virtual RooFitResult* fit(bool fitToys = kTRUE) = 0;
  virtual void          generateToys(int SeedShift = 0) = 0;
  
  void                  initData(const TString& name);
  void                  initObservables(const TString& setName);
  void                  initParameters(const TString& setName);
  void                  initObservables(const vector<TString>& obsNames);
  void                  initParameters(const vector<TString>& parNames);
  void                  initPDF(const TString& name);

  OptParser*            getArg(){ return arg; };
  TString               getConstraintName(){return constraintName;};
  TString               getExternalPdfName(){return pdfWspcName;};
  TString               getDataName(){return dataName;};
  inline int            getFitStatus(){return fitStatus;};
  inline int            getFitStrategy(){return fitStrategy;};
  TString               getGlobVarsName(){return globalVarsName;};
  float                 getMinNllFree(){return minNllFree;};
  float                 getMinNllScan(){return minNllScan;};
  TString               getObsName(){return obsName;};
  //const RooArgSet*      getObservables();
  TString               getParName(){return parName;};
  //const RooArgSet*      getParameters();
  TString               getPdfName(){return pdfName;};
  RooWorkspace*         getWorkspace(){return wspc;};
  // setters
  inline void           setConstraints(const TString& setName){constraintName = setName;};
  inline void           setExternalPdf(const TString& pName){pdfWspcName = pName;};
  inline void           setDataName(const TString& objName){dataName = objName;};
  inline void           setFitStatus(int stat = 0){fitStatus = stat;};
  inline void           setFitStrategy(int strat = 0){fitStrategy = strat;};
  inline void           setGlobalVars(const TString& globName){globalVarsName = globName;}; 
  inline void           setMinNllFree(float mnll){minNllFree = mnll;}; 
  inline void           setMinNllScan(float mnll){minNllScan = mnll;}; 
  void                  setNCPU(int n){NCPU = n;}; 
  void                  setPdfName(const TString& name);
  void                  setVarRange(const TString &varName, const TString &rangeName, 
                                    const double &rangeMin, const double &rangeMax);
  void                  setToyData(RooDataSet* ds);

  void                  print();

  inline  bool          areObservablesSet(){ return areObsSet; };
  inline  bool          areParametersSet(){ return areParsSet; };
  inline  bool          isPdfInitialized(){ return isPdfSet; };
  inline  bool          isDataInitialized(){ return isDataSet; };
  inline  bool          notSetupToFit(bool fitToys){return (!(isPdfSet && isDataSet) || (fitToys && !(isPdfSet && isToyDataSet))); }; // this comes from a previous if-statement
  int                   NCPU;         //> number of CPU used
  RooDataSet*           globVals; //> values for a set of global vars
  float                 minNll;

  virtual void          randomizeConstraintMeans(bool useConstrPdf = true){cout << "Child specific" << endl; };         //> helper function to draw new means for constraints every toy


protected:
  RooWorkspace*   wspc;
  RooDataSet*     data;
  RooAbsPdf*      _constraintPdf;
  RooAbsReal*     _NLL; // possible pointer to minimization function 
  TString         pdfName;
  TString         obsName;
  TString         parName;
  TString         dataName;       //> name of the data set in the workspace
  TString         pdfWspcName;    //> name of the pdf in an external workspace
  TString         constraintName; //> name of the set with all constraint pdfs 
  TString         globalVarsName; //> name of the set of global vars, e.g. mean of gaussian constraints...
  OptParser*      arg;
  int             fitStrategy;
  int             fitStatus;
  float           minNllFree;
  float           minNllScan;
  bool areObsSet;       //> Forces user to set observables
  bool areParsSet;      //> Forces user to set parameters
  bool areRangesSet;    //> Flag deciding if necessary ranges are set
  bool isPdfSet;        //> Flag deciding if PDF is set
  bool isDataSet;       //> Flag deciding if Data is set
  bool isToyDataSet;    //> Flag deciding if ToyData is set
};

#endif