#pragma once

#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"

#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <string>

namespace PLSC::GL
{
    class Shader
    {
    public:
        unsigned int ID;

        explicit Shader(const std::string vertexSource, const std::string fragmentSource)
        {
            const char * vdata = vertexSource.c_str();
            const char * fdata = fragmentSource.c_str();

            u32 vertex = glCreateShader(GL_VERTEX_SHADER);

            glShaderSource(vertex, 1, &vdata, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, 1);
            u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fdata, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, 2);
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);
            checkCompileErrors(ID, 0);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        const char * stupidAsciify(const std::string s)
        {
            char * result = (char *) malloc(sizeof(char) * s.length() + 1);
            size_t ri     = 0;
            for (size_t i = 0; i < s.length(); i++)
            {
                char c = s[i];
                if (isprint(c) || c == '\n') { result[ri++] = c; }
                else if (c == '\r')
                    result[ri++] = '\n';
            }
            for (; ri < s.length(); ri++) { result[ri] = ' '; }
            result[s.length()] = '\0';
            return result;
        }

        void load(const char * vertexPath, const char * fragmentPath)
        {
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            const char *  vShaderCode;
            const char *  fShaderCode;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                vShaderFile.open(vertexPath);
                std::string vertexCode((std::istreambuf_iterator<char>(vShaderFile)),
                                       std::istreambuf_iterator<char>());
                vShaderCode = stupidAsciify(vertexCode);

                fShaderFile.open(fragmentPath);
                std::string fragmentCode((std::istreambuf_iterator<char>(fShaderFile)),
                                         std::istreambuf_iterator<char>());
                fShaderCode = stupidAsciify(fragmentCode);
            }
            catch (std::ifstream::failure &e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            }
            // compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, 1);
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, 2);
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);
            checkCompileErrors(ID, 0);
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }
        // activate the shader
        // ------------------------------------------------------------------------
        inline void use() { glUseProgram(ID); }
        // utility uniform functions
        // ------------------------------------------------------------------------
        inline void setBool(const std::string &name, bool value)
        {
            use();
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
        }
        // ------------------------------------------------------------------------
        inline void setInt(const std::string &name, int value)
        {
            use();
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        // ------------------------------------------------------------------------
        inline void setFloat(const std::string &name, float value)
        {
            use();
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }

        inline void setVec2(const std::string &name, const vec2 &value)
        {
            use();
            glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
        }

        inline void setVec2(const std::string &name, const f32 x, const f32 y)
        {
            use();
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }

    private:
        // utility function for checking shader compilation/linking errors.
        void checkCompileErrors(unsigned int shader, u32 type)
        {
            int  success;
            char infoLog[1024];
            if (type)
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR: SHADER_COMPILATION_ERROR "
                              << ((type == 1) ? "VERTEX\n" : "FRAGMENT\n") << infoLog << std::endl;
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR: SHADER_PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
                }
            }
        }
    };

} // namespace PLSC::GL