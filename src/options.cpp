#include <options.hpp>

#include <iostream>
#include <filesystem>

#include <clipp.h>

#include <log.hpp>

namespace Options {

static auto app_options = Options{
  Mode::PRINT_HELP,
  "",
  "./plugins",
  "FileInput",
  600,
  300,
  "127.0.0.1",
  20'000
};

static auto config_file = toml::value{};

/* Define CLI */

using namespace clipp;

static auto mode_process = "Process options:"
  % (
      command("process").set(app_options.mode, Mode::PROCESS),
      value("plugin_argument", app_options.argument)
      % "Argument plugin depends on plugin implementation",

      (option("-I", "--input_plugin")
      & value("plugin_name", app_options.input_plugin))
      % "Input plugin name to use [default: FileInput]",

      (option("-a", "--active_timeout")
      & value("seconds", app_options.active_timeout))
      % "Active timeout, after which the flow should be exported",

      (option("-i", "--idle_timeout")
      & value("seconds", app_options.idle_timeout))
      % "Idle timeout, after which the flow should be exported",

      (option("-o", "--ip_address")
      & value("address", app_options.ip_address))
      % "IP address of IPFIX collector",

      (option("-p", "--port")
      & value("port", app_options.port))
      % "TCP port of IPFIX collector"
    );

static auto mode_print_plugins = "Prints all available plugins"
  % (command("plugins").set(app_options.mode, Mode::PRINT_PLUGINS));

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
    (option("--plugins_dir")
    & value("directory", app_options.plugins_dir))
    % "Set plugins directory"
    );

static auto cli = (
    logging_flags,
    plugins_flags,
    mode_process
    | mode_print_plugins
    | (command("-h", "--help") >> set(app_options.mode, Mode::PRINT_HELP))
    % "Print this help"
    | (command("-v", "--version") >> set(app_options.mode, Mode::PRINT_VERSION))
    % "Prints version"
    );

/* Functions */

void
merge_args(int argc, char** argv)
{
  if (!parse(argc, argv, cli)) {
    std::printf("Invalid input!\n");
    app_options.mode = Mode::PRINT_HELP;
  }
}

void
merge_file(const std::string& path)
{
  using namespace std::filesystem;

  if (!is_regular_file(path))
    return;

  config_file = toml::parse(path);

  /* Global options */
  app_options.active_timeout = toml::find_or(config_file, "active_timeout",
      app_options.active_timeout);
  app_options.idle_timeout = toml::find_or(config_file, "idle_timeout",
      app_options.idle_timeout);
  app_options.ip_address = toml::find_or(config_file, "ip_address",
      app_options.ip_address);
  app_options.port = toml::find_or(config_file, "port",
      app_options.port);
  app_options.plugins_dir = toml::find_or(config_file, "plugins_dir",
      app_options.plugins_dir);
}

void
print_help(const char* app_name)
{
  std::cout << make_man_page(cli, app_name) << '\n';
}

const toml::value&
config()
{
  return config_file;
}

const Options&
options()
{
  return app_options;
}

} // namespace Options
