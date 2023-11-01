
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

    struct OpenglBuffer {};

    class OpenglRHI final : public RHI
    {
    public:
        ~OpenglRHI() override final;
        void initialize(RHIInitInfo init_info) override final;
        void clear() override final;

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

        void upateTexture(const std::shared_ptr<TextureData> texture, OpenglTexture& glTexture);
        void compileShader(const RenderShaderData& shaderData, uint32_t& shader);

    private:
        std::shared_ptr<WindowSystem> m_window_system;
        std::unordered_map<size_t, OpenglMesh> m_meshes;
        std::unordered_map<size_t, OpenglMaterial> m_materials;
        OpenglInstance m_ibo{ 0 };

        uint32_t m_current_shader{ 0 };
        Matrix4x4 m_proj_view_matrix;
    };
}