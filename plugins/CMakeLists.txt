cmake_minimum_required(VERSION 3.12)

add_library(file_provider MODULE file_provider.c)
target_link_libraries(file_provider PRIVATE flower_plugins pcap)

add_library(interface_provider MODULE interface_provider.c)
target_link_libraries(interface_provider PRIVATE flower_plugins pcap)
