add_library(RawEdit.IO STATIC
    imageloader.cpp
)
target_include_directories(RawEdit.IO PUBLIC ../)
target_link_libraries(RawEdit.IO PUBLIC RawEdit.utils RawEdit.Image)
target_link_libraries(RawEdit.IO PUBLIC stbimage)
