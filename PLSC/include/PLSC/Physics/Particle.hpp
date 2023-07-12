#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/Math/Util.hpp" // rsqrt_fast
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"

#include <cfloat> // FLT_EPSILON

namespace PLSC
{
    struct Particle
    {
        vec2 P, dP;

        Particle() = default;

        constexpr explicit Particle(vec2 const &p) : P(p), dP(p) { }
        constexpr explicit Particle(vec2 const &p, vec2 const &d) : P(p), dP(d) { }
        constexpr explicit Particle(f32 const &x, f32 const &y) : P(x, y), dP(P) { }

        inline void update(const vec2& gravity)
        {
            const vec2 v = P - dP;
            dP           = P;
            P += v + gravity;
        }

        inline f32 KE()
        {
            // "in joules"
            // TODO: Update if we add units.
            return Constants::CircleHalfMass * std::fabs(P.distSq(dP));
        }

        inline bool Collide(Particle * ob)
        {
            vec2  vd   = P - ob->P;
            float dist = std::fabs(vd.dot(vd));
            if (dist > FLT_EPSILON && dist < (Constants::CircleDiameter))
            {
                dist = sqrtf(dist);
                vd /= dist;
                vd *= Constants::ResponseCoef * (dist - Constants::CircleDiameter);
                P -= vd;
                ob->P += vd;
                return true;
            }
            return false;
        }

        inline void CollideFast(Particle * ob)
        {
            vec2  vd   = P - ob->P;
            float dist = std::fabs(vd.dot(vd));
            if (dist < (Constants::CircleDiameter)) // + 0.005f))
            {
                if (dist > FLT_EPSILON)
                    vd *= Constants::ResponseCoef * (1.0f - rsqrt_fast(dist));
                else vd *= Constants::ResponseCoef * (1.0f - rsqrt_fast(Constants::CircleRadius));
                P -= vd;
                ob->P += vd;
            }
        }
    };
} // namespace PLSC
