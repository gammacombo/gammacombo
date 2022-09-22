/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "PDF_Abs.h"

PDF_Abs::PDF_Abs(int nObs):
    covMatrix(nObs),
    corMatrix(nObs),
    corStatMatrix(nObs),
    corSystMatrix(nObs)
{
    this->nObs  = nObs;
    parameters  = nullptr;
    theory      = nullptr;
    observables = nullptr;
    pdf         = nullptr;
    pdfBkg      = nullptr;
    multipdf    = nullptr;
    isBkgPdfSet = false;
    isBkgMultipdfSet = false;
    toyObservables = nullptr;
    nToyObs = 1000;
    iToyObs = 0;
    for ( int i=0; i<nObs; i++ ){
        StatErr.push_back(0.0);
        SystErr.push_back(0.0);
    }
    title = "(no title)";
    corSource = "n/a";
    obsValSource = "n/a";
    obsErrSource = "n/a";
    uniqueID = "UID0";
    counter++;
    uniqueGlobalID = counter;
    m_isCrossCorPdf = false;
    gcId = -1;

}

unsigned long long PDF_Abs::counter = 0;

///
/// Clean off all objects in the trash bin.
/// See also addToTrash().
///
PDF_Abs::~PDF_Abs()
{
    // clean objects in the 'parameters' container
    for(int i=0; i<parameters->getSize(); i++){
        delete parameters->at(i);
    }
    delete parameters;

    // clean objects in the 'theory' container
    for(int i=0; i<theory->getSize(); i++){
        delete theory->at(i);
    }
    delete theory;

    // clean objects in the 'observables' container
    for(int i=0; i<observables->getSize(); i++){
        delete observables->at(i);
    }
    delete observables;

    // clean pregenerated toys
    if ( toyObservables!=0 ) delete toyObservables;

    // clean pdf
    if ( pdf!=0 ) delete pdf;

    // empty trash
    map<string, TObject*>::iterator iter;
    for(iter = trash.begin(); iter != trash.end(); ++iter)
    {
        if ( iter->second )
        {
            delete iter->second;
            iter->second = 0;
        }
    }
}

void PDF_Abs::initParameters(){assert(0);};
void PDF_Abs::initRelations(){assert(0);};
void PDF_Abs::initObservables(){assert(0);};
void PDF_Abs::buildPdf(){assert(0);};
void PDF_Abs::setObservables(TString c){assert(0);};
void PDF_Abs::setUncertainties(TString c){assert(0);};
void PDF_Abs::setCorrelations(TString c){assert(0);};

///
/// Shortcut for buildCov() and buildPdf().
///
void PDF_Abs::build()
{
    buildCov();
    buildPdf();
}

///
/// Set all observables to 'truth' values computed from the
/// current parameters.
///
void PDF_Abs::setObservablesTruth()
{
    obsValSource = "truth";
    for ( int i=0; i<nObs; i++ )
    {
        RooRealVar* pObs = (RooRealVar*)((RooArgList*)observables)->at(i);
        pObs->setVal(((RooRealVar*)((RooArgList*)theory)->at(i))->getVal());
    }
}

///
/// Set all observables to 'toy' values drawn from the
/// PDF using the current parameter values. A certain number
/// of toys is pregenerated to speed up when doing mulitple toy fits.
///
void PDF_Abs::setObservablesToy()
{
    obsValSource = "toy";
    if( !pdf ){ cout<< "PDF_Abs::setObservables(): ERROR: pdf not initialized."<<endl; exit(1); }
    if ( toyObservables==0 || iToyObs==nToyObs )
    {
        RooRandom::randomGenerator()->SetSeed(0);
        if ( iToyObs==nToyObs ) delete toyObservables;
        toyObservables = pdf->generate(*(RooArgSet*)observables, nToyObs);
        iToyObs=0;
    }
    for ( int i=0; i<nObs; i++ )
    {
        RooRealVar* pObs = (RooRealVar*)((RooArgList*)observables)->at(i);
        pObs->setVal(((RooRealVar*)toyObservables->get(iToyObs)->find(pObs->GetName()))->getVal());
    }
    iToyObs+=1;
}

///
/// Set all correlations to zero.
///
void PDF_Abs::resetCorrelations()
{
    for ( int i=0; i<nObs; i++ )
        for ( int j=0; j<nObs; j++ )
        {
            float c = 0.0;
            if ( i==j ) c = 1.0;
            corStatMatrix[i][j] = c;
            corSystMatrix[i][j] = c;
        }
}

