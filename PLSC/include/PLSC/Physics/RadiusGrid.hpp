#pragma once

#include "Collider.hpp"
// #include "PLSC/Constants.hpp"
#include "PLSC/DBG/Profile.hpp"
#include "PLSC/Physics/Collision.hpp"
#include "PLSC/Typedefs.hpp"

#include <array>
#include <bitset>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#define RADIUSGRID_ROWCOL_ORDER 1 // 0 = ROW, 1 = COL
namespace PLSC
{
    using id_t = u32;

    template <PCFG::Settings CFG>
    class RadiusGrid
    {
    public:
        template <size_t N>
        explicit RadiusGrid(const std::array<const Collider::Union, N> &immobile, Particle<CFG> * objects) :
            m_objects(objects)
        {
            mkStatic(immobile);
        }

        template <size_t N>
        void mkStatic(const std::array<const Collider::Union, N> &colliders)
        {
            //- Build grid of static colliders:
            //- Run a particle through every corner of the grid tiles, for every collider which
            //- intersects the particle at the corner, add collider to tiles sharing this corner

            std::array<std::bitset<N>, NSize> bitmaps;
            // std::vector<std::vector<bool>> bitmaps(NSize, std::vector<bool>(v.size(), false));

            Particle<CFG> test_ob;

            for (u32 x = 0; x <= XSize; ++x)
            {
                for (u32 y = 0; y <= YSize; ++y)
                {
                    const f32 fx = static_cast<float>(x) * 0.5f - fBfrSize;
                    const f32 fy = static_cast<float>(y) * 0.5f - fBfrSize;
                    test_ob.P.x  = fx;
                    test_ob.P.y  = fy;

                    for (u32 i = 0; i < N; ++i)
                    {
                        if (IntersectsRT(colliders[i], test_ob))
                        {
                            id_t xmin       = std::max((i32) x - 1, 0);
                            id_t ymin       = std::max((i32) y - 1, 0);
                            id_t xmax       = std::min(x + 1, XSize - 1);
                            id_t ymax       = std::min(y + 1, YSize - 1);
                            id_t i00        = hash(xmin, ymin); // - -
                            id_t i01        = hash(xmin, ymax); // - +
                            id_t i10        = hash(xmax, ymin); // + -
                            id_t i11        = hash(xmax, ymax); // + +
                            bitmaps[i00][i] = true;
                            bitmaps[i01][i] = true;
                            bitmaps[i10][i] = true;
                            bitmaps[i11][i] = true;
                        }
                    }
                }
            }

            u32 count    = 0;
            u32 more_cnt = 0;
            for (u32 i = 0; i < NSize; ++i)
            {
                m_aStaticLUT[i] = count;

                for (id_t j = 0; j < N; ++j)
                {
                    if (bitmaps[i][j])
                    {
                        m_vStaticGrid.push_back(&colliders[j]);
                        ++count;
                    }
                }
                if (count - m_aStaticLUT[i] > 1) ++more_cnt;
            }
            m_aStaticLUT[NSize] = count;
            std::cout << "Static collider grid size: " << m_vStaticGrid.size() << " (cells>1: " << more_cnt
                      << " [" << (long double) more_cnt / (long double) m_vStaticGrid.size() << "])\n";
        }

        void update(const u32 active)
        {
//        m_uCollideObjects += active;
#ifdef COUNT_COLLISION_PAIRS
            m_dbgPairCounter.accumulate();
#endif
            // if (m_uUpdates % 6 == 0)
            reconstruct(active);

            collideSubset(0, active);
            ++m_uUpdates;
        }

    public:
        //-- Profiling data
        //        u64 m_uCollideObjects = 0;
        //        u64 m_uCollideAttempt = 0;
        //        u64 m_uCollideSuccess = 0;

        //-- Constants
        static constexpr id_t BfrSize   = 8;
        static constexpr f32  fBfrSize  = static_cast<f32>(BfrSize);
        static constexpr f32  fBfrSize2 = fBfrSize * 2.0f;

        static constexpr id_t XSize = roundexpr::ceil<id_t>(CFG::Width * 2.0f) + (BfrSize * 4);
        static constexpr id_t YSize = roundexpr::ceil<id_t>(CFG::Height * 2.0f) + (BfrSize * 4);
        static constexpr id_t NSize = XSize * YSize;

    private:
        //-- Member data
        //        std::array<Particle, Constants::MaxDynamicInstances> m_objects;
        Particle<CFG> * const       m_objects;
        std::array<id_t, NSize + 1> m_aDynamicLUT {0};
        std::array<id_t, NSize + 1> m_aStaticLUT {0};

        //-- Min/max bounds used in reconstruct
        //        id_t m_uMinH = 0;
        //        id_t m_uMaxH = NSize;

        std::array<id_t, CFG::Particles>     m_aDynamicGrid {0};
        std::vector<Collider::Union const *> m_vStaticGrid;

#ifdef COUNT_COLLISION_PAIRS
        DBG::PairCounter<Constants::MaxDynamicInstances> m_dbgPairCounter;
#endif
        u32 m_uUpdates = 0;

