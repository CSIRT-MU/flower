#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <input.hpp>
#include <plugin.hpp>
#include <log.hpp>

namespace Plugins {

class Manager {
  std::unordered_map<std::string, Plugin> _inputs;

public:

  void load_from_folder(const std::string& path) {
    try {
      for (const auto& f: std::filesystem::directory_iterator(path)) {
        if (f.path().extension() == PLUGIN_EXTENSION) {
          try {
            auto plugin = Plugin{f.path()};
            switch (plugin.info().type) {
              case INPUT_PLUGIN:
                _inputs.emplace(plugin.info().name, std::move(plugin));
                break;
            }
          } catch (const std::runtime_error& e) {
            Log::error("Failed to load plugin: %s\n", e.what());
          }
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      Log::error("Plugins directory failure %s\n", e.what());
    }
  }

  // TODO(dudoslav): Moved plugin must be cleaned
  Input create_input(const std::string& name, const char* arg) {
    auto search = _inputs.find(name);

    if (search == _inputs.end()) {
      throw std::runtime_error{"Input plugin not found: " + name};
    }

    return {std::move(search->second), arg};
  }

  [[nodiscard]] const auto& inputs() const {
    return _inputs;
  }
};

} // namespace Plugins
