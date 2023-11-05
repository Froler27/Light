
#pragma once

#include <array>
#include <memory>

namespace Light
{
    class WindowSystem;
    class RHI;
    class RenderCamera;
    class RenderScene;
    class RenderPipelineBase;
    class RenderResourceBase;

    struct RenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
    };

    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

        void initialize(RenderSystemInitInfo init_info);
        void tick(float delta_time);
        void clear();

        std::shared_ptr<RenderCamera> getRenderCamera() const;
        std::shared_ptr<RHI>          getRHI() const;

    private:
        void initScene();
        void drawAxis();
        static void windowSizeChanged(int w, int h);

        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderCamera>       m_render_camera;
        std::shared_ptr<RenderPipelineBase> m_render_pipeline;
        std::shared_ptr<RenderResourceBase> m_render_resource;
        std::shared_ptr<RenderScene>        m_render_scene;
    };
}