///
/// Add an object to the trash bin which gets emptied
/// when this PDF object gets deleted. If an object of
/// the same name is already in the trash, we'll delete
/// that and replace by the new one. This way we can call
/// e.g. buildPdf() more than once.
///
void PDF_Abs::addToTrash(TObject* o)
{
    map<string, TObject*>::iterator iter;
    for (iter = trash.begin(); iter != trash.end(); ++iter){
        if ( o->GetName() == iter->first ){
            delete iter->second;
            iter->second = o;
            return;
        }
    }
    trash.insert(pair<string,TObject*>(o->GetName(),o));
}

///
/// Return the base name, which is the name without any
/// unique ID.
///
TString PDF_Abs::getBaseName()
{
    TString baseName = name;
    baseName.ReplaceAll(uniqueID,"");
    return baseName;
}

///
/// Uniquify all relevant names by adding a unique ID following
/// the pattern "UID2". This way we can have mulitple
/// instances of the same PDF in the same combination.
/// The pattern is defined in uniquifyThisString().
///
/// \param uID  - unique ID
///
void PDF_Abs::uniquify(int uID)
{
    if ( uniqueID==TString("") ) {
        cout << "PDF_Abs::uniquify() : ERROR : uniqueID is the empty string!" << endl;
        exit(1);
    }

    name = uniquifyThisString(name, uID);
    // change name of pdf
    pdf->SetName(uniquifyThisString(pdf->GetName(), uID));

    // change names of observables and parameters, but not for
    // cross correlation PDFs, because they need the same names as
    // the main PDFs
    if ( !m_isCrossCorPdf ){
        // change names of observables
        for ( int i=0; i<observables->getSize(); i++ ){
            observables->at(i)->SetName(uniquifyThisString(observables->at(i)->GetName(), uID));
        }
        // change names of theory parameters
        for ( int i=0; i<theory->getSize(); i++ ){
            theory->at(i)->SetName(uniquifyThisString(theory->at(i)->GetName(), uID));
        }
    }
    uniqueID = uniquifyThisString("", uID);
}

///
/// Helper function for uniquify(). Compute a unique string
/// by attaching "UID3" or similar to it.
///
TString PDF_Abs::uniquifyThisString(TString s, int uID)
{
    TString newUniqueID = Form("UID%i",uID);
    if ( s.Contains(uniqueID) ) s.ReplaceAll(uniqueID,newUniqueID);
    else s = s + newUniqueID;
    return s;
}

///
/// Set all parameters to values found in
/// a provided fit result.
///
void PDF_Abs::loadExtParameters(RooFitResult *r)
{
    RooArgSet *tmp = new RooArgSet();
    tmp->add(r->floatParsFinal());
    tmp->add(r->constPars());
    setParameters(parameters, tmp);
    delete tmp;
}

