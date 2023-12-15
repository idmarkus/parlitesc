#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/DBG/Profile.hpp"
#include "PLSC/Definition.hpp"
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Settings.hpp"
#include "PLSC/Typedefs.hpp"
#include "Particle.hpp"
#include "RadiusGrid.hpp"

#include <array>

namespace PLSC
{
    template <PCFG::Settings CFG = PLSC::Settings<>, PCFG::Definition DEF = PLSC::Definition<CFG>>
    class Solver
    {
    public:
        explicit Solver() : m_collisionStructure(&m_objects[0]) { }

        std::array<Particle<CFG>, CFG::Particles> m_objects;
        //        Particle<CFG> m_objects[CFG::Particles];

        u32  m_active  = 0u;
        u32  m_updates = 0u;
        vec2 m_gravity = CFG::GravityP;

        // void init();
        void update()
        {
            PROFILE_COMPLEXITY(m_active);
            for (u32 i(CFG::Substeps); i--;)
            {
                updateCollisions();
                updateObjects();
            }
        }
        void spawnRandom()
        {
            if (m_active >= CFG::Particles) return;
            for (u32 i = 0; i < CFG::ParticlesPerWidth; ++i)
            {
                if (m_active > CFG::Particles - 1) return;
                f32 rand_norm0 = ((f32) rand() / (f32) RAND_MAX);
                f32 rand_norm1 = ((f32) rand() / (f32) RAND_MAX);

                f32 x = CFG::Radius + (CFG::Diameter * static_cast<f32>(i));
                f32 y = CFG::Height * 0.5f * rand_norm1;

                x += (CFG::Diameter * rand_norm0) - CFG::Radius;

                vec2 P              = {x, y};
                m_objects[m_active] = Particle<CFG>(P);
                ++m_active;
            }
        }
        f32 getKE()
        {
            f32 sum = 0;
            for (u32 i = 0; i < m_active; ++i) { sum += m_objects[i].KE(); }
            return sum;
        }

    private:
    public:
        RadiusGrid<CFG, DEF> m_collisionStructure;

        void updateObjects()
        {
            for (u32 i = 0; i < m_active; ++i) { m_objects[i].update(m_gravity); }
        }
        void updateCollisions() { m_collisionStructure.update(m_active); }
    };

} // namespace PLSC
