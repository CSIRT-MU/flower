cmake_minimum_required(VERSION 3.12)

add_library(file_provider MODULE file_provider.c)
target_include_directories(file_provider PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_library(interface_provider MODULE interface_provider.c)
target_include_directories(interface_provider PRIVATE ${PROJECT_SOURCE_DIR}/include)
