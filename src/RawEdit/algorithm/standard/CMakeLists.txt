add_library(RawEdit.Algorithm.Standard INTERFACE)
target_link_libraries(RawEdit.Algorithm.Standard INTERFACE 
    RawEdit.Algorithm.Base
)
target_include_directories(RawEdit.Algorithm.Standard INTERFACE ../)
