cmake_minimum_required(VERSION 3.14)
enable_testing()

project(flower VERSION 1.0)

option(ENABLE_TESTS "Enable tests" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

# Download dependencies
include(cmake/Dependencies.cmake)

# Find system libraries
find_package(Threads REQUIRED)
find_package(libtins REQUIRED)

configure_file(config.h.in config.h)

file(GLOB SOURCES src/*.cpp)

add_executable(flower main.cpp ${SOURCES})
target_link_libraries(flower PRIVATE
  Threads::Threads
  tins
  clipp::clipp
  toml11::toml11
  ${CMAKE_DL_LIBS})
target_include_directories(flower PUBLIC
  $<BUILD_INTERFACE:${flower_SOURCE_DIR}/include>  
  $<INSTALL_INTERFACE:include>
)
target_include_directories(flower PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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

install(TARGETS flower EXPORT flowerTargets DESTINATION bin)
install(FILES include/plugin.h DESTINATION include)
install(FILES include/input.h DESTINATION include)

install(EXPORT flowerTargets
  FILE flowerTargets.cmake
  NAMESPACE flower::
  DESTINATION lib/cmake/flower
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("flowerConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES "cmake/flowerConfig.cmake" "${CMAKE_BINARY_DIR}/flowerConfigVersion.cmake"
  DESTINATION lib/cmake/flower
)
