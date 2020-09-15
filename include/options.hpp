#pragma once

#include <iostream>

#include <clipp.h>
#include <toml.hpp>

namespace Options {
  namespace Definitions {
    struct IP {
      bool src{false};
      bool dst{false};
    } ip;
  } // namespace Definitions

  using namespace clipp;

  enum class Mode {
    PRINT_PLUGINS,
    PRINT_CONFIG,
    PRINT_HELP,
    PRINT_VERSION,
    PROCESS
  };

  auto mode = Mode::PRINT_HELP;
  auto argument = std::string{};

  auto input_plugin = std::string{"FileInput"};
  auto export_interval = 120u;
  auto output_ip_address = std::string{};
  auto output_port = 20'000u;

  static auto mode_process = (
      command("process") >> set(mode, Mode::PROCESS),
      value("arg", argument),
      option("-I", "--input_plugin") & value("name") >> input_plugin,
      option("-e", "--export_interval") & value("seconds") >> export_interval,
      option("-o", "--ip_address") & value("address") >> output_ip_address,
      option("-p", "--port") & value("port") >> output_port
      );

  static auto mode_print_plugins = (
      command("plugins") >> set(mode, Mode::PRINT_PLUGINS)
      );

  static auto mode_print_config = (
      command("config") >> set(mode, Mode::PRINT_CONFIG)
      );

  static auto cli = (
      mode_process
      | mode_print_plugins
      | mode_print_config
      | (command("-h", "--help") >> set(mode, Mode::PRINT_HELP))
      | (command("-v", "--version") >> set(mode, Mode::PRINT_VERSION))
      );

  void parse(int argc, char** argv) {
    if (!clipp::parse(argc, argv, cli)) {
      std::cout << "FAILED TO PARSE ARGUMENTS!\n";
      std::cout << usage_lines(cli, argv[0]) << '\n';
      return;
    }

    if (mode == Mode::PRINT_HELP) {
      std::cout << make_man_page(cli, argv[0]);
    } else if (mode == Mode::PRINT_VERSION) {
      std::printf("version 1.0\n");
    }
  }

  void load_file(const std::string& path) {
    auto data = toml::parse(path);

    input_plugin = toml::find_or(data, "input_plugin", input_plugin);
    export_interval = toml::find_or(data, "export_interval", export_interval);
    output_ip_address = toml::find_or(data, "ip_address", output_ip_address);
    output_port = toml::find_or(data, "port", output_port);

    auto ip = toml::find_or(data, "ip", {});
    Definitions::ip.src = toml::find_or(ip, "src", Definitions::ip.src);
    Definitions::ip.dst = toml::find_or(ip, "dst", Definitions::ip.dst);
  }
} // namespace Options
