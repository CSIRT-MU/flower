#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <input.hpp>
#include <plugin.hpp>

namespace Plugins {

class Manager {
  std::vector<Plugin> _inputs;

public:

  explicit Manager(const std::string& path) {
    if (!std::filesystem::is_directory(path)) {
      return;
    }

    for (const auto& f: std::filesystem::directory_iterator(path)) {
      if (f.path().extension() == PLUGIN_EXTENSION) {
        auto plugin = Plugin{f.path()};
        switch (plugin.info().type) {
          case INPUT_PLUGIN:
            _inputs.emplace_back(std::move(plugin));
            break;
        }
      }
    }
  }

  // TODO(dudoslav): Moved plugin must be cleaned
  Input create_input(const std::string& name, const char* arg) {
    auto search = std::find_if(_inputs.begin(), _inputs.end(),
        [&name](const auto& plugin){
        return plugin.info().name == name;
        });

    if (search == _inputs.end()) {
      throw std::runtime_error{"Input plugin not found: " + name};
    }

    return {std::move(*search), arg};
  }

  const auto& inputs() {
    return _inputs;
  }
};

} // namespace Plugins
