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
        Solver() : m_collisionStructure(&m_objects[0], &m_deltas[0]) { }
        
        std::array<Particle, Constants::MaxDynamicInstances> m_objects;
        std::array<ParticleDelta, Constants::MaxDynamicInstances> m_deltas;
        Static::Definition                                   m_static;

        u32 m_active  = 0u;
        u32 m_updates = 0u;

        void init();
        void update();
        void spawnRandom();

    private:
        RadiusGrid m_collisionStructure;

        void updateObjects();
        void updateCollisions();
    };

} // namespace PLSC
