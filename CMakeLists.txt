cmake_minimum_required (VERSION 2.6)

project (fontgen)

message ("Build fontgen")

# The version number
set (fontgen_VERSION_MAJOR 0)
set (fontgen_VERSION_MINOR 1)

configure_file (
    config.h.in
    config.h
    )

set (SOURCE_FILES
    main.c)

add_executable (fontgenuter ${SOURCE_FILES})
