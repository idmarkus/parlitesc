#pragma once

#include "PLSC/Functional.hpp"
#include "PLSC/Typedefs.hpp"

// Inline asm is not supported on MSVCx64
#if defined(_MSC_VER) && !defined(__clang__)
    #define PLSC_ASM 0
#else
    #define PLSC_ASM 1
#endif

namespace PLSC
{
#if !PLSC_ASM
    #include <xmmintrin.h>
#endif
    inline float rsqrt_fast(float x)
    {
#if PLSC_ASM // Optimal
        float result;
        asm( // Note AT&T order
    #ifdef __AVX__
            "vrsqrtss %1, %1, %0"
    #else
            "rsqrtss %1, %0"
    #endif
            : "=x"(result)
            : "x"(x));
        return result;
#else // TODO: not optimal when in AVX mode or when not inlined
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ps1(x)));
#endif
    }

    template <typename T>
    inline constexpr T clamp(T x, const T min, const T max)
    {
        const T t = x < min ? min : x;
        return t > max ? max : t;
    }

    /**
     * @roundexpr Constexpr rounding functions
     */
    namespace roundexpr
    {
        /**
         * @ceil Follows ceil(), intmax(x) if x is whole, otherwise smallest int greater than x (always up)
         */
        template <typename Floating>
        static constexpr inline intmax_t ceil(const Floating x)
        {
            return (static_cast<Floating>(static_cast<intmax_t>(x)) == x)
                       ? static_cast<intmax_t>(x)
                       : static_cast<intmax_t>(x) + ((x > 0) ? 1 : 0);
        }

        /**
         * @ceilabs ceil() but always away from zero -- abs(ceilabs(-x)) == abs(ceilabs(x))
         */
        template <typename Floating, EnableIf::Floating<Floating> = true>
        static constexpr inline intmax_t ceilabs(Floating x)
        {
            return (static_cast<Floating>(static_cast<intmax_t>(x)) == x)
                       ? static_cast<intmax_t>(x)
                       : static_cast<intmax_t>(x) + ((x > 0) ? 1 : -1);
        }

        /**
         * @floor Constexpr floor()
         */
        template <typename Floating, EnableIf::Floating<Floating> = true>
        static constexpr inline intmax_t floor(const Floating x)
        {
            return (static_cast<Floating>(static_cast<intmax_t>(x)) == x)
                       ? static_cast<intmax_t>(x)
                       : static_cast<intmax_t>(x) - ((x < 0) ? 1 : 0);
        }

        /**
         * @floorabs floor() but always toward zero -- like an int cast.
         */
        template <typename Floating, EnableIf::Floating<Floating> = true>
        static constexpr inline intmax_t floorabs(const Floating x)
        {
            return static_cast<intmax_t>(x);
        }

        template <typename Floating, EnableIf::Floating<Floating> = true>
        static constexpr inline Floating round(const Floating x)
        {
            return (x > 0.0L) ? static_cast<Floating>(static_cast<uintmax_t>(x + 0.5L))
                              : -(static_cast<Floating>(static_cast<uintmax_t>((-x) + 0.5L)));
        }
    } // namespace roundexpr

} // namespace PLSC