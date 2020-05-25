cmake_minimum_required(VERSION 3.12)

add_test(NAME Runs COMMAND flower)

add_test(NAME Usage COMMAND flower)
set_tests_properties(Usage
  PROPERTIES PASS_REGULAR_EXPRESSION "./flower .*"
  )

add_test(NAME Plugins
  COMMAND flower --list-plugins
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
set_tests_properties(Plugins
  PROPERTIES PASS_REGULAR_EXPRESSION ".*InterfaceInput.*FileInput.*"
  )

find_package(GTest REQUIRED)

unset(CMAKE_CXX_CLANG_TIDY)

add_executable(unit_tests unit_tests.cpp)
target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(unit_tests GTest::GTest GTest::Main)
target_compile_features(unit_tests PRIVATE cxx_std_20)
gtest_add_tests(TARGET unit_tests AUTO)