///
/// Build both the covariance and the correlation matrix
/// (members covMatrix and corMatrix)
/// from the stat and syst correlation matrics and the
/// respective errors.
///
void PDF_Abs::buildCov()
{
    auto name = getName();
    auto corr_source = getCorrelationSourceString();
    auto fatal = [name, corr_source](std::string message) {
        std::cout << "FATAL: " << message << "\n"
                  << "       PDF name: " << name << "\n"
                  << "       Correlation source: " << corr_source << "\n" << std::endl;
        exit(1);
    };
    auto warning = [name, corr_source](std::string message) {
        std::cout << "WARNING: " << message << "\n"
                  << "         PDF name: " << name << "\n"
                  << "         Correlation source: " << corr_source << "\n" << std::endl;
    };

    // add diagonals, symmetrize
    if (!buildCorMatrix(corStatMatrix)) fatal("The statistical correlation matrix is ill-formed");
    if (!buildCorMatrix(corSystMatrix)) fatal("The systematic correlation matrix is ill-formed");

    auto n = getNobs();
    if (n > 1
            && !corr_source.Contains("none", TString::kIgnoreCase)
            && !corr_source.Contains("no correlation", TString::kIgnoreCase)
            && !corr_source.Contains("correlations off", TString::kIgnoreCase)
            && !corr_source.Contains("correlations are off", TString::kIgnoreCase))
    {
        // check that off-diagonal terms of the correlation matrices are not all zero
        auto is_nonzero = [](double x){ return std::abs(x) > 1e-6; };
        if (std::find_if(StatErr.begin(), StatErr.end(), is_nonzero) != StatErr.end()) {
            bool warn = true;
            for (int i=0; i<n; ++i) {
                for (int j=0; j<n; ++j) {
                    if (i != j && is_nonzero(corStatMatrix[i][j])) warn = false;
                }
            }
            if (warn) warning("All off-diagonal elements of the stat. corr. matrix are zero. Is this OK?");
        }
        if (std::find_if(SystErr.begin(), SystErr.end(), is_nonzero) != SystErr.end()) {
            bool warn = true;
            for (int i=0; i<n; ++i) {
                for (int j=0; j<n; ++j) {
                    if (i != j && is_nonzero(corSystMatrix[i][j])) warn = false;
                }
            }
            if (warn) warning("All off-diagonal elements of the syst. corr. matrix are zero. Is this OK?");
        }
    }

    // make total cov matrix
    TMatrixDSym *covStat = buildCovMatrix(corStatMatrix, StatErr);
    TMatrixDSym *covSyst = buildCovMatrix(corSystMatrix, SystErr);
    covMatrix = *covStat + *covSyst;

    // check if total cov matrix is invertible
    if ( covMatrix.Determinant()==0 ) {
        cout << "PDF_Abs::buildCov() : ERROR : Total covariance matrix is not invertable (det(COV)=0)." << endl;
        cout << "PDF_Abs::buildCov() : ERROR : Check inputs! Ordering correct? Nobs correct?" << endl;
        cout << "PDF_Abs::buildCov() : PDF: " << name << endl;
        cout << "PDF_Abs::buildCov() : stat cov: " << endl;
        covStat->Print("v");
        cout << "PDF_Abs::buildCov() : syst cov: " << endl;
        covSyst->Print("v");
        cout << "PDF_Abs::buildCov() : full cov: " << endl;
        covMatrix.Print("v");
        //exit(1);
        throw TString("need help");
    }

    // make total cor matrix
    for ( int i=0; i<covMatrix.GetNcols(); i++ )
        for ( int j=0; j<covMatrix.GetNcols(); j++ )
        {
            corMatrix[i][j] = covMatrix[i][j]/sqrt(covMatrix[i][i])/sqrt(covMatrix[j][j]);
        }

    // check if total cor matrix is positive definite
    if ( ! isPosDef(&corMatrix) ) {
        cout << "PDF_Abs::buildCov() : ERROR : Total correlation matrix is not positive definite." << endl;
        cout << "PDF_Abs::buildCov() : ERROR : Check inputs! Ordering correct?" << endl;
        cout << "PDF_Abs::buildCov() :         Sometimes this happens when for very large correlations" << endl;
        cout << "PDF_Abs::buildCov() :         the given precision is not enough (e.g. rho=0.98 rather than 0.978)." << endl;
        cout << "PDF_Abs::buildCov() : PDF: " << name << endl;
        cout << "PDF_Abs::buildCov() : stat cor: " << endl;
        corStatMatrix.Print("v");
        cout << "PDF_Abs::buildCov() : syst cor: " << endl;
        corSystMatrix.Print("v");
        //exit(1);
        throw TString("need help");
    }

    delete covStat;
    delete covSyst;

    // this is needed for the pull computation and the PDF_Abs::print() function:
    storeErrorsInObs();
}

///
/// Helper function for print(): it prints correlation matrices,
/// stat, syst, stat+syst
///
void PDF_Abs::printCorMatrix(TString title, TString source, const TMatrixDSym& cor) const
{
    cout << "    correlation " << title << ":" << endl;
    cout << "      cor. source: " << source << endl;
    printf("%30s", " ");
    for ( int i=0; i<nObs; i++ ) printf("%5i ", i);
    cout << endl;
    for ( int i=0; i<nObs; i++ ){
        TString obsName = ((RooRealVar*)((RooArgList*)observables)->at(i))->GetName();
        obsName.ReplaceAll(uniqueID,"");
        printf("      %-20s %2i ", obsName.Data(), i);
        for ( int j=0; j<nObs; j++ ){
            if (fabs(cor[i][j])<0.005) printf("%6s ", "-");
            else printf("%6.3f ", cor[i][j]);
        }
        cout << endl;
    }
    cout << endl;
}

