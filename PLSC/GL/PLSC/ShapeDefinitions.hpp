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
    struct ShapeDefinition<PLSC::Collider::TYPE>                                                             \
    {                                                                                                        \
        static std::string      VShaderSource() { return VSHADER; };                                         \
        static std::string      FShaderSource() { return FSHADER; };                                         \
        static std::vector<f32> vertices(PLSC::Collider::TYPE &);                                            \
        static std::vector<u32> indices(PLSC::Collider::TYPE &);                                             \
    }
#define VFN_(TYPE) std::vector<f32> ShapeDefinition<PLSC::Collider::TYPE>::vertices(PLSC::Collider::TYPE &o)
#define IFN_(TYPE) std::vector<u32> ShapeDefinition<PLSC::Collider::TYPE>::indices(PLSC::Collider::TYPE &o)
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
        return {o.draw_minX(), o.draw_maxY(), o.draw_maxX(), o.draw_maxY(),
                o.draw_maxX(), o.draw_minY(), o.draw_minX(), o.draw_minY()};
    }
    IFN_(AABB) {_IDefault_}

    // ----- Collider::InverseAABB
    SDF_(InverseAABB, vertRect, fragRect);
    VFN_(InverseAABB)
    {
        f32 border = 1.0f;
        f32 x1     = o.draw_minX();
        f32 y1     = o.draw_minY();
        f32 x2     = o.draw_maxX();
        f32 y2     = o.draw_maxY();
        f32 x0     = x1 - border;
        f32 y0     = y1 - border;
        f32 x3     = x2 + border;
        f32 y3     = y2 + border;

        return {
            /*0*/ x0, y0, /*1*/ x0, y3, /*2*/ x3, y3, /*3*/ x3, y0, // Outer
            /*4*/ x1, y1, /*5*/ x1, y2, /*6*/ x2, y2, /*7*/ x2, y1, // Inner
        };
    }
    IFN_(InverseAABB)
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