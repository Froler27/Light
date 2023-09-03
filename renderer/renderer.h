#pragma  once

#include <memory>
#include <chrono>

namespace Light
{
    class WindowSystem;

    class Renderer
    {
        static const float s_fps_alpha;
    public:
        Renderer(std::shared_ptr<WindowSystem> windowSystem) :m_windowSystem(windowSystem) {}
        virtual ~Renderer() = default;

        int getFPS() const { return m_fps; }

        void run();

    protected:
        void calculateFPS(float delta_time);
        float calculateDeltaTime();

        virtual void initialize() {}
        virtual void clear() {}
        virtual void logicalTick(float delta_time) {}
        virtual void rendererTick(float delta_time) {}

    protected:
        std::shared_ptr<WindowSystem> m_windowSystem;
        std::chrono::steady_clock::time_point m_last_tick_time_point{ std::chrono::steady_clock::now() };

        float m_average_duration{ 0.f };
        int   m_frame_count{ 0 };
        int   m_fps{ 0 };
    };

}