///
/// Print this PDF in a verbose way:
/// - observables
/// - correlations
/// - parameters
///
void PDF_Abs::print() const
{
    TString cleanName = name;
    cout << "PDF: " << cleanName.ReplaceAll(uniqueID,"") << " (" << uniqueID << ")" << endl << endl;

    if ( observables ){
        cout << "    observables:" << endl;
        cout << "      nObs = " << nObs << endl;
        cout << "      values from: " << obsValSource << endl;
        cout << "      errors from: " << obsErrSource << endl;
        for ( int iObs=0; iObs<nObs; iObs++ ){
            RooRealVar* v = (RooRealVar*)observables->at(iObs);
            TString obsName = v->GetName();
            obsName.ReplaceAll(uniqueID,"");
            printf("      %-20s = %8.5f +/- %7.5f +/- %7.5f\n", obsName.Data(), v->getVal(), StatErr[iObs], SystErr[iObs]);
        }
    }
    else cout << "PDF_Abs::print() : observables not initialized. Call initObservables() first." << endl;
    cout << endl;

    if ( nObs>1 ){
        printCorMatrix("(stat+syst)", corSource, corMatrix);
        printCorMatrix("(stat)", corSource, corStatMatrix);
        printCorMatrix("(syst)", corSource, corSystMatrix);
    }

    if ( parameters ){
        cout << "    parameters:" << endl;
        cout << "      nPar = " << parameters->getSize() << endl;
        cout << "      ";
        bool first=true;
        TIterator* it = parameters->createIterator();
        while ( RooAbsReal* v = (RooAbsReal*)it->Next() ){
            cout << (first?"":", ") << v->GetName();
            first=false;
        }
        delete it;
        cout << endl;
    }
    else cout << "PDF_Abs::print() : parameters not initialized. Call initParameters() first." << endl;
    cout << endl;

    if ( theory ){
        cout << "    relations:" << endl;
        TIterator* it = theory->createIterator();
        while ( RooAbsReal* v = (RooAbsReal*)it->Next() ){
            // it's not easy to extract the formula from a RooFormulaVar.
            TString thName = v->GetName();
            thName.ReplaceAll(uniqueID,"");
            printf("      %-20s = ", thName.Data());
            ostringstream stream;
            v->printMetaArgs(stream);
            TString formula = stream.str();
      if ( formula.Contains("formula=") ) { // this is a RooFormulaVar
        RooFormulaVar *form = dynamic_cast<RooFormulaVar*>(v);
        int nFormPars = form->getVariables()->getSize();
        for (int i=0; i<nFormPars; i++) {
          if ( ! form->getParameter(i) ) continue;
          formula.ReplaceAll( Form("x[%d]",i), form->getParameter(i)->GetName() );
        }
      }
            formula.ReplaceAll("formula=", "");
            formula.ReplaceAll("\"", "");
            if ( formula=="" ) formula = v->ClassName(); // compiled custom Roo*Var classes don't have a formula
            cout << formula << endl;
        }
        delete it;
    }
    else cout << "PDF_Abs::print() : theory not initialized. Call initRelations() first." << endl;
    cout << endl;
}


void PDF_Abs::printParameters()
{
    if ( parameters )
    {
        cout << "      parameters:  ";
        bool first=true;
        TIterator* it = parameters->createIterator();
        while ( RooAbsReal* v = (RooAbsReal*)it->Next() )
        {
            TString vName = v->GetName();
            cout << (first?"":", ") << vName;
            first=false;
        }
        cout << "  (nPar=" << parameters->getSize() << ")" << endl;
    }
    else cout << "PDF_Abs::print() : parameters not initialized. Call initParameters() first." << endl;
}


void PDF_Abs::printObservables()
{
    if ( observables ){
        cout << "      observables: ";
        bool first=true;
        TIterator* it = observables->createIterator();
        while ( RooAbsReal* v = (RooAbsReal*)it->Next() ){
            TString vName = v->GetName();
            vName.ReplaceAll("_obs","");
            vName.ReplaceAll(uniqueID,"");
            cout << (first?"":", ") << vName;
            first=false;
        }
        cout << "  (nObs=" << observables->getSize() << ")" << endl;
    }
    else cout << "PDF_Abs::print() : observables not initialized. Call initObservables() first." << endl;
}

///
/// Store the errors as RooFit error into the observables
/// to have them easily available for the pull computation.
///
void PDF_Abs::storeErrorsInObs()
{
    if ( covMatrix==0 )
    {
        cout << "PDF_Abs::storeErrorsInObs() : ERROR : covMatrix not initialized." << endl;
        return;
    }

    for ( int i=0; i<nObs; i++ )
    {
        RooRealVar* pObs = (RooRealVar*)((RooArgList*)observables)->at(i);
        pObs->setError(sqrt(covMatrix[i][i]));
    }
}

