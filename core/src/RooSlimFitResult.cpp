#include "RooSlimFitResult.h"

RooSlimFitResult::RooSlimFitResult(RooFitResult* r, bool storeCorrelation)
{
	init(r, storeCorrelation);
}

RooSlimFitResult::RooSlimFitResult(RooSlimFitResult* r)
{
	init(r);
}

///
/// copy constructor
///
RooSlimFitResult::RooSlimFitResult(const RooSlimFitResult &r) :
	TObject(reinterpret_cast<const TObject&>(r))
{
	init(&r);
}

///
/// default constructor (needed for TObject serialization)
///
	RooSlimFitResult::RooSlimFitResult()
: _correlationMatrix(0)
{
	_edm = std::numeric_limits<double>::quiet_NaN(); // set to nan
	_minNLL = std::numeric_limits<double>::quiet_NaN();
	_covQual = -9;
	_status = -9;
	_isConfirmed = false;
}

RooSlimFitResult::~RooSlimFitResult()
{
}

RooSlimFitResult* RooSlimFitResult::Clone()
{
	return new RooSlimFitResult(this);
}

///
/// Return a RooArgList of RooRealVars that constitute the
/// constant fit parameters. Ownership belongs to this class,
/// this is what we need the dummy variable for.
///
RooArgList& RooSlimFitResult::constPars() const
{
	// return if filled already - else it's a performance nightmare when this gets called in a loop
	if ( _constParsDummy.getSize()>0 ) return _constParsDummy;
	// create a RooArgList out of the content in the map
	_constParsDummy.removeAll();
	for ( int i=0; i<_parsNames.size(); i++ ){
		if (!_parsConst[i]) continue;
		TString name(_parsNames[i]);
		float value = _parsVal[i];
		RooRealVar var(name,name,value);
		var.setConstant(true);
		var.setUnit(_parsAngle[i] ? "Rad" : "" );
		_constParsDummy.addClone(var);
	}
	return _constParsDummy;
}

///
/// Return a RooArgList of RooRealVars that constitute the
/// floating fit parameters. Ownership belongs to this class.
///
RooArgList& RooSlimFitResult::floatParsFinal() const
{
	// return if filled already - else it's a performance nightmare when this gets called in a loop
	if ( _floatParsFinalDummy.getSize()>0 ) return _floatParsFinalDummy;
	// create a RooArgList out of the content in the map
	_floatParsFinalDummy.removeAll();
	for ( int i=0; i<_parsNames.size(); i++ ){
		if (_parsConst[i]) continue;
		TString name(_parsNames[i]);
		float value = _parsVal[i];
		float error = _parsErr[i];
		RooRealVar var(name,name,value);
		var.setError(error);
		var.setConstant(false);
		var.setUnit(_parsAngle[i] ? "Rad" : "" );
		_floatParsFinalDummy.addClone(var);
	}
	return _floatParsFinalDummy;
}

///
/// Return the value of a constant parameter contained in this
/// fit result.
/// \param name - the parameter name
/// \return - the value, NaN if the parameter wasn't found.
///
float RooSlimFitResult::getConstParVal(TString name) const
{
	for ( int i=0; i<_parsNames.size(); i++ ){
		if ( !_parsConst[i] ) continue;
		if ( TString(_parsNames[i])==name ) return _parsVal[i];
	}
	return std::numeric_limits<float>::quiet_NaN(); // return nan
}

///
/// Return the value of a floating parameter contained in this
/// fit result.
/// \param name - the parameter name
/// \return - the value, NaN if the parameter wasn't found.
///
float RooSlimFitResult::getFloatParFinalVal(TString name) const
{
	for ( int i=0; i<_parsNames.size(); i++ ){
		if ( _parsConst[i] ) continue;
		if ( TString(_parsNames[i])==name ) return _parsVal[i];
	}
	return std::numeric_limits<float>::quiet_NaN(); // return nan
}

///
/// Return the value of a parameter contained in this
/// fit result.
/// \param name - the parameter name
/// \return - the value, NaN if the parameter wasn't found.
///
float RooSlimFitResult::getParVal(TString name) const
{
	for ( int i=0; i<_parsNames.size(); i++ ){
		if ( TString(_parsNames[i])==name ) return _parsVal[i];
	}
	return std::numeric_limits<double>::quiet_NaN(); // return nan
}

///
/// Return the error of a parameter contained in this
/// fit result.
/// \param name - the parameter name
/// \return - the value, NaN if the parameter wasn't found.
///
float RooSlimFitResult::getParErr(TString name) const
{
	for ( int i=0; i<_parsNames.size(); i++ ){
		if ( TString(_parsNames[i])==name ) return _parsErr[i];
	}
	return std::numeric_limits<double>::quiet_NaN(); // return nan
}

