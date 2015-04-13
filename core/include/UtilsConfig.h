/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: June 2014
 *
 **/

#ifndef UtilsConfig_h
#define UtilsConfig_h
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <cassert>

#include "TString.h"

using namespace std;

namespace Utils
{
	enum config {
		year2014,
		babar,
		babar2007,
		babar2008,
		babar2010,
		babar2012,
		belle,
		belle2005cleo2009,
		belle2006,
		belle2007,
		belle2009,
		belle2012,
		belle2012preliminary,
		belle2013,
		belle2014,
		belle2013cleo2014,
		cdf2007,
		cdf2012,
		cdf2013,
		ckm2014,
		cleo,
		cleo2001,
		cleo2009,
		cleo2012,
		cleo2014,
		cleoFullDP,
		combos2008,
		default_config,
		excludeKdDdK3pi,
		exclusive2014,
		inclusive2014,
		focus2000,
		hfag,
		hfagFP2014,
		hfagLP2011,
		highrb,
		highstattoy,
		lhcb,
		lhcb2011,
		lhcb2012,
		lhcb2013,
		lhcb2013KK,
		lhcb2013pipi,
		lhcb2013preliminary,
		lhcb2014,
		lhcb2018KK_extrap,
		lhcb_upgrade_extrap,
		lhcbphis,
		lhcbcomb,
		lumi1fb,
		lumi1fbConfcFit,
		lumi1fbConfsFit,
		lumi1fbNoAfav,
		lumi1fbPapercFit,
		lumi1fbPapercFitExpected,
		lumi1fbPapersFit,
		lumi1fbSystCor,
		lumi1fbprompt,
		lumi1fbsl,
		lumi2fb,
		lumi3fb,
		lumi3fbCPVA,
		lumi3fbFix,
		lumi3fbFullDP,
		lumi3fbPaper,
		lumi3fbDKstz,
		lumi5ab,
		lumi5fb,
		lumi9fb,
		lumi50ab,
		lumi50fb,
		lambda1_3fb,
		lambdafree_3fb,
		manual,
		none,
		nophicorr,
		sneha,
		toy,
		truth,
		useBicubic,
		useCartCoords,
		useGaussian,
		useHistogram,
		useParametric,
		usePolarCoords,
		useTradObs,
		zero
	};

	config  TStringToConfig(TString s);
	TString ConfigToTString(config c);
}

#endif
