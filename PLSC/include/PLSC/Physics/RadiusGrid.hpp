#pragma once

#include "Collider.hpp"
// #include "PLSC/Constants.hpp"
#include "PLSC/DBG/Profile.hpp"
#include "PLSC/Definition.hpp"
#include "PLSC/Physics/Collision.hpp"
#include "PLSC/Physics/Particle.hpp"
#include "PLSC/Typedefs.hpp"

#include <array>
#include <bitset>
#include <cmath>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#define RADIUSGRID_ROWCOL_ORDER 1 // 0 = ROW, 1 = COL
namespace PLSC
{
    using id_t = u32;

    template <PCFG::Settings CFG, PCFG::Definition DEF>
    class RadiusGrid
    {
    public:
        //-- Constants
        static constexpr id_t BfrSize   = 8;
        static constexpr f32  fBfrSize  = static_cast<f32>(BfrSize);
        static constexpr f32  fBfrSize2 = fBfrSize * 2.0f;

        static constexpr id_t XSize = roundexpr::ceil<id_t>(CFG::Width * 2.0f) + (BfrSize * 4);
        static constexpr id_t YSize = roundexpr::ceil<id_t>(CFG::Height * 2.0f) + (BfrSize * 4);
        static constexpr id_t NSize = XSize * YSize;

        static constexpr size_t NCol = std::tuple_size_v<decltype(DEF::Tuple)>;

        static constexpr vec2 LeftUp    = {-CFG::Radius, -CFG::Radius};
        static constexpr vec2 LeftDown  = {-CFG::Radius, CFG::Radius};
        static constexpr vec2 RightDown = {CFG::Radius, CFG::Radius};
        static constexpr vec2 RightUp   = {CFG::Radius, -CFG::Radius};

        using Intersects    = std::bitset<NCol>;
        using Intersections = std::array<Intersects, NSize>;

        using Composite  = Fun<Particle<CFG> *>;
        using Collisions = std::array<Composite, NSize>;

    public:
        explicit RadiusGrid(Particle<CFG> * const objects) : m_objects(objects) { }

        template <auto a, auto b>
        struct Zip
        {
            static constexpr auto A = a;
            static constexpr auto B = b;
        };

        template <Zip... zips>
        struct ComposeIf
        {
        };

        template <Zip zip, Zip... zips>
        struct ComposeIf<zip, zips...>
        {
            static constexpr void F(Particle<CFG> * p)
            {
                if constexpr (zip.a) zip.b(p);
                ComposeIf<zips...>::F(p);
            }
        };

        template <Zip zip>
        struct ComposeIf<zip>
        {
            static constexpr void F(Particle<CFG> * p)
            {
                if constexpr (zip.a) zip.b(p);
            }
        };

        template <size_t... Inds>
        static constexpr Composite ComposeCollision(Intersects map, std::index_sequence<Inds...>)
        {
            if constexpr (!map) return Nop<Particle<CFG> *>;
            return ComposeIf<(Zip<std::get<Inds>(map), std::get<Inds>(DEF::Tuple)>(), ...)>::F;
        }

        template <size_t... HashInds>
        static constexpr Collisions ComposeColliders(Intersections const &bitmaps,
                                                     std::index_sequence<HashInds...>)
        {
            return {((ComposeCollision(std::get<HashInds>(bitmaps), std::make_index_sequence<NCol>())), ...)};
        }

