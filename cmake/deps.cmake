# Fetch raylib
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

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

# Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.1  # Use a stable tag
)
FetchContent_MakeAvailable(imgui)

# Fetch ImGui bindings for raylib
FetchContent_Declare(
    rlImGui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)
FetchContent_MakeAvailable(rlImGui)

# Create a custom ui library
add_library(ui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_link_libraries(ui PUBLIC raylib)
target_include_directories(ui PUBLIC
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)


