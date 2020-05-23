cmake_minimum_required(VERSION 3.12)

project(Flower)

find_package(libtins)
add_subdirectory(lib)

add_executable(flower main.cpp)
target_link_libraries(flower PRIVATE tins flower_plugins)
target_include_directories(flower PRIVATE include)
target_compile_features(flower PRIVATE cxx_std_17)

add_subdirectory(plugins)

# add_subdirectory(test)
