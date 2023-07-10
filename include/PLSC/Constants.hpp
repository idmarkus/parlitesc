#pragma once

#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"

namespace PLSC
{
    namespace Constants
    {
        using number = long double;

        //-- User Definitions
        namespace CFG
        {
            // World dimensions, any range (width, height):
            static constexpr number WorldWidth  = 1.5;
            static constexpr number WorldHeight = 1.0;

            // Particle radius relative to world dimensions
            static constexpr number CircleRadius = 0.0035;

            // Maximum allowed particles
            static constexpr number MaxInstances = 5000;

            // Solver properties:
            static constexpr number Substep      = 12;         // Steps per update
            static constexpr number FixedTime    = 1.0 / 60.0; // Seconds per update
            static constexpr number Gravity1d    = 1.3;        // 1d (y) of g
            static constexpr number ResponseCoef = 1.0;        // Collision response

            static constexpr number CircleRestitution = 0.95;
            static constexpr number WorldRestitution  = 0.95;
        } // namespace CFG
    }     // namespace Constants

    namespace Constants
    {
        //-- Static helper functions
        template <typename Integral, typename Floating>
        static constexpr inline Integral static_ceil(const Floating x)
        {
            return (static_cast<Floating>(static_cast<Integral>(x)) == x)
                       ? static_cast<Integral>(x)
                       : static_cast<Integral>(x) + ((x > 0) ? 1 : 0);
        }

        template <typename Floating>
        static constexpr inline intmax_t static_ceil(const Floating x)
        {
            return (static_cast<Floating>(static_cast<intmax_t>(x)) == x)
                       ? static_cast<intmax_t>(x)
                       : static_cast<intmax_t>(x) + ((x > 0) ? 1 : 0);
        }

        static constexpr inline number static_round(const number x)
        {
            return (x > 0.0L) ? static_cast<number>(static_cast<uintmax_t>(x + 0.5L))
                              : -(static_cast<number>(static_cast<uintmax_t>((-x) + 0.5L)));
        }
    } // namespace Constants

    namespace Constants
    {
        //-- Implementation
        static constexpr f32 CircleRadius     = 0.5f;
        static constexpr f32 CircleDiameter   = 1.0f;
        static constexpr f32 CircleDiameterSq = 1.0f;

        using namespace PLSC;

        namespace HIGHP
        {
            static constexpr number impl_Scale = static_cast<number>(0.5) / CFG::CircleRadius;

            static constexpr number WorldWidth        = static_round(impl_Scale * CFG::WorldWidth);
            static constexpr number WorldHeight       = static_round(impl_Scale * CFG::WorldHeight);
            static constexpr number Gravity1d         = impl_Scale * CFG::Gravity1d;
            static constexpr number SubstepDelta      = CFG::FixedTime / CFG::Substep;
            static constexpr number Gravity1dPosition = Gravity1d * (SubstepDelta * SubstepDelta);
        } // namespace HIGHP

        static constexpr f32  WorldWidth  = static_cast<f32>(HIGHP::WorldWidth);
        static constexpr f32  WorldHeight = static_cast<f32>(HIGHP::WorldHeight);
        static constexpr vec2 WorldSize   = vec2(WorldWidth, WorldHeight);

        static constexpr f32 CircleXMin = CircleRadius;
        static constexpr f32 CircleYMin = CircleRadius;
        static constexpr f32 CircleXMax = WorldWidth - CircleRadius;
        static constexpr f32 CircleYMax = WorldHeight - CircleRadius;

        static constexpr u32 MaxDynamicInstances = static_cast<u32>(CFG::MaxInstances);
        static constexpr u32 CirclesPerWidth     = static_cast<u32>(WorldWidth / CircleDiameter) - 1;
        static constexpr u32 CirclesPerHeight    = static_cast<u32>(WorldHeight / CircleDiameter) - 1;

        static constexpr u32 Substep        = static_cast<u32>(CFG::Substep);
        static constexpr f32 FixedDeltaTime = static_cast<f32>(CFG::FixedTime);
        static constexpr f32 SubstepDelta   = static_cast<f32>(HIGHP::SubstepDelta);

        static constexpr f32 V_dt    = static_cast<f32>(HIGHP::SubstepDelta);
        static constexpr f32 A_dt    = static_cast<f32>(HIGHP::SubstepDelta * HIGHP::SubstepDelta * 0.5L);
        static constexpr f32 Half_dt = static_cast<f32>(HIGHP::SubstepDelta * 0.5L);

        static constexpr vec2 Gravity         = vec2(0.0f, static_cast<f32>(HIGHP::Gravity1d));
        static constexpr vec2 GravityPosition = vec2(0.0f, static_cast<f32>(HIGHP::Gravity1dPosition));
        static constexpr vec2 GravityA        = vec2(0.0f, static_cast<f32>(HIGHP::Gravity1d));

        static constexpr f32 Drag              = 0.5f * 0.1f;
        static constexpr f32 CircleRestitution = -(1.0f + CFG::CircleRestitution) * 0.5f;
        static constexpr f32 WorldRestitution  = static_cast<f32>(0.5L * CFG::WorldRestitution);
        // Constants for solver

        // 0.25 pre-multiply weight ratios for constant radii
        static constexpr f32 ResponseCoef       = 0.25f * static_cast<f32>(CFG::ResponseCoef);
        static constexpr f32 StaticFrictionCoef = 0.995f;
        static constexpr f32 StaticRestitution  = 0.85f; // 0.95f;

        static constexpr f32 SleepLimit   = 0.005f;
        static constexpr f32 SleepLimit2  = SleepLimit + SleepLimit;
        static constexpr f32 SleepLimitSq = SleepLimit * SleepLimit;
    } // namespace Constants
} // namespace PLSC