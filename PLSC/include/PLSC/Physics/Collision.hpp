#pragma once

#include "Collider.hpp"
#include "PLSC/Settings.hpp"
// #include "PLSC/Constants.hpp"
#include "PLSC/Math/vec2.hpp"

namespace PLSC
{
    using IntersectsBucketFn = bool (*)(vec2 const &);

    /// Templated interface lookup, implements collision routines for all shapes. Requires library to be
    /// static and any non-particle collider constexpr. Can be substituted with runtime lookup if support
    /// for dynamic linking or runtime definition of static colliders is necessary later.
    template <PCFG::Settings CFG, auto collider>
    struct Collision
    {
        //        static constexpr bool Intersects(Collider::Union const &uc, Particle<CFG> const &p) { return
        //        false; } static inline bool    Collide(Collider::Union const &uc, Particle<CFG> * p) {
        //        return false; } static inline void    CollideFast(Collider::Union const &uc, Particle<CFG> *
        //        p) { }
    };

    template <PCFG::Settings CFG, AABB collider>
    struct Collision<CFG, collider>
    {
        static constexpr bool IntersectsBucket(vec2 const &c)
        {
            const auto [min, max] = collider.minmax(CFG::Diameter);
            return (c.x > min.x && c.x < max.x && c.y > min.y && c.y < max.y);
        }
        static inline constexpr bool Intersects(Particle<CFG> const &ob)
        {
            const auto [min, max] = collider.minmax(CFG::Radius);
            return (ob.P.x > min.x && ob.P.x < max.x && ob.P.y > min.y && ob.P.y < max.y);
        }

        static inline void Collide(Particle<CFG> * ob)
        {
            const auto [min, max] = collider.minmax(CFG::Radius);
            if (ob->P.x < min.x || ob->P.x > max.x || ob->P.y < min.y || ob->P.y > max.y) return;

            constexpr vec2 const &C      = collider.C;
            constexpr vec2 const &extent = collider.extent;

            vec2       vd = ob->P - C;
            const bool hz = extent.y - std::fabs(vd.y) >= extent.x - std::fabs(vd.x);
            vd.x          = hz ? std::copysign(extent.x, vd.x) : vd.x;
            vd.y          = !hz ? std::copysign(extent.y, vd.y) : vd.y;

            // Closest point outward
            ob->P = C + vd;

            // Reflect dP if outside
            const vec2 dPclamp = vec2(clamp(ob->dP.x, min.x, max.x), clamp(ob->dP.y, min.y, max.y));
            const vec2 dPd     = dPclamp - ob->dP;
            ob->dP             = dPclamp + dPd;
        }
    };

    template <PCFG::Settings CFG, InvBB collider>
    struct Collision<CFG, collider>
    {
        static constexpr bool IntersectsBucket(vec2 const &c)
        {
            const auto [min, max] = collider.minmax(CFG::Diameter);
            return (c.x < min.x || c.x > max.x || c.y < min.y || c.y > max.y);
        }
        static inline constexpr bool Intersects(Particle<CFG> const &ob)
        {
            const auto [min, max] = collider.minmax(CFG::Radius);
            return (ob.P.x < min.x || ob.P.x > max.x || ob.P.y < min.y || ob.P.y > max.y);
        }

        static inline void Collide(Particle<CFG> * ob)
        {
            const auto [min, max] = collider.minmax(CFG::Radius);

            vec2 &P  = ob->P;
            vec2 &dP = ob->dP;

            if (P.x > max.x)
            {
                dP.x = max.x + ((max.x - dP.x) * 0.95f);
                dP.y = P.y - ((P.y - dP.y) * 0.95f);
                P.x  = max.x;
            }
            else if (P.x < min.x)
            {
                dP.x = min.x + ((min.x - dP.x) * 0.95f);
                dP.y = P.y - ((P.y - dP.y) * 0.95f);
                P.x  = min.x;
            }
            if (P.y > max.y)
            {
                dP.y = max.y + ((max.y - dP.y) * 0.95f);
                dP.x = dP.x + ((P.x - dP.x) * 0.04f); //(1.0f - 0.95f));
                P.y  = max.y;
            }
            else if (P.y < min.y)
            {
                dP.y = min.y + ((min.y - dP.y) * 0.95f);
                dP.x = P.x - ((P.x - dP.x) * 0.95f);
                P.y  = min.y;
            }
        }
    };

    template <PCFG::Settings CFG, Circle collider>
    struct Collision<CFG, collider>
    {
        static constexpr bool IntersectsBucket(vec2 const &c)
        {
            const vec2  min = c - CFG::Diameter - collider.r;
            const vec2  max = c + CFG::Diameter + collider.r;
            const vec2 &P   = collider.P;
            return (P.x > min.x && P.x < max.x && P.y > min.y && P.y < max.y);
        }

        static constexpr bool Intersects(Particle<CFG> const &ob)
        {
            constexpr f32 R = collider.r + CFG::Radius;
            return (collider.P.distSq(ob.P) < R * R);
        }
        static inline void Collide(Particle<CFG> * p) { }
    };

} // namespace PLSC

  // namespace PLSC
//{
//     struct OldUnion
//     {
//         const union
//         {
//             Shape::AABB   AABB;
//             Shape::InvBB  InvBB;
//             Shape::Circle Circle;
//         } data;
//         const Any::Kind which;
//
//         OldUnion() = delete;
//         constexpr OldUnion(Shape::AABB const &v) : data {.AABB = v}, which(Kind::AABB) { }
//         constexpr OldUnion(Shape::InvBB const &v) : data {.InvBB = v}, which(Kind::InvBB) { }
//         constexpr OldUnion(Shape::Circle const &v) : data {.Circle = v}, which(Kind::Circle) { }
//
//         template <Kind test, Kind check>
//         using which_is = typename std::enable_if<
//             std::is_same<std::integral_constant<Kind, test>, std::integral_constant<Kind, check>>::value,
//             bool>::type;
//
//         template <Kind W, which_is<W, Kind::AABB> = true>
//         [[nodiscard]] constexpr Shape::AABB as() const
//         {
//             return data.AABB;
//         }
//
//         template <Kind W, which_is<W, Kind::InvBB> = true>
//         [[nodiscard]] constexpr Shape::InvBB as() const
//         {
//             return data.InvBB;
//         }
//
//         template <Kind W, which_is<W, Kind::Circle> = true>
//         [[nodiscard]] constexpr Shape::Circle as() const
//         {
//             return data.Circle;
//         }
//     };
//
//     template <Any::Collider AC>
//     constexpr auto Original()
//     {
//         return AC.as<AC.which>();
//     }
// } // namespace PLSC