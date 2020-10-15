#pragma once

#include <string>

#include <definition.hpp>

namespace Options {

static constexpr auto SYSTEM_PLUGINS_DIR = "/var/flower/plugins";

enum class Mode {
  PRINT_PLUGINS,
  PRINT_CONFIG,
  PRINT_HELP,
  PRINT_VERSION,
  PROCESS
};

// OPTIONS
extern Mode mode;
extern std::string argument;
extern std::string plugins_dir;
extern std::string input_plugin;
extern unsigned int active_timeout;
extern unsigned int idle_timeout;
extern std::string ip_address;
extern short port;
extern Flow::Definition definition;

// FUNCTIONS
void parse_args(int, char**);
void load_file(const std::string&);
void print_help(const char*);

} // namespace Options
