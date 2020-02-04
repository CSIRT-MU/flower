#pragma once

#include "shared_object.hpp"

enum class PluginType { PacketProvider };

struct PluginInfo {
  const char* name;
  PluginType type;
};

class Plugin: public SharedObject {
  PluginInfo info_;
  public:
  Plugin(const char* file):
    SharedObject{file} {
      auto get_info = function<PluginInfo()>("info");
      info_ = get_info();
  }
  
  PluginInfo info() {
    return info_;
  }
};
