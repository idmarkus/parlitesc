cmake_minimum_required(VERSION 3.14)

project(PLSC
        VERSION 0.1
        LANGUAGES CXX)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif ()

add_subdirectory(PLSC)
add_subdirectory(examples)





