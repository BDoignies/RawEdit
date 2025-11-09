add_library(
    RawEditCore STATIC
    # image.cpp
    rawimage.cpp
)
target_link_libraries(RawEditCore PUBLIC libraw)
target_include_directories(RawEditCore PUBLIC ${CMAKE_CURRENT_LIST_DIR}})
