include(FetchContent)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
)

FetchContent_GetProperties(toml11)
if(NOT toml11_POPULATED)
  FetchContent_Populate(toml11)
  add_subdirectory(${toml11_SOURCE_DIR} ${toml11_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  clipp
  GIT_REPOSITORY https://github.com/muellan/clipp.git
)

FetchContent_GetProperties(clipp)
if(NOT clipp_POPULATED)
  FetchContent_Populate(clipp)
  add_subdirectory(${clipp_SOURCE_DIR} ${clipp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
