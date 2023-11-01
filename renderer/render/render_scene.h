
#pragma once

#include "render/render_type.h"
#include "render/render_guid_allocator.h"
#include "core/math/axis_aligned.h"
#include "core/math/matrix4.h"

#include <unordered_map>

namespace Light
{
    class RenderResourceBase;
    class RHI;

    class RenderEntity {
    public:
        explicit RenderEntity(uint32_t id): m_instance_id(id){}
        RenderEntity(uint32_t id, const Matrix4x4& mat): m_model_matrix(mat) {}

        uint32_t  m_instance_id{ 0 };
        uint32_t m_shader{ 0 };
        Matrix4x4 m_model_matrix{ Matrix4x4::IDENTITY };
    };

    class RenderScene
    {
        friend class RenderSystem;
    public:
        explicit RenderScene(std::shared_ptr<RHI> rhi);

        void addMesh(const MeshSourceDesc& meshDesc, const Matrix4x4& mat = Matrix4x4::IDENTITY, const MaterialSourceDesc& material = MaterialSourceDesc());

    private:
        GuidAllocator<MeshSourceDesc> m_mesh_asset_id_allocator;
        GuidAllocator<MaterialSourceDesc> m_material_asset_id_allocator;
        std::shared_ptr<RenderResourceBase> m_render_resource;
        std::shared_ptr<RHI> m_rhi;
        std::unordered_multimap<size_t, size_t> m_material_to_meshs;
        std::unordered_multimap<size_t, RenderEntity> m_render_entities;
        std::unordered_map<size_t, AxisAlignedBox> m_aabbs;
    };
}