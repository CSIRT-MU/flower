#include <options.hpp>

#include <iostream>

#include <clipp.h>
#include <toml.hpp>

#include <log.hpp>

namespace Options {

// OPTIONS
Mode mode = Mode::PRINT_HELP;
std::string argument = "";
std::string input_plugin = "FileInput";
unsigned int export_interval = 4;
unsigned int active_timeout = 600;
unsigned int idle_timeout = 300;
std::string ip_address = "127.0.0.1";
short port = 20'000;
Flow::Definition definition = {};

using namespace clipp;

static auto mode_process = "Process options:"
  % (
      command("process").set(mode, Mode::PROCESS),
      value("plugin_argument", argument)
      % "Argument plugin depends on plugin implementation",
      (option("-I", "--input_plugin") & value("plugin_name", input_plugin))
      % "Input plugin name to use [default: FileInput]",
      (option("-e", "--export_interval") & value("seconds", export_interval))
      % "CHANGEME",
      (option("-a", "--active_timeout") & value("seconds", active_timeout))
      % "CHANGEME",
      (option("-i", "--idle_timeout") & value("seconds", idle_timeout))
      % "CHANGEME",
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
      option("-d", "--debug")([](){ Log::set_level(Log::Level::DEBUG); })
      % "Set log level to debug",
      option("-i", "--info")([](){ Log::set_level(Log::Level::INFO); })
      % "Set log level to info",
      option("-w", "--warn")([](){ Log::set_level(Log::Level::WARN); })
      % "Set log level to info",
      option("-e", "--error")([](){ Log::set_level(Log::Level::ERROR); })
      % "Set log level to info"
    );

static auto cli = (
    mode_process
    | mode_print_plugins
    | mode_print_config
    | (command("-h", "--help") >> set(mode, Mode::PRINT_HELP))
    % "Print this help"
    | (command("-v", "--version") >> set(mode, Mode::PRINT_VERSION))
    % "Prints version"
    , logging_flags
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

void load_file(const std::string& path) {
  auto file = toml::parse(path);

  // Global values
  auto global = toml::find(file, "global");
  export_interval = toml::find_or(global, "export_interval", export_interval);
  ip_address = toml::find_or(global, "ip_address", ip_address);
  port = toml::find_or(global, "port", port);

  // IP values
  if (file.contains("ip")) {
    auto ip = toml::find(file, "ip");
    auto& ip_def = definition.ip;
    ip_def.process = true;
    ip_def.src = toml::find_or(ip, "src", ip_def.src);
    ip_def.dst = toml::find_or(ip, "dst", ip_def.dst);
  } else {
    definition.ip.process = false;
  }

  // IPv6 values
  if (file.contains("ipv6")) {
    auto ipv6 = toml::find(file, "ipv6");
    auto& ipv6_def = definition.ipv6;
    ipv6_def.process = true;
    ipv6_def.src = toml::find_or(ipv6, "src", ipv6_def.src);
    ipv6_def.dst = toml::find_or(ipv6, "dst", ipv6_def.dst);
  } else {
    definition.ipv6.process = false;
  }

  // TCP values
  if (file.contains("tcp")) {
    auto tcp = toml::find(file, "tcp");
    auto& tcp_def = definition.tcp;
    tcp_def.process = true;
    tcp_def.src = toml::find_or(tcp, "src", tcp_def.src);
    tcp_def.dst = toml::find_or(tcp, "dst", tcp_def.dst);
  } else {
    definition.tcp.process = false;
  }

  // UDP values
  if (file.contains("udp")) {
    auto udp = toml::find(file, "udp");
    auto& udp_def = definition.udp;
    udp_def.process = true;
    udp_def.src = toml::find_or(udp, "src", udp_def.src);
    udp_def.dst = toml::find_or(udp, "dst", udp_def.dst);
  } else {
    definition.udp.process = false;
  }

  // VLAN values
  if (file.contains("vlan")) {
    auto vlan = toml::find(file, "vlan");
    auto& vlan_def = definition.dot1q;
    vlan_def.process = true;
    vlan_def.id = toml::find_or(vlan, "id", vlan_def.id);
  } else {
    definition.dot1q.process = false;
  }
}

}

