#include "PLSC/Physics/RadiusGrid.hpp"

#include "PLSC/Constants.hpp"
#include "PLSC/DBG/Profile.hpp"

#include <cmath>   // FP_FAST_FMAF, fmaf
#include <cstring> // memset
#include <iostream>

namespace PLSC
{
    //    RadiusGrid::RadiusGrid(std::array<Particle, Constants::MaxDynamicInstances> objects) :
    //    m_objects(objects)
    //    {
    //    }
    //    RadiusGrid::~RadiusGrid()
    //    {
    //        //        PrintSpatialStats(m_uCollideObjects, m_uCollideAttempt, m_uCollideSuccess);
    //    }
    void RadiusGrid::mkStatic(RadiusGrid::VCollider &v)
    {
        //- Build grid of static colliders:
        //- Run a particle through every corner of the grid tiles, for every collider which
        //- intersects the particle at the corner, add collider to tiles sharing this corner
        using namespace Constants;

        std::vector<std::vector<bool>> bitmaps(NSize, std::vector<bool>(v.size(), false));

        Particle test_ob;

        for (u32 x = 0; x <= XSize; ++x)
        {
            for (u32 y = 0; y <= YSize; ++y)
            {
                const f32 fx = static_cast<float>(x) * 0.5f - fBfrSize;
                const f32 fy = static_cast<float>(y) * 0.5f - fBfrSize;
                test_ob.P.x  = fx;
                test_ob.P.y  = fy;

                for (u32 i = 0; i < v.size(); ++i)
                {
                    if (v[i]->Intersects(&test_ob))
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

            for (id_t j = 0; j < v.size(); ++j)
            {
                if (bitmaps[i][j])
                {
                    m_vStaticGrid.push_back(v[j]);
                    ++count;
                }
            }
            if (count - m_aStaticLUT[i] > 1) ++more_cnt;
        }
        m_aStaticLUT[NSize] = count;
        std::cout << "Static collider grid size: " << m_vStaticGrid.size() << " (cells>1: " << more_cnt
                  << " [" << (long double) more_cnt / (long double) m_vStaticGrid.size() << "])\n";
    }

    inline id_t RadiusGrid::Ix(const f32 x) const
    {
#if FP_FAST_FMAF == 1
        return static_cast<id_t>(std::fmaf(x, 2.0f, fBfrSize2));
#else
        return static_cast<id_t>(x * 2.0f + fBfrSize2);
#endif
    }
    inline id_t RadiusGrid::Iy(const f32 y) const
    {
#if FP_FAST_FMAF == 1
        return static_cast<id_t>(std::fmaf(y, 2.0f, fBfrSize2));
#else
        return static_cast<id_t>(y * 2.0f + fBfrSize2);
#endif
    }

    //    inline id_t RadiusGrid::Ix_min(const f32 x) const
    //    {
    //        const f32 fx = std::max(x, Constants::CircleXMin);
    //        return static_cast<id_t>(fx);
    //    }
    //
    //    inline id_t RadiusGrid::Ix_max(const f32 x) const
    //    {
    //        const f32 fx = std::min(x, Constants::CircleXMax);
    //        return static_cast<id_t>(fx);
    //    }
    //
    //    inline id_t RadiusGrid::Iy_min(const f32 y) const
    //    {
    //        const f32 fy = std::max(y, Constants::CircleYMin);
    //        return static_cast<id_t>(fy);
    //    }
    //
    //    inline id_t RadiusGrid::Iy_max(const f32 y) const
    //    {
    //        const f32 fy = std::min(y, Constants::CircleYMax);
    //        return static_cast<id_t>(fy);
    //    }

    inline id_t RadiusGrid::hash(const Particle &ob) const
    {
        const id_t ix = Ix(ob.P.x);
        const id_t iy = Iy(ob.P.y);
        return hash(ix, iy);
    }

    inline id_t RadiusGrid::hash(const f32 x, const f32 y) const
    {
        const id_t ix = Ix(x);
        const id_t iy = Iy(y);
        return hash(ix, iy);
    }

    inline id_t RadiusGrid::hash(const id_t ix, const id_t iy) const
    {
#if RADIUSGRID_ROWCOL_ORDER == 0
        return iy * XSize + ix;
#else // Column ordered
        return ix * YSize + iy;
#endif
    }

    inline void RadiusGrid::reconstruct(const id_t active)
    {
        PROFILE();
        // Counting sort of flat positions, allowing O(n) collision testing at the cost of O(n+m) memory, plus
        // O(n) additional work (this function). Very fast, very single threaded and very confusingly scaled,
        // but works excellently for smaller numbers of particles, maybe < ~50000 or so.

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

    inline void RadiusGrid::collideSubset(const u32 start, const u32 end)
    {
        PROFILE_COMPLEXITY(end - start);
        //        m_uCollideObjects += (end - start);
        for (u32 grid_id = start; grid_id < end; ++grid_id)
        {
            const id_t &ob1_id = m_aDynamicGrid[grid_id];
            Particle &  ob     = m_objects[ob1_id];

            //- Collide static objects
            id_t h0 = hash(ob);
            for (id_t i = m_aStaticLUT[h0]; i < m_aStaticLUT[h0 + 1]; ++i)
            {
                m_vStaticGrid[i]->CollideFast(&ob);
            }

            // if (!ob.isAwake()) continue;
            id_t cell0 = m_aDynamicLUT[h0 - 2]; // std::min(grid_id, m_aDynamicLUT[h0-2]);
            for (; cell0 < grid_id; ++cell0)
            {
                const id_t       ob2_id = m_aDynamicGrid[cell0];
                Particle * const ob2    = &m_objects[ob2_id];

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
                h0 -= XSize;                        // h(x+i, y)
#else                                               // Column ordered
                if (h0 < YSize) break;
                h0 -= YSize;
#endif
                cell0      = m_aDynamicLUT[h0 - 2]; // h(x+i, y-2)
                id_t cell1 = m_aDynamicLUT[h0 + 3]; // std::min(grid_id, m_aDynamicLUT[h0+3]); // h(x+i, y+2)
                for (; cell0 < cell1; ++cell0)
                {
                    const id_t       ob2_id = m_aDynamicGrid[cell0];
                    Particle * const ob2    = &m_objects[ob2_id];
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

    void RadiusGrid::update(const u32 active)
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

} // namespace PLSC