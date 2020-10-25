#pragma once

#include <input.hpp>

namespace Plugins {

void load_plugins(const std::string&);
Input create_input(const std::string&, const char*);
void print_plugins();

} // namespace Plugins
