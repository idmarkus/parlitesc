#if 0
    #include "PLSC/Physics/Solver.hpp"

    #include "PLSC/Constants.hpp"
    #include "PLSC/DBG/Profile.hpp"
    #include "PLSC/Math/Util.hpp" // clamp

namespace PLSC
{
    //    template <PCFG::Settings CFG>
    //    void Solver::init() { m_collisionStructure.mkStatic(m_static.m_interfaces); }
    template <PCFG::Settings CFG>
    void Solver<CFG>::update()
    {
        PROFILE_COMPLEXITY(m_active);
        for (u32 i(Constants::Substep); i--;)
        {
            updateCollisions();
            updateObjects();
        }
        ++m_updates;
    }
    template <PCFG::Settings CFG>
    void Solver<CFG>::spawnRandom()
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
            m_objects[m_active] = Particle<CFG>(P);
            ++m_active;
        }
    }
    template <PCFG::Settings CFG>
    void Solver<CFG>::updateObjects()
    {
        for (u32 i = 0; i < m_active; ++i) { m_objects[i].update(m_gravity); }
    }
    template <PCFG::Settings CFG>
    void Solver<CFG>::updateCollisions()
    {
        m_collisionStructure.update(m_active);
    }
} // namespace PLSC
#endif