///
/// Set an external systematic correlation matrix.
/// After modifying, call buildCov() and buildPdf();
///
void PDF_Abs::setSystCorrelation(TMatrixDSym &corSystMatrix)
{
    assert(corSystMatrix.GetNcols()==nObs);
    this->corSystMatrix = corSystMatrix;
    corSource = corSource + " (syst. cor. set manually)";
}

///
/// Set the observed central value of an observable. To be used
/// in setObservables() of the derived PDF classes.
///
/// \param obsName - observable name
/// \param value - central value
///
void PDF_Abs::setObservable(TString obsName, float value)
{
    RooRealVar* obs = (RooRealVar*)observables->find(obsName);
    if ( obs==0 ) { cout << "PDF_Abs::setObservable() : ERROR : observable "+obsName+" not found!" << endl; exit(1); }
    obs->setVal(value);
}

///
/// Set the uncertainties of an observable. To be used
/// in setUncertainties() of the derived PDF classes.
/// This function fills the StatErr and SystErr arrays
/// in the correct place.
///
/// \param obsName - observable name
/// \param stat - statistical error
/// \param syst - systematic error
///
void PDF_Abs::setUncertainty(TString obsName, float stat, float syst)
{
    for ( int i=0; i<nObs; i++ ){
        RooRealVar* obs = (RooRealVar*)observables->at(i);
        if ( TString(obs->GetName()).EqualTo(obsName) ){
            StatErr[i] = stat;
            SystErr[i] = syst;
            return;
        }
    }
    cout << "PDF_Abs::setUncertainty() : ERROR : observable "+name+" not found!" << endl;
    exit(1);
}

///
/// Perform a couple of consistency checks to make it easier
/// to find bugs:
/// - check if all observables end with '_obs'
/// - check if all predicted observables end with '_th'
/// - check if the 'observables' and 'theory' lists are correctly ordered
///
bool PDF_Abs::checkConsistency()
{
    if ( m_isCrossCorPdf ) return true;
    bool allOk = true;

    // check if all observables end with '_obs'
    TIterator* it = observables->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ){
        TString pObsName = p->GetName();
        pObsName.ReplaceAll(uniqueID,"");
        if ( !pObsName.EndsWith("_obs") ){
            cout << "PDF_Abs::checkConsistency() : " << name << " : observable " << p->GetName() << " doesn't end with '_obs'" << endl;
            allOk = false;
        }
    }

    // check if all predicted observables end with '_th'
    delete it; it = theory->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() ){
        TString pThName = p->GetName();
        pThName.ReplaceAll(uniqueID,"");
        if ( !pThName.EndsWith("_th") ){
            cout << "PDF_Abs::checkConsistency() : " << name << " : theory " << p->GetName() << " doesn't end with '_th'" << endl;
            allOk = false;
        }
    }

    // check if the 'observables' and 'theory' lists are correctly ordered
    for ( int i=0; i<nObs; i++ ){
        RooAbsArg* pTh = theory->at(i);
        TString base = pTh->GetName();
        base.ReplaceAll("_th","");
        base.ReplaceAll(uniqueID,"");
        TString pObsName = observables->at(i)->GetName();
        pObsName.ReplaceAll(uniqueID,"");
        if ( pObsName != base+"_obs"){
            cout << "PDF_Abs::checkConsistency() : " << name << " : " << pTh->GetName() << " doesn't match its observable." << endl;
            cout << "                              Expected '" << base+"_obs" << "'. Found '" << pObsName << "'." << endl;
            cout << "                              Check ordering of the 'theory' and 'observables' lists!" << endl;
            allOk = false;
        }
    }

    return allOk;
}

///
/// Test PDF implementation.
/// Performs a fit to the minimum.
///
bool PDF_Abs::test()
{
    bool quiet = false;
    if(quiet) RooMsgService::instance().setGlobalKillBelow(ERROR);
    fixParameters(observables);
    floatParameters(parameters);
    setLimit(parameters, "free");
    RooFormulaVar ll("ll", "ll", "-2*log(@0)", RooArgSet(*pdf));
    RooMinimizer m(ll);
    if(quiet) m.setPrintLevel(-2);
    m.setLogFile("/dev/zero");
    m.setErrorLevel(1.0);
    m.setStrategy(2);
    // m.setProfile(1);
    m.migrad();
    RooFitResult *f = m.save();
    bool status = !(f->edm()<1 && f->status()==0);
    if(!quiet) f->Print("v");
    delete f;
    if(quiet) RooMsgService::instance().setGlobalKillBelow(INFO);
    if(!quiet) cout << "pdf->getVal() = " << pdf->getVal() << endl;
    return status;
}

