#include "renderer.h"

#include <iostream>
#include <string>

#include "window_system.h"

namespace Light
{
    void Renderer::run()
    {
        initialize();
        while (!m_windowSystem->shouldClose()) {
            auto delta_time = calculateDeltaTime();
            calculateFPS(delta_time);
            
            logicalTick(delta_time);
            rendererTick(delta_time);

            m_windowSystem->pollEvents();
            m_windowSystem->swapBuffers();

            m_windowSystem->setTitle(
                std::string("Light - " + std::to_string(getFPS()) + " FPS").c_str());
        }
        clear();
    }

    const float Renderer::s_fps_alpha = 1.f / 100;
    void Renderer::calculateFPS(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1) {
            m_average_duration = delta_time;
        }
        else {
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }

    float Renderer::calculateDeltaTime()
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
