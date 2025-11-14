add_library(RawEditAlgorithm STATIC
    rescale.cpp
)
add_custom_command(
    TARGET RawEditAlgorithm
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)
target_link_libraries(RawEditAlgorithm PUBLIC RawEditCore OpenMP::OpenMP_CXX)
