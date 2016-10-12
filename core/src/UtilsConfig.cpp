/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: June 2014
 *
 **/

#include "UtilsConfig.h"

TString Utils::ConfigToTString(config s)
{
	if ( s == babar )                return "babar";
	if ( s == babar2007 )            return "babar2007";
	if ( s == babar2008 )            return "babar2008";
	if ( s == babar2010 )            return "babar2010";
	if ( s == babar2012 )            return "babar2012";
	if ( s == babar_dpi0 )           return "babar_dpi0";
	if ( s == babar_dg )             return "babar_dg";
	if ( s == belle2006 )            return "belle2006";
	if ( s == belle2007 )            return "belle2007";
	if ( s == belle2009 )            return "belle2009";
	if ( s == belle2012 )            return "belle2012";
	if ( s == belle2012preliminary ) return "belle2012preliminary";
	if ( s == belle2013 )            return "belle2013";
	if ( s == belle2014 )            return "belle2014";
	if ( s == belle_dpi0 )           return "belle_dpi0";
	if ( s == belle_dg )             return "belle_dg";
	if ( s == cdf2007 )              return "cdf2007";
	if ( s == cdf2012 )              return "cdf2012";
	if ( s == cdf2013 )              return "cdf2013";
	if ( s == ckm2014 )              return "ckm2014";
	if ( s == cleo )                 return "cleo";
	if ( s == cleo2001 )             return "cleo2001";
	if ( s == cleo2012 )             return "cleo2012";
	if ( s == cleo2014 )             return "cleo2014";
	if ( s == cleoFullDP )           return "cleoFullDP";
	if ( s == combos2008 )           return "combos2008";
	if ( s == default_config )       return "default_config";
	if ( s == excludeKdDdK3pi )      return "excludeKdDdK3pi";
	if ( s == focus2000 )            return "focus2000";
	if ( s == hfag )                 return "hfag";
	if ( s == highrb )               return "highrb";
	if ( s == highstattoy )          return "highstattoy";
	if ( s == lhcb)                  return "lhcb";
	if ( s == lhcb2011 )             return "lhcb2011";
	if ( s == lhcb2012 )             return "lhcb2012";
	if ( s == lhcb2013 )             return "lhcb2013";
	if ( s == lhcb2013KK )           return "lhcb2013KK";
	if ( s == lhcb2013pipi )         return "lhcb2013pipi";
	if ( s == lhcb2013preliminary )  return "lhcb2013preliminary";
	if ( s == lhcb2014 )             return "lhcb2014";
	if ( s == lhcb2018KK_extrap )    return "lhcb2018KK_extrap";
	if ( s == lhcb_upgrade_extrap )  return "lhcb_upgrade_extrap";
	if ( s == lhcbcomb )  					 return "lhcbcomb";
	if ( s == lhcbphis ) 						 return "lhcbphis";
	if ( s == lumi1fb )	             return "lumi1fb";
	if ( s == lumi1fbConfcFit )      return "lumi1fbConfcFit";
	if ( s == lumi1fbConfsFit )      return "lumi1fbConfsFit";
	if ( s == lumi1fbNoAfav )        return "lumi1fbNoAfav";
	if ( s == lumi1fbPapercFit )     return "lumi1fbPapercFit";
	if ( s == lumi1fbPapercFitExpected )     return "lumi1fbPapercFitExpected";
	if ( s == lumi1fbPapersFit )     return "lumi1fbPapersFit";
	if ( s == lumi1fbSystCor )       return "lumi1fbSystCor";
	if ( s == lumi1fbprompt )        return "lumi1fbprompt";
	if ( s == lumi1fbsl )            return "lumi1fbsl";
	if ( s == lumi2fb )              return "lumi2fb";
	if ( s == lumi3fb )              return "lumi3fb";
	if ( s == lumi3fbDKstz )         return "lumi3fbDKstz";
	if ( s == lumi3fbFix )           return "lumi3fbFix";
	if ( s == lumi3fbPaper )         return "lumi3fbPaper";
	if ( s == lumi50fb )             return "lumi50fb";
	if ( s == lumi9fb )              return "lumi9fb";
	if ( s == manual )               return "manual";
	if ( s == none )                 return "none";
	if ( s == nophicorr ) 					 return "nophicorr";
	if ( s == sneha )                return "sneha";
	if ( s == toy )                  return "toy";
	if ( s == truth )                return "truth";
	if ( s == useBicubic )           return "useBicubic";
	if ( s == useCartCoords )        return "useCartCoords";
	if ( s == useGaussian )          return "useGaussian";
	if ( s == useHistogram )         return "useHistogram";
	if ( s == useParametric )        return "useParametric";
	if ( s == usePolarCoords )       return "usePolarCoords";
	if ( s == useTradObs )           return "useTradObs";
  if ( s == world_average )        return "world_average";
	if ( s == year2014 )             return "year2014";
	if ( s == zero )                 return "zero";

	cout << "PDF_Abs::ConfigToTString() : ERROR : Config not found: " << s << endl;
	exit(1);
}

Utils::config Utils::TStringToConfig(TString c)
{
	if ( c == "lumi1fb" )           return lumi1fb;
	if ( c == "truth"   )           return truth;
	if ( c == "zero"    )           return zero;
	if ( c == "manual"  )           return manual;
	cout << "PDF_Abs::TStringToConfig() : ERROR : Config not found." << endl;
	exit(1);
}
