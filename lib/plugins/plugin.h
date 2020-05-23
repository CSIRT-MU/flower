#pragma once

/**
 * Structure used to define every plugin.
 */
struct PluginInfo {
  /**
   * Name should be unique for every plugin type.
   */
  const char* name;

  /**
   * Type is used as enum. Name together with type 
   * uniquely define each plugin.
   */
  int type;
};
