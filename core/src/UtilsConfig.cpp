/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: June 2014
 *
 **/

#include <UtilsConfig.h>

#include <cstdlib>
#include <iostream>
#include <iterator>

namespace {
  struct ConfigName {
    Utils::config id;
    const char* name;
  };

  // In enum order, so that kConfigNames[c].id == c.
  constexpr ConfigName kConfigNames[] = {
#define GAMMACOMBO_CONFIG_NAME(name) {Utils::name, #name},
      GAMMACOMBO_UTILS_CONFIGS(GAMMACOMBO_CONFIG_NAME)
#undef GAMMACOMBO_CONFIG_NAME
  };
  static_assert(std::size(kConfigNames) == Utils::string_config,
                "every predefined configuration needs a name, and string_config must be the last enum value");
}  // namespace

TString Utils::ConfigToTString(config c) {
  if (c == string_config) return "string_config";
  if (c >= 0 && c < string_config) return kConfigNames[c].name;
  std::cout << "Utils::ConfigToTString() : ERROR : Config not found: " << static_cast<int>(c) << std::endl;
  std::exit(1);
}

bool Utils::findConfig(const TString& name, config& c) {
  for (const auto& entry : kConfigNames) {
    if (name == entry.name) {
      c = entry.id;
      return true;
    }
  }
  return false;
}

Utils::config Utils::TStringToConfig(TString s) {
  config c;
  if (findConfig(s, c)) return c;
  std::cout << "Utils::TStringToConfig() : ERROR : Config not found: " << s << std::endl;
  std::exit(1);
}

Utils::Config::Config(config c) : fId(c), fName(ConfigToTString(c)) {}

Utils::Config::Config(const TString& name) : fId(string_config), fName(name) { findConfig(name, fId); }

Utils::Config::Config(const char* name) : Config(TString(name)) {}

Utils::Config::Config(const std::string& name) : Config(TString(name.c_str())) {}

TString Utils::ConfigToTString(const Config& c) { return c.name(); }

std::ostream& Utils::operator<<(std::ostream& os, const Config& c) { return os << c.name(); }
