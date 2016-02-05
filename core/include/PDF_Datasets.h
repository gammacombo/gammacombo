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

#ifndef PDF_Datasets_h
#define PDF_Datasets_h

#include "PDF_Abs.h"

class PDF_Datasets : public PDF_Abs
{
public:
  PDF_Datasets(RooWorkspace* w, int nObs, OptParser* opt);
  PDF_Datasets(RooWorkspace* w);
  ~PDF_Datasets();
  void                  deleteNLL(){if(_NLL){delete _NLL; _NLL=NULL;}};

  virtual RooFitResult* fit(bool fitToys = kTRUE);
  virtual void          generateToys(int SeedShift = 0);
  virtual void          generateToysGlobalObservables(int SeedShift = 0);
  
  void                  initData(const TString& name);
  void                  initObservables(const TString& setName);
  virtual void          initObservables();  //overriding the inherited virtual method
  void                  initConstraints(const TString& setName);
  void                  initGlobalObservables(const TString& setName);
  void                  initParameters(const TString& setName);
  virtual void          initParameters(); //overriding the inherited virtual method
  void                  initPDF(const TString& name);

  OptParser*            getArg();
  TString               getConstraintName(){return constraintName;};
  TString               getDataName(){return dataName;};
  inline int            getFitStatus(){return fitStatus;};
  inline int            getFitStrategy(){return fitStrategy;};
  TString               getGlobalParsName(){return globalParsName;};
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
  inline void           setDataName(const TString& objName){dataName = objName;};
  inline void           setFitStatus(int stat = 0){fitStatus = stat;};
  inline void           setFitStrategy(int strat = 0){fitStrategy = strat;};
  inline void           setMinNllFree(float mnll){minNllFree = mnll;}; 
  inline void           setMinNllScan(float mnll){minNllScan = mnll;}; 
  void                  setNCPU(int n){NCPU = n;}; 
  void                  setVarRange(const TString &varName, const TString &rangeName, 
                                    const double &rangeMin, const double &rangeMax);
  void                  setToyData(RooDataSet* ds);
  void                  setGlobalObservables(bool toToys);

  void                  print();

  inline  bool          areObservablesSet(){ return areObsSet; };
  inline  bool          areParametersSet(){ return areParsSet; };
  inline  bool          isPdfInitialized(){ return isPdfSet; };
  inline  bool          isDataInitialized(){ return isDataSet; };
  inline  bool          notSetupToFit(bool fitToys){return (!(isPdfSet && isDataSet) || (fitToys && !(isPdfSet && isToyDataSet))); }; // this comes from a previous if-statement


  int                   NCPU;         //> number of CPU used
  float                 minNll;


protected:
  RooWorkspace*   wspc;
  RooDataSet*     data;
  RooAbsReal*     _NLL; // possible pointer to minimization function 
  TString         pdfName; //> name of the pdf in the workspace
  TString         obsName;
  TString         parName;
  TString         dataName;       //> name of the data set in the workspace
  TString         constraintName; //> name of the set with all constraint pdfs 
  TString         globalParsName; //> name of the set of global parameters in the workspace, that is, the parameters that occur (not only) in the  constraints...
  TString         globalObsName;   //> name of the set of global observables in the workspace.
  const TString         globalObsDataSnapshotName = "globalObsDataSnapshotName";
  //> name of a snapshot that stores the values of the global observables in data
  const TString         globalObsToySnapshotName = "globalObsToySnapshotName";
  //> name of a snapshot that stores the latest simulated values for the global observables
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