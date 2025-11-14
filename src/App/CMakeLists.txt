add_executable(RawEditor
    main.cpp 
    app.cpp
    raweditraylib.cpp
    imagemanager.cpp
)
target_link_libraries(RawEditor PUBLIC RawEdit ui spdlog)
