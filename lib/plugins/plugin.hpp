#pragma once

#include <plugin.h>
#include <shared_object.hpp>

namespace Plugins {

static constexpr auto INFO_FUNCTION = "info";

/**
 * Class that represents plugin. All plugins must provide info
 * about themselves. This class inherits from shared object.
 */
class Plugin: public SharedObject {
  PluginInfo _info = {};

  public:

  /**
   * Constructor that takes file path and construct plugin object.
   * @param file file path.
   * @see SharedObject::SharedObject()
   */
  template<typename T>
  explicit Plugin(T&& file):
    SharedObject(std::forward<T>(file)) {
      auto get_info = function<PluginInfo()>(INFO_FUNCTION);
      _info = get_info();
  }
  
  [[nodiscard]] PluginInfo info() const {
    return _info;
  }
};

} // namespace Plugins
