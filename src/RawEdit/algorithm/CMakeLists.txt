add_subdirectory(base)
add_subdirectory(standard)

add_library(RawEdit.Algorithm INTERFACE)
target_link_libraries(RawEdit.Algorithm INTERFACE 
    RawEdit.Algorithm.Base
    RawEdit.Algorithm.Standard
)
target_include_directories(RawEdit.Algorithm INTERFACE ../)
