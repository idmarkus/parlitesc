#pragma once

#include "PLSC/Typedefs.hpp"

#include <cmath>
#include <ostream>
#include <string>

namespace PLSC
{
    struct vec2
    {
        f32 x, y;

        constexpr vec2() noexcept : x(0.0f), y(0.0f) { }
        constexpr vec2(const f32 x, const f32 y) noexcept : x(x), y(y) { }

        inline constexpr void operator+=(const vec2 &v)
        {
            x += v.x;
            y += v.y;
        }
        inline constexpr void operator-=(const vec2 &v)
        {
            x -= v.x;
            y -= v.y;
        }
        inline constexpr void operator*=(const f32 &f)
        {
            x *= f;
            y *= f;
        }
        inline constexpr void operator*=(const vec2 &v)
        {
            x *= v.x;
            y *= v.y;
        }
        inline constexpr void operator/=(const f32 &f)
        {
            x /= f;
            y /= f;
        }
        inline constexpr void operator/=(const vec2 &v)
        {
            x /= v.x;
            y /= v.y;
        }

        inline constexpr f32 dot(const vec2 &v) const { return x * v.x + y * v.y; }
        inline constexpr f32 magSq() const { return std::fabs(x * x + y * y); }
        inline constexpr f32 mag() const { return std::sqrt(magSq()); }
        inline constexpr f32 distSq(const vec2 &v) const
        {
            const f32 dx = x - v.x;
            const f32 dy = y - v.y;
            return dx * dx + dy * dy;
        }

        friend std::ostream &operator<<(std::ostream &o, const vec2 &v)
        {
            return o << "(" << v.x << ", " << v.y << ")";
        }

        friend std::string to_string(vec2 const &v)
        {
            return '(' + std::to_string(v.x) + ',' + std::to_string(v.y) + ')';
        }
    };

    inline constexpr vec2 operator+(const vec2 &v1, const vec2 &v2) { return {v1.x + v2.x, v1.y + v2.y}; }
    inline constexpr vec2 operator-(const vec2 &v1, const vec2 &v2) { return {v1.x - v2.x, v1.y - v2.y}; }
    inline constexpr vec2 operator+(const vec2 &v, f32 f) { return {v.x + f, v.y + f}; }
    inline constexpr vec2 operator-(const vec2 &v, f32 f) { return {v.x - f, v.y - f}; }
    // inline constexpr vec2 operator+(const vec2 &v) { return {-v.x, -v.y}; }
    inline constexpr vec2 operator-(const vec2 &v) { return {-v.x, -v.y}; }
    inline constexpr vec2 operator*(f32 f, const vec2 &v) { return {v.x * f, v.y * f}; }
    inline constexpr vec2 operator*(const vec2 &v, f32 f) { return {v.x * f, v.y * f}; }
    inline constexpr vec2 operator*(const vec2 &v1, const vec2 &v2) { return {v1.x * v2.x, v1.y * v2.y}; }
    inline constexpr vec2 operator/(const vec2 &v, f32 f) { return {v.x / f, v.y / f}; }
    inline constexpr vec2 operator/(const vec2 &v1, const vec2 &v2) { return {v1.x / v2.x, v1.y / v2.y}; }
    inline constexpr f32  dot(const vec2 &v1, const vec2 &v2) { return v1.x * v2.x + v1.y * v2.y; }
    inline constexpr vec2 fabs(const vec2 &v) { return {std::fabs(v.x), std::fabs(v.y)}; }

} // namespace PLSC