
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

    const TextureData& SdfRenderer::setTexture(const std::string& path)
    {
        if (auto itr = m_textures.find(path); itr != m_textures.end()) {
            return itr->second;
        }
        
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_set_flip_vertically_on_load(true);
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);


        auto res = m_textures.emplace(path, TextureData{});
        TextureData& textureData = res.first->second;
        textureData.m_id = texture;
        textureData.m_width = width;
        textureData.m_height = height;
        textureData.m_channelCount = nrChannels;
        textureData.m_pixels = nullptr;

        return textureData;
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
        m_shader->setFloat2("iMouse", resolution);

        auto texture0 = setTexture("../resources/textures/awesomeface.png");
        m_shader->setInt("iChannel0", 0);
        m_shader->setFloat2("iChannel0Resolution", texture0.m_width, texture0.m_height);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0.m_id);
    }

    void SdfRenderer::logicalTick(float delta_time)
    {

    }

    void SdfRenderer::rendererTick(float delta_time)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->setFloat("iTime", m_windowSystem->getTime());
        m_shader->setInt("iFrame", m_frame_count);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

}
