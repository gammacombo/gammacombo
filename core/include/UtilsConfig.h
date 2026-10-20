/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: June 2014
 *
 **/

#ifndef UtilsConfig_h
#define UtilsConfig_h

#include <TString.h>

#include <iosfwd>
#include <string>

/// The predefined configurations. X(name) for each; this one list defines the
/// Utils::config enum values and their names (ConfigToTString, TStringToConfig).
/// New configurations do not need to be added here any more: a PDF that takes
/// Utils::Config arguments also accepts any name as a string (see below).
// clang-format off
#define GAMMACOMBO_UTILS_CONFIGS(X) \
  X(year2014)                  \
  X(babar)                     \
  X(babar2007)                 \
  X(babar2008)                 \
  X(babar2010)                 \
  X(babar2012)                 \
  X(babar_dpi0)                \
  X(babar_dg)                  \
  X(belle)                     \
  X(belle2005cleo2009)         \
  X(belle2006)                 \
  X(belle2007)                 \
  X(belle2009)                 \
  X(belle2012)                 \
  X(belle2012preliminary)      \
  X(belle2013)                 \
  X(belle2014)                 \
  X(belle2013cleo2014)         \
  X(belle_dpi0)                \
  X(belle_dg)                  \
  X(belle_md)                  \
  X(belle_1ab)                 \
  X(belle_5ab)                 \
  X(belle_50ab)                \
  X(belle2)                    \
  X(belle2_1ab)                \
  X(belle2_5ab)                \
  X(belle2_50ab)               \
  X(belle2_1ab_had)            \
  X(belle2_5ab_had)            \
  X(belle2_50ab_had)           \
  X(belle2_1ab_sl)             \
  X(belle2_5ab_sl)             \
  X(belle2_50ab_sl)            \
  X(belle2_1ab_WA)             \
  X(belle2_5ab_WA)             \
  X(belle2_50ab_WA)            \
  X(belle2_1ab_Dstar)          \
  X(belle2_5ab_Dstar)          \
  X(belle2_50ab_Dstar)         \
  X(belle2_1ab_D)              \
  X(belle2_5ab_D)              \
  X(belle2_50ab_D)             \
  X(belle2_1ab_tagged)         \
  X(belle2_5ab_tagged)         \
  X(belle2_50ab_tagged)        \
  X(belle2_1ab_untagged)       \
  X(belle2_5ab_untagged)       \
  X(belle2_50ab_untagged)      \
  X(belle2_387)                \
  X(bes3)                      \
  X(cdf)                       \
  X(cdf2007)                   \
  X(cdf2012)                   \
  X(cdf2013)                   \
  X(check)                     \
  X(ckm)                       \
  X(ckm_dir)                   \
  X(ckm_dir_m1)                \
  X(ckm_dir_m2)                \
  X(ckm_dir_m3)                \
  X(ckm_indir)                 \
  X(ckm2014)                   \
  X(ckm2015)                   \
  X(cleo)                      \
  X(cleo2001)                  \
  X(cleo2009)                  \
  X(cleo2012)                  \
  X(cleo2014)                  \
  X(cleo2015)                  \
  X(cleo2016)                  \
  X(cleoFullDP)                \
  X(combos2008)                \
  X(combpipi)                  \
  X(delta)                     \
  X(default_config)            \
  X(d0)                        \
  X(dst_cln)                   \
  X(dst_bgs)                   \
  X(dst_bgk)                   \
  X(excludeKdDdK3pi)           \
  X(exclusive2014)             \
  X(inclusive2014)             \
  X(focus2000)                 \
  X(hfag)                      \
  X(hfagFP2014)                \
  X(hfagLP2011)                \
  X(hfagCHARM2015)             \
  X(hfagCKM2016)               \
  X(hflav)                     \
  X(hflavMoriond19)            \
  X(hflavMoriond24)            \
  X(hflavMor19)                \
  X(hflav_alt)                 \
  X(highrb)                    \
  X(highstattoy)               \
  X(lhcb)                      \
  X(lhcb_prelim)               \
  X(lhcb_WA)                   \
  X(lhcb_md)                   \
  X(lhcb_mi)                   \
  X(lhcb_md_syst)              \
  X(lhcb_mi_syst)              \
  X(lhcb_md_syst_50fb)         \
  X(lhcb_mi_syst_50fb)         \
  X(lhcb2011)                  \
  X(lhcb2012)                  \
  X(lhcb2013)                  \
  X(lhcb2013KK)                \
  X(lhcb2013pipi)              \
  X(lhcb2013preliminary)       \
  X(lhcb2014)                  \
  X(lhcb2018KK_extrap)         \
  X(lhcb_upgrade_extrap)       \
  X(lhcbphis)                  \
  X(lhcbcomb)                  \
  X(lhcb_old)                  \
  X(lhcb_pipi)                 \
  X(lhcb_kk)                   \
  X(lhcb_r1)                   \
  X(lhcb_r2)                   \
  X(lhcb_3fb)                  \
  X(lhcb_5fb)                  \
  X(lhcb_8fb)                  \
  X(lhcb_9fb)                  \
  X(lhcb_10fb)                 \
  X(lhcb_22fb)                 \
  X(lhcb_50fb)                 \
  X(lhcb_300fb)                \
  X(lhcb_3fb_WA)               \
  X(lhcb_10fb_WA)              \
  X(lhcb_22fb_WA)              \
  X(lhcb_50fb_WA)              \
  X(lhcb_sim_3fb)              \
  X(lhcb_sim_8fb)              \
  X(lhcb_sim_50fb)             \
  X(lhcb_sim_300fb)            \
  X(lhcb_simst_3fb)            \
  X(lhcb_simst_8fb)            \
  X(lhcb_simst_50fb)           \
  X(lhcb_simst_300fb)          \
  X(lhcb_pr_9fb)               \
  X(lhcb_fr_9fb)               \
  X(lumi40pb)                  \
  X(lumi1fb)                   \
  X(lumi1fbConfcFit)           \
  X(lumi1fbConfsFit)           \
  X(lumi1fbNoAfav)             \
  X(lumi1fbPapercFit)          \
  X(lumi1fbPapercFitExpected)  \
  X(lumi1fbPapersFit)          \
  X(lumi1fbSystCor)            \
  X(lumi1fbprompt)             \
  X(lumi1fbsl)                 \
  X(lumi2fb)                   \
  X(lumi3fb)                   \
  X(lumi3fbCPVA)               \
  X(lumi3fbFix)                \
  X(lumi3fbFullDP)             \
  X(lumi3fbPaper)              \
  X(lumi3fbDKstz)              \
  X(lumi3fb_estimate)          \
  X(lumi3fbUpdated)            \
  X(lumi5ab)                   \
  X(lumi5fb)                   \
  X(lumi5fbOld)                \
  X(lumi9fb)                   \
  X(lumi50ab)                  \
  X(lumi50fb)                  \
  X(lumi300fb)                 \
  X(lambda1_3fb)               \
  X(lambdafree_3fb)            \
  X(manual)                    \
  X(milc_update)               \
  X(none)                      \
  X(nophicorr)                 \
  X(onlyGsDGs)                 \
  X(pos)                       \
  X(neg)                       \
  X(pdg)                       \
  X(pdg_incl)                  \
  X(pdg_excl)                  \
  X(pdg_incl_m1)               \
  X(pdg_excl_m1)               \
  X(pdg_incl_m2)               \
  X(pdg_excl_m2)               \
  X(run2)                      \
  X(sneha)                     \
  X(SM)                        \
  X(SM_prediction)             \
  X(best_prediction)           \
  X(statonly)                  \
  X(test)                      \
  X(test1)                     \
  X(test2)                     \
  X(test3)                     \
  X(test4)                     \
  X(test5)                     \
  X(test6)                     \
  X(test7)                     \
  X(test8)                     \
  X(test9)                     \
  X(toy)                       \
  X(truth)                     \
  X(useBicubic)                \
  X(useCartCoords)             \
  X(useGaussian)               \
  X(useHistogram)              \
  X(useParametric)             \
  X(usePolarCoords)            \
  X(useTradObs)                \
  X(ut2014)                    \
  X(world_average)             \
  X(zero)
