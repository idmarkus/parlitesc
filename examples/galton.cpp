#include "PLSC.hpp"
#include "PLSC/DBG/Profile.hpp"
#include "PLSC/GL.hpp"
#include "Window.hpp"

#include <chrono>
#include <string>

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

    PLSC::Demo::Window          window(1280, 720, 50, 50, false);
    PLSC::Solver                solver;
    PLSC::GL::Renderer          staticRenderer;
    PLSC::GL::ParticleInstancer particleRenderer(&solver.m_objects[0]);

    auto bins = solver.m_static.Register(MkBins, NBins);
    staticRenderer.Register(bins);

    auto border = solver.m_static.Register(
        PLSC::Collider::InverseAABB(0, 0, PLSC::Constants::WorldWidth, PLSC::Constants::WorldHeight));
    staticRenderer.Register(border);

    solver.init();
    staticRenderer.init(window.width(), window.height());
    particleRenderer.init(window.width(), window.height());

    using namespace std::chrono;
    while (!window.keyQuit())
    {
        // Update screen size
        if (window.sizeChanged())
        {
            particleRenderer.setScreenSize(window.width(), window.height());
            staticRenderer.setScreenSize(window.width(), window.height());
        }
        auto t0 = high_resolution_clock::now();

        // Update physics
        solver.spawnRandom();
        solver.update();
        particleRenderer.updatePositions(solver.m_active);

        auto t1 = high_resolution_clock::now();

        // Update graphics
        window.clear();
        staticRenderer.draw();
        particleRenderer.draw();
        window.swap();

        auto t2 = high_resolution_clock::now();

        duration<double> dtPHX = t1 - t0;
        duration<double> dtGFX = t2 - t1;
        duration<double> dtFPS = t2 - t0;

        std::string fps = std::to_string(1.0 / dtFPS.count()).substr(0, 4) + "fps ";
        std::string gfx = std::to_string(dtGFX.count() * 1000.0) + "ms ";
        std::string phx = std::to_string(dtPHX.count() * 1000.0) + "ms ";
        //        std::string act = std::to_string(solver.m_active);

        window.setTitle("[PLSC] " + fps + gfx + phx);
    }

    PROFILE_OUTPUT();

    return 0;
}
