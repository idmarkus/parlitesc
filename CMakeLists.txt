cmake_minimum_required(VERSION 3.14)

project(
        PLSC
        VERSION 0.1
        LANGUAGES CXX)

add_subdirectory(src)
add_subdirectory(PLSCGL)

set_property(TARGET PLSC PROPERTY CXX_STANDARD 20)
set_property(TARGET PLSC PROPERTY USE_FOLDERS ON)





