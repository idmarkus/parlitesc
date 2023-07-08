cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${PLSC_SOURCE_DIR}/include/*.h*")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "${PLSC_SOURCE_DIR}/src/*.c*")

add_library(PLSC ${SOURCE_LIST} ${HEADER_LIST})

target_include_directories(PLSC PUBLIC ../include)

source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)