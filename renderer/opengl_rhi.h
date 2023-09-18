
#pragma  once

#include "rhi.h"

namespace Light
{
    class WindowSystem;

    class OpenglRHI final : public RHI
    {
    public:
        virtual void initialize(RHIInitInfo init_info) override final;

    private:
        std::shared_ptr<WindowSystem> m_window_system;
    };
}