#pragma once

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

} // namespace PLSC