#include "render/rhi/opengl/opengl_rhi.h"

#include <glad/glad.h>

#include "core/base/macro.h"
#include "window_system.h"
#include "render/render_camera.h"

namespace Light
{

    void OpenglRHI::initialize(RHIInitInfo init_info)
    {
        m_window_system = init_info.window_system;
        if (!gladLoadGLLoader((GLADloadproc)init_info.window_system->getLoadProcFun()))
        {
            LOG_ERROR("Failed to initialize OpenglRHI");
        }
        m_window_system->registerOnWindowSizeFunc(windowSizeCallback);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
    }

    void OpenglRHI::clear()
    {

    }

    OpenglRHI::~OpenglRHI()
    {
        clear();
    }

    void OpenglRHI::windowSizeCallback(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void OpenglRHI::bindMaterial(size_t materialId)
    {
        auto it = m_materials.find(materialId);
        if (it != m_materials.end()) {
            m_current_shader = it->second.shader;
            glUseProgram(m_current_shader);
            glBindTexture(GL_TEXTURE_2D, it->second.base_color_texture.texture);

            glUniformMatrix4fv(glGetUniformLocation(m_current_shader, "proj_view_matrix"),
                1, GL_TRUE, reinterpret_cast<const float*>(m_proj_view_matrix.m_mat));
        }
    }

    void OpenglRHI::bindMesh(size_t meshId)
    {
        auto it = m_meshes.find(meshId);
        if (it != m_meshes.end()) {
            glBindVertexArray(it->second.vao);
        }
    }

    void OpenglRHI::drawMesh(size_t meshId, const Matrix4x4& model_matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_current_shader, "model_matrix"),
            1, GL_TRUE, reinterpret_cast<const float*>(model_matrix.m_mat));
        
        auto it = m_meshes.find(meshId);
        if (it == m_meshes.end()) {
            return;
        }
        OpenglMesh& mesh = it->second;
        glDrawElements(GL_TRIANGLES, mesh.index_num, GL_UNSIGNED_SHORT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void OpenglRHI::drawMesh(size_t meshId, uint32_t instance_count)
    {
        auto it = m_meshes.find(meshId);
        if (it == m_meshes.end()) {
            return;
        }
        OpenglMesh& mesh = it->second;
        glDrawElementsInstanced(GL_TRIANGLES, mesh.index_num, GL_UNSIGNED_SHORT, 0, instance_count);
    }

    bool OpenglRHI::updateRHIMesh(const RenderMeshData& meshData, size_t meshId)
    {
        auto it = m_meshes.find(meshId);
        if (it != m_meshes.end()) {
            return false;
        }
        auto res = m_meshes.emplace(meshId, OpenglMesh{});
        assert(res.second);
        OpenglMesh& now_mesh = res.first->second;
        now_mesh.index_num = meshData.m_static_mesh_data.m_index_buffer->m_size / sizeof(uint16_t);

        glGenVertexArrays(1, &now_mesh.vao);
        glGenBuffers(1, &now_mesh.vbo);
        glGenBuffers(1, &now_mesh.ebo);

        glBindVertexArray(now_mesh.vao);

        glBindBuffer(GL_ARRAY_BUFFER, now_mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, meshData.m_static_mesh_data.m_vertex_buffer->m_size,
            meshData.m_static_mesh_data.m_vertex_buffer->m_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, now_mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.m_static_mesh_data.m_index_buffer->m_size,
            meshData.m_static_mesh_data.m_index_buffer->m_data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)0);

        // color attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(6 * sizeof(float)));

        // texture coordinate attribute
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataDefinition), (void*)(9 * sizeof(float)));

        return true;
    }

    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                LOG_ERROR("Failed to compile {} shader. Error: {}", type, infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                LOG_ERROR("Failed to link program. Error: {}", infoLog);
            }
        }
    }

    bool OpenglRHI::updateMaterial(const RenderMaterialData& materailData, size_t materialId)
    {
        auto it = m_materials.find(materialId);
        if (it != m_materials.end()) {
            return false;
        }
        auto res = m_materials.emplace(materialId, OpenglMaterial{});
        assert(res.second);
        OpenglMaterial& now_material = res.first->second;
        
        
        compileShader(materailData.m_shader, now_material.shader);
        upateTexture(materailData.m_base_color_texture, now_material.base_color_texture);

        return true;
    }

    bool OpenglRHI::updateImage(const std::shared_ptr<TextureData> texture)
    {
        if (texture) {

            return true;
        }
        return false;
    }

    void OpenglRHI::upateTexture(const std::shared_ptr<TextureData> texture, OpenglTexture& glTexture)
    {
        if (!texture->isValid()) {
            return;
        }
        glGenTextures(1, &glTexture.texture);
        glBindTexture(GL_TEXTURE_2D, glTexture.texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
            0,                                      // 指定多级渐远纹理的级别
            GL_RGBA,                                // 把纹理储存为何种格式
            texture->m_height, texture->m_height,
            0,                                      // 总是被设为0（历史遗留的问题
            texture->m_format == RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB ?
            GL_SRGB_ALPHA : GL_RGBA ,               // 源图的格式 GL_SRGB GL_RGBA
            GL_UNSIGNED_BYTE,                       // 源图的数据格式
            texture->m_pixels);                     // 图像数据
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void OpenglRHI::compileShader(const RenderShaderData& shaderData, uint32_t& shader)
    {
        auto vertexShaderCode = shaderData.m_vertex_shader_code.c_str();
        uint32_t vertex;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        auto fragmentShaderCode = shaderData.m_fragment_shader_code.c_str();
        uint32_t fragment;
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        shader = glCreateProgram();
        glAttachShader(shader, vertex);
        glAttachShader(shader, fragment);
        glLinkProgram(shader);
        checkCompileErrors(shader, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void OpenglRHI::bindCamera(std::shared_ptr<RenderCamera> camera)
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_proj_view_matrix = camera->getPersProjMatrix() * camera->getViewMatrix();
    }

}