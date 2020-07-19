#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <input.hpp>
#include <plugin.hpp>

namespace Plugins {

class Manager {
  std::vector<Plugin> _inputs;

public:

  explicit Manager(const std::string& path) {
    try {
      for (const auto& f: std::filesystem::directory_iterator(path)) {
        if (f.path().extension() == PLUGIN_EXTENSION) {
          try {
            auto plugin = Plugin{f.path()};
            switch (plugin.info().type) {
              case INPUT_PLUGIN:
                _inputs.emplace_back(std::move(plugin));
                break;
            }
          } catch (const std::runtime_error& e) {
            std::cerr << "Failed to load plugin: " << e.what() << std::endl;
          }
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Failed to traverse plugin directory: " << e.what() << std::endl;
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

  [[nodiscard]] const auto& inputs() const {
    return _inputs;
  }
};

} // namespace Plugins
