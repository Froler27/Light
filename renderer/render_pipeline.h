
#pragma  once

#include "rhi.h"

namespace Light
{
    struct RenderPipelineInitInfo
    {
        bool                                enable_fxaa{ false };
        //std::shared_ptr<RenderResource> render_resource;
    };

    class RenderPipeline
    {
    public:
        RenderPipeline() = default;
        void initialize(RenderPipelineInitInfo init_info);

    private:
        std::shared_ptr<RHI> m_rhi;
    };
}