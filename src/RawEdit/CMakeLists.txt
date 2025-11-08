add_subdirectory(core)

add_library(RawEdit INTERFACE)
target_link_libraries(RawEdit INTERFACE RawEditCore)
target_include_directories(RawEdit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..)
