cmake_minimum_required(VERSION 3.12)

add_library(flower_plugins INTERFACE)
target_link_libraries(flower_plugins INTERFACE ${CMAKE_DL_LIBS})
target_include_directories(flower_plugins INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  
  $<INSTALL_INTERFACE:include/plugins>
  )
