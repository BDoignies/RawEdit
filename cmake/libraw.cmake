# Fetch LibRaw source
FetchContent_Declare(
  LibRaw
  GIT_REPOSITORY https://github.com/LibRaw/LibRaw.git
  GIT_TAG        master
)
FetchContent_Populate(LibRaw)

# TODO: Find a way better way to compile libraw 
# But at least this one is somewhat cross-platform

# Create static lib from LibRaw source
file(GLOB_RECURSE LIBRAW_SRC
  ${libraw_SOURCE_DIR}/src/decoders/*.cpp
  ${libraw_SOURCE_DIR}/src/decompressors/*.cpp
  ${libraw_SOURCE_DIR}/src/metadata/*.cpp
  ${libraw_SOURCE_DIR}/src/demosaic/*.cpp
  ${libraw_SOURCE_DIR}/src/tables/*.cpp
  ${libraw_SOURCE_DIR}/src/utils/*.cpp
  ${libraw_SOURCE_DIR}/src/write/*.cpp
  ${libraw_SOURCE_DIR}/src/postprocessing/[a-m]*.cpp
  ${libraw_SOURCE_DIR}/src/postprocessing/*aux.cpp
  ${libraw_SOURCE_DIR}/src/postprocessing/*_utils*.cpp
  ${libraw_SOURCE_DIR}/src/preprocessing/*.cpp
)
list(APPEND LIBRAW_SRC 
  ${libraw_SOURCE_DIR}/src/libraw_c_api.cpp
  ${libraw_SOURCE_DIR}/src/libraw_datastream.cpp
)


add_library(libraw STATIC ${LIBRAW_SRC})
target_include_directories(libraw PUBLIC
  ${libraw_SOURCE_DIR}
  ${libraw_SOURCE_DIR}/libraw
  ${libraw_SOURCE_DIR}/internal
)


