#pragma once

#include "Functional.hpp"
#include "Physics/Collider.hpp"
#include "Physics/Collision.hpp"
#include "Typedefs.hpp"

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace PCFG
{
    struct DefinitionDummy
    {
    };

    template <class T>
    concept Definition = std::is_base_of_v<DefinitionDummy, T> && !std::is_same_v<DefinitionDummy, T>;
} // namespace PCFG

namespace PLSC
{
    //    using Collidex = Tuplex; //<const Collider::Union>;
    //
    //    template <PCFG::Settings CFG, auto... cs>
    //        requires((IsCollider<decltype(cs)>) && ...)
    //    struct ComposeColliders
    //    {
    //        static constexpr auto F = Composer<Particle<CFG> *, ((Collision<CFG, cs>::Collide), ...)>::F;
    //    };

    template <auto... Colliders>
    struct Definition : PCFG::DefinitionDummy
    {
        static constexpr auto Tuple = Tuplex::Flatten(Colliders...);

        template <class X, class C, class... Cs>
        struct HasImpl
        {
            static constexpr bool value = std::is_same_v<X, C> || HasImpl<X, Cs...>::value;
        };
        template <class X, class C>
        struct HasImpl<X, C>
        {
            static constexpr bool value = std::is_same_v<X, C>;
        };

        //        template <class X, class C>
        //        struct HasArg<X, C>
        //        {
        //            static constexpr bool value = std::is_same_v<X, C>;
        //        };

        //        template <class C, class... Cs>
        //        static constexpr bool Has_Impl(std::tuple<Cs...> _)
        //        {
        //            return HasArg<C, Cs...>::value;
        //        }
        //

        template <class X, class... Cs>
        static constexpr bool Has_(std::tuple<Cs...>)
        {
            return HasImpl<X, Cs...>::value;
        }

        template <class X>
        static constexpr bool Has()
        {
            return Has_<X>(Tuple);
        }
        //        template <auto tuple>
        //        struct Expand
        //        {
        //        };
        //
        //        template <class C, class... Cs, std::tuple<C, Cs...> tuple>
        //        struct Expand<tuple>
        //        {
        //            using T = C;
        //        };
        //
        //        static constexpr auto   Tuple = Tuplex::Flatten(Colliders...);
        //        static constexpr size_t N     = std::tuple_size_v<decltype(Tuple)>;
        //
        //        using Exp                = Expand<Tuple>;
        //        static constexpr auto et = Exp::T;

        //        static_assert(IsCollider<decltype(Tuple)>, "All parameters must be Colliders!");
        //
        //        using Intersects = std::array<IntersectsBucketFn, N>;
        //        using Collisions = std::array<Fun<Particle<CFG> *>, N>;
        //
        //        static constexpr Intersects TestFns {
        //            []<size_t... Inds>(std::index_sequence<Inds...>)
        //            {
        //                return Intersects {(Collision<CFG, std::get<Inds>(Tuple)>::IntersectsBucket)...};
        //            }(std::make_index_sequence<N>())};
        //
        //        static constexpr Collisions CollideFns {
        //            []<size_t... Inds>(std::index_sequence<Inds...>)
        //            {
        //                return Collisions {(Collision<CFG, std::get<Inds>(Tuple)>::Collide)...};
        //            }(std::make_index_sequence<N>())};
    };
}; // namespace PLSC
//
//    template <class... Cs>
//    struct Definition2
//    {
//        //        static constexpr size_t                    N = Collidex::Count<Cs...>();
//        // const std::array<const Collider::Union, N> data;
//
//        constexpr explicit Definition2(Cs... cs) : data(Collidex::Array(cs...)) { }
//
//        template <class C>
//        static inline constexpr size_t count()
//        {
//            return Collidex::CountSpecific<C, Cs...>();
//        }
//
//        template <class C>
//        constexpr inline auto all() const
//        {
//            assert_collider<C>();
//            constexpr size_t n = count<C>();
//            static_assert(n > 0, "No such colliders are defined");
//            return M_fetch<C>(std::make_index_sequence<n>());
//        }
//
//    private:
//        template <class C>
//        static constexpr inline void assert_collider()
//        {
//            static_assert(std::is_convertible<C, Collider::Union>::value, "Type is not a collider.");
//        }
//
//        template <class C, size_t... Indices>
//        [[nodiscard]] constexpr size_t M_count(std::index_sequence<Indices...>) const
//        {
//            return (0 + ... + (is_same<C>(std::get<Indices>(data))));
//        }
//
//        template <class C>
//        static constexpr bool is_same(Collider::Union const &uc)
//        {
//            return (C::Which == uc.which);
//        }
//
//        template <class C, size_t... Indices>
//        constexpr auto M_fetch(std::index_sequence<Indices...>) const
//        {
//            return std::array<const C * const, sizeof...(Indices)> {(M_fetchOne<C, Indices>(0, 0))...};
//        }
//
//        // TODO: O(n log N) lookup, maybe fix later.
//        template <class C, size_t toFind>
//        constexpr const C * M_fetchOne(const size_t index, const size_t found) const
//        {
//            if (is_same<C>(data[index]))
//            {
//                if (found >= toFind)
//                {
//                    //                    const auto uc = data[index];
//                    return Any::AsPtr<C>(data[index]).ptr;
//                }
//                else
//                    return M_fetchOne<C, toFind>(index + 1, found + 1);
//            }
//            else
//                return M_fetchOne<C, toFind>(index + 1, found);
//        }
//    };
//} // namespace PLSC

