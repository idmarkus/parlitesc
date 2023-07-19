#pragma once

#include "PLSC/Typedefs.hpp"
#include "ShaderSources.hpp"

#include <string>
#include <vector>

namespace PLSC::GL
{
    //:: Define renderable shape definitions using
    //:: template specializations of this static struct.
    //// Shapes can then be used in templated functions:
    ////   shader.load(ShapeDefinition<T>::VShaderFilename) etc.
    template <typename T>
    struct ShapeDefinition
    {
        static std::string      VShaderSource();
        static std::string      FShaderSource();
        static std::vector<f32> vertices(T &);
        static std::vector<u32> indices(T &);
    };

//:: These are just macros to make the specializations //"not messy"//
//:: Vertices/indices defaults are:
////  Rectangle covering screen in clip space: [-1,1]
////  Triangle indices of any rectangle / 4 vertices
#define SDF_(TYPE, VSHADER, FSHADER)                                                                         \
    template <>                                                                                              \
    struct ShapeDefinition<PLSC::TYPE>                                                                       \
    {                                                                                                        \
        static std::string      VShaderSource() { return VSHADER; };                                         \
        static std::string      FShaderSource() { return FSHADER; };                                         \
        static std::vector<f32> vertices(PLSC::TYPE &);                                                      \
        static std::vector<u32> indices(PLSC::TYPE &);                                                       \
    }
#define VFN_(TYPE) std::vector<f32> ShapeDefinition<PLSC::TYPE>::vertices(PLSC::TYPE &o)
#define IFN_(TYPE) std::vector<u32> ShapeDefinition<PLSC::TYPE>::indices(PLSC::TYPE &o)
#define _VDefault_                                                                                           \
    (void) o;                                                                                                \
    return {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};
#define _IDefault_                                                                                           \
    (void) o;                                                                                                \
    return {0, 1, 2, 0, 2, 3};

    // ----- DEFINITIONS BEGIN HERE -----
    // ----- Collider::AABB
    SDF_(AABB, vertRect, fragRect);
    VFN_(AABB)
    {
        auto [min, max] = o.draw_minmax();
        return {min.x, max.y, max.x, max.y, max.x, min.y, min.x, min.y};
    }
    IFN_(AABB) {_IDefault_}

    // ----- Collider::InverseAABB
    SDF_(InvBB, vertRect, fragRect);
    VFN_(InvBB)
    {
        auto [min, max] = o.draw_minmax();
        f32 border      = 1.0f;
        f32 x1          = min.x;
        f32 y1          = min.y;
        f32 x2          = max.x;
        f32 y2          = max.y;
        f32 x0          = x1 - border;
        f32 y0          = y1 - border;
        f32 x3          = x2 + border;
        f32 y3          = y2 + border;

        return {
            /*0*/ x0, y0, /*1*/ x0, y3, /*2*/ x3, y3, /*3*/ x3, y0, // Outer
            /*4*/ x1, y1, /*5*/ x1, y2, /*6*/ x2, y2, /*7*/ x2, y1, // Inner
        };
    }
    IFN_(InvBB)
    {
        (void) o; // Squelch Wunused-parameter
        return {
            0, 4, 1, /**/ 4, 1, 5, /**/ 1, 5, 2, /**/ 5, 2, 6,
            2, 6, 3, /**/ 6, 3, 7, /**/ 3, 7, 0, /**/ 7, 0, 4,
        };
    }

    // ----- Collider::Circle
    SDF_(Circle, vertCircle, fragCircle);
    VFN_(Circle) {_VDefault_} IFN_(Circle) { _IDefault_ }

#undef _SDF
#undef _VFN
#undef _IFN
#undef _VDefault
#undef _IDefault
} // namespace PLSC::GL