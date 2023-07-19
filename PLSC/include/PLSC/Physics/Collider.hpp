#pragma once

#include "PLSC/Constants.hpp"
#include "PLSC/Math/vec2.hpp"
#include "PLSC/Typedefs.hpp"
#include "Particle.hpp"

namespace PLSC
{
    namespace Any
    {
        enum Kind : uint8_t
        {
            AABB,
            InvBB,
            Circle
        };
    }

    struct AABB
    {
        static constexpr Any::Kind Which = Any::AABB;

        const vec2 extent, C;

        constexpr explicit AABB(f32 x0, f32 y0, f32 x1, f32 y1) :
            extent((x1 - x0) * 0.5f, (y1 - y0) * 0.5f), C(x0 + extent.x, y0 + extent.y)
        {
        }

        inline constexpr std::pair<vec2, vec2> minmax() const { return {C - extent, C + extent}; }
        inline constexpr std::pair<vec2, vec2> draw_minmax() const
        {
            return minmax();
            //            return {(C - extent) + Constants::CircleRadius, (C + extent) -
            //            Constants::CircleRadius};
        }
    };

    /// Inverse AABB, same data but objects will remain inside the bounding box. Useful for borders, etc.
    struct InvBB : public AABB
    {
        static constexpr Any::Kind Which = Any::InvBB;
        constexpr explicit InvBB(f32 x0, f32 y0, f32 x1, f32 y1) : AABB(x0, y0, x1, y1) { }
    };

    struct Circle // TODO: Implement Circle collider
    {
        static constexpr Any::Kind Which = Any::Circle;
        const vec2                 P;
        const f32                  r;

        constexpr explicit Circle(const vec2 &P, f32 r) : P(P), r(r) { }
        constexpr explicit Circle(f32 x, f32 y, f32 r) : P(x, y), r(r) { }
    };

    namespace Collider
    {
        /// Collider either monad, works like an interface but can be stored sequentially. Since shapes
        /// need about the same amount of data there isn't much extra overhead by implementing what could be
        /// replaced by a virtual base-class in this way, but in return we get the ability to define and store
        /// any static collider as constexpr at compile-time
        struct Union
        {
            const union
            {
                AABB   AABB;
                InvBB  InvBB;
                Circle Circle;
            } data;
            const Any::Kind which;

            Union()                        = delete;
            constexpr Union(Union const &) = default;
            constexpr Union(AABB const &v) : data {.AABB = v}, which(Any::AABB) { }
            constexpr Union(InvBB const &v) : data {.InvBB = v}, which(Any::InvBB) { }
            constexpr Union(Circle const &v) : data {.Circle = v}, which(Any::Circle) { }

            //            template <class C>
            //            constexpr const auto as() const
            //            {
            //            }
            //
            //            template <>
            //            constexpr const auto as<AABB>() const
            //            {
            //                return data.AABB;
            //            }
            //
            //            template <>
            //            constexpr const auto as<InvBB>() const
            //            {
            //                return data.InvBB;
            //            }
            //
            //            template <>
            //            constexpr const auto as<Circle>() const
            //            {
            //                return data.Circle;
            //            }

            template <Any::Kind test, Any::Kind check>
            using which_is =
                typename std::enable_if<std::is_same<std::integral_constant<Any::Kind, test>,
                                                     std::integral_constant<Any::Kind, check>>::value,
                                        bool>::type;

            template <typename C>
            constexpr C get() const
            {
                switch (C::Which)
                {
                    case (Any::AABB): return data.AABB;
                    case (Any::InvBB): return data.InvBB;
                    case (Any::Circle): return data.Circle;
                }
            }

            template <typename C>
            constexpr const C * get_ptr() const
            {
                switch (C::Which)
                {
                    case (Any::AABB): return &data.AABB;
                    case (Any::InvBB): return &data.InvBB;
                    case (Any::Circle): return &data.Circle;
                }
            }

            template <Any::Kind w, which_is<w, Any::AABB>>
            constexpr auto as()
            {
                return data.AABB;
            }

            template <Any::Kind w, which_is<w, Any::InvBB>>
            constexpr auto as()
            {
                return data.InvBB;
            }

            template <Any::Kind w, which_is<w, Any::Circle>>
            constexpr auto as()
            {
                return data.Circle;
            }

            template <Any::Kind w, which_is<w, Any::AABB>>
            constexpr const auto as_ptr() const
            {
                return &data.AABB;
            }

            template <Any::Kind w, which_is<w, Any::InvBB>>
            constexpr const auto as_ptr() const
            {
                return &data.InvBB;
            }

            template <Any::Kind w, which_is<w, Any::Circle>>
            constexpr const auto as_ptr() const
            {
                return &data.Circle;
            }
        };
    } // namespace Collider
    namespace Any
    {
        template <Collider::Union UC>
        static constexpr auto Original_ptr()
        {
            return UC.as_ptr<UC.which>();
        }

        template <class C>
        static constexpr const C * As_ptr(const Collider::Union &uc)
        {
            switch (C::Which)
            {
                case (Any::AABB): return &uc.data.AABB;
                case (Any::InvBB): return &uc.data.InvBB;
                case (Any::Circle): return &uc.data.Circle;
            }
        }

        template <class C>
        struct AsPtr
        {
        };

        template <>
        struct AsPtr<PLSC::AABB>
        {
            const PLSC::AABB * const ptr;
            constexpr AsPtr(const Collider::Union &uc) : ptr(&uc.data.AABB) { }
        };

        template <>
        struct AsPtr<PLSC::InvBB>
        {
            const PLSC::InvBB * const ptr;
            constexpr AsPtr(const Collider::Union &uc) : ptr(&uc.data.InvBB) { }
        };

        template <>
        struct AsPtr<PLSC::Circle>
        {
            const PLSC::Circle * const ptr;
            constexpr AsPtr(const Collider::Union &uc) : ptr(&uc.data.Circle) { }
        };

        //        template <class C>
        //        static constexpr auto As(Collider::Union const &uc)
        //        {
        //        }
        //
        //        template <>
        //        static constexpr auto As<PLSC::AABB>(Collider::Union const &uc)
        //        {
        //            return &uc.data.AABB;
        //        }
        //
        //        template <>
        //        static constexpr auto As<PLSC::InvBB>(Collider::Union const &uc)
        //        {
        //            return &uc.data.InvBB;
        //        }
        //
        //        template <>
        //        static constexpr auto As<PLSC::Circle>(Collider::Union const &uc)
        //        {
        //            return &uc.data.Circle;
        //        }

    } // namespace Any
} // namespace PLSC::Collider
