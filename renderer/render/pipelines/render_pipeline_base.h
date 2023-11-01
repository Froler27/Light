
#pragma  once

#include <memory>
#include <string>

#include "render/rhi/rhi.h"

namespace Light
{
    struct RenderPipelineInitInfo
    {
        bool                                enable_fxaa{ false };
        //std::shared_ptr<RenderResource> render_resource;
    };

    class RenderPipelineBase
    {
        friend class RenderSystem;
    public:
        static std::shared_ptr<RenderPipelineBase> CreatePipeline(const std::string& name);

        RenderPipelineBase() = default;
        void initialize(RenderPipelineInitInfo init_info);

        virtual void clear() {};

        virtual void forwardRender(std::shared_ptr<RHI> rhi);
        virtual void deferredRender(std::shared_ptr<RHI> rhi);

    private:
        std::shared_ptr<RHI> m_rhi;
    };
}