#pragma once

#include <fstream>
#include <string>
#include <tuple>
#include <vector>

constexpr auto USAGE_STRING = R"(
Usage: ./flower argument [options | actions] ...

Argument is used by the input plugin.

Options:
  -I, --input_plugin
  --ip_address
  --port
  --export_interval

All options can be also set in configuration file `~/.flower.conf`.

Actions:
  --list_plugins
  --print_config
)";

struct Options {
  enum class Activity { LIST_PLUGINS, SHOW_USAGE, MAIN_ACTIVITY, PRINT_CONFIG };

private:

  using Option = std::tuple<const char*, const char*>;

  static constexpr auto CONFIG_SEPARATOR = '=';
  static constexpr auto CONFIG_COMMENT = '#';

  // Options can be set both in command line arguments and in config file
  static constexpr auto INPUT_PLUGIN_OPTION = "input_plugin";
  static constexpr auto INPUT_PLUGIN_FLAG = "I";
  static constexpr auto OUTPUT_IP_ADDRESS_OPTION = "ip_address";
  static constexpr auto OUTPUT_PORT_OPTION = "port";

  static constexpr auto EXPORT_INTERVAL_OPTION = "export_interval";

  // Actions can only be set in command line arguments
  static constexpr auto LIST_PLUGINS_ACTION = "list_plugins";
  static constexpr auto PRINT_CONFIG_ACTION = "print_config";

  Activity _activity = Activity::SHOW_USAGE;
  std::string _argument = "";

  std::string _input_plugin = "FileInput";
  std::string _export_interval = "120";
  std::string _output_ip_address = "127.0.0.1";
  std::string _output_port = "20000";

  Options() = default;

public:

  static Options& instance() {
    static auto options = Options{};
    return options;
  }

  [[nodiscard]] Activity activity() const {
    return _activity;
  }

  [[nodiscard]] const std::string& argument() const {
    return _argument;
  }

  [[nodiscard]] const std::string& input_plugin() const {
    return _input_plugin;
  }

  [[nodiscard]] const std::string& export_interval() const {
    return _export_interval;
  }

  [[nodiscard]] const std::string& output_ip_address() const {
    return _output_ip_address;
  }

  [[nodiscard]] uint16_t output_port() const {
    return std::stoi(_output_port);
  }

  void parse(int argc, char** argv) {
    auto args = std::vector<std::string>{argv + 1, argv + argc}; // NOLINT: Pointer arithmetics are fine here

    // TODO(dudoslav): Implement

    for (auto arg = args.begin(); arg != args.end(); ++arg) {
      if (arg->starts_with("--")) {
        auto option = arg->substr(2, std::string::npos);
        if (option == INPUT_PLUGIN_OPTION) {
          _input_plugin = *(++arg);
        } else if (option == EXPORT_INTERVAL_OPTION) {
          _export_interval = *(++arg);
        } else if (option == OUTPUT_IP_ADDRESS_OPTION) {
          _output_ip_address = *(++arg);
        } else if (option == OUTPUT_PORT_OPTION) {
          _output_port = *(++arg);
        } else if (option == LIST_PLUGINS_ACTION) {
          _activity = Activity::LIST_PLUGINS;
        } else if (option == PRINT_CONFIG_ACTION) {
          _activity = Activity::PRINT_CONFIG;
        }
      } else if (arg->starts_with("-")) {
        auto option = arg->substr(1, std::string::npos);
        if (option == INPUT_PLUGIN_FLAG) {
          _input_plugin = *(++arg);
        }
      } else {
        _argument = *arg;
        _activity = Activity::MAIN_ACTIVITY;
      }
    }
  }

  void load(const std::string& path) {
    auto file = std::ifstream{path};

    for (auto line = std::string{}; std::getline(file, line);) {
      if (line.empty()) {
        continue;
      }

      if (line.starts_with(CONFIG_COMMENT)) {
        continue;
      }

      auto eq_pos = line.find_first_of(CONFIG_SEPARATOR);
      // TODO(dudoslav): Trim
      auto key = line.substr(0, eq_pos);
      auto value = line.substr(eq_pos + 1, std::string::npos);

      if (key == INPUT_PLUGIN_OPTION) {
        _input_plugin = value;
      } else if (key == EXPORT_INTERVAL_OPTION) {
        _export_interval = value;
      } else if (key == OUTPUT_IP_ADDRESS_OPTION) {
        _output_ip_address = value;
      } else if (key == OUTPUT_PORT_OPTION) {
        _output_port = value;
      }
    }
  }
};
