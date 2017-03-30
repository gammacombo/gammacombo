/**
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
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
    void                  deleteNLL() {if (_NLL) {delete _NLL; _NLL = NULL;}};

    virtual RooFitResult* fit(RooDataSet* dataToFit);
    virtual RooFitResult* fitBkg(RooDataSet* dataToFit);
    virtual void          generateToys(int SeedShift = 0);
    virtual void          generateToysGlobalObservables(int SeedShift = 0);

    void                  initConstraints(const TString& setName);
    void                  initData(const TString& name);
    void                  initObservables(const TString& setName);
    virtual void          initObservables();  //overriding the inherited virtual method
    void                  initGlobalObservables(const TString& setName);
    void                  initParameters(const TString& setName);
    void                  initParameters(const vector<TString>& parNames);
    virtual void          initParameters(); //overriding the inherited virtual method
    void                  initPDF(const TString& name);
    void                  initBkgPDF(const TString& name);

    OptParser*            getArg();
    TString               getConstraintName() {return constraintName;};
    TString               getDataName() {return dataName;};
    RooDataSet*           getData() {return this->data;};
    inline int            getFitStatus() {return fitStatus;};
    inline int            getFitStrategy() {return fitStrategy;};
    TString               getGlobalObsName() {return globalObsName;};
    float                 getMinNll() {return minNll;};
    float                 getMinNllFree() {return minNllFree;};
    float                 getMinNllBkg() {return minNllBkg;};
    float                 getMinNllScan() {return minNllScan;};
    TString               getObsName() {return obsName;};
    TString               getParName() {return parName;};
    TString               getPdfName() {return pdfName;};
    TString               getBkgPdfName() {return pdfBkgName;};
    RooDataSet*           getToyObservables() {return this->toyObservables;};
    RooWorkspace*         getWorkspace() {return wspc;};
    // setters
    inline void           setFitStatus(int stat = 0) {fitStatus = stat;};
    inline void           setFitStrategy(int strat = 0) {fitStrategy = strat;};
    inline void           setMinNll(float mnll) {minNll = mnll;};
    inline void           setMinNllFree(float mnll) {minNllFree = mnll;};
    inline void           setMinNllScan(float mnll) {minNllScan = mnll;};
    void                  setNCPU(int n) {NCPU = n;};
    void                  setVarRange(const TString &varName, const TString &rangeName,
                                      const double &rangeMin, const double &rangeMax);
    void                  setToyData(RooDataSet* ds);

    void                  print();
    void                  printParameters();
    inline  bool          areObservglobalablesSet() { return areObsSet; };
    inline  bool          areParametersSet() { return areParsSet; };
    inline  bool          isPdfInitialized() { return isPdfSet; };
    inline  bool          isDataInitialized() { return isDataSet; };
    inline  bool          notSetupToFit(bool fitToys) {return (!(isPdfSet && isDataSet) || (fitToys && !(isPdfSet && isToyDataSet))); }; // this comes from a previous if-statement


    int                   NCPU;         //> number of CPU used
    float                 minNll;

    const TString         globalObsDataSnapshotName = "globalObsDataSnapshotName";
    //> name of a snapshot that stores the values of the global observables in data
    const TString         globalObsToySnapshotName = "globalObsToySnapshotName";
    //> name of a snapshot that stores the latest simulated values for the global observables

protected:
    void initializeRandomGenerator(int seedShift);
    RooWorkspace*   wspc;
    RooDataSet*     data;
    RooAbsReal*     _NLL; // possible pointer to minimization function
    RooAbsPdf*      _constraintPdf;
    TString         pdfName; //> name of the pdf in the workspace
    TString         pdfBkgName; //> name of the bkg pdf in the workspace
    TString         obsName;
    TString         parName;
    TString         dataName;       //> name of the data set in the workspace
    TString         constraintName; //> name of the set with all constraint pdfs
    TString         globalParsName; //> name of the set of global parameters in the workspace, that is, the parameters that occur (not only) in the  constraints...
    TString         globalObsName;   //> name of the set of global observables in the workspace.
    OptParser*      arg;
    int             fitStrategy;
    int             fitStatus;
    float           minNllFree;
    // float           minNll;
    float           minNllBkg;
    float           minNllScan;
    bool areObsSet;       //> Forces user to set observables
    bool areParsSet;      //> Forces user to set parameters
    bool areRangesSet;    //> Flag deciding if necessary ranges are set
    bool isPdfSet;        //> Flag deciding if PDF is set
    bool isBkgPdfSet;     //> Flag deciding if Bkg PDF is set
    bool isDataSet;       //> Flag deciding if Data is set
    bool isToyDataSet;    //> Flag deciding if ToyData is set
};

#endif