// clang-format on

namespace Utils {
  enum config {
#define GAMMACOMBO_CONFIG_ENUM_VALUE(name) name,
    GAMMACOMBO_UTILS_CONFIGS(GAMMACOMBO_CONFIG_ENUM_VALUE)
#undef GAMMACOMBO_CONFIG_ENUM_VALUE
    /// Not a predefined configuration: a Utils::Config given by a name that is
    /// not in the list above has this value. Always the last enum value.
    string_config
  };

  /// Name of a predefined configuration ("string_config" for string_config).
  /// Exits for a value that is not a configuration.
  TString ConfigToTString(config c);
  /// Predefined configuration with this name. Exits if there is none (use
  /// findConfig() to test).
  config TStringToConfig(TString s);
  /// Looks up a predefined configuration by name. Returns false (and leaves c
  /// unchanged) if there is none.
  bool findConfig(const TString& name, config& c);

  /**
   * A configuration given either as a predefined Utils::config value or as a
   * string, e.g.
   *
   *   new PDF_BdPizPizBR(Utils::config::hflav, Utils::config::hflav, Utils::config::none);
   *   new PDF_BdPizPizBR("hflav", "hflav", "none");                   // the same
   *   new PDF_BdPizPizBR("hflav_rd_nov26", "hflav_rd_nov26", "none"); // no enum value needed
   *
   * A string that names a predefined configuration gives that enum value;
   * any other string gives Utils::string_config and is only known by its name.
   *
   * To let a PDF take strings, change the type of its constructor arguments
   * and of the arguments of setObservables(), setUncertainties() and
   * setCorrelations() from "Utils::config" to "const Utils::Config&". Calls with
   * enum values still compile, and so does the existing
   *
   *   switch (c) { case Utils::config::hflav: ... default: ... ConfigToTString(c) ... }
   *
   * (the switch uses the enum value). Handle new names before the switch:
   *
   *   if (c == "hflav_rd_nov26") { obsValSource = "..."; setObservable(...); return; }
   */
  class Config {
   public:
    Config(config c);
    Config(const char* name);
    Config(const TString& name);
    Config(const std::string& name);

