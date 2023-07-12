#pragma once

#include "Particle.hpp"

#include <memory>

namespace PLSC::Collider
{
    class ICollider
    {
    public:
        virtual bool         Intersects(Particle *) const = 0;
        virtual bool         Collide(ParticleDelta *)          = 0;
        virtual void         CollideFast(ParticleDelta *)      = 0;
        virtual const char * Name() const                 = 0;

        virtual ~ICollider() { }
    };
} // namespace PLSC::Collider

namespace PLSC
{
    using collider_ptr = std::shared_ptr<Collider::ICollider>;
}

namespace PLSC::Collider
{
    struct AABB : public ICollider
    {
        const vec2 extent, C;
        const f32  minX, minY, maxX, maxY;

        constexpr explicit AABB(const f32 minx, const f32 miny, const f32 maxx, const f32 maxy) :
            extent(std::fabs(maxx - minx) * 0.5f + Constants::CircleRadius,
                   std::fabs(maxy - miny) * 0.5f + Constants::CircleRadius),
            C(minx + ((maxx - minx) * 0.5f), miny + ((maxy - miny) * 0.5f)),
            minX(C.x - extent.x),
            minY(C.y - extent.y),
            maxX(C.x + extent.x),
            maxY(C.y + extent.y)
        {
        }

        constexpr f32 draw_minX() const { return C.x - extent.x + Constants::CircleRadius; }
        constexpr f32 draw_minY() const { return C.y - extent.y + Constants::CircleRadius; }
        constexpr f32 draw_maxX() const { return C.x + extent.x - Constants::CircleRadius; }
        constexpr f32 draw_maxY() const { return C.y + extent.y - Constants::CircleRadius; }

        // virtual ~AABB() override = delete;

        virtual const char * Name() const final { return "AABB"; }

        inline bool Intersects(Particle * ob) const final
        {
            return (ob->P.x > minX && ob->P.x < maxX && ob->P.y > minY && ob->P.y < maxY);
        }

        inline bool Collide(ParticleDelta * ob) final
        {
            if (ob->P->x < minX || ob->P->x > maxX || ob->P->y < minY || ob->P->y > maxY) return false;

            vec2       vd = *ob->P - C;
            const bool hz = extent.y - std::fabs(vd.y) >= extent.x - std::fabs(vd.x);
            vd.x          = hz ? std::copysign(extent.x, vd.x) : vd.x;
            vd.y          = !hz ? std::copysign(extent.y, vd.y) : vd.y;

            // Closest point outward
            *ob->P = C + vd;

            // Reflect dP if outside
            const vec2 dPclamp = vec2(clamp(ob->dP.x, minX, maxX), clamp(ob->dP.y, minY, maxY));
            const vec2 dPd     = dPclamp - ob->dP;
            ob->dP             = dPclamp + dPd;
#ifdef PARTICLE_DBG_COLOR
            ob->dbgColStatic += DBG_COL_INCR;
#endif
            return true;
        }

        inline void CollideFast(ParticleDelta * ob) final { Collide(ob); }
    };

    struct InverseAABB : public ICollider
    {
        const vec2 C, extent;
        const f32  minX, minY, maxX, maxY;

        constexpr explicit InverseAABB(const f32 minx, const f32 miny, const f32 maxx, const f32 maxy) :
            C(minx + ((maxx - minx) * 0.5f), miny + ((maxy - miny) * 0.5f)),
            extent(std::fabs(maxx - minx) * 0.5f - Constants::CircleRadius,
                   std::fabs(maxy - miny) * 0.5f - Constants::CircleRadius),
            minX(C.x - extent.x),
            minY(C.y - extent.y),
            maxX(C.x + extent.x),
            maxY(C.y + extent.y)
        {
        }

        constexpr f32 draw_minX() const { return minX - Constants::CircleRadius; }
        constexpr f32 draw_minY() const { return minY - Constants::CircleRadius; }
        constexpr f32 draw_maxX() const { return maxX + Constants::CircleRadius; }
        constexpr f32 draw_maxY() const { return maxY + Constants::CircleRadius; }

        const char * Name() const final { return "InverseAABB"; }

        inline bool Intersects(Particle * ob) const final
        {
            return (ob->P.x < minX || ob->P.x > maxX || ob->P.y < minY || ob->P.y > maxY);
        }

        inline bool Collide(ParticleDelta * ob) final
        {
            using namespace Constants;

            vec2 *P   = ob->P;
            vec2 &dP  = ob->dP;
            bool  ret = false;
            if (P->x > maxX)
            {
                dP.x = maxX + ((maxX - dP.x) * StaticRestitution);
                dP.y = P->y - ((P->y - dP.y) * StaticFrictionCoef);
                P->x  = maxX;
                ret  = true;
            }
            else if (P->x < minX)
            {
                dP.x = minX + ((minX - dP.x) * StaticRestitution);
                dP.y = P->y - ((P->y - dP.y) * StaticFrictionCoef);
                P->x  = minX;
                ret  = true;
            }
            if (P->y > maxY)
            {
                dP.y = maxY + ((maxY - dP.y) * StaticRestitution);
                dP.x = dP.x + ((P->x - dP.x) * 0.04f); //(1.0f - StaticFrictionCoef));
                P->y  = maxY;
                ret  = true;
            }
            else if (P->y < minY)
            {
                dP.y = minY + ((minY - dP.y) * StaticRestitution);
                dP.x = P->x - ((P->x - dP.x) * StaticFrictionCoef);
                P->y  = minY;
                ret  = true;
            }
#ifdef PARTICLE_DBG_COLOR
            if (ret) ob->dbgColStatic += DBG_COL_INCR;
#endif
            return ret;
        }

        inline void CollideFast(ParticleDelta * ob) final { Collide(ob); }
    };

    struct Circle : public ICollider
    {
        const vec2 P;
        const f32  r;

        constexpr explicit Circle(const vec2 &_P, const f32 _r) : P(_P), r(_r) { }
        constexpr explicit Circle(const vec2 &_P) : P(_P), r(Constants::CircleRadius) { }
        constexpr explicit Circle(const f32 x, const f32 y, const f32 _r) : P(x, y), r(_r) { }
        constexpr explicit Circle(const f32 x, const f32 y) : P(x, y), r(Constants::CircleRadius) { }

        const char * Name() const final { return "Circle"; }

        inline bool Intersects(Particle * ob) const final
        {
            const f32 R = r + Constants::CircleRadius;
            return (P.distSq(ob->P) < R * R);
        }

        inline bool Collide(ParticleDelta * ob) final
        {
            (void) ob;
            return false;
        }

        inline void CollideFast(ParticleDelta * ob) final { (void) ob; }
    };
} // namespace PLSC::Collider