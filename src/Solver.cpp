#include "PLSC/Physics/Solver.hpp"

#include "PLSC/Constants.hpp"
#include "PLSC/Math/Util.hpp" // clamp

namespace PLSC
{
    void Solver::init() { m_collisionStructure.mkStatic(m_static.m_interfaces); }

    void Solver::update()
    {
        for (u32 i(Constants::Substep); i--;)
        {
            updateCollisions();
            updateObjects();
        }
        ++m_updates;
    }

    void Solver::spawnRandom()
    {
        if (m_active >= Constants::MaxDynamicInstances) return;
        for (u32 i = 0; i < Constants::CirclesPerWidth; ++i)
        {
            if (m_active > Constants::MaxDynamicInstances - 1) return;
            f32 rand_norm0 = ((f32) rand() / (f32) RAND_MAX);
            f32 rand_norm1 = ((f32) rand() / (f32) RAND_MAX);

            f32 x = Constants::CircleRadius + (Constants::CircleDiameter * static_cast<f32>(i));
            f32 y = Constants::WorldHeight * 0.5f * rand_norm1;

            x += (Constants::CircleDiameter * rand_norm0) - Constants::CircleRadius;

            vec2 P              = {x, y};
            m_objects[m_active] = Particle(P);
            ++m_active;
        }
    }

    void Solver::updateObjects()
    {
        for (u32 i = 0; i < m_active; ++i) { m_objects[i].update(); }
    }

    void Solver::updateCollisions() { m_collisionStructure.update(m_active); }
} // namespace PLSC