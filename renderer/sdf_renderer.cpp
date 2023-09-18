
#include "sdf_renderer.h"

#include <iostream>
#include <array>

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "window_system.h"
#include "shader.h"
#include "asset_manager.h"

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
        unsigned int VBO, VAO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        auto mesh = AssetManager::loadStaticMesh("../resources/models/sphere.obj");

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.m_vertex_buffer->m_size, mesh.m_vertex_buffer->m_data, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.m_index_buffer->m_size, mesh.m_index_buffer->m_data, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // texture coord attribute
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);
        

        glGenFramebuffers(1, &m_framebuffer.fb_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fb_id);
        // create a color attachment texture
        glGenTextures(1, &m_framebuffer.texture_id);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer.texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_windowSystem->getWindowSize()[0], m_windowSystem->getWindowSize()[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer.texture_id, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_windowSystem->getWindowSize()[0], m_windowSystem->getWindowSize()[1]); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


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
        

        m_shaderPostprocessing = std::make_unique<Shader>("../resources/shaders/postprocessing/postprocessing.vs", "../resources/shaders/postprocessing/postprocessing.vs");
        m_shaderPostprocessing->use();

        m_shaderModel = std::make_unique<Shader>("../resources/shaders/postprocessing/postprocessing.vs", "../resources/shaders/postprocessing/postprocessing.vs");
        m_shaderModel->use();
    }

    void SdfRenderer::logicalTick(float delta_time)
    {

    }

    void SdfRenderer::rendererTick(float delta_time)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fb_id);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        m_shader->use();
        m_shader->setFloat("iTime", m_windowSystem->getTime());
        m_shader->setInt("iFrame", m_frame_count);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, setTexture("../resources/textures/awesomeface.png").m_id);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shaderPostprocessing->use();
        glBindTexture(GL_TEXTURE_2D, m_framebuffer.texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

}
