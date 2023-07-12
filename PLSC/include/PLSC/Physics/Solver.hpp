#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"
#include "Particle.hpp"
#include "RadiusGrid.hpp"
#include "Static.hpp"

#include <array>

namespace PLSC
{

    class Solver
    {
    public:
        Solver() : m_collisionStructure(&m_objects[0]) { }

        std::array<Particle, Constants::MaxDynamicInstances> m_objects;
        Static::Definition                                   m_static;

        u32  m_active  = 0u;
        u32  m_updates = 0u;
        vec2 m_gravity = Constants::GravityPosition;

        void init();
        void update();
        void spawnRandom();
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
