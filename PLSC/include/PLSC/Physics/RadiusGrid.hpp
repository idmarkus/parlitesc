#pragma once

#include "Collider.hpp"
#include "PLSC/Constants.hpp"
#include "PLSC/Typedefs.hpp"

#include <memory>
#include <vector>
#include <array>

#define RADIUSGRID_ROWCOL_ORDER 1 // 0 = ROW, 1 = COL
namespace PLSC
{
    using id_t = u32;

    class RadiusGrid
    {
    public:
        using VCollider = std::vector<collider_ptr>;

        //        explicit RadiusGrid(std::array<Particle, Constants::MaxDynamicInstances>);
        explicit RadiusGrid(Particle * objects) : m_objects(objects) { }
        //            ~RadiusGrid();

        void update(u32);
        void mkStatic(VCollider &);

    public:
        //-- Profiling data
        //        u64 m_uCollideObjects = 0;
        //        u64 m_uCollideAttempt = 0;
        //        u64 m_uCollideSuccess = 0;

        //-- Constants
        static constexpr id_t BfrSize   = 8;
        static constexpr f32  fBfrSize  = static_cast<f32>(BfrSize);
        static constexpr f32  fBfrSize2 = fBfrSize * 2.0f;
        static constexpr id_t XSize
            = Constants::static_ceil<id_t>(Constants::WorldWidth * 2.0f) + (BfrSize * 4);
        static constexpr id_t YSize
            = Constants::static_ceil<id_t>(Constants::WorldHeight * 2.0f) + (BfrSize * 4);
        static constexpr id_t NSize = XSize * YSize;

    private:
        //-- Member data
        //        std::array<Particle, Constants::MaxDynamicInstances> m_objects;
        Particle *                  m_objects;
        std::array<id_t, NSize + 1> m_aDynamicLUT = {0};
        std::array<id_t, NSize + 1> m_aStaticLUT  = {0};

        //-- Min/max bounds used in reconstruct
        //        id_t m_uMinH = 0;
        //        id_t m_uMaxH = NSize;

        std::array<id_t, Constants::MaxDynamicInstances> m_aDynamicGrid = {0};
        VCollider                                        m_vStaticGrid;

#ifdef COUNT_COLLISION_PAIRS
        DBG::PairCounter<Constants::MaxDynamicInstances> m_dbgPairCounter;
#endif
        u32 m_uUpdates = 0;

        id_t Ix(f32) const;
        id_t Iy(f32) const;
        //        id_t Ix_min(f32) const;
        //        id_t Ix_max(f32) const;
        //        id_t Iy_min(f32) const;
        //        id_t Iy_max(f32) const;
        id_t hash(const Particle &) const;
        id_t hash(f32, f32) const;
        id_t hash(id_t, id_t) const;
        void reconstruct(id_t);
        // void collideStatic(const u32, const u32);
        void collideSubset(u32, u32);
    };

} // namespace PLSC
