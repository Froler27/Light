#include "engine.h"

#include "core/base/macro.h"
#include "global/global_context.h"
#include "input/input_system.h"
#include "render/render_system.h"
#include "window_system.h"

namespace Light
{

    void Engine::startEngine(const std::string& config_file_path)
    {
        g_runtime_global_context.startSystems(config_file_path);

        LOG_INFO("engine start");
    }

    void Engine::shutdownEngine()
    {
        LOG_INFO("engine shutdown");

        g_runtime_global_context.shutdownSystems();
    }

    void Engine::initialize()
    {

    }

    void Engine::clear()
    {

    }

    void Engine::run()
    {
        std::shared_ptr<WindowSystem> window_system = g_runtime_global_context.m_window_system;
        ASSERT(window_system);

        while (!window_system->shouldClose())
        {
            const float delta_time = calculateDeltaTime();
            tickOneFrame(delta_time);
        }
    }

    void Engine::tickOneFrame(float delta_time)
    {
        logicalTick(delta_time);
        calculateFPS(delta_time);

        rendererTick(delta_time);

        g_runtime_global_context.m_window_system->pollEvents();
        g_runtime_global_context.m_window_system->swapBuffers();

        g_runtime_global_context.m_window_system->setTitle(
            std::string("Piccolo - " + std::to_string(getFPS()) + " FPS").c_str());
    }

    void Engine::logicalTick(float delta_time)
    {
        g_runtime_global_context.m_input_system->tick();
    }

    void Engine::rendererTick(float delta_time)
    {
        g_runtime_global_context.m_render_system->tick(delta_time);
    }

    const float Engine::s_fps_alpha = 1.f / 100;
    void Engine::calculateFPS(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1)
        {
            m_average_duration = delta_time;
        }
        else
        {
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }

    float Engine::calculateDeltaTime()
    {
        float delta_time;
        {
            using namespace std::chrono;

            steady_clock::time_point tick_time_point = steady_clock::now();
            duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
            delta_time = time_span.count();

            m_last_tick_time_point = tick_time_point;
        }
        return delta_time;
    }

}