        static consteval auto MkBitmaps()
        {
            std::array<std::bitset<NCol>, NSize> bitmaps {0};

            Particle<CFG> test_ob;

            for (u32 x = 0; x <= XSize; ++x)
            {
                for (u32 y = 0; y <= YSize; ++y)
                {
                    const f32 fx = static_cast<float>(x) * 0.5f - fBfrSize;
                    const f32 fy = static_cast<float>(y) * 0.5f - fBfrSize;
                    test_ob.P.x  = fx;
                    test_ob.P.y  = fy;

                    constexpr std::bitset<NCol> bucket
                        = TestBucket(test_ob, std::make_index_sequence<NCol>());

                    id_t xmin = std::max((i32) x - 1, 0);
                    id_t ymin = std::max((i32) y - 1, 0);
                    id_t xmax = std::min(x + 1, XSize - 1);
                    id_t ymax = std::min(y + 1, YSize - 1);
                    id_t i00  = hash(xmin, ymin); // - -
                    id_t i01  = hash(xmin, ymax); // - +
                    id_t i10  = hash(xmax, ymin); // + -
                    id_t i11  = hash(xmax, ymax); // + +
                    bitmaps[i00] |= bucket;
                    bitmaps[i01] |= bucket;
                    bitmaps[i10] |= bucket;
                    bitmaps[i11] |= bucket;
                }
            }

            return bitmaps;
        }
        template <size_t... Inds>
        static consteval std::array<Fun<Particle<CFG> *>, NSize> MkStatic(std::index_sequence<Inds...>)
        {
            //- Build grid of static colliders:
            //- Run a particle through every corner of the grid tiles, for every collider which
            //- intersects the particle at the corner, add collider to tiles sharing this corner

            // If we have no static colliders
            if constexpr (NCol == 0) return std::array<Fun<Particle<CFG> *>, NSize> {Nop<Particle<CFG> *>};

            // For each bin, find which colliders intersect a particle at any corner
            // Intersections bitmaps = MakeBitmaps(std::make_index_sequence<NSize>());

            return std::array<Fun<Particle<CFG> *>, NSize> {Nop<Particle<CFG> *>};
            // For each bin, compose Collide() functions of every collider found true in bitmap
            // return ComposeColliders(bitmaps, std::make_index_sequence<NSize>());
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
        static constexpr std::array<Fun<Particle<CFG> *>, NSize> m_aStatic = MkStatic();
        // static constexpr std::array<std::bitset<NCol>, NSize>    m_bitmaps = MkBitmaps();

    private:
        //-- Member data
        //        std::array<Particle, Constants::MaxDynamicInstances> m_objects;
        Particle<CFG> * const            m_objects;
        std::array<id_t, NSize + 1>      m_aDynamicLUT {0};
        std::array<id_t, CFG::Particles> m_aDynamicGrid {0};

        //-- Min/max bounds used in reconstruct
        //        id_t m_uMinH = 0;
        //        id_t m_uMaxH = NSize;

#ifdef COUNT_COLLISION_PAIRS
        DBG::PairCounter<Constants::MaxDynamicInstances> m_dbgPairCounter;
#endif
        u32 m_uUpdates = 0;

        static constexpr id_t Ix(const f32 x)
        {
#if FP_FAST_FMAF == 1
            return static_cast<id_t>(std::fmaf(x, 2.0f, fBfrSize2));
#else
            return static_cast<id_t>(x * 2.0f + fBfrSize2);
#endif
        }
        static constexpr id_t Iy(const f32 y)
        {
#if FP_FAST_FMAF == 1
            return static_cast<id_t>(std::fmaf(y, 2.0f, fBfrSize2));
#else
            return static_cast<id_t>(y * 2.0f + fBfrSize2);
#endif
        }

        static constexpr id_t hash(const Particle<CFG> &ob)
        {
            const id_t ix = Ix(ob.P.x);
            const id_t iy = Iy(ob.P.y);
            return hash(ix, iy);
        }
        static constexpr id_t hash(const f32 x, const f32 y)
        {
            const id_t ix = Ix(x);
            const id_t iy = Iy(y);
            return hash(ix, iy);
        }

        static constexpr id_t hash(const id_t ix, const id_t iy)
        {
#if RADIUSGRID_ROWCOL_ORDER == 0
            return iy * XSize + ix;
#else // Column ordered
            return ix * YSize + iy;
#endif
        }

        static constexpr vec2 unhash(const id_t h)
        {
#if RADIUSGRID_ROWCOL_ORDER == 0
            const id_t ix = h % XSize;
            const id_t iy = h / XSize;
#else
            const id_t ix = h / YSize;
            const id_t iy = h % YSize;
#endif
            const f32 x = (static_cast<f32>(ix) - fBfrSize2) * 0.5f;
            const f32 y = (static_cast<f32>(iy) - fBfrSize2) * 0.5f;
            return {x, y};
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

                //- Collide static
                id_t h0 = hash(ob);
                m_aStatic[h0](&ob);

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

    private:
        //- Implementation for MakeBitmaps
        template <auto Collider>
        static constexpr bool TestCollider(const vec2 &center)
        {
            return Collision<CFG, Collider>::Intersects({center + LeftUp})
                   || Collision<CFG, Collider>::Intersects({center + LeftDown})
                   || Collision<CFG, Collider>::Intersects({center + RightDown})
                   || Collision<CFG, Collider>::Intersects({center + RightUp});
        }

        template <size_t Hash, size_t... ColliderInds>
        static constexpr std::bitset<NCol> TestBin(std::index_sequence<ColliderInds...>)
        {
            constexpr vec2 Center = unhash(Hash);
            return {((TestCollider<std::get<ColliderInds>(DEF::Tuple)>(Center)), ...)};
        }

        template <auto Collider>
        static constexpr bool IntersectsParticle(const Particle<CFG> &p)
        {
            return Collision<CFG, Collider>::Intersects(p);
        }
        template <size_t... ColliderInds>
        static constexpr std::bitset<NCol> TestBucket(const Particle<CFG> &p,
                                                      std::index_sequence<ColliderInds...>)
        {
            return {((IntersectsParticle<std::get<ColliderInds>(DEF::Tuple)>(p) << ColliderInds) | ...)};
        }

        template <size_t... HashInds>
        static constexpr std::array<std::bitset<NCol>, NSize> MakeBitmaps(std::index_sequence<HashInds...>)
        {
            return {(TestBin<HashInds>(std::make_index_sequence<NCol>()))...};
        }
    };

} // namespace PLSC
