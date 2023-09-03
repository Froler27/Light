
#pragma  once

#include <string>

#include <renderer.h>

namespace Light
{

    class Shader;
    class WindowSystem;

    class SdfRenderer: public Renderer
    {
        static const float s_fps_alpha;
    public:
        explicit SdfRenderer(std::shared_ptr<WindowSystem> windowSystem);
        void setSdfShader(const char* shader);
        void setSdfShader(const std::string& shader);

    protected:
        void initialize() override;
        void clear() override {}
        void logicalTick(float delta_time) override;
        void rendererTick(float delta_time) override;

    protected:
        std::unique_ptr<Shader> m_shader;
    };

}