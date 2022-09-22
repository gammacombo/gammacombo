/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2013
 *
 * Class to mimic a RooFitResult, but only store the things
 * we actually need, therefore saving a lot of memory in the
 * 2d scans.
 *
 **/

#ifndef RooSlimFitResult_h
#define RooSlimFitResult_h

#include <fstream>

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "TMath.h"
#include "TDatime.h"
// #include "Utils.h" // doesn't compile when included

using namespace std;

///
/// Class that essentially mimics the functionality of a RooFitResult,
/// but uses less internal memory by not storing the correlation matrix,
/// if not specifically requested. It also contains a few extra getters.
///
class RooSlimFitResult : public TObject
{
    public:
        RooSlimFitResult();
        RooSlimFitResult(RooFitResult* r, bool storeCorrelation=false);
        RooSlimFitResult(RooSlimFitResult* other);
        RooSlimFitResult(const RooSlimFitResult &r);
        ~RooSlimFitResult();

        RooSlimFitResult*  Clone();
        RooArgList&        constPars() const;
        inline TMatrixDSym correlationMatrix() const {return _correlationMatrix;};
        inline Int_t       covQual() const {return _covQual;};
        inline Double_t    edm() const {return _edm;};
        RooArgList&        floatParsFinal() const;
        float              getParVal(TString name) const;
        float              getParErr(TString name) const;
        float              getConstParVal(TString name) const;
        float              getFloatParFinalVal(TString name) const;
        bool               hasParameter(TString name) const;
        inline bool        isConfirmed(){return _isConfirmed;};
        inline Double_t    minNll() const {return _minNLL;};
        void               Print(bool verbose=false, bool printcor=false);
        void               SaveLatex(ofstream &outfile, bool verbose=false, bool printcor=false);
        inline void        setConfirmed(bool c){_isConfirmed = c;};
        inline Int_t       status() const {return _status;};

        // private:

        template<class FitResult> void init(const FitResult *r, bool storeCorrelation=false);
        bool                           isAngle(RooRealVar* v);

        vector<string> _parsNames;   // variable names
        vector<int>    _parsFloatId; // ID of floating parameter - this corresponds to the correlation matrix position
        vector<float>  _parsVal;     // values of const parameters, index given by position in _variable names
        vector<float>  _parsErr;
        vector<bool>   _parsAngle; // is it an angle?
        vector<bool>   _parsConst; // is it constant?
        Double_t       _edm;
        Double_t       _minNLL;
        Int_t          _covQual;
        Int_t          _status;
        TMatrixDSym    _correlationMatrix;

        // dummy variables only needed for constPars() and floatParsFinal():
        mutable RooArgList _constParsDummy; //! <- The exlcamation mark turns off storing in a root file (marks the member transient)
        mutable RooArgList _floatParsFinalDummy; //! mutables can be changed in const methods

        ClassDef(RooSlimFitResult, 1) // defines version number, ClassDef is a macro

    private:
        bool _isConfirmed;
};

//
// TEMPLATE IMPLEMENTATION
//

template<class FitResult> void RooSlimFitResult::init(const FitResult *r, bool storeCorrelation)
{
    assert(r);
    // copy over const parameters
    int size = r->constPars().getSize();
    for ( int i=0; i<size; i++ ){
        RooRealVar* p = (RooRealVar*)r->constPars().at(i);
        _parsNames.push_back(p->GetName());
        _parsVal.push_back(p->getVal());
        _parsErr.push_back(0.);
        _parsAngle.push_back(isAngle(p));
        _parsConst.push_back(true);
        _parsFloatId.push_back(-1); // floating ID doesn't exist for constant parameters
    }
    // copy over floating parameters
    size = r->floatParsFinal().getSize();
    for ( int i=0; i<size; i++ ){
        RooRealVar* p = (RooRealVar*)r->floatParsFinal().at(i);
        _parsNames.push_back(p->GetName());
        _parsVal.push_back(p->getVal());
        _parsErr.push_back(p->getError());
        _parsAngle.push_back(isAngle(p));
        _parsConst.push_back(false);
        _parsFloatId.push_back(i); // needed to store the parameter's position in the COR matrix (matches floatParsFinal())
    }
    // copy over numeric values
    _covQual = r->covQual();
    _edm = r->edm();
    _minNLL = r->minNll();
    _status = r->status();
    // store correlation matrix
    if ( storeCorrelation ){
        _correlationMatrix.ResizeTo(r->correlationMatrix());
        _correlationMatrix = r->correlationMatrix();
    }
    // initialize other members
    _isConfirmed = false;
}

#endif
