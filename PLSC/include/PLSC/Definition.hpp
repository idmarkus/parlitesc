/*
 * TODO: Support C++11:
 *   Settings:
 *     - For Solver<cfg>: Make Setting store data as integer w/ an operator() reinterpret_cast to defined type
 *     - Remove C++17 fold expressions and implement manual folding in EnableIf:: template specializations
 *   Definition:
 *     - ??? Tuplex has a lot of folds...
 */
#pragma once

#include "Functional.hpp"
#include "Physics/Collider.hpp"
#include "Physics/Collision.hpp"
#include "Typedefs.hpp"

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace PLSC
{

#define define_setting(SETTING__NAME, VALUE__NAME, DEFAULT__VALUE)                                           \
    struct SETTING__NAME                                                                                     \
    {                                                                                                        \
        static constexpr Number Default         = (DEFAULT__VALUE);                                          \
        const Number            m_##VALUE__NAME = (DEFAULT__VALUE);                                          \
        constexpr SETTING__NAME()               = default;                                                   \
        constexpr explicit SETTING__NAME(const Number _) : m_##VALUE__NAME(_) { }                            \
        template <typename T, EnableIf::Arithmetic<T> = true>                                                \
        constexpr operator T() const                                                                         \
        {                                                                                                    \
            return static_cast<T>(m_##VALUE__NAME);                                                          \
        }                                                                                                    \
    };

    define_setting(Width, width, 1.5);
    define_setting(Height, height, 1.0);
    define_setting(Radius, radius, 0.0035);
    define_setting(MaxParticles, maxParticles, 1000);
    define_setting(Substeps, substeps, 12);
    define_setting(DeltaTime, deltaTime, 1.0 / 60.0);
    define_setting(Gravity1d, gravity1d, 1.3);
    define_setting(ResponseCoef, responseCoef, 1.0);
    define_setting(CircleRestitution, circleRestitution, 0.95);
    define_setting(WorldRestitution, worldRestitution, 0.95);
#undef define_setting

    struct UserConfig : public Width,
                        public Height,
                        public Radius,
                        public MaxParticles,
                        public Substeps,
                        public DeltaTime,
                        public Gravity1d,
                        public ResponseCoef,
                        public CircleRestitution,
                        public WorldRestitution
    {
        struct Impl
        {
            const Number scale;
            const Number width;
            const Number height;
            const Number gravity1d;
            const Number substepDelta;
            const Number gravity1dPosition;

            constexpr Impl() = delete;
            constexpr explicit Impl(UserConfig const &cfg) :
                scale(0.5L / cfg.m_radius),
                width(roundexpr::round(scale * cfg.m_width)),
                height(roundexpr::round(scale * cfg.m_height)),
                gravity1d(scale * cfg.m_gravity1d),
                substepDelta(cfg.m_deltaTime / cfg.m_substeps),
                gravity1dPosition(gravity1d * (substepDelta * substepDelta))
            {
            }
        };
        // constexpr explicit UserConfig() : impl(*this) { }
        constexpr explicit UserConfig() = default;

        /// Unpack-construct every setting to set user defined values, every setting not in args is therefore
        /// default constructed, giving us its default value using the same interface, ie m_radius
        template <class... Setting, EnableIf::Derives<UserConfig, Setting...> = true>
        constexpr explicit UserConfig(Setting... args) : Setting(args)... //, impl(*this)
        {
        }
    };

    /// @Config Defines constexpr-friendly settings in named parameter types, with default values and variadic
    /// overrides. Config() defines individual settings in any order, unspecified setting remain default.
    ///
    ///     Config cfg { Radius(5), Width(25) }
    ///
    /// @Settings Settings are defined as monadic wrappers of some value, whose valid operations are Identity
    /// and Unwrap. They can implicitly be cast to any arithmetic type (Unwrap), int r = radius, and cannot be
    /// cast to another setting, unless explicit (which implies Unwrap x -> Wrap x).
    ///
    /// Radius r = Width(5) is an error, but Radius r = (Radius)Width(5) is fine.
    struct Settings
    {
    public:
        const f32 Radius     = 0.5f;
        const f32 Diameter   = 1.0f;
        const f32 DiameterSq = Diameter;

        const f32  Width;
        const f32  Height;
        const vec2 WorldSize;

        const f32 CircleXMin = Radius;
        const f32 CircleYMin = Radius;
        const f32 CircleXMax;
        const f32 CircleYMax;

        const u32 MaxParticles;
        const u32 ParticlesPerWidth;
        const u32 ParticlesPerHeight;

        const u32 Substeps;
        const f32 DeltaTime;
        const f32 SubstepDelta;

        const vec2 Gravity;
        const vec2 GravityPosition;

        const f32 CircleRestitution;
        const f32 WorldRestitution;

        const f32 ResponseCoef;

        const f32 CircleArea;
        const f32 CircleAreaDensity;
        const f32 CircleMass;
        const f32 CircleHalfMass;

        constexpr explicit Settings() : Settings(UserConfig()) { }

        template <class... Setting, EnableIf::Derives<UserConfig, Setting...> = true>
        constexpr explicit Settings(Setting... args) : Settings(UserConfig(args...))
        {
        }

        constexpr explicit Settings(UserConfig const &cfg) : Settings(cfg, UserConfig::Impl(cfg)) { }

        constexpr explicit Settings(UserConfig const &cfg, UserConfig::Impl const &impl) :
            Width(static_cast<f32>(impl.width)),
            Height(static_cast<f32>(impl.height)),
            WorldSize(Width, Height),
            CircleXMax(Width - Radius),
            CircleYMax(Height - Radius),
            MaxParticles(static_cast<u32>(cfg.m_maxParticles)),
            ParticlesPerWidth(u32(impl.width / Diameter) - 1),
            ParticlesPerHeight(u32(impl.height / Diameter) - 1),
            Substeps(static_cast<u32>(cfg.m_substeps)),
            DeltaTime(static_cast<f32>(cfg.m_deltaTime)),
            SubstepDelta(static_cast<f32>(impl.substepDelta)),
            Gravity(0.0f, static_cast<f32>(impl.gravity1d)),
            GravityPosition(0.0f, static_cast<f32>(impl.gravity1dPosition)),
            CircleRestitution(static_cast<f32>(-(1.0L + cfg.m_circleRestitution)) * 0.5f),
            WorldRestitution(static_cast<f32>(0.5L * cfg.m_worldRestitution)),
            ResponseCoef(0.25f * static_cast<f32>(cfg.m_responseCoef)),
            CircleArea(M_PI * (Radius * Radius)),
            CircleAreaDensity(1.0f),
            CircleMass(CircleArea * CircleAreaDensity),
            CircleHalfMass(CircleMass * 0.5f)
        {
        }
    };

    // Can be implemented as specialized static members with cxx20
    //    template <Definition Def>
    //    struct TemplTest
    //    {
    //        static constexpr f32 ParticleRadius = Def.radius;
    //    };

    using Collidex = Tuplex<const Collider::Union>;

    template <class... Cs>
    struct Definition
    {
        static constexpr size_t                    N = Collidex::Count<Cs...>();
        const std::array<const Collider::Union, N> data;

        constexpr explicit Definition(Cs... cs) : data(Collidex::Array(cs...)) { }

        template <class C>
        static inline constexpr size_t count()
        {
            return Collidex::CountSpecific<C, Cs...>();
        }

        template <class C>
        constexpr inline auto all() const
        {
            assert_collider<C>();
            constexpr size_t n = count<C>();
            static_assert(n > 0, "No such colliders are defined");
            return M_fetch<C>(std::make_index_sequence<n>());
        }

    private:
        template <class C>
        static constexpr inline void assert_collider()
        {
            static_assert(std::is_convertible<C, Collider::Union>::value, "Type is not a collider.");
        }

        template <class C, size_t... Indices>
        [[nodiscard]] constexpr size_t M_count(std::index_sequence<Indices...>) const
        {
            return (0 + ... + (is_same<C>(std::get<Indices>(data))));
            //            return ((count_same(std::get<Indices>(data))) + ...);
            //            return (0 + ... + count_same<C>(std::get(data, Indices)));
        }

        template <class C>
        static constexpr bool is_same(Collider::Union const &uc)
        {
            return (C::Which == uc.which);
        }

        template <class C, size_t... Indices>
        constexpr auto M_fetch(std::index_sequence<Indices...>) const
        {
            return std::array<const C * const, sizeof...(Indices)> {(M_fetchOne<C, Indices>(0, 0))...};
        }

        // TODO: O(n log N) lookup, maybe fix later.
        template <class C, size_t toFind>
        constexpr auto M_fetchOne(const size_t index, const size_t found) const
        {
            if (is_same<C>(data[index]))
            {
                if (found >= toFind) { return data[index].template as_ptr<C>(); }
                else
                    return M_fetchOne<C, toFind>(index + 1, found + 1);
            }
            else
                return M_fetchOne<C, toFind>(index + 1, found);
        }
    };

    struct UnitTest
    {
        static constexpr Settings def {Width(5), Radius(5)};
        static constexpr Number   w = def.Width;
        static constexpr f32      r = def.Radius;

        static constexpr Radius rr = Radius(5.0);
        static constexpr Number rx = rr;

        static constexpr Settings def2 {Width(1500)};
        static constexpr Number   r2 = def2.MaxParticles;
        static constexpr Number   w2 = def2.Width;

        static constexpr Radius runit0 {5.0};
        static constexpr Radius runit1   = Radius(5.0);
        static constexpr Radius runit2   = Width(5.0);         // should be an error
        static constexpr Radius runit3   = (Radius) Width(50); // should be fine
        static constexpr f32    runit0_0 = runit0;             // should not warn abt narrowing-conversion
    };
} // namespace PLSC

void test_function()
{
    //    constexpr PLSC::Settings cfg {PLSC::Radius(15), PLSC::Width(1280), PLSC::Height(720)};
    static constexpr PLSC::Settings   cfg;
    static constexpr PLSC::Definition def(
        []()
        {
            constexpr f32 BinSize   = cfg.Radius * 8.0f;
            constexpr f32 BinWidth  = cfg.Radius;
            constexpr f32 BinHeight = cfg.Radius * 0.3f;

            constexpr f32  BinIncr = BinSize + BinWidth;
            constexpr auto N       = static_cast<size_t>((cfg.Width - (BinIncr)) / BinIncr);
            return PLSC::Iterate<N>(
                [](size_t i)
                {
                    f32 cx = BinIncr + (BinIncr * static_cast<f32>(i));
                    f32 x0 = cx;
                    f32 x1 = cx + BinWidth;
                    f32 y0 = cfg.Height - BinHeight;
                    f32 y1 = cfg.Height;
                    return PLSC::AABB(x0, y0, x1, y1);
                });
        }(),
        PLSC::InvBB(0, 0, cfg.Width, cfg.Height));

    constexpr auto caabb   = def.count<PLSC::AABB>();
    constexpr auto ccirc   = def.count<PLSC::Circle>();
    constexpr auto cinvb   = def.count<PLSC::InvBB>();
    constexpr auto carr2   = def.data;
    constexpr auto cwhich2 = carr2[0].which;

    constexpr auto all_aabb  = def.all<PLSC::AABB>();
    constexpr auto all_invbb = def.all<PLSC::InvBB>();

    constexpr auto c = all_invbb[0]->C.x;

    const PLSC::Settings   cfg_rt;
    const PLSC::Definition def_rt(
        //        [&]()
        //        {
        //            f32 BinSize   = cfg_rt.Radius * 8.0f;
        //            f32 BinWidth  = cfg_rt.Radius;
        //            f32 BinHeight = cfg_rt.Radius * 0.3f;
        //
        //            f32        BinIncr = BinSize + BinWidth;
        //            const auto N       = static_cast<size_t>((cfg_rt.Width - (BinIncr)) / BinIncr);
        //            return PLSC::Iterate(N,
        //                                 [&](size_t i)
        //                                 {
        //                                     f32 cx = BinIncr + (BinIncr * static_cast<f32>(i));
        //                                     f32 x0 = cx;
        //                                     f32 x1 = cx + BinWidth;
        //                                     f32 y0 = cfg.Height - BinHeight;
        //                                     f32 y1 = cfg.Height;
        //                                     return PLSC::AABB(x0, y0, x1, y1);
        //                                 });
        //        }(),
        PLSC::InvBB(0, 0, cfg.Width, cfg.Height), PLSC::InvBB(0, 0, cfg.Width, cfg.Height));

    auto       caabb2   = def_rt.count<PLSC::InvBB>();
    const auto all_inv2 = def_rt.all<PLSC::InvBB>();
}