///
/// Check if a parameter is contained in this
/// fit result. Can be either floating or constant.
/// \param name - the parameter name
/// \return - true if the parameter was found
///
bool RooSlimFitResult::hasParameter(TString name) const
{
	for ( int i=0; i<_parsNames.size(); i++ ){
		if ( TString(_parsNames[i])==name ) return true;
	}
	return false;
}

void RooSlimFitResult::SaveLatex(ofstream &outfile, bool verbose, bool printcor)
{
	outfile << "\%  FCN: " << minNll() << ", EDM: " << edm() << endl;
	outfile << "\%  COV quality: " << covQual() << ", status: " << status()
		<< ", confirmed: " << (_isConfirmed?"yes":"no") << endl;
	outfile << endl;
  outfile << "\\begin{tabular}{ l | l l l }" << endl;
	outfile << "  Parameter &  Value & & Uncertainty \\\\" << endl;
  vector<TString> myParNames;
	for ( int i=0; i<_parsNames.size(); i++ ){
    TString printName = "\\" + TString(_parsNames[i]).ReplaceAll("_","");
		float val = _parsVal[i];
		float err = _parsErr[i];
		if (_parsAngle[i]){
			val *= 180./TMath::Pi();
			err *= 180./TMath::Pi();
		}
		// print constant parameters
		if (_parsConst[i]){
			if ( ! TString(_parsNames[i]).Contains("obs") ){
        outfile << Form(" %-22s  &  $%5.3f$ & $\\pm$ & $%5.3f$",printName.Data(), val, err) ;
				if (_parsAngle[i]) outfile << " (Deg)";
        outfile << " \\\\" << endl;
			}
		}
		// print floating parameters
		else{
      outfile << Form(" %-22s  &  $%5.3f$ & $\\pm$ & $%5.3f$",printName.Data(), val, err) ;
      if (_parsAngle[i]) outfile << " (Deg)";
      outfile << " \\\\" << endl;
      myParNames.push_back(printName);
		}
	}
  outfile << "\\end{tabular}" << endl;

  // print correlations
  outfile << "\n\%Correlation matrix" << endl;
  outfile << "\\begin{tabular}{ l |";
  for (int i=0; i<_correlationMatrix.GetNcols(); i++) outfile << " l";
  outfile << " }" << endl;
  outfile << Form("  %-10s"," ");
  for (int i=0; i<_correlationMatrix.GetNcols(); i++) {
    outfile << " & " << Form("%5s",myParNames[i].Data());
  }
  outfile << " \\\\" << endl;
  for (int j=0; j<_correlationMatrix.GetNrows(); j++) {
    outfile << Form("  %-22s",myParNames[j].Data()) ;
    for (int i=0; i<_correlationMatrix.GetNcols(); i++) {
      outfile << " & $";
      if (TMath::Abs(_correlationMatrix[i][j])<0.01) outfile << "\\phantom{-}0   $";
      else {
        if (_correlationMatrix[i][j]>0) outfile << "\\phantom{-}";
        else outfile << "          ";
        outfile << Form("%4.2f$",_correlationMatrix[i][j]);
      }
    }
    outfile << " \\\\" << endl;
  }
  outfile << "\\end{tabular}" << endl;
  outfile << endl;
}

void RooSlimFitResult::Print(bool verbose, bool printcor)
{
	cout << "  FCN: " << minNll() << ", EDM: " << edm() << endl;
	cout << "  COV quality: " << covQual() << ", status: " << status()
		<< ", confirmed: " << (_isConfirmed?"yes":"no") << endl;
	cout << endl;
	cout << "    Parameter                      FinalValue +/- Error " << (_isConfirmed?"(HESSE)":"(MIGRAD)") << endl;
	cout << "  ----------------------------   ---------------------------------" << endl;
	for ( int i=0; i<_parsNames.size(); i++ ){
		float val = _parsVal[i];
		float err = _parsErr[i];
		if (_parsAngle[i]){
			val *= 180./TMath::Pi();
			err *= 180./TMath::Pi();
		}
		// print constant parameters
		if (_parsConst[i]){
			if ( ! TString(_parsNames[i]).Contains("obs") ){
				printf("       %22s    %11.6g +/- %10.6g (const)", TString(_parsNames[i]).Data(), val, err);
				if (_parsAngle[i]) cout << " (Deg)";
				cout << endl;
			}
		}
		// print floating parameters
		else{
			printf("    %2i %22s    %11.6g +/- %10.6g", _parsFloatId[i], TString(_parsNames[i]).Data(), val, err);
			if (_parsAngle[i]) cout << " (Deg)";
			cout << endl;
		}
	}
	if ( printcor ){
		// print correlations
		cout << "\n    Correlation matrix" << endl;
		cout << "  ----------------------------" << endl;
		_correlationMatrix.Print();
	}
	cout << endl;
}


bool RooSlimFitResult::isAngle(RooRealVar* v)
{
	return v->getUnit()==TString("Rad") || v->getUnit()==TString("rad");
}
