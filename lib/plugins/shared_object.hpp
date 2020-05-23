#pragma once

#include <dlfcn.h>
#include <string>

// TODO: Handle after move operations

class SharedObject {
  void* handle;
  public:
  SharedObject(const char* file):
    handle{dlopen(file, RTLD_LAZY)} {
    if (!handle)
      throw std::string{dlerror()};
  }

  SharedObject(const std::string& file):
    SharedObject{file.c_str()} {}

  ~SharedObject() {
    if (handle)
      dlclose(handle);
  }

  SharedObject(const SharedObject&) = delete;
  SharedObject& operator=(const SharedObject&) = delete;

  SharedObject(SharedObject&& o): handle{o.handle} {
    o.handle = 0;
  }

  SharedObject& operator=(SharedObject&& o) {
    handle = o.handle;
    o.handle = 0;
    return *this;
  }

  template<typename T>
  auto function(const char* sym) const {
    auto result = reinterpret_cast<T*>(dlsym(handle, sym));
    auto error = dlerror();
    if (error != nullptr) throw std::string{error};
    return result;
  }
};
