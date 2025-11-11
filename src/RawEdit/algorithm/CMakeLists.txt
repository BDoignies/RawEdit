add_library(RawEditAlgorithm STATIC
    rescale.cpp
)
target_link_libraries(RawEditAlgorithm PUBLIC RawEditCore OpenMP::OpenMP_CXX)
