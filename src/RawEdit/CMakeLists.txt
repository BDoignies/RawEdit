add_subdirectory(core)
add_subdirectory(algorithm)

add_library(RawEdit INTERFACE)
target_link_libraries(RawEdit INTERFACE RawEditCore RawEditAlgorithm)
target_include_directories(RawEdit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..)
