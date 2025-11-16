add_library(RawEditAlgorithm STATIC
    rescale.cpp
    mask.cpp
)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
file(GLOB shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)
foreach(shader ${shaders})
    get_filename_component(shadername ${shader} NAME)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shadername}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${shadername}
        COPYONLY
    )
endforeach()
target_link_libraries(RawEditAlgorithm PUBLIC RawEditCore OpenMP::OpenMP_CXX)
