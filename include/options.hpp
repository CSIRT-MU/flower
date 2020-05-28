#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Options {

static constexpr auto CONFIG_SEPARATOR = '=';
static constexpr auto CONFIG_COMMENT = '#';

static constexpr auto INPUT_PLUGIN_FLAG = "-I";

static constexpr auto LIST_PLUGINS_LONG_FLAG = "--list-plugins";
static constexpr auto INPUT_PLUGIN_LONG_FLAG = "--input-plugin";
static constexpr auto EXPORT_INTERVAL_LONG_FLAG = "--export-interval";

static constexpr auto DEFAULT_INPUT = "FileInput";
static constexpr auto DEFAULT_EXPORT_INTERVAL = std::chrono::seconds{10};

enum Activity { LIST_PLUGINS, SHOW_USAGE, MAIN_ACTIVITY };

struct Options {
  Activity activity = Activity::SHOW_USAGE;
  std::string input_name = DEFAULT_INPUT;
  std::string input_argument = "";
  std::chrono::seconds export_interval = DEFAULT_EXPORT_INTERVAL;
};

static auto global_options = Options{};

inline const Options& instance() {
  return global_options;
}

/**
 * Simple parser class designed to parse command line arguments.
 * It is solely used for dividing parsing code into multiple
 * methods.
 */
class Parser {
  using Arguments = std::vector<std::string>;
  using Argument = Arguments::const_iterator;

  Arguments _arguments;
  Options _options;

  void input_plugin(Argument& argument) {
    if (argument + 1 == _arguments.cend()) {
      throw std::runtime_error{"Input plugin requires 1 argument"};
    }

    _options.input_name = *(++argument);
  }

  void export_interval(Argument& argument) {
    if (argument + 1 == _arguments.cend()) {
      throw std::runtime_error{"Export interval requires 1 argument"};
    }

    // TODO(dudoslav): Try-catch std::stoi exception
    _options.export_interval = std::chrono::seconds{std::stoi(*(++argument))};
  }

public:

  Parser(int argc, char** argv):
    _arguments{argv + 1, argv + argc} { // NOLINT
      for (auto arg = _arguments.cbegin(); arg != _arguments.cend(); ++arg) {
        if (arg->starts_with("--")) {
          if (*arg == LIST_PLUGINS_LONG_FLAG) {
            _options.activity = Activity::LIST_PLUGINS;
          } else if (*arg == INPUT_PLUGIN_LONG_FLAG) {
            input_plugin(arg);
          } else if (*arg == EXPORT_INTERVAL_LONG_FLAG) {
            export_interval(arg);
          }
        } else if (arg->starts_with("-")) {
          if (*arg == INPUT_PLUGIN_FLAG) {
            input_plugin(arg);
          }
        } else {
          _options.activity = Activity::MAIN_ACTIVITY;
          _options.input_argument = *arg;
        }
      }
    }

  [[nodiscard]] Options options() const {
    return _options;
  }
};

/**
 * Function that parses command line arguments. The supplied arguments
 * must be in the same format as arguments of main() function. The result
 * is saved in the global singleton.
 * @see Options::instance()
 * @param argc number of arguments
 * @param argv array of arguments
 */
inline void parse(int argc, char** argv) {
  global_options = Parser(argc, argv).options();
}

// TODO(dudoslav): Implement loading of config file
inline void load(const std::string& path) {
  try {
    // TODO(dudoslav): Throw exception if not opened
    auto file = std::ifstream{path};

    for (std::string line; std::getline(file, line);) {
      if (!line.empty() && line[0] == CONFIG_COMMENT) {
        continue;
      }

      auto eq = line.find_first_of(CONFIG_SEPARATOR);
      if (eq == std::string::npos) {
        continue;
      }

      // TODO(dudoslav): Trim
      auto key = line.substr(0, eq);
      auto value = line.substr(eq + 1, std::string::npos);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

} // namespace Options
