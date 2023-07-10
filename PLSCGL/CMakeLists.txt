cmake_minimum_required(VERSION 3.14)

project(
        PLSCGL
        VERSION 0.1
        LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(PLSCGL PLSCGL.cpp Renderer.hpp ParticleInstancer.hpp Shader.hpp ShapeDefinitions.hpp ShaderSources.hpp)

target_include_directories(PLSCGL PUBLIC "${PLSCGL_SOURCE_DIR}")