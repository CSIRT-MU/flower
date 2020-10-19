#include <options.hpp>

#include <iostream>

#include <clipp.h>
#include <toml.hpp>

#include <log.hpp>

namespace Options {

// OPTIONS
Mode mode = Mode::PRINT_HELP;
std::string argument = "";
std::string plugins_dir = "./plugins";
std::string input_plugin = "FileInput";
unsigned int active_timeout = 600;
unsigned int idle_timeout = 300;
std::string ip_address = "127.0.0.1";
short port = 20'000;

using namespace clipp;

static toml::value toml_file;

static auto mode_process = "Process options:"
  % (
      command("process").set(mode, Mode::PROCESS),
      value("plugin_argument", argument)
      % "Argument plugin depends on plugin implementation",
      (option("-I", "--input_plugin") & value("plugin_name", input_plugin))
      % "Input plugin name to use [default: FileInput]",
      (option("-a", "--active_timeout") & value("seconds", active_timeout))
      % "Active timeout, after which the flow should be exported",
      (option("-i", "--idle_timeout") & value("seconds", idle_timeout))
      % "Idle timeout, after which the flow should be exported",
      (option("-o", "--ip_address") & value("address", ip_address))
      % "IP address of IPFIX collector",
      (option("-p", "--port") & value("port", port))
      % "TCP port of IPFIX collector"
    );

static auto mode_print_plugins = "Prints all available plugins"
  % (command("plugins").set(mode, Mode::PRINT_PLUGINS));

static auto mode_print_config = "Prints current program configuration"
  % (command("config").set(mode, Mode::PRINT_CONFIG));

static auto logging_flags = "Logging options:"
  % (
      option("--debug")([](){ Log::set_level(Log::Level::DEBUG); })
      % "Set log level to debug",
      option("--info")([](){ Log::set_level(Log::Level::INFO); })
      % "Set log level to info",
      option("--warn")([](){ Log::set_level(Log::Level::WARN); })
      % "Set log level to info",
      option("--error")([](){ Log::set_level(Log::Level::ERROR); })
      % "Set log level to info"
    );

static auto plugins_flags = (
    option("--plugins_dir") & value("directory", plugins_dir)
    % "Set plugins directory"
    );

static auto cli = (
    logging_flags,
    plugins_flags,
    mode_process
    | mode_print_plugins
    | mode_print_config
    | (command("-h", "--help") >> set(mode, Mode::PRINT_HELP))
    % "Print this help"
    | (command("-v", "--version") >> set(mode, Mode::PRINT_VERSION))
    % "Prints version"
    );

void parse_args(int argc, char** argv) {
  if (!parse(argc, argv, cli)) {
    std::printf("Invalid input!\n");
    mode = Mode::PRINT_HELP;
  }
}

void print_help(const char* app_name) {
  std::cout << make_man_page(cli, app_name) << '\n';
}

const toml::value& get_toml() {
  return toml_file;
}

void load_file(const std::string& path) {
  toml_file = toml::parse(path);

  // Global values
  auto global = toml::find(toml_file, "global");
  active_timeout = toml::find_or(global, "active_timeout", active_timeout);
  idle_timeout = toml::find_or(global, "idle_timeout", idle_timeout);
  ip_address = toml::find_or(global, "ip_address", ip_address);
  port = toml::find_or(global, "port", port);
}

}

