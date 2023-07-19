#pragma once

// #include "Functional.hpp"
#include "PLSC/Math/Util.hpp"
#include "PLSC/Math/vec2.hpp"
#include "Typedefs.hpp"

// #include <cfloat>
// #include <cmath>
// #include <type_traits>

namespace PCFG
{
    using namespace PLSC;

    struct Value
    {
        const Number x;
        Value() = delete;

        template <typename T>
        constexpr Value(T x) : x(static_cast<Number>(x))
        {
            static_assert(std::is_arithmetic_v<T>);
        }
    };

    struct SettingDummy
    {
    };

    struct SettingsDummy
    {
    };

    // Trait for one specific setting
    template <class T>
    concept Setting = std::is_base_of<SettingDummy, T>::value;

    // Trait for the settings interface
    template <class T>
    concept Settings = std::is_base_of_v<SettingsDummy, T>;

#define mk_setting(NAME, TYPE, DEFAULT)                                                                      \
    namespace Impl                                                                                           \
    {                                                                                                        \
        struct NAME                                                                                          \
        {                                                                                                    \
            using Type                      = TYPE;                                                          \
            static constexpr Type m_default = DEFAULT;                                                       \
        };                                                                                                   \
    }                                                                                                        \
    template <Value value>                                                                                   \
    struct NAME : SettingDummy, Impl::NAME                                                                   \
    {                                                                                                        \
        static constexpr Number m_value = value.x;                                                           \
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
    } // namespace Impl
} // namespace PCFG

namespace PLSC
{
    template <PCFG::Setting... Setting>
    struct Settings : PCFG::SettingsDummy
    {
    private:
        /// Setting::m_value if Setting : Base in (class... Setting), otherwise Base::m_default
        template <class Base>
        static constexpr auto Get_OrDefault()
        {
            return PCFG::Impl::GetDefault<Base, Setting...>::value;
        }

#define M_GetSetting(NAME) Get_OrDefault<PCFG::Impl::NAME>()

        static constexpr auto UserRadius = M_GetSetting(Radius);
        static constexpr auto m_Scale    = 0.5 / UserRadius;

    public:
        static constexpr f32 Radius     = 0.5f;
        static constexpr f32 Diameter   = 1.0f;
        static constexpr f32 DiameterSq = 1.0f;

        static constexpr f32 Width        = roundexpr::round(m_Scale * M_GetSetting(Width));
        static constexpr f32 Height       = roundexpr::round(m_Scale * M_GetSetting(Height));
        static constexpr u32 Particles    = M_GetSetting(Particles);
        static constexpr u32 Substeps     = M_GetSetting(Substeps);
        static constexpr f32 DeltaTime    = 1.0 / M_GetSetting(Framerate);
        static constexpr f32 ResponseCoef = 0.25f * M_GetSetting(ResponseCoef);

        static constexpr f32  SubstepDelta = DeltaTime / f32(Substeps);
        static constexpr f32  Gravity1d    = m_Scale * M_GetSetting(Gravity1d);
        static constexpr f32  Gravity1dP   = Gravity1d * (SubstepDelta * SubstepDelta);
        static constexpr vec2 GravityP     = vec2(0.0f, Gravity1dP);

        static constexpr f32 CircleXMin = Radius;
        static constexpr f32 CircleYMin = Radius;
        static constexpr f32 CircleXMax = Width - Radius;
        static constexpr f32 CircleYMax = Height - Radius;

        static constexpr u32 ParticlesPerWidth  = (u32) (Width / Diameter) - 1;
        static constexpr u32 ParticlesPerHeight = (u32) (Height / Diameter) - 1;

        static constexpr f32 ParticleArea        = M_PI * (Radius * Radius);
        static constexpr f32 ParticleAreaDensity = 1.0f; // TODO: Update if we add units.
        static constexpr f32 ParticleMass        = ParticleArea * ParticleAreaDensity;
        static constexpr f32 ParticleHalfMass    = ParticleMass * 0.5f;
#undef M_GetSetting
    };

    template <PCFG::Settings CFG>
    struct TestImpl
    {
        static constexpr auto rad = CFG::Particles;
    };

} // namespace PLSC

using Set = PLSC::Settings<PCFG::Radius<14.1>, PCFG::Width<720>, PCFG::Height<1280>>;

static constexpr auto r = Set::Radius;
static constexpr auto w = Set::Width;

static constexpr PLSC::Settings s2;
static constexpr auto           ww = decltype(s2)::Width;

static constexpr PLSC::TestImpl<Set> tt;
static constexpr auto                rr = tt.rad;
