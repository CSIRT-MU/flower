#pragma once

#include <plugin.h>
#include <shared_object.hpp>

namespace Plugins {

/**
 * Class that represents plugin. All plugins must provide info
 * about themselves. This class inherits from shared object.
 */
class Plugin: public SharedObject {
  static constexpr auto INFO_FUNCTION = "info";

  PluginInfo _info = {};

  public:

  /**
   * Creates plugin in invalid state,
   * used for move operations.
   */
  Plugin() noexcept = default;

  /**
   * Constructor that takes file path and construct plugin object.
   * @param file file path.
   * @see SharedObject::SharedObject()
   */
  explicit Plugin(const std::string& file):
    SharedObject(file) {
      auto get_info = function<PluginInfo()>(INFO_FUNCTION);
      _info = get_info();
  }
  
  [[nodiscard]] PluginInfo info() const {
    return _info;
  }
};

} // namespace Plugins