// void test_function()
//{
//     //    constexpr PLSC::Settings cfg {PLSC::Radius(15), PLSC::Width(1280), PLSC::Height(720)};
//     static constexpr PLSC::Settings   cfg;
//     static constexpr PLSC::Definition def(
//         []()
//         {
//             constexpr f32 BinSize   = cfg.Radius * 8.0f;
//             constexpr f32 BinWidth  = cfg.Radius;
//             constexpr f32 BinHeight = cfg.Radius * 0.3f;
//
//             constexpr f32  BinIncr = BinSize + BinWidth;
//             constexpr auto N       = static_cast<size_t>((cfg.Width - (BinIncr)) / BinIncr);
//             return PLSC::Iterate<N>(
//                 [](size_t i)
//                 {
//                     f32 cx = BinIncr + (BinIncr * static_cast<f32>(i));
//                     f32 x0 = cx;
//                     f32 x1 = cx + BinWidth;
//                     f32 y0 = cfg.Height - BinHeight;
//                     f32 y1 = cfg.Height;
//                     return PLSC::AABB(x0, y0, x1, y1);
//                 });
//         }(),
//         PLSC::InvBB(0, 0, cfg.Width, cfg.Height));
//
//     constexpr auto caabb   = def.count<PLSC::AABB>();
//     constexpr auto ccirc   = def.count<PLSC::Circle>();
//     constexpr auto cinvb   = def.count<PLSC::InvBB>();
//     constexpr auto carr2   = def.data;
//     constexpr auto cwhich2 = carr2[0].which;
//
//     constexpr auto all_aabb  = def.all<PLSC::AABB>();
//     constexpr auto all_invbb = def.all<PLSC::InvBB>();
//
//     constexpr auto c = all_invbb[0]->C.x;
//
//     const PLSC::Settings   cfg_rt;
//     const PLSC::Definition def_rt(
//         //        [&]()
//         //        {
//         //            f32 BinSize   = cfg_rt.Radius * 8.0f;
//         //            f32 BinWidth  = cfg_rt.Radius;
//         //            f32 BinHeight = cfg_rt.Radius * 0.3f;
//         //
//         //            f32        BinIncr = BinSize + BinWidth;
//         //            const auto N       = static_cast<size_t>((cfg_rt.Width - (BinIncr)) / BinIncr);
//         //            return PLSC::Iterate(N,
//         //                                 [&](size_t i)
//         //                                 {
//         //                                     f32 cx = BinIncr + (BinIncr * static_cast<f32>(i));
//         //                                     f32 x0 = cx;
//         //                                     f32 x1 = cx + BinWidth;
//         //                                     f32 y0 = cfg.Height - BinHeight;
//         //                                     f32 y1 = cfg.Height;
//         //                                     return PLSC::AABB(x0, y0, x1, y1);
//         //                                 });
//         //        }(),
//         PLSC::InvBB(0, 0, cfg.Width, cfg.Height), PLSC::InvBB(0, 0, cfg.Width, cfg.Height));
//
//     auto       caabb2   = def_rt.count<PLSC::InvBB>();
//     const auto all_inv2 = def_rt.all<PLSC::InvBB>();
// }
