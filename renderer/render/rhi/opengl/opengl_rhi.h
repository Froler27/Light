
#pragma  once

#include "render/rhi/rhi.h"
#include "render/render_guid_allocator.h"

namespace Light
{
    class WindowSystem;

    struct OpenglMesh {
        uint32_t vao{ 0 };
        uint32_t vbo{ 0 };
        uint32_t ebo{ 0 };
        uint32_t index_num{ 0 };
        uint32_t instance_buffer{ 0 };
    };

    struct OpenglTexture {
        uint32_t texture{ 0 };
    };

    struct OpenglMaterial {
        uint32_t shader{ 0 };
        OpenglTexture base_color_texture{};
    };

    struct OpenglInstance
    {
        uint32_t ibo{ 0 };
    };

    struct OpenglBuffer {
        uint32_t ubo{ 0 };
    };

    struct OpenglShadowData {
        uint32_t shadow_texture{ 0 };
        uint32_t shadow_fbo{ 0 };
        uint32_t shadow_shader{ 0 };
        uint32_t shadow_ubo{ 0 };
        std::shared_ptr<BufferData> shadow_uniform_buffer;
    };

    struct SteadyUniformBlock {
        Vector4 light_color{1.f, 1.f, 1.f, 1.f};
        float light_pos_or_dir[4]{0.f, 0.f, -1.f, 0.f};
        float ambient_color[3]{ 1.f, 1.f, 1.f };
        float ambient_intensity{ 0.1f };
        float camera_position[3]{};
        float _padding_camera_position{};
        Matrix4x4_ view_matrix;
        Matrix4x4_ proj_matrix;
        Matrix4x4_ proj_view_matrix;
        Matrix4x4_ light_proj_view_matrix;
    };

    struct TransientUniformBlock
    {
        Vector4 color;
        Vector4 ks_p;
    };

    class OpenglRHI final : public RHI
    {
    public:
        ~OpenglRHI() override final;
        void initialize(RHIInitInfo init_info) override final;
        void clear() override final;

        void setViewport(int x, int y, int w, int h) override;

        void bindCamera(std::shared_ptr<RenderCamera> camera) override;

        void bindMesh(size_t meshId) override;
        void drawMesh(size_t meshId, const Matrix4x4& matrix) override;
        void drawMesh(size_t meshId, uint32_t instance_count) override;
        bool updateRHIMesh(const RenderMeshData& meshData, size_t meshId) override;

        void bindMaterial(size_t materialId) override;
        bool updateMaterial(const RenderMaterialData& materailData, size_t materialId) override;
        
        bool updateImage(const std::shared_ptr<TextureData> texture) override;

        void initInstanceBuffer();

    private:
        static void windowSizeCallback(int width, int height);

        void initLight();
        void initSteadyUniform();
        void upateTexture(const std::shared_ptr<TextureData> texture, OpenglTexture& glTexture);
        void compileShader(const RenderShaderData& shaderData, uint32_t& shader);

    private:
        std::shared_ptr<WindowSystem> m_window_system;
        std::unordered_map<size_t, OpenglMesh> m_meshes;
        std::unordered_map<size_t, OpenglMaterial> m_materials;
        OpenglInstance m_ibo{ 0 };

        std::shared_ptr<BufferData> m_steady_uniform_buffer;
        std::shared_ptr<BufferData> m_transient_uniform_buffer_block;
        OpenglBuffer m_steady_uniform_buffer_gpu;
        OpenglBuffer m_transient_uniform_buffer_block_gpu;

        OpenglShadowData m_shadow_data;
    };
}