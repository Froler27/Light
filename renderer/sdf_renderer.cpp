
#include "sdf_renderer.h"
#include <iostream>
#include <array>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "window_system.h"
#include "shader.h"

namespace Light
{

    SdfRenderer::SdfRenderer(std::shared_ptr<WindowSystem> windowSystem)
        : m_windowSystem(windowSystem)
    {
        if (!gladLoadGLLoader((GLADloadproc)windowSystem->getLoadProcFun()))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
    }

    void SdfRenderer::setSdfShader(const char* shader)
    {
        m_shader = std::make_unique<Shader>("../resources/sdf_shaders/sdf.vs", shader);
    }

    void SdfRenderer::setSdfShader(const std::string& shader)
    {
        setSdfShader(shader.c_str());
    }

    void SdfRenderer::run() 
    {
        unsigned int VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glPointSize(40.f);

        auto windowSize = m_windowSystem->getWindowSize();
        //std::array<float, 2> resolution{ float(windowSize[0]), float(windowSize[1]) };
        float resolution[2] { float(windowSize[0]), float(windowSize[1]) };
        
        m_shader->use();

        while (!m_windowSystem->shouldClose()) {
            glClear(GL_COLOR_BUFFER_BIT);

            m_shader->setFloat2("iResolution", resolution);
            m_shader->setFloat("iTime", m_windowSystem->getTime());
            
            glDrawArrays(GL_TRIANGLES , 0, 3);

            m_windowSystem->pollEvents();
            m_windowSystem->swapBuffers();
        }
    }

}
