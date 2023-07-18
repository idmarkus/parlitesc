#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/Definition.hpp"
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"
#include "Particle.hpp"
#include "RadiusGrid.hpp"

#include <array>

namespace PLSC
{
    template <Settings CFG, class... Colliders>
    class Solver
    {
    public:
        constexpr explicit Solver(Definition<Colliders...> def) :
            m_objects {}, m_collisionStructure(def, &m_objects[0])
        {
        }

        std::array<Particle<CFG>, CFG.MaxParticles> m_objects;

        u32  m_active  = 0u;
        u32  m_updates = 0u;
        vec2 m_gravity = CFG.GravityPosition;

        // void init();
        void update()
        {
            PROFILE_COMPLEXITY(m_active);
            for (u32 i(CFG.Substeps); i--;)
            {
                updateCollisions();
                updateObjects();
            }
        }
        void spawnRandom()
        {
            if (m_active >= CFG.MaxParticles) return;
            for (u32 i = 0; i < CFG.ParticlesPerWidth; ++i)
            {
                if (m_active > CFG.MaxParticles - 1) return;
                f32 rand_norm0 = ((f32) rand() / (f32) RAND_MAX);
                f32 rand_norm1 = ((f32) rand() / (f32) RAND_MAX);

                f32 x = Constants::CircleRadius + (Constants::CircleDiameter * static_cast<f32>(i));
                f32 y = Constants::WorldHeight * 0.5f * rand_norm1;

                x += (Constants::CircleDiameter * rand_norm0) - Constants::CircleRadius;

                vec2 P              = {x, y};
                m_objects[m_active] = Particle<CFG>(P);
                ++m_active;
            }
        }
        f32  getKE()
        {
            f32 sum = 0;
            for (u32 i = 0; i < m_active; ++i) { sum += m_objects[i].KE(); }
            return sum;
        }

    private:
        RadiusGrid m_collisionStructure;

        void updateObjects();
        void updateCollisions();
    };

} // namespace PLSC
