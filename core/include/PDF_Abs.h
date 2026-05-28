/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef PDF_Abs_h
#define PDF_Abs_h

#include <TMatrixDSym.h>
#include <TString.h>

#include <map>
#include <string>
#include <vector>

class ParametersAbs;
class ParameterCache;
class RooMultiPdf;

class RooAbsData;
class RooAbsPdf;
class RooArgList;
class RooCategory;
class RooFitResult;

class TObject;

class PDF_Abs {
 public:
  PDF_Abs(int nObs);
  PDF_Abs(int nObs, ParametersAbs& pars);
  virtual ~PDF_Abs();

  PDF_Abs(const PDF_Abs&) = delete;
  PDF_Abs& operator=(const PDF_Abs&) = delete;

  virtual void build();
  virtual void buildPdf();
  virtual void buildCov();
  bool bkgpdfset() const { return isBkgPdfSet; };
  bool bkgmultipdfset() const { return isBkgMultipdfSet; };
  virtual bool checkConsistency() const;
  void deleteToys();

  // Getters.
  inline TString getCorrelationSourceString() const { return corSource; };
  TString getBaseName() const;
  inline TString getErrorSourceString() const { return obsErrSource; };
  inline int getGcId() const { return gcId; }
  inline TString getName() const { return name; };
  inline int getNobs() const { return nObs; };
  inline TString getUniqueID() const { return uniqueID; };
  inline unsigned long long getUniqueGlobalID() const { return uniqueGlobalID; }
  inline RooArgList* getObservables() { return observables; };
  inline std::vector<TString> getLatexObservables() const { return latexObservables; };
  inline TString getObservableSourceString() const { return obsValSource; };
  double getObservableValue(TString obsname) const;
  inline RooArgList* getParameters() { return parameters; };
  inline RooAbsPdf* getPdf() { return pdf; };
  inline RooAbsPdf* getBkgPdf() { return pdfBkg; };
  inline RooMultiPdf* getMultipdf() { return multipdf; };
  inline RooMultiPdf* getBkgMultipdf() { return multipdfBkg; };
  void getSubCorrelationStat(TMatrixDSym& target, const std::vector<int>& indices) const;
  void getSubCorrelationSyst(TMatrixDSym& target, const std::vector<int>& indices) const;
  inline RooArgList* getTheory() { return theory; };
  inline TString getTitle() const { return title; };

  bool hasObservable(TString obsname) const;
  inline bool isCrossCorPdf() const { return m_isCrossCorPdf; }
  virtual void initParameters();
  virtual void initRelations();
  virtual void initObservables();
  void loadExtParameters(const RooFitResult* r);
  void loadExtParameters(const ParameterCache* pc);
  void print() const;
  void printParameters() const;
  void printObservables() const;
  bool ScaleError(TString obsname, double scale);

  // Setters.
  virtual void setCorrelations(TString c);
  inline void setErrorSourceString(TString source) { obsErrSource = source; };
  inline void setGcId(int id) { gcId = id; };
  inline void setName(TString myName) { this->name = myName; }
  void setObservable(TString name, double value);
  virtual void setObservables(TString c);
  void setObservablesTruth();
  void setObservablesToy();
  inline void setObservableSourceString(TString source) { obsValSource = source; };
  inline void setNObs(int val) { nObs = val; };
  inline void setTitle(TString t) { title = t; };
  void setUncertainty(TString obsName, double stat, double syst);
  virtual void setUncertainties(TString c);
  void setSystCorrelation(TMatrixDSym& corSystMatrix);

  void storeErrorsInObs();
  void resetCorrelations();
  virtual bool test();
  void uniquify(int uID);  ///< used to uniquify all names when added

  // covariance
  TMatrixDSym covMatrix;
  TMatrixDSym corMatrix;
  TMatrixDSym corStatMatrix;
  TMatrixDSym corSystMatrix;
  std::vector<double> StatErr;
  std::vector<double> SystErr;
  TString corSource = "n/a";
  TString obsValSource = "n/a";
  TString obsErrSource = "n/a";

 protected:
  void addToTrash(TObject*);
  void getSubMatrix(TMatrixDSym& target, const TMatrixDSym& source, const std::vector<int>& indices) const;

  RooArgList* parameters = nullptr;   // holds all fit parameters of this PDF
  RooArgList* theory = nullptr;       // holds all truth relations
  RooArgList* observables = nullptr;  // holds all observables
  TString name;
  TString title = "(no title)";           // to be printed in human readable summaries
  RooAbsPdf* pdf = nullptr;               // the PDF
  RooAbsPdf* pdfBkg = nullptr;            // Bkg PDF for building CLs teststatistic
  RooMultiPdf* multipdf = nullptr;        // the multipdf
  RooMultiPdf* multipdfBkg = nullptr;     // Bkg version of the multipdf
  RooCategory* multipdfCat = nullptr;     // the multipdf category
  bool isBkgPdfSet = false;               ///< Flag deciding if Bkg PDF is set
  bool isBkgMultipdfSet = false;          ///< Flag deciding if Bkg MultiPDF is set
  int nObs = -1;                          // number of observables
  std::map<std::string, TObject*> trash;  // trash bin, gets emptied in destructor
  bool m_isCrossCorPdf = false;  // Cross correlation PDFs need some extra treatment in places, e.g. in uniquify()
  std::vector<TString> latexObservables;  // holds latex labels for observables

  // The following three members are to gain performance during
  // toy generation - generating 1000 toys is much faster than 1000 times one toy.
  int nToyObs = 1000;                       // Number of toy observables to be pregenerated.
  RooAbsData* toyObservables = nullptr;     // A dataset holding nToyObs pregenerated bkg only toy observables.
  RooAbsData* toyBkgObservables = nullptr;  // A dataset holding nToyObs pregenerated toy observables.
  int iToyObs = 0;                          // Index of next unused set of toy observables.
  int gcId = -1;                            // ID of this PDF inside a GammaCombo object. Used to refer to this PDF.

 private:
  void printCorMatrix(TString title, TString source, const TMatrixDSym& cor) const;
  TString uniquifyThisString(TString s, int uID);
  TString uniqueID = "UID0";          // see also uniquify()
  static unsigned long long counter;  // Counts the total number of PDF_Abs objects that are created.

  // Used only in the Combiner::delPdf() mechanism. Uses the counter, so will depend on creation order.
  unsigned long long uniqueGlobalID = -1;
};

#endif
