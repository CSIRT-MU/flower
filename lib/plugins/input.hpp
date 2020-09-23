#pragma once

#include <input.h>
#include <plugin.hpp>

namespace Plugins {

/**
 * Class providing input data to flow processing
 */
class Input {
  static constexpr auto INIT_FUNCTION = "init";
  static constexpr auto FINALIZE_FUNCTION = "finalize";
  static constexpr auto GET_PACKET_FUNCTION = "get_packet";

  using InitFun = InitRT(const char*);
  using FinalizeFun = FinalizeRT();
  using GetPacketFun = GetPacketRT();

  Plugin _plugin;

  InitFun* _init = nullptr;
  FinalizeFun* _finalize = nullptr;
  GetPacketFun* _get_packet = nullptr;

public:

  /**
   * Constructor of input class. It takes plugin and loads it
   * as input plugin. This class initializes plugin on construction.
   * @param plugin plugin to be used as input.
   * @param arg argument for plugin initialization.
   */
  Input(Plugin&& plugin, const char* arg):
    _plugin(std::move(plugin)),
    _init(_plugin.function<InitFun>(INIT_FUNCTION)),
    _finalize(_plugin.function<FinalizeFun>(FINALIZE_FUNCTION)),
    _get_packet(_plugin.function<GetPacketFun>(GET_PACKET_FUNCTION)) {
      auto result = _init(arg);
      if (result.type == ERROR) {
        throw std::runtime_error{result.error_msg};
      }
    }

  Input(const std::string& file, const char* arg):
    Input(Plugin{file}, arg) {}

  // Copy
  Input(const Input&) = delete;
  Input& operator=(const Input&) = delete;

  // Move
  Input(Input&& other) noexcept {
    *this = std::move(other);
  }

  Input& operator=(Input&& other) noexcept {
    std::swap(_plugin, other._plugin);
    std::swap(_init, other._init);
    std::swap(_finalize, other._finalize);
    std::swap(_get_packet, other._get_packet);

    return *this;
  }

  ~Input() noexcept {
    if (_finalize != nullptr) {
      _finalize();
    }
  }

  /**
   * Gets packet from plugin. This packet must be valid until next get_packet
   * is called and the data provided are freed by the plugin. This is not a
   * thread safe call.
   * @return Packet packet provided by plugin, if empty packet.data is equal to nullptr
   */
  GetPacketRT get_packet() {
    return _get_packet();
  }
};

} // namespace Plugins
