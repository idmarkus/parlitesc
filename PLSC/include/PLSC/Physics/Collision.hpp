#pragma once

#include "Collider.hpp"
#include "PLSC/Constants.hpp"
#include "PLSC/Math/vec2.hpp"

namespace PLSC
{
    /// Templated interface lookup, implements collision routines for all shapes. Requires library to be
    /// static and any non-particle collider constexpr. Can be substituted with runtime lookup if support
    /// for dynamic linking or runtime definition of static colliders is necessary later.
    template <Any::Kind _>
    struct Collision
    {
        static constexpr bool Intersects(Collider::Union const &uc, Particle const &p) { return false; }
        static inline bool    Collide(Collider::Union const &uc, Particle * p) { return false; }
        static inline void    CollideFast(Collider::Union const &uc, Particle * p) { }
    };

    template <>
    struct Collision<Any::AABB>
    {
        static inline constexpr bool Intersects(Collider::Union const &uc, Particle const &ob)
        {
            const auto [min, max] = uc.data.AABB.minmax();
            return (ob.P.x > min.x && ob.P.x < max.x && ob.P.y > min.y && ob.P.y < max.y);
        }

        static inline bool Collide(Collider::Union const &uc, Particle * ob)
        {
            const auto [min, max] = uc.data.AABB.minmax();
            if (ob->P.x < min.x || ob->P.x > max.x || ob->P.y < min.y || ob->P.y > max.y) return false;

            vec2 const &C      = uc.data.AABB.C;
            vec2 const &extent = uc.data.AABB.extent;

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

            return true;
        }

        static inline void CollideFast(Collider::Union const &uc, Particle * ob) { (void) Collide(uc, ob); }
    };

    template <>
    struct Collision<Any::InvBB>
    {
        static inline constexpr bool Intersects(Collider::Union const &uc, Particle const &ob)
        {
            const auto [min, max] = uc.data.InvBB.minmax();
            return (ob.P.x < min.x || ob.P.x > max.x || ob.P.y < min.y || ob.P.y > max.y);
        }

        static inline bool Collide(Collider::Union const &uc, Particle * ob)
        {
            using namespace Constants;
            static const auto [min, max] = uc.data.InvBB.minmax();

            vec2 &P   = ob->P;
            vec2 &dP  = ob->dP;
            bool  ret = false;
            if (P.x > max.x)
            {
                dP.x = max.x + ((max.x - dP.x) * StaticRestitution);
                dP.y = P.y - ((P.y - dP.y) * StaticFrictionCoef);
                P.x  = max.x;
                ret  = true;
            }
            else if (P.x < min.x)
            {
                dP.x = min.x + ((min.x - dP.x) * StaticRestitution);
                dP.y = P.y - ((P.y - dP.y) * StaticFrictionCoef);
                P.x  = min.x;
                ret  = true;
            }
            if (P.y > max.y)
            {
                dP.y = max.y + ((max.y - dP.y) * StaticRestitution);
                dP.x = dP.x + ((P.x - dP.x) * 0.04f); //(1.0f - StaticFrictionCoef));
                P.y  = max.y;
                ret  = true;
            }
            else if (P.y < min.y)
            {
                dP.y = min.y + ((min.y - dP.y) * StaticRestitution);
                dP.x = P.x - ((P.x - dP.x) * StaticFrictionCoef);
                P.y  = min.y;
                ret  = true;
            }

            return ret;
        }

        static inline void CollideFast(Collider::Union const &uc, Particle * ob) { (void) Collide(uc, ob); }
    };

    template <>
    struct Collision<Any::Circle>
    {
        static constexpr bool Intersects(Collider::Union const &uc, Particle const &ob)
        {
            const f32 R = uc.data.Circle.r + Constants::CircleRadius;
            return (uc.data.Circle.P.distSq(ob.P) < R * R);
        }
        static inline bool Collide(Collider::Union const &uc, Particle * p) { return false; }
        static inline void CollideFast(Collider::Union const &uc, Particle * p) { }
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