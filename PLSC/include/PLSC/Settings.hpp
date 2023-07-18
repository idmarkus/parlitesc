#pragma once

#include "PLSC/Math/Util.hpp"
#include "PLSC/Math/vec2.hpp"
#include "Typedefs.hpp"

#include <cfloat>
#include <cmath>
#include <type_traits>

namespace PLSC
{

    namespace Impl
    {
        template <class X, class... Ys>
        struct GetDefault
        {
        };

        template <class X, class Y, class... Ys>
        struct GetDefault<X, Y, Ys...>
        {
            static constexpr auto value
                = (std::is_base_of<X, Y>::value) ? Y::m_value : GetDefault<X, Ys...>::value;
        };

        template <class X, class Y>
        struct GetDefault<X, Y>
        {
            static constexpr auto value = (std::is_base_of<X, Y>::value) ? Y::m_value : X::m_default;
        };

        template <class X>
        struct GetDefault<X>
        {
            static constexpr auto value = X::m_default;
        };

        static constexpr auto Expand(Number x)
        {
            auto   whole = static_cast<intmax_t>(x);
            Number dec   = std::abs(x - static_cast<Number>(whole));
            for (int i = 0; i < std::numeric_limits<uintmax_t>::digits10; ++i) { dec *= 10.0; }

            return std::make_pair(whole, static_cast<uintmax_t>(dec));
        }

        static constexpr Number Implode(std::pair<intmax_t, uintmax_t> exp)
        {
            auto whole = static_cast<Number>(exp.first);
            auto dec   = static_cast<Number>(exp.second);
            for (int i = 0; i < std::numeric_limits<uintmax_t>::digits10; ++i) { dec /= 10.0; }
            return whole + std::copysign(dec, whole);
        }

        template <typename T>
        static constexpr T Implode(intmax_t a, uintmax_t b)
        {
            return static_cast<T>(Implode(std::make_pair(a, b)));
        }

#define mk_setting(NAME, TYPE, DEFAULT)                                                                      \
    struct NAME                                                                                              \
    {                                                                                                        \
        using Type                      = TYPE;                                                              \
        static constexpr TYPE m_default = DEFAULT;                                                           \
    };                                                                                                       \
    template <intmax_t Whole, uintmax_t Decimal>                                                             \
    struct User##NAME : NAME                                                                                 \
    {                                                                                                        \
        static constexpr TYPE m_value = Implode<TYPE>(Whole, Decimal);                                       \
    };

        mk_setting(Radius, f32, 0.0035);
        mk_setting(Width, f32, 1.5);
        mk_setting(Height, f32, 1.0);
        mk_setting(Framerate, f32, 60);
        mk_setting(Gravity1d, f32, 1.3);
        mk_setting(ResponseCoef, f32, 1.0);
        mk_setting(Particles, u32, 1000);
        mk_setting(Substeps, u32, 12);

#undef mk_setting
    } // namespace Impl

    template <class... Setting>
    struct Settings
    {
    private:
        /// Get Setting if setting is in Setting..., otherwise get default for Setting
        /// @return X::m_value if X : S in Xs, else S::m_default
        template <class S>
        static constexpr auto Get()
        {
            return Impl::GetDefault<S, Setting...>::value;
        }

        static constexpr auto UserRadius = Get<Impl::Radius>();
        static constexpr auto UserScale  = 0.5 / UserRadius;

    public:
        static constexpr f32 Radius     = 0.5f;
        static constexpr f32 Diameter   = 1.0f;
        static constexpr f32 DiameterSq = 1.0f;

        static constexpr f32 Width  = roundexpr::round(UserScale * Get<Impl::Width>());
        static constexpr f32 Height = roundexpr::round(UserScale * Get<Impl::Height>());

        static constexpr u32 Particles = Get<Impl::Particles>();
        static constexpr u32 Substeps  = Get<Impl::Substeps>();
        static constexpr f32 DeltaTime = 1.0 / Get<Impl::Framerate>();

        static constexpr f32 SubstepDelta = DeltaTime / f32(Substeps);
        static constexpr f32 Gravity1d    = UserScale * Get<Impl::Gravity1d>();
        static constexpr f32 Gravity1dP   = Gravity1d * (SubstepDelta * SubstepDelta);

        static constexpr f32 CircleXMin = Radius;
        static constexpr f32 CircleYMin = Radius;
        static constexpr f32 CircleXMax = Width - Radius;
        static constexpr f32 CircleYMax = Height - Radius;

        static constexpr u32 ParticlesPerWidth  = (u32) (Width / Diameter) - 1;
        static constexpr u32 ParticlesPerHeight = (u32) (Height / Diameter) - 1;

        static constexpr vec2 GravityP     = vec2(0.0f, Gravity1dP);
        static constexpr f32  ResponseCoef = 0.25f * Get<Impl::ResponseCoef>();

        static constexpr f32 ParticleArea        = M_PI * (Radius * Radius);
        static constexpr f32 ParticleAreaDensity = 1.0f; // TODO: Update if we add units.
        static constexpr f32 ParticleMass        = ParticleArea * ParticleAreaDensity;
        static constexpr f32 ParticleHalfMass    = ParticleMass * 0.5f;
    };
    //-- Decimal settings
#define SetRadius(VAL)                                                                                       \
    PLSC::Impl::UserRadius<PLSC::Impl::Expand((VAL)).first, PLSC::Impl::Expand((VAL)).second>
#define SetWidth(VAL)                                                                                        \
    PLSC::Impl::UserWidth<PLSC::Impl::Expand(((VAL))).first, PLSC::Impl::Expand((VAL)).second>
#define SetHeight(VAL)                                                                                       \
    PLSC::Impl::UserHeight<PLSC::Impl::Expand((VAL)).first, PLSC::Impl::Expand((VAL)).second>
#define SetFramerate(VAL)                                                                                    \
    PLSC::Impl::UserFramerate<PLSC::Impl::Expand((VAL)).first, PLSC::Impl::Expand((VAL)).second>
#define SetGravity(VAL)                                                                                      \
    PLSC::Impl::UserGravity1d<PLSC::Impl::Expand((VAL)).first, PLSC::Impl::Expand((VAL)).second>
#define SetResponseCoef(VAL)                                                                                 \
    PLSC::Impl::UserResponseCoef<PLSC::Impl::Expand((VAL)).first, PLSC::Impl::Expand((VAL)).second>
    //-- Integral settings
#define SetParticles(VAL) PLSC::Impl::UserParticles<intmax_t((VAL)), 0>
#define SetSubsteps(VAL)  PLSC::Impl::UserSubsteps<intmax_t((VAL)), 0>

    using Set = PLSC::Settings<SetRadius(14.155), SetWidth(720), SetHeight(1280), SetFramerate(30),
                               SetGravity(2), SetParticles(5000), SetSubsteps(4)>;

    static constexpr auto r = Set::Radius;
    static constexpr auto w = Set::Width;

    static constexpr Settings<> s2;
    static constexpr auto       r2 = decltype(s2)::Radius;

} // namespace PLSC