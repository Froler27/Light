#include "render/render_scene.h"
#include "render/render_resource_base.h"
#include "render/rhi/rhi.h"

namespace Light
{

    RenderScene::RenderScene(std::shared_ptr<RHI> rhi) : m_rhi{rhi}
    {
        m_render_resource = std::make_shared<RenderResourceBase>();
    }

    void RenderScene::addMesh(const MeshSourceDesc& meshDesc, const Matrix4x4& mat, const MaterialSourceDesc& material)
    {
        auto mesh_asset_id = m_mesh_asset_id_allocator.allocGuid(meshDesc);
        if (m_render_entities.count(mesh_asset_id) == 0) {
            auto material_asset_id = m_material_asset_id_allocator.allocGuid(material);
            m_material_to_meshs.emplace(material_asset_id, mesh_asset_id);
            RenderMaterialData materialData = m_render_resource->loadMaterialData(material);
            m_rhi->updateMaterial(materialData, material_asset_id);
        }
        m_render_entities.emplace(mesh_asset_id, RenderEntity(m_render_entities.count(mesh_asset_id), mat));

        auto res_aabb = m_aabbs.emplace(mesh_asset_id, AxisAlignedBox{});
        RenderMeshData meshData = m_render_resource->loadMeshData(meshDesc, res_aabb.first->second);

        m_rhi->updateRHIMesh(meshData, mesh_asset_id);
    }

}