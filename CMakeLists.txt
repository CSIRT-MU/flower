cmake_minimum_required(VERSION 3.14)
enable_testing()

project(flower)

option(ENABLE_TESTS "Enable tests" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

# Download dependencies
include(cmake/Dependencies.cmake)

# Find system libraries
find_package(Threads REQUIRED)
find_package(libtins REQUIRED)

add_subdirectory(lib)

file(GLOB SOURCES src/*.cpp)

add_executable(flower main.cpp ${SOURCES})
target_link_libraries(flower PRIVATE
  Threads::Threads
  tins
  clipp::clipp
  toml11::toml11
  flower_plugins)
target_include_directories(flower PRIVATE include)
target_compile_features(flower PRIVATE cxx_std_17)
target_compile_options(flower PRIVATE -Wall -Wextra -pedantic)

add_subdirectory(plugins)

# if(ENABLE_TESTS)
#   add_subdirectory(test) 
# endif()

if(ENABLE_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
  doxygen_add_docs(doxygen ${PROJECT_SOURCE_DIR})
endif (DOXYGEN_FOUND)

