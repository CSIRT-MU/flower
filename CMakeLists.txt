cmake_minimum_required(VERSION 3.12)

project(Flower)

set(CMAKE_CXX_CLANG_TIDY
  clang-tidy
  -header-filter=.
  -checks=*,-modernize-use-trailing-return-type,-llvm-header-guard)

find_package(libtins)
add_subdirectory(lib)

add_executable(flower main.cpp)
target_link_libraries(flower PRIVATE tins flower_plugins)
target_include_directories(flower PRIVATE include)
target_compile_features(flower PRIVATE cxx_std_17)
target_compile_options(flower PRIVATE -Wall -Wextra -pedantic)

add_subdirectory(plugins)

# add_subdirectory(test) # TODO(dudoslav): Implement tests

find_package(Doxygen)
if (DOXYGEN_FOUND)
  doxygen_add_docs(doxygen ${PROJECT_SOURCE_DIR})
endif (DOXYGEN_FOUND)

