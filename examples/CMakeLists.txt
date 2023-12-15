cmake_minimum_required(VERSION 3.14)

project(PLSC-Examples
        LANGUAGES CXX)

add_subdirectory(libdemo)

add_executable(PLSC-Galton galton.cpp)

target_link_libraries(
        PLSC-Galton
        PRIVATE
        PLSC::PLSC
        PLSC::GL
        PLSC::Demo
)

add_executable(PLSC-GaltonHeadless galton-headless.cpp)

target_link_libraries(
        PLSC-GaltonHeadless
        PRIVATE
        PLSC::PLSC
)

