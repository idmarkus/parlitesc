#include "PLSC.hpp"

static constexpr f64 BinSize   = PLSC::Constants::CircleDiameter * 4.0f;
static constexpr f64 BinWidth  = PLSC::Constants::CircleRadius;
static constexpr f64 BinHeight = PLSC::Constants::WorldHeight * 0.3f;

static constexpr f64    BinIncr = BinSize + BinWidth;
static constexpr size_t NBins
    = static_cast<size_t>((PLSC::Constants::HIGHP::WorldWidth - (BinIncr)) / BinIncr);

static PLSC::Collider::AABB MkBins(size_t i)
{
    f32 cx = BinIncr + (BinIncr * static_cast<f32>(i));
    f32 x0 = cx;
    f32 x1 = cx + BinWidth;
    f32 y0 = PLSC::Constants::WorldHeight - BinHeight;
    f32 y1 = PLSC::Constants::WorldHeight;
    return PLSC::Collider::AABB(x0, y0, x1, y1);
}

int main(int argc, char ** argv)
{
    (void) argc;
    (void) argv;

    PLSC::Solver                solver;

    (void) solver.m_static.Register(MkBins, NBins);
    (void) solver.m_static.Register(
        PLSC::Collider::InverseAABB(0, 0, PLSC::Constants::WorldWidth, PLSC::Constants::WorldHeight));

    solver.init();
    bool flipGravity = false;
    for (int i = 0; i < 5000; ++i)
    {
        solver.spawnRandom();
        solver.update();

        f32 KE    = solver.getKE();
        f32 KEavg = (KE / (f32) solver.m_active) * 1000.0f;

        if (KEavg < 0.008f)
        {
            if (!flipGravity)
            {
                solver.m_gravity.y = -solver.m_gravity.y;
                flipGravity                        = true;
            }
        }
        else if (flipGravity)
            flipGravity = false;
    }

    return 0;
}