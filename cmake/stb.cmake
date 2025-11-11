FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb
  GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

add_library(stbimage INTERFACE)
target_include_directories(stbimage INTERFACE ${stb_SOURCE_DIR}/)
