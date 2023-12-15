cmake_minimum_required(VERSION 3.14)

project(PLSCDEMOLIB LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_library(PLSCDEMOLIB Window.cpp Window.hpp)

# SDL_MAIN_HANDLED is needed on Windows
#if (WIN32 OR MSYS OR MINGW)
target_compile_definitions(PLSCDEMOLIB PUBLIC SDL_MAIN_HANDLED)
#endif ()

target_include_directories(PLSCDEMOLIB PUBLIC ${PLSCDEMOLIB_SOURCE_DIR})

target_link_libraries(
        PLSCDEMOLIB
        PUBLIC
        PLSC::PLSC
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

add_library(PLSC::Demo ALIAS PLSCDEMOLIB)