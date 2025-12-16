add_subdirectory(utils)
add_subdirectory(image)
add_subdirectory(io)
add_subdirectory(algorithm)

add_library(RawEdit INTERFACE)
target_link_libraries(
    RawEdit 
    INTERFACE
        RawEdit.utils
        RawEdit.Image
        RawEdit.IO
        RawEdit.Algorithm
)
target_include_directories(RawEdit INTERFACE "..")