///
/// Check if this PDF has an observable of the given name.
/// \param obsname  - observable name
/// \return true if found
///
bool PDF_Abs::hasObservable(TString obsname)
{
    RooRealVar* obs = (RooRealVar*)observables->find(obsname);
    if ( obs==0 ) return false;
    return true;
}

///
/// Scale the error of a given observable.
/// Both stat and syst errors are being scaled by the same factor.
/// In order to become effective, the PDF needs to be rebuild by
/// calling buildCov() and buildPdf().
///
/// \param obsname  - observable name. It may or may not include a unique ID string, both works.
/// \param scale    - the scale factor the current error is being multiplied with
/// \return     - true if successful
///
bool PDF_Abs::ScaleError(TString obsname, float scale)
{
    // remove unique ID if necessary
    TString UID = "UID";
    if ( obsname.Contains(UID) ){
        obsname.Replace(obsname.Index(UID), obsname.Length(), ""); // delete the unique ID. That should leave just the observable name.
    }
    // find the index of the observable - if it exists at all!
    if ( !hasObservable(obsname) ){
        cout << "PDF_Abs::ScaleError() : ERROR : observable '" << obsname << "' not found." << endl;
        return false;
    }
    int index = -1;
    for ( int i=0; i<getNobs(); i++ ){
        if ( observables->at(i)->GetName()==obsname ){
            index = i;
            break;
        }
    }
    if ( index==-1 ){
        // this should never happen...
        cout << "PDF_Abs::ScaleError() : ERROR : internal self inconsistency discovered. Exit." << endl;
        assert(0);
    }
    // scale error
    StatErr[index] *= scale;
    SystErr[index] *= scale;
    // update error source string
    obsErrSource += " (PDF_Abs::ScaleError(): scaled error of " + obsname + ")";
    return true;
}

///
/// Return the numerical value of an observable of a given name.
///
/// \param obsname  - observable name. If the PDF was uniquified before by
/// calling uniquify(), it has to include the unique ID string.
/// \return         - the value
///
float PDF_Abs::getObservableValue(TString obsname)
{
    // check if requested observable exits
    if ( ! hasObservable(obsname) ){
        cout << "PDF_Abs::getObservableValue() : ERROR : Requested observable doesn't exist: " << obsname << ". Exit." << endl;
        exit(1);
    }
    return ((RooRealVar*)observables->find(obsname))->getVal();
}

///
/// Return a submatrix of a given input matrix, defined by the rows
/// and columns provided.
///
/// \param source - the input matrix
/// \param target - the output matrix
/// \param indices - vector of the row/column indices that should make up the submatrix
///
void PDF_Abs::getSubMatrix(TMatrixDSym& target, TMatrixDSym& source, vector<int>& indices)
{
    if ( indices.size()==0 ){
        cout << "PDF_Abs::getSubMatrix() : vector 'indices' can't be empty" << endl;
        exit(1);
    }
    if ( target.GetNcols() != indices.size() ){
        cout << "PDF_Abs::getSubMatrix() : 'target' matrix doesn't have size of 'indices' vector" << endl;
        exit(1);
    }
    for ( int i=0; i<indices.size(); i++ ){
        // check requested index
        if ( indices[i]<0 || indices[i]>=source.GetNcols() ){
            cout << "PDF_Abs::getSubMatrix() : ERROR : requested index for submatrix is out of range of parent matrix" << endl;
            exit(1);
        }
        // copy over row and column
        for ( int j=0; j<indices.size(); j++ ){
            target[i][j] = source[indices[i]][indices[j]];
            target[j][i] = source[indices[j]][indices[i]];
        }
    }
}

///
/// Return a submatrix of the statistical correlation matrix, defined by the rows
/// and columns provided.
///
/// \param target - the output matrix
/// \param indices - vector of the row/column indices that should make up the submatrix
///
void PDF_Abs::getSubCorrelationStat(TMatrixDSym& target, vector<int>& indices)
{
    getSubMatrix(target, corStatMatrix, indices);
}

///
/// Return a submatrix of the systematic correlation matrix, defined by the rows
/// and columns provided.
///
/// \param target - the output matrix
/// \param indices - vector of the row/column indices that should make up the submatrix
///
void PDF_Abs::getSubCorrelationSyst(TMatrixDSym& target, vector<int>& indices)
{
    getSubMatrix(target, corSystMatrix, indices);
}