    /// The predefined enum value, or Utils::string_config. Implicit, so that
    /// switch (c) and code written for Utils::config keep working.
    operator config() const { return fId; }
    config id() const { return fId; }
    const TString& name() const { return fName; }
    /// True if this is one of the predefined Utils::config values.
    bool isPredefined() const { return fId != string_config; }

   private:
    config fId;
    TString fName;
  };

  /// Name of the configuration (the given string for a string-only one).
  TString ConfigToTString(const Config& c);

  // Comparisons. Against another Config or a string: by name. Against an enum
  // value: by enum value (so a string-only Config equals Utils::string_config).
  inline bool operator==(const Config& a, const Config& b) { return a.name() == b.name(); }
  inline bool operator==(const Config& a, config b) { return a.id() == b; }
  inline bool operator==(config a, const Config& b) { return a == b.id(); }
  inline bool operator==(const Config& a, const char* b) { return a.name() == b; }
  inline bool operator==(const char* a, const Config& b) { return b.name() == a; }
  inline bool operator==(const Config& a, const TString& b) { return a.name() == b; }
  inline bool operator==(const TString& a, const Config& b) { return a == b.name(); }
  inline bool operator==(const Config& a, const std::string& b) { return a.name() == b.c_str(); }
  inline bool operator==(const std::string& a, const Config& b) { return b.name() == a.c_str(); }
  inline bool operator!=(const Config& a, const Config& b) { return !(a == b); }
  inline bool operator!=(const Config& a, config b) { return !(a == b); }
  inline bool operator!=(config a, const Config& b) { return !(a == b); }
  inline bool operator!=(const Config& a, const char* b) { return !(a == b); }
  inline bool operator!=(const char* a, const Config& b) { return !(a == b); }
  inline bool operator!=(const Config& a, const TString& b) { return !(a == b); }
  inline bool operator!=(const TString& a, const Config& b) { return !(a == b); }
  inline bool operator!=(const Config& a, const std::string& b) { return !(a == b); }
  inline bool operator!=(const std::string& a, const Config& b) { return !(a == b); }

  /// Prints the name.
  std::ostream& operator<<(std::ostream& os, const Config& c);
}  // namespace Utils

#endif
