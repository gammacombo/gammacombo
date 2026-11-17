#include <FileNameBuilder.h>

#include <Combiner.h>
#include <MethodAbsScan.h>
#include <OptParser.h>

#include <TString.h>

#include <cassert>
#include <vector>

using namespace std;

///
/// Constructor.
///
/// \param arg - command line parsing object
/// \param name - the base name, e.g. "gammacombo"
///
FileNameBuilder::FileNameBuilder(const OptParser* arg, TString name) : m_basename{name} {
  assert(arg);
  m_arg = arg;
}

///
/// Get base name, e.g. "gammacombo". The base name is set
/// in the constructor.
///
/// \return - the base name
///
TString FileNameBuilder::getBaseName() const { return m_basename; }

///
/// Compute the base name identifying a combination performed in a given GammaComboEngine setting, without any scan
/// variable.
///
/// @param c Combiner object
/// @return  Filename, in the format `<GammaComboEngine-instance-name>_<combiner-name>`
///
TString FileNameBuilder::getCombinerFileName(const Combiner* c) const { return m_basename + "_" + c->getName(); }

TString FileNameBuilder::getCombinerFileName(const MethodAbsScan* s) const {
  return getCombinerFileName(s->getCombiner());
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
TString FileNameBuilder::getFileBaseName(const Combiner* c) const {
  TString name = m_basename;
  name += "_" + c->getName();
  name += "_" + m_arg->var[0];
  if (m_arg->var.size() == 2) name += "_" + m_arg->var[1];
  return name;
}
TString FileNameBuilder::getFileBaseName(const MethodAbsScan* s) const { return getFileBaseName(s->getCombiner()); }

///
/// Compute the file name of the start parameter file.
/// Format of returned filename:
///
/// plots/par/basename_combinername[_+N][_-N]_var1[_var2]_start.dat
///
/// \param c - Combiner object
/// \return - filename
///
TString FileNameBuilder::getFileNameStartPar(const Combiner* c) const {
  TString name = "plots/par/";
  name += getFileBaseName(c);
  name += "_start.dat";
  return name;
}
TString FileNameBuilder::getFileNameStartPar(const MethodAbsScan* s) const {
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
TString FileNameBuilder::getFileNameAsimovPar(const Combiner* c) const {
  TString name = "plots/par/";
  name += getFileBaseName(c);
  // remove any string after the "Asimov" token and the first "_" after that
  // e.g.: "combinerAsimov3_" -> "combinerAsimov_"
  int startOfToken = name.Index(m_asimov);
  int startOfFirstUnderscore = name.Index("_", startOfToken);
  int length = startOfFirstUnderscore - (startOfToken + m_asimov.Sizeof()) + 1;
  name.Replace(startOfToken + m_asimov.Sizeof() - 1, length, "");
  name += "_genpoints.dat";
  return name;
}
TString FileNameBuilder::getFileNameAsimovPar(const MethodAbsScan* s) const {
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
TString FileNameBuilder::getFileNamePar(const Combiner* c) const {
  TString name = "plots/par/";
  if (m_arg->filenamechange != "") {
    name += m_arg->filenamechange;
  } else {
    name += getFileBaseName(c);
  }
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
TString FileNameBuilder::getFileNamePar(const MethodAbsScan* s) const { return getFileNamePar(s->getCombiner()); }

///
/// Compute the file name of the file to which a scanner gets saved.
/// Format of returned filename:
///
/// plots/scanner/scanner_scannername[_Plugin]_var1[_var2].root
///
/// \return - filename
///
TString FileNameBuilder::getFileNameScanner(const MethodAbsScan* c) const {
  TString name = "plots/scanner/" + m_basename + "_scanner_" + c->getName();
  if (c->getMethodName() != TString("Prob")) name += "_" + c->getMethodName();
  name += "_" + m_arg->var[0];
  if (m_arg->var.size() == 2) name += "_" + m_arg->var[1];
  name += ".root";
  return name;
}

///
/// Compute the file name of the file to which a solution gets saved.
/// Format of returned filename:
///
/// plots/latex/combo_solution[_Plugin]_var1[_var2].root
///
/// \return - filename
///
TString FileNameBuilder::getFileNameSolution(const MethodAbsScan* c) const {
  TString name = "plots/latex/" + m_basename + "_solution_" + c->getName();
  if (c->getMethodName() != TString("Prob")) name += "_" + c->getMethodName();
  name += "_" + m_arg->var[0];
  if (m_arg->var.size() == 2) name += "_" + m_arg->var[1];
  name += ".tex";
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
TString FileNameBuilder::getFileNamePlot(const vector<Combiner*>& cmb) const {
  TString name = m_basename;
  if (m_arg->filenamechange != "") {
    name += "_" + m_arg->filenamechange;
    return name;
  }

  for (int i = 0; i < m_arg->combid.size(); i++) {
    name += "_" + cmb[m_arg->combid[i]]->getName();
    // if ( m_arg->isAsimovCombiner(i) ) name += Form("Asimov%i",m_arg->asimov[i]);
  }
  name += "_" + m_arg->var[0];
  if (m_arg->var.size() == 2) name += "_" + m_arg->var[1];
  if (m_arg->plotpluginonly)
    name += "_" + getPluginOnlyNameAddition();
  else if (m_arg->isAction("plugin"))
    name += "_" + getPluginNameAddition();
  if (m_arg->cls.size() > 0) name += "_" + getCLsNameAddition();
  if (m_arg->plotprelim) name += "_" + getPreliminaryNameAddition();
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
TString FileNameBuilder::getFileNamePlotSingle(const vector<Combiner*>& cmb, int cId) const {
  TString name = m_basename;
  name += "_" + cmb[m_arg->combid[cId]]->getName();
  name += "_" + m_arg->var[0];
  if (m_arg->var.size() == 2) name += "_" + m_arg->var[1];
  if (m_arg->plotpluginonly)
    name += "_" + getPluginOnlyNameAddition();
  else if (m_arg->isAction("plugin"))
    name += "_" + getPluginNameAddition();
  if (m_arg->plotprelim) name += "_" + getPreliminaryNameAddition();
  return name;
}

///
/// Define the addition to combiner names for Asimov combiners.
///
TString FileNameBuilder::getAsimovCombinerNameAddition(int id) const { return m_asimov + Form("%i", id); }

///
/// Define the addition for plugin plots.
///
TString FileNameBuilder::getPluginNameAddition() const { return "plugin"; }

///
/// Define the addition for plugin-only plots.
///
TString FileNameBuilder::getPluginOnlyNameAddition() const { return "pluginonly"; }

///
/// Define the addition for preliminary plots.
///
TString FileNameBuilder::getPreliminaryNameAddition() const { return "prelim"; }

///
/// Define the addition for cls plots.
///
TString FileNameBuilder::getCLsNameAddition() const { return "cls"; }
