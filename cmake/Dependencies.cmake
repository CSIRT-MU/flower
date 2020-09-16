include(FetchContent)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
)

FetchContent_MakeAvailable(toml11)

FetchContent_Declare(
  clipp
  GIT_REPOSITORY https://github.com/muellan/clipp.git
)

FetchContent_MakeAvailable(clipp)
