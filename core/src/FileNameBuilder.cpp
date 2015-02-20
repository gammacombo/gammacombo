#include "FileNameBuilder.h"

#include "MethodAbsScan.h"
#include "GammaComboEngine.h"
#include "Combiner.h"

FileNameBuilder::FileNameBuilder(OptParser *arg, TString name)
{
	assert(arg);
	_arg = arg;
	_basename = name;
}

FileNameBuilder::~FileNameBuilder()
{}

///
/// Compute the file base name of when including several
/// into one file.
///
/// \param gc - an engine object
/// \return the filename: basename_combiner1name[_addPdfN][_delPdfN][_combiner2name[_addPdfN][_delPdfN]]_var1[_var2]
///
TString FileNameBuilder::getFileBaseName(const GammaComboEngine *gc)
{
	TString name = gc->getBasename();
	for ( int i=0; i<_arg->combid.size(); i++ ) name += "_"+gc->getCombiner(_arg->combid[i])->getName();
	name += "_"+_arg->var[0];
	if ( _arg->var.size()==2 ) name += "_"+_arg->var[1];
	return name;
}

///
/// Compute the file base name of individual combinations.
///
/// \param c - Combiner object
/// \return the filename: basename_combinername[_addPdfN][_delPdfN]_var1[_var2]
///
TString FileNameBuilder::getFileBaseName(const Combiner *c)
{
	TString name = _basename;
	name += "_"+c->getName();
	name += "_"+_arg->var[0];
	if ( _arg->var.size()==2 ) name += "_"+_arg->var[1];
	return name;
}

///
/// Compute the file name of the file to which a scanner gets saved.
/// \return the filename: plots/scanner/scanner_scannername[_Plugin]_var1[_var2].root
///
TString FileNameBuilder::getFileNameScanner(const MethodAbsScan *c)
{
	TString name = "plots/scanner/"+_basename+"_scanner_"+c->getName();
	if ( c->getMethodName()!=TString("Prob") ) name += "_"+c->getMethodName();
	name += "_"+_arg->var[0];
	if ( _arg->var.size()==2 ) name += "_"+_arg->var[1];
	name += ".root";
	return name;
}
