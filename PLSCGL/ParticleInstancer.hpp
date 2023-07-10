#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Physics/Particle.hpp"
#include "Shader.hpp"
#include "ShaderSources.hpp"

#include <GL/glew.h>
#include <array>
namespace PLSC::GL
{
    class ParticleInstancer
    {
    private:
        GLuint VAO, VBO, EBO, instanceVBO;

        std::array<vec2, Constants::MaxDynamicInstances> m_aData;
        Particle *                                       m_objects;
        //        std::array<Particle, Constants::MaxDynamicInstances> m_objects;

    public:
        Shader shader;
        u32    m_active = 0u;

        ParticleInstancer(Particle * objects) : shader(vertCircle, fragCircle), m_objects(objects)
        {
            shader.setFloat("radius", Constants::CircleRadius);
            shader.setVec2("worldSize", Constants::WorldWidth, Constants::WorldHeight);
        }

        void init(i32 w, i32 h)
        {
            shader.setVec2("screenSize", static_cast<f32>(w), static_cast<f32>(h));
            static const f32 vertices[] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};
            static const u32 indices[]  = {0, 1, 2, 0, 2, 3};

            glGenBuffers(1, &instanceVBO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(m_aData), &m_aData[0], GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (GLvoid *) 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid *) 0);

            glVertexAttribDivisor(1, 1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        void updatePositions(const u32 active)
        {
            m_active = active;
            for (u32 i = 0; i < active; ++i)
            {
                PLSC::Particle * p = &m_objects[i];
                m_aData[i]         = p->P;
            }
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * active, &m_aData[0]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void setScreenSize(const i32 w, const i32 h)
        {
            shader.setVec2("screenSize", {static_cast<float>(w), static_cast<float>(h)});
        }

        void setOpts()
        {
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_BLEND);
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POLYGON_SMOOTH);
            // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        }

        void draw()
        {
            if (m_active)
            {
                shader.use();
                setOpts();
                glBindVertexArray(VAO);
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_active);
            }
        }

        ~ParticleInstancer()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            glDeleteBuffers(1, &instanceVBO);
        }
    };

} // namespace PLSC::GL