#pragma once

// #include "PLSC/Constants.hpp"
#include "PLSC/Settings.hpp"
#include "Shader.hpp"
#include "ShapeDefinitions.hpp"

#include <GL/glew.h>
#include <memory>

namespace PLSC::GL
{
    template <PCFG::Settings CFG>
    class StaticRenderer
    {
        GLuint VAO, VBO, EBO;
        Shader m_shader;

    public:
        const std::vector<f32> m_vertices;
        const std::vector<u32> m_indices;

        template <typename T>
        explicit StaticRenderer(std::shared_ptr<T> p) :
            m_shader(ShapeDefinition<T>::VShaderSource(), ShapeDefinition<T>::FShaderSource()),
            m_vertices(ShapeDefinition<T>::vertices(*p)),
            m_indices(ShapeDefinition<T>::indices(*p))
        {
            m_shader.setVec2("worldSize", CFG::Width, CFG::Height);
        }

        void init(i32 w, i32 h)
        {
            m_shader.setVec2("screenSize", static_cast<f32>(w), static_cast<f32>(h));
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * m_vertices.size(), &m_vertices.data()[0],
                         GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (GLvoid *) 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * m_indices.size(), &m_indices.data()[0],
                         GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        ~StaticRenderer()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }

        inline void setScreenSize(i32 screenWidth, i32 screenHeight)
        {
            m_shader.use();
            m_shader.setVec2("screenSize", {static_cast<f32>(screenWidth), static_cast<f32>(screenHeight)});
            //            m_shader.setVec2("worldSize", {Constants::WorldWidth, Constants::WorldHeight});
        }

        void setOpts()
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wire
            // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_POLYGON_SMOOTH);
            glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        }

        inline void draw()
        {
            setOpts();
            m_shader.use();
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        }
    };
    template <PCFG::Settings CFG>
    class Renderer
    {
    private:
        std::vector<StaticRenderer<CFG>> m_vStatic;

    public:
        template <typename T>
        inline void Register(std::vector<T> &v)
        {
            for (T &t : v) { m_vStatic.emplace_back(t); }
        }

        inline void init(i32 w, i32 h)
        {
            for (auto &x : m_vStatic) { x.init(w, h); }
        }

        inline void draw()
        {
            for (auto &x : m_vStatic) { x.draw(); }
        }

        inline void setScreenSize(i32 w, i32 h)
        {
            for (auto &x : m_vStatic) { x.setScreenSize(w, h); }
        }
    };
} // namespace PLSC::GL