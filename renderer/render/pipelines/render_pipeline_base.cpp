
#include "render/pipelines/render_pipeline_base.h"

namespace Light
{

    std::shared_ptr<RenderPipelineBase> RenderPipelineBase::CreatePipeline(const std::string& name)
    {
        return std::make_shared<RenderPipelineBase>();
    }

    void RenderPipelineBase::initialize(RenderPipelineInitInfo init_info)
    {

    }

    void RenderPipelineBase::forwardRender(std::shared_ptr<RHI> rhi)
    {

    }

    void RenderPipelineBase::deferredRender(std::shared_ptr<RHI> rhi)
    {

    }

}