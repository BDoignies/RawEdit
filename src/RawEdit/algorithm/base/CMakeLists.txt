add_library(RawEdit.Algorithm.Base INTERFACE)
target_link_libraries(RawEdit.Algorithm.Base INTERFACE 
    RawEdit.utils
    RawEdit.Image
)
target_include_directories(RawEdit.Algorithm.Base INTERFACE ../)