        static id_t Ix(const f32 x)
        {
#if FP_FAST_FMAF == 1
            return static_cast<id_t>(std::fmaf(x, 2.0f, fBfrSize2));
#else
            return static_cast<id_t>(x * 2.0f + fBfrSize2);
#endif
        }
        static id_t Iy(const f32 y)
        {
#if FP_FAST_FMAF == 1
            return static_cast<id_t>(std::fmaf(y, 2.0f, fBfrSize2));
#else
            return static_cast<id_t>(y * 2.0f + fBfrSize2);
#endif
        }

        id_t hash(const Particle<CFG> &ob) const
        {
            const id_t ix = Ix(ob.P.x);
            const id_t iy = Iy(ob.P.y);
            return hash(ix, iy);
        }
        id_t hash(const f32 x, const f32 y) const
        {
            const id_t ix = Ix(x);
            const id_t iy = Iy(y);
            return hash(ix, iy);
        }

        id_t hash(const id_t ix, const id_t iy) const
        {
#if RADIUSGRID_ROWCOL_ORDER == 0
            return iy * XSize + ix;
#else // Column ordered
            return ix * YSize + iy;
#endif
        }

        void reconstruct(const id_t active)
        {
            PROFILE();
            // Counting sort of flat positions, allowing O(n) collision testing at the cost of O(n+m) memory,
            // plus O(n) additional work (this function). Very fast, very single threaded and very confusingly
            // scaled, but works excellently for smaller numbers of particles, maybe < ~50000 or so.

            // Zero out previous indices
            //        memset(m_aDynamicLUT.data(), 0, sizeof(id_t) * m_aDynamicLUT.size());
            m_aDynamicLUT.fill(0);

            // Count objects in each cell
            for (u32 i = 0; i < active; ++i)
            {
                const id_t h = hash(m_objects[i]);
                ++m_aDynamicLUT[h];
            }

            // Compute partial sum for cell starts
            id_t sum = 0;
            for (id_t i = 0; i <= NSize; ++i)
            {
                sum += m_aDynamicLUT[i];
                m_aDynamicLUT[i] = sum;
            }

            // Stage objects into dense grid
            for (id_t i = 0; i < active; ++i)
            {
                const id_t id   = hash(m_objects[i]);
                id_t &     cell = m_aDynamicLUT[id];
                --cell;
                m_aDynamicGrid[cell] = i;
            }
        }
        // void collideStatic(const u32, const u32);
        void collideSubset(const u32 start, const u32 end)
        {
            PROFILE_COMPLEXITY_NAMED("RadiusGrid::collideSubset", end - start);
            //         m_uCollideObjects += (end - start);
            for (u32 grid_id = start; grid_id < end; ++grid_id)
            {
                const id_t &   ob1_id = m_aDynamicGrid[grid_id];
                Particle<CFG> &ob     = m_objects[ob1_id];

                //- Collide static objects
                id_t h0 = hash(ob);
                for (id_t i = m_aStaticLUT[h0]; i < m_aStaticLUT[h0 + 1]; ++i)
                {
                    const Collider::Union &uc = *m_vStaticGrid[i];
                    CollideRT(uc, &ob);
                    //                Collision<uc.which, CFG>::CollideFast(uc, &ob);
                }

                // if (!ob.isAwake()) continue;
                id_t cell0 = m_aDynamicLUT[h0 - 2]; // std::min(grid_id, m_aDynamicLUT[h0-2]);
                for (; cell0 < grid_id; ++cell0)
                {
                    const id_t            ob2_id = m_aDynamicGrid[cell0];
                    Particle<CFG> * const ob2    = &m_objects[ob2_id];

                    //                ++m_uCollideAttempt;
                    //                m_uCollideSuccess += ob.CollideFast(ob2);
                    ob.CollideFast(ob2);
#ifdef COUNT_COLLISION_PAIRS
                    m_dbgPairCounter.add(ob1_id, ob2_id);
#endif
                }

                for (id_t i = 0; i < 2; ++i)
                {
#if RADIUSGRID_ROWCOL_ORDER == 0
                    if (h0 < XSize) break;
                    h0 -= XSize;                   // h(x+i, y)
#else                                              // Column ordered
                    if (h0 < YSize) break;
                    h0 -= YSize;
#endif
                    cell0 = m_aDynamicLUT[h0 - 2]; // h(x+i, y-2)
                    id_t cell1
                        = m_aDynamicLUT[h0 + 3];   // std::min(grid_id, m_aDynamicLUT[h0+3]); // h(x+i, y+2)
                    for (; cell0 < cell1; ++cell0)
                    {
                        const id_t            ob2_id = m_aDynamicGrid[cell0];
                        Particle<CFG> * const ob2    = &m_objects[ob2_id];
                        //                    ++m_uCollideAttempt;
                        //                    m_uCollideSuccess += ob.CollideFast(ob2);
                        ob.CollideFast(ob2);
#ifdef COUNT_COLLISION_PAIRS
                        m_dbgPairCounter.add(ob1_id, ob2_id);
#endif
                    }
                }
            }
        }
    };

} // namespace PLSC
