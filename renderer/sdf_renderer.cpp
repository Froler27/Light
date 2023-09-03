
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
        : Renderer(windowSystem)
    {
        if (!gladLoadGLLoader((GLADloadproc)windowSystem->getLoadProcFun()))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
        m_windowSystem->registerOnWindowSizeFunc([this](int w, int h) {
            if (this->m_shader) {
                float resolution[2]{ float(w), float(h) };
                this->m_shader->setFloat2("iResolution", resolution);
            }
        });
    }

    void SdfRenderer::setSdfShader(const char* shader)
    {
        m_shader = std::make_unique<Shader>("../resources/sdf_shaders/sdf.vs", shader);
    }

    void SdfRenderer::setSdfShader(const std::string& shader)
    {
        setSdfShader(shader.c_str());
    }

    void SdfRenderer::initialize()
    {
        unsigned int VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glPointSize(40.f);

        auto windowSize = m_windowSystem->getWindowSize();
        float resolution[2]{ float(windowSize[0]), float(windowSize[1]) };

        m_shader->use();
        m_shader->setFloat2("iResolution", resolution);
    }

    void SdfRenderer::logicalTick(float delta_time)
    {

    }

    void SdfRenderer::rendererTick(float delta_time)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->setFloat("iTime", m_windowSystem->getTime());

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

}
