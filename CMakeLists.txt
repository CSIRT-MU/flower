cmake_minimum_required(VERSION 3.12)

project(Flower)

set(CMAKE_CXX_CLANG_TIDY clang-tidy)

find_package(Threads)
find_package(libtins)
add_subdirectory(lib)

add_executable(flower main.cpp)
target_link_libraries(flower PRIVATE Threads::Threads tins flower_plugins)
target_include_directories(flower PRIVATE include)
target_compile_features(flower PRIVATE cxx_std_20)
target_compile_options(flower PRIVATE -Wall -Wextra -pedantic)

add_subdirectory(plugins)

enable_testing()
add_subdirectory(test) 

find_package(Doxygen)
if (DOXYGEN_FOUND)
  doxygen_add_docs(doxygen ${PROJECT_SOURCE_DIR})
endif (DOXYGEN_FOUND)

