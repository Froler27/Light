#include "renderer.h"
#include <iostream>
#include <glad/glad.h>

namespace Light
{
    void Renderer::initialize(GLADloadproc proc)
    {
        if (!gladLoadGLLoader((GLADloadproc)proc))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    }

    void Renderer::clear()
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

}
