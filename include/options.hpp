#pragma once

#include <string>

#include <toml.hpp>

namespace Options {

constexpr static auto CONFIG_NAME = "flower.conf";
static constexpr auto SYSTEM_PLUGINS_DIR = "/var/flower/plugins";

enum class Mode {
  PRINT_PLUGINS,
  PRINT_HELP,
  PRINT_VERSION,
  PROCESS
};

/**
 * Struct holding all options that can be loaded both from file
 * and from command line.
 */
struct Options {
  Mode mode;
  std::string argument;
  std::string plugins_dir;
  std::string input_plugin;
  std::uint32_t active_timeout;
  std::uint32_t idle_timeout;
  std::string ip_address;
  std::uint16_t port;
};

/* Modifiers */
void merge_args(int, char**);
void merge_file(const std::string&);

/* Helpers */
void print_help(const char*);

/* Getters */
const toml::value& config();
const Options& options();

} // namespace Options
