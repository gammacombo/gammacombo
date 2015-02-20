#include "FileNameBuilder.h"

#include "MethodAbsScan.h"
#include "GammaComboEngine.h"
#include "Combiner.h"

///
/// Constructor.
///
/// \param arg - command line parsing object
/// \param name - the base name, e.g. "gammacombo"
///
FileNameBuilder::FileNameBuilder(OptParser *arg, TString name)
{
	assert(arg);
	m_arg = arg;
	m_basename = name;
}

FileNameBuilder::~FileNameBuilder()
{}

///
/// Get base name, e.g. "gammacombo". The base name is set
/// in the constructor.
///
/// \return - the base name
///
TString FileNameBuilder::getBaseName(){
	return m_basename;
}

///
/// Compute the file base name of individual combinations.
/// Format of returned filename:
///
/// basename_combinername[_+N][_-N]_var1[_var2]
///
/// \param c - Combiner object
/// \return - filename
///
TString FileNameBuilder::getFileBaseName(const Combiner *c)
{
	TString name = m_basename;
	name += "_"+c->getName();
	name += "_"+m_arg->var[0];
	if ( m_arg->var.size()==2 ) name += "_"+m_arg->var[1];
	return name;
}

///
/// Compute the file name of the file to which a scanner gets saved.
/// Format of returned filename:
///
/// plots/scanner/scanner_scannername[_Plugin]_var1[_var2].root
///
/// \return - filename
///
TString FileNameBuilder::getFileNameScanner(const MethodAbsScan *c)
{
	TString name = "plots/scanner/"+m_basename+"_scanner_"+c->getName();
	if ( c->getMethodName()!=TString("Prob") ) name += "_"+c->getMethodName();
	name += "_"+m_arg->var[0];
	if ( m_arg->var.size()==2 ) name += "_"+m_arg->var[1];
	name += ".root";
	return name;
}

///
/// Compute the file name for plots. It can contain multiple combiners
/// in the name. Format of the file name:
///
/// basename_combiner1name[_+/-N][_combiner2name[_+/-N]_var1[_var2][_pluginonly]
///
/// \return - the filename
///
TString FileNameBuilder::getPlotFileName(const vector<Combiner*>& cmb)
{
	TString name = m_basename;
	for ( int i=0; i<m_arg->combid.size(); i++ ){
		name += "_"+cmb[m_arg->combid[i]]->getName();
		//if ( m_arg->isAsimovCombiner(i) ) name += Form("Asimov%i",m_arg->asimov[i]);
	}
	name += "_"+m_arg->var[0];
	if ( m_arg->var.size()==2 ) name += "_"+m_arg->var[1];
	if ( m_arg->plotpluginonly ) name += "_pluginonly";
	return name;
}

