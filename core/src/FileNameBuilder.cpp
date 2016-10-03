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
	m_asimov = "Asimov";
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
TString FileNameBuilder::getFileBaseName(const MethodAbsScan *s)
{
	return getFileBaseName(s->getCombiner());
}

///
/// Compute the file name of the start parameter file.
/// Format of returned filename:
///
/// plots/par/basename_combinername[_+N][_-N]_var1[_var2]_start.dat
///
/// \param c - Combiner object
/// \return - filename
///
TString FileNameBuilder::getFileNameStartPar(const Combiner *c)
{
	TString name = "plots/par/";
	name += getFileBaseName(c);
	name += "_start.dat";
	return name;
}
TString FileNameBuilder::getFileNameStartPar(const MethodAbsScan *s)
{
	return getFileNameStartPar(s->getCombiner());
}

///
/// Compute the file name of the parameter file defining the Asimov
/// point where the Asimov toy is generated at.
/// The combiner name will be followed by the Asimov addition (getAsimovCombinerNameAddition()),
/// but without the number denoting the Asimov point in the file, as the file contains all points.
/// Format of returned filename:
///
/// plots/par/basename_combinernameAsimov[_+N][_-N]_var1[_var2]_genpoints.dat
///
/// \param c - Combiner object
/// \return - filename
///
TString FileNameBuilder::getFileNameAsimovPar(const Combiner *c)
{
	TString name = "plots/par/";
	name += getFileBaseName(c);
	// remove any string after the "Asimov" token and the first "_" after that
	// e.g.: "combinerAsimov3_" -> "combinerAsimov_"
	int startOfToken = name.Index(m_asimov);
	int startOfFirstUnderscore = name.Index("_",startOfToken);
	int length = startOfFirstUnderscore-(startOfToken+m_asimov.Sizeof())+1;
	name.Replace(startOfToken+m_asimov.Sizeof()-1, length, "");
	name += "_genpoints.dat";
	return name;
}
TString FileNameBuilder::getFileNameAsimovPar(const MethodAbsScan *s)
{
	return getFileNameStartPar(s->getCombiner());
}

///
/// Compute the file name of the parameter file.
/// Format of returned filename:
///
/// plots/par/basename_combinername[_+N][_-N]_var1[_var2].dat
///
/// \param c - Combiner object
/// \return - filename
///
TString FileNameBuilder::getFileNamePar(const Combiner *c)
{
	TString name = "plots/par/";
	name += getFileBaseName(c);
	name += ".dat";
	return name;
}

///
/// Compute the file name of the parameter file.
/// Format of returned filename:
///
/// plots/par/basename_combinername[_+N][_-N]_var1[_var2].dat
///
/// \param s - Scanner object
/// \return - filename
///
TString FileNameBuilder::getFileNamePar(const MethodAbsScan *s)
{
	return getFileNamePar(s->getCombiner());
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
TString FileNameBuilder::getFileNamePlot(const vector<Combiner*>& cmb)
{
	TString name = m_basename;
    if ( m_arg->filenamechange != "" ) {
        name += "_" + m_arg->filenamechange;
        return name;
    }

	for ( int i=0; i<m_arg->combid.size(); i++ ){
		name += "_"+cmb[m_arg->combid[i]]->getName();
		//if ( m_arg->isAsimovCombiner(i) ) name += Form("Asimov%i",m_arg->asimov[i]);
	}
	name += "_"+m_arg->var[0];
	if ( m_arg->var.size()==2 )           name += "_"+m_arg->var[1];
	if ( m_arg->plotpluginonly )          name += "_"+getPluginOnlyNameAddition();
	else if ( m_arg->isAction("plugin") ) name += "_"+getPluginNameAddition();
	if ( m_arg->plotprelim )              name += "_"+getPreliminaryNameAddition();
	return name;
}

///
/// Compute the file name for plots holding a single combiner, in particular
/// the full likelihood plots.
/// Format of the file name:
///
/// basename_combinername[_+/-N]_var1[_var2][_pluginonly]
///
/// \return - the filename
///
TString FileNameBuilder::getFileNamePlotSingle(const vector<Combiner*>& cmb, int cId)
{
	TString name = m_basename;
	name += "_"+cmb[m_arg->combid[cId]]->getName();
	name += "_"+m_arg->var[0];
	if ( m_arg->var.size()==2 )           name += "_"+m_arg->var[1];
	if ( m_arg->plotpluginonly )          name += "_"+getPluginOnlyNameAddition();
	else if ( m_arg->isAction("plugin") ) name += "_"+getPluginNameAddition();
	if ( m_arg->plotprelim )              name += "_"+getPreliminaryNameAddition();
	return name;
}

///
/// Define the addition to combiner names for Asimov combiners.
///
TString FileNameBuilder::getAsimovCombinerNameAddition(int id)
{
	return m_asimov + Form("%i", id);
}

///
/// Define the addition for plugin plots.
///
TString FileNameBuilder::getPluginNameAddition()
{
	return "plugin";
}

///
/// Define the addition for plugin-only plots.
///
TString FileNameBuilder::getPluginOnlyNameAddition()
{
	return "pluginonly";
}

///
/// Define the addition for preliminary plots.
///
TString FileNameBuilder::getPreliminaryNameAddition()
{
	return "prelim";
}

