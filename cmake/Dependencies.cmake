include(FetchContent)

# Set this so that we can later disable tests
cmake_policy(SET CMP0077 NEW)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        v3.5.0
)

set(toml11_BUILD_TEST OFF)
FetchContent_MakeAvailable(toml11)

FetchContent_Declare(
  clipp
  GIT_REPOSITORY https://github.com/muellan/clipp.git
)

FetchContent_MakeAvailable(clipp)
