#include "opengl_rhi.h"

#include <iostream>

#include <glad/glad.h>

#include "window_system.h"

namespace Light
{

    void OpenglRHI::initialize(RHIInitInfo init_info)
    {
        m_window_system = init_info.window_system;
        if (!gladLoadGLLoader((GLADloadproc)init_info.window_system->getLoadProcFun()))
        {
            std::cout << "Failed to initialize OpenglRHI" << std::endl;
        }
    }

}