#include "render/rhi/opengl/opengl_rhi.h"

#include <glad/glad.h>

#include "core/base/macro.h"
#include "window_system.h"
#include "render/render_camera.h"
#include "render/render_resource_base.h"

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
        m_steady_uniform_buffer = std::make_shared<BufferData>(sizeof(SteadyUniformBlock));
        {
            SteadyUniformBlock* steadyBlock = (SteadyUniformBlock*)(m_steady_uniform_buffer->m_data);
            steadyBlock->light_color = Vector4(Vector3(Math_PI), 0);
            *(Vector4*)(steadyBlock->light_pos_or_dir) = -Vector4(-50, 10, 100, 0);
        }
        m_transient_uniform_buffer_block = std::make_shared<BufferData>(sizeof(TransientUniformBlock));

        glGenBuffers(1, &m_steady_uniform_buffer_gpu.ubo);
        glGenBuffers(1, &m_transient_uniform_buffer_block_gpu.ubo);

        TransientUniformBlock* transientBlock = (TransientUniformBlock*)(m_transient_uniform_buffer_block->m_data);
        transientBlock->color = Vector4(1.f);
        transientBlock->ks_p = Vector4(Vector3(1. / Math_PI), 32);
        glBindBuffer(GL_UNIFORM_BUFFER, m_transient_uniform_buffer_block_gpu.ubo);
        glBufferData(GL_UNIFORM_BUFFER, m_transient_uniform_buffer_block->m_size,
            m_transient_uniform_buffer_block->m_data, GL_DYNAMIC_DRAW);


        initLight();


        glEnable(GL_DEPTH_TEST);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glPolygonMode(GL_BACK, GL_POINT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);
        glCullFace(GL_BACK);
        //glFrontFace(GL_CW);
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

    void OpenglRHI::setViewport(int x, int y, int w, int h)
    {
        glViewport(x, y, w, h);
    }

    static const uint32_t SHADOW_WIDTH = 2048;
    static const uint32_t SHADOW_HEIGHT = 2048;

    void OpenglRHI::bindCamera(std::shared_ptr<RenderCamera> camera)
    {
        SteadyUniformBlock* steadyBlock = (SteadyUniformBlock*)(m_steady_uniform_buffer->m_data);

        
        if (camera->m_current_camera_type == RenderCameraType::Shadow) {
            *(Vector3*)(steadyBlock->light_pos_or_dir) = camera->forward();

            auto light_proj_view_matrix = (camera->getOrthoPrgjMatrix()* camera->getViewMatrix()).toMatrix4x4_();
            steadyBlock->light_proj_view_matrix = light_proj_view_matrix;

            *(Matrix4x4_*)(m_shadow_data.shadow_uniform_buffer->m_data) = light_proj_view_matrix;
            glBindBuffer(GL_UNIFORM_BUFFER, m_shadow_data.shadow_ubo);
            glBufferData(GL_UNIFORM_BUFFER, m_shadow_data.shadow_uniform_buffer->m_size,
                m_shadow_data.shadow_uniform_buffer->m_data, GL_STATIC_DRAW);

            glUseProgram(m_shadow_data.shadow_shader);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_shadow_data.shadow_ubo);

            auto viewport = camera->getViewport();
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_data.shadow_fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            return;
        }
        
        auto view_matrix = camera->getViewMatrix();
        //auto proj_matrix = camera->getOrthoPrgjMatrix();
        auto proj_matrix = camera->getProjMatrix();
        steadyBlock->view_matrix = view_matrix.toMatrix4x4_();
        steadyBlock->proj_matrix = proj_matrix.toMatrix4x4_();
        steadyBlock->proj_view_matrix = (proj_matrix * view_matrix).toMatrix4x4_();
        *(Vector3*)(steadyBlock->camera_position) = camera->position();
        
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        auto viewport = camera->getViewport();
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_UNIFORM_BUFFER, m_steady_uniform_buffer_gpu.ubo);
        glBufferData(GL_UNIFORM_BUFFER, m_steady_uniform_buffer->m_size,
            m_steady_uniform_buffer->m_data, GL_STATIC_DRAW);

        
        
    }

    void OpenglRHI::bindMaterial(size_t materialId)
    {
        auto it = m_materials.find(materialId);
        if (it != m_materials.end()) {


            glUseProgram(it->second.shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, it->second.base_color_texture.texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_shadow_data.shadow_texture);

            glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_steady_uniform_buffer_gpu.ubo);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_transient_uniform_buffer_block_gpu.ubo);
            glUniform1i(1, 0); // 绑定纹理
            glUniform1i(2, 1); // 绑定纹理
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
        glUniformMatrix4fv(0, 1, GL_TRUE, reinterpret_cast<const float*>(model_matrix.m_mat));

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
        if (texture && !texture->isValid()) {
            return;
        }
        glGenTextures(1, &glTexture.texture);
        glBindTexture(GL_TEXTURE_2D, glTexture.texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
            0,                                     
            GL_RGBA,                              
            texture->m_height, texture->m_height,
            0,                                     
            //texture->m_format == RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB ?
           // GL_SRGB_ALPHA : GL_RGBA ,              
            GL_RGBA,
            GL_UNSIGNED_BYTE,                     
            texture->m_pixels);
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

    void OpenglRHI::initLight()
    {
        glGenFramebuffers(1, &m_shadow_data.shadow_fbo);
        glGenTextures(1, &m_shadow_data.shadow_texture);

        glBindTexture(GL_TEXTURE_2D, m_shadow_data.shadow_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_data.shadow_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadow_data.shadow_texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenBuffers(1, &m_shadow_data.shadow_ubo);
        m_shadow_data.shadow_uniform_buffer = std::make_shared<BufferData>(sizeof(Matrix4x4_));

        RenderShaderData shadow_shader_file{ 
            RenderResourceBase::loadFile("../resources/shaders/mesh_directional_light_shadow.vs"), 
            RenderResourceBase::loadFile("../resources/shaders/mesh_directional_light_shadow.fs") };
        compileShader(shadow_shader_file, m_shadow_data.shadow_shader);
    }
    

}