
#pragma  once

#include <string>
#include <map>

#include <renderer.h>
#include <render_type.h>

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
        const TextureData& setTexture(const std::string& path);

    protected:
        void initialize() override;
        void clear() override {}
        void logicalTick(float delta_time) override;
        void rendererTick(float delta_time) override;

    protected:
        std::unique_ptr<Shader> m_shader;
        std::map<std::string, TextureData> m_textures;
    };

}