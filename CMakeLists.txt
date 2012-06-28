cmake_minimum_required (VERSION 2.6)

project (fontgen)

include_directories (/usr/include/freetype2)

message ("Build fontgen")

# The version number
set (fontgen_VERSION_MAJOR 0)
set (fontgen_VERSION_MINOR 1)

configure_file (
    ${PROJECT_SOURCE_DIR}/config.h.in
    ${PROJECT_SOURCE_DIR}/config.h
    )

set (SOURCE_FILES
    main.c)

add_executable (fontgen ${SOURCE_FILES})
target_link_libraries (fontgen freetype)
