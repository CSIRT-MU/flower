#pragma once

#include <string>
#include <vector>

namespace Arguments {

static constexpr auto INPUT_PLUGIN_FLAG = "-I";

static constexpr auto LIST_PLUGINS_LONG_FLAG = "--list-plugins";
static constexpr auto INPUT_PLUGIN_LONG_FLAG = "--input-plugin";

static constexpr auto DEFAULT_INPUT = "FileInput";

enum Activity { LIST_PLUGINS, SHOW_USAGE, MAIN_ACTIVITY };

struct Options {
  Activity activity = Activity::SHOW_USAGE;
  std::string input_name = DEFAULT_INPUT;
  std::string input_argument = "";
};

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

  void list_plugins(Argument& argument) {
    if (argument + 1 == _arguments.cend()) {
      throw std::runtime_error{"Input plugin requires 1 arguments"};
    }

    _options.activity = Activity::MAIN_ACTIVITY;
    _options.input_name = *(++argument);
  }

public:

  Parser(int argc, char** argv):
    _arguments{argv + 1, argv + argc} { // NOLINT
      for (auto arg = _arguments.cbegin(); arg != _arguments.cend(); ++arg) {
        if (arg->starts_with("--")) {
          if (*arg == LIST_PLUGINS_LONG_FLAG) {
            _options.activity = Activity::LIST_PLUGINS;
          } else if (*arg == INPUT_PLUGIN_LONG_FLAG) {
            list_plugins(arg);
          }
        } else if (arg->starts_with("-")) {
          if (*arg == INPUT_PLUGIN_FLAG) {
            list_plugins(arg);
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
 * must be in the same format as arguments of main() function.
 * @param argc number of arguments
 * @param argv array of arguments
 * @return Options structure filled with parsed arguments
 */
inline Options parse(int argc, char** argv) {
  return Parser(argc, argv).options();
}

} // namespace Arguments
