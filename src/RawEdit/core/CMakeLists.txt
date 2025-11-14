add_library(
    RawEditCore STATIC
    image.cpp
    openglimage.cpp
    shader.cpp
    shadermanager.cpp
)
target_link_libraries(RawEditCore PUBLIC libraw stbimage)
target_include_directories(RawEditCore PUBLIC ${CMAKE_CURRENT_LIST_DIR}})
