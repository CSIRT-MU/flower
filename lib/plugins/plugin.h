#pragma once

enum PluginType { PacketProvider };

struct PluginInfo {
  const char* name;
  enum PluginType type;
};
