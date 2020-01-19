#pragma once

#include <dlfcn.h>

class SharedObject {
  void* handle;
  public:
  SharedObject(const char* file):
    handle(dlopen(file, RTLD_LAZY)) {
    if (!handle)
      throw dlerror();
  }

  SharedObject(const std::string& file):
    SharedObject(file.c_str()) {}

  ~SharedObject() {
    if (handle)
      dlclose(handle);
  }

  template<typename T>
  auto function(const char* sym) const {
    auto result = reinterpret_cast<T*>(dlsym(handle, sym));
    auto error = dlerror();
    if (error != nullptr) throw error;
    return result;
  }

  template<typename T, typename... Args>
  auto factory(const char* sym) const {
    auto factory = function<T*(Args...)>(sym);
    return [factory](Args... args){ return std::unique_ptr<T>{factory(args...)}; };
  }
};
