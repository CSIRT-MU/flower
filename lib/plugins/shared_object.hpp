#pragma once

#include <dlfcn.h>
#include <stdexcept>
#include <string>

// TODO(dudoslav): After destruction all function pointers should be invalidated

namespace Plugins {

constexpr auto PLUGIN_EXTENSION = ".so";

/**
 * RAII class representing linkable objects. The inner linkable object
 * will be closed on destruction. This might lead to error when function
 * pointers point to freed resource.
 */
class SharedObject {
  void* handle = nullptr;

public:

  /**
   * Creates shared object in invalid state,
   * used for move operations.
   */
  SharedObject() noexcept = default;

  /**
   * Creates shared object from file path.
   * @param file path to .so file.
   * @throw std::runtime_error if file does not exists.
   */
  explicit SharedObject(const char* file):
    handle(dlopen(file, RTLD_LAZY)) {
    if (handle == nullptr) {
      throw std::runtime_error{dlerror()};
    }
  }

  explicit SharedObject(const std::string& file):
    SharedObject(file.c_str()) {}

  // TODO(dudoslav): Find if virtual is necessary
  virtual ~SharedObject() {
    if (handle != nullptr) {
      dlclose(handle);
    }
  }

  // Copy
  SharedObject(const SharedObject&) = delete;
  SharedObject& operator=(const SharedObject&) = delete;

  // Move
  SharedObject(SharedObject&& o) noexcept : handle{o.handle} { o.handle = nullptr; }

  SharedObject& operator=(SharedObject&& o) noexcept {
    handle = o.handle;
    o.handle = nullptr;
    return *this;
  }

  /**
   * Loads given symbol as function pointer of proper type.
   * @param sym a symbol to load.
   * @return Function pointer of function described by given symbol.
   * @throw std::runtime_error if symbol was no found.
   */
  template<typename T>
  auto function(const char* sym) const {
    auto result = reinterpret_cast<T*>(dlsym(handle, sym)); // NOLINT

    auto* error = dlerror();
    if (error != nullptr) {
      throw std::runtime_error{error};
    }

    return result;
  }
};

} // namespace Plugins
