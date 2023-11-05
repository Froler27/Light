#include "render/render_scene.h"
#include "render/render_resource_base.h"
#include "render/rhi/rhi.h"

namespace Light
{

    const std::vector<std::string> ShapeString{
        "Sphere",
        "Plane",
    };

    RenderScene::RenderScene(std::shared_ptr<RHI> rhi) : m_rhi{rhi}
    {
        m_render_resource = std::make_shared<RenderResourceBase>();
    }

    void RenderScene::addMesh(const MeshSourceDesc& meshDesc, const Matrix4x4& matrix, const MaterialSourceDesc& material)
    {
        auto mesh_asset_id = m_mesh_asset_id_allocator.allocGuid(meshDesc);
        if (m_render_entities.count(mesh_asset_id) == 0) {
            auto material_asset_id = m_material_asset_id_allocator.allocGuid(material);
            m_material_to_meshs.emplace(material_asset_id, mesh_asset_id);
            RenderMaterialData materialData = m_render_resource->loadMaterialData(material);
            m_rhi->updateMaterial(materialData, material_asset_id);
        }
        m_render_entities.emplace(mesh_asset_id, RenderEntity(m_render_entities.count(mesh_asset_id), matrix));

        auto res_aabb = m_aabbs.emplace(mesh_asset_id, AxisAlignedBox{});
        RenderMeshData meshData = m_render_resource->loadMeshData(meshDesc, res_aabb.first->second);

        m_rhi->updateRHIMesh(meshData, mesh_asset_id);
    }

    void RenderScene::addShape(Shape shape, const Matrix4x4& matrix, const MaterialSourceDesc& material)
    {

        auto mesh_asset_id = m_mesh_asset_id_allocator.allocGuid(MeshSourceDesc{ ShapeString[size_t(shape)] });
        if (m_render_entities.count(mesh_asset_id) == 0) {
            auto material_asset_id = m_material_asset_id_allocator.allocGuid(material);
            m_material_to_meshs.emplace(material_asset_id, mesh_asset_id);

            
            RenderMaterialData materialData = m_render_resource->loadMaterialData(material);
            m_rhi->updateMaterial(materialData, material_asset_id);
        }
        m_render_entities.emplace(mesh_asset_id, RenderEntity(m_render_entities.count(mesh_asset_id), matrix));
        Matrix4x4 m(Quaternion(Radian(Degree(90.f)), Vector3::UNIT_X));
        //m_render_entities.emplace(mesh_asset_id, RenderEntity(m_render_entities.count(mesh_asset_id), m));
        m_aabbs.emplace(mesh_asset_id, AxisAlignedBox{ Vector3(0.f), Vector3(1.f) });
        
        RenderMeshData meshData;

        getShapeData(shape, meshData);

        m_rhi->updateRHIMesh(meshData, mesh_asset_id);
    }

    void RenderScene::getShapeData(Shape shape, RenderMeshData& meshData)
    {
        static std::vector<std::function<void(RenderMeshData&)>> dataFuncs{
            getSphereData,
            getPlaneData
        };
        dataFuncs[size_t(shape)](meshData);
    }

    void RenderScene::getSphereData(RenderMeshData& meshData)
    {
        const int row = 18;
        const int col = 36;
        const float deltaRow = 180. / row;
        const float deltaCol = 360. / col;
        const float deltaU = 1. / col;
        const float deltaV = 1. / row;
        const float r = 1.f;
        meshData.m_static_mesh_data.m_vertex_buffer =
            std::make_shared<BufferData>(sizeof(MeshVertexDataDefinition) * (row + 1) * (col + 1));
        meshData.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(sizeof(short) * row * col * 6);

        MeshVertexDataDefinition vertexData;
        vertexData.tx = vertexData.ty = vertexData.tz = 0;

        MeshVertexDataDefinition* vertexes = (MeshVertexDataDefinition*)(meshData.m_static_mesh_data.m_vertex_buffer->m_data);
        for (int i = 0; i <= row; ++i) {
            float angle = Degree(i * deltaRow).valueRadians();
            float len_n_xy = Math::sin(angle);
            vertexData.nz = Math::cos(angle);
            vertexData.z = r * vertexData.nz;
            vertexData.v = 1 - deltaV * i;
            for (int j = 0; j <= col; ++j) {
                angle = Degree(j * deltaCol).valueRadians();
                vertexData.nx = Math::cos(angle) * len_n_xy;
                vertexData.x = r * vertexData.nx;
                vertexData.ny = Math::sin(angle) * len_n_xy;
                vertexData.y = r * vertexData.ny;
                vertexData.u = deltaU * j;
                vertexes[i * (col + 1) + j] = vertexData;
            }
        }
        std::vector< uint16_t> iii(row * col * 6);
        uint16_t* indices = (uint16_t*)(meshData.m_static_mesh_data.m_index_buffer->m_data);
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                auto s = i * 6 * col + j * 6;
                indices[s] = i * (col + 1) + j;
                indices[s + 4] = indices[s + 1] = indices[s] + (col + 1);
                indices[s + 3] = indices[s + 2] = (indices[s] + 1) % (col + 1) + i * (col + 1);
                indices[s + 5] = (indices[s + 4] + 1) % (col + 1) + (i + 1) * (col + 1);

                iii[s] = i * (col + 1) + j;
                iii[s + 4] = iii[s + 1] = iii[s] + (col + 1);
                iii[s + 3] = iii[s + 2] = (iii[s] + 1) % (col + 1) + i * (col + 1);
                iii[s + 5] = (iii[s + 4] + 1) % (col + 1) + (i + 1) * (col + 1);
            }
        }
    }

    void RenderScene::getPlaneData(RenderMeshData& meshData)
    {
        meshData.m_static_mesh_data.m_vertex_buffer;

        std::vector<MeshVertexDataDefinition> vertexData;
        std::vector<uint16_t> indexData;

        {
            const float len = 10.f;
            vertexData.reserve(4);
            MeshVertexDataDefinition vertex;
            vertex.tx = vertex.ty = vertex.tz = 0;
            vertex.z = vertex.ny = vertex.nx = 0;

            vertex.nz = 1.f;
            vertex.x = vertex.y = len;
            vertex.u = vertex.v = 1.f;
            vertexData.push_back(vertex);

            vertex.x = -len;
            vertex.u = 0;
            vertexData.push_back(vertex);

            vertex.y = -len;
            vertex.v = 0;
            vertexData.push_back(vertex);

            vertex.x = len;
            vertex.u = 1.f;
            vertexData.push_back(vertex);
        }

        {
            indexData.reserve(6);
            indexData.push_back(0);
            indexData.push_back(1);
            indexData.push_back(2);
            indexData.push_back(0);
            indexData.push_back(2);
            indexData.push_back(3);
        }

        meshData.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(sizeof(MeshVertexDataDefinition) * vertexData.size());
        meshData.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(indexData.size() * sizeof(uint16_t));
        meshData.m_static_mesh_data.m_vertex_buffer->copyData((void*)vertexData.data());
        meshData.m_static_mesh_data.m_index_buffer->copyData((void*)indexData.data());
    }

    void RenderScene::getTestData(RenderMeshData& meshData)
    {
        std::vector<MeshVertexDataDefinition> vertexData;
        std::vector<uint16_t> indexData;

        {
            vertexData.reserve(4);
            MeshVertexDataDefinition vertex;
            vertex.tx = vertex.ty = vertex.tz = 0;
            vertex.z = vertex.ny = vertex.nx = 0;
            vertex.nz = 1.f;

            vertex.x = 1.f;
            vertex.y = 1.f;
            vertex.u = vertex.v = 1.f;
            vertexData.push_back(vertex);

            vertex.x = -1.f;
            vertex.u = 0;
            vertexData.push_back(vertex);

            vertex.y = -1.f;
            vertex.v = 0;
            vertexData.push_back(vertex);

            vertex.x = 1.f;
            vertex.u = 1.f;
            vertexData.push_back(vertex);
        }

        {
            indexData.reserve(6);
            indexData.push_back(0);
            indexData.push_back(1);
            indexData.push_back(2);
            indexData.push_back(0);
            indexData.push_back(2);
            indexData.push_back(3);
        }

        {
            vertexData.clear();
            vertexData.reserve(3);
            MeshVertexDataDefinition vertex;
            vertex.tx = vertex.ty = vertex.tz = 0;
            vertex.y = vertex.nz = vertex.nx = 0;
            vertex.ny = -1.f;

            vertex.x = 0.f;
            vertex.z = 2.f;
            vertex.u = 0.5f;
            vertex.v = 1.f;
            vertexData.push_back(vertex);
            vertex.x = -1.f;
            vertex.z = 0.f;
            vertex.u = 0.f;
            vertex.v = 0.f;
            vertexData.push_back(vertex);
            vertex.x = 1.f;
            vertex.z = 0.f;
            vertex.u = 1.f;
            vertex.v = 0.f;
            vertexData.push_back(vertex);

            vertex.x = 0.f;
            vertex.y = 2.f;
            vertex.z = 0.f;
            vertex.u = 0.5f;
            vertex.v = 1.f;
            vertex.nx = vertex.ny = 0.f;
            vertex.nz = 1.f;
            vertexData.push_back(vertex);
            vertex.x = -1.f;
            vertex.y = 0.f;
            vertex.u = 0.f;
            vertex.v = 0.f;
            vertexData.push_back(vertex);
            vertex.x = 1.f;
            vertex.y = 0.f;
            vertex.u = 1.f;
            vertex.v = 0.f;
            vertexData.push_back(vertex);
        }

        {
            indexData.clear();
            indexData.reserve(3);
            indexData.push_back(0);
            indexData.push_back(1);
            indexData.push_back(2);
            indexData.push_back(3);
            indexData.push_back(4);
            indexData.push_back(5);
        }

        meshData.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(sizeof(MeshVertexDataDefinition) * vertexData.size());
        meshData.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(indexData.size() * sizeof(uint16_t));
        meshData.m_static_mesh_data.m_vertex_buffer->copyData((void*)vertexData.data());
        meshData.m_static_mesh_data.m_index_buffer->copyData((void*)indexData.data());
    }

    void RenderScene::getAxisData(RenderMeshData& meshData)
    {
        std::vector<MeshVertexDataDefinition> vertexData;
        std::vector<uint16_t> indexData;

        const float b = 0.1f;
        const float nb = Math::sqrt(0.5);
        MeshVertexDataDefinition vertex;
        vertex.tx = vertex.ty = vertex.tz = 0;
        vertex.u = vertex.v = 0.1f;
        vertex.y = vertex.z = vertex.ny = vertex.nz = 0;
        vertex.x = 1.f;
        vertex.nx = 1.f;
        vertexData.push_back(vertex);
        vertex.nx = 0; vertex.ny = vertex.nz = nb;
        vertex.x = 0; vertex.y = vertex.z = b; vertexData.push_back(vertex);
        vertex.ny = -nb; vertex.y = -b; vertexData.push_back(vertex);
        vertex.nz = -nb; vertex.z = -b; vertexData.push_back(vertex);
        vertex.ny = nb; vertex.y = b; vertexData.push_back(vertex);

        indexData.push_back(0); indexData.push_back(1); indexData.push_back(2);
        indexData.push_back(0); indexData.push_back(2); indexData.push_back(3);
        indexData.push_back(0); indexData.push_back(3); indexData.push_back(4);
        indexData.push_back(0); indexData.push_back(4); indexData.push_back(1);

        vertex.u = vertex.v = 0.5f;
        vertex.nx = vertex.nz = 0; vertex.ny = 1.f;
        vertex.x = vertex.z = 0; vertex.y = 1.f; vertexData.push_back(vertex);
        vertex.ny = 0.f; vertex.nx = vertex.nz = nb;
        vertex.y = 0.f; vertex.x = vertex.z = b; vertexData.push_back(vertex);
        vertex.nz = -nb; vertex.z = -b; vertexData.push_back(vertex);
        vertex.nx = -nb; vertex.x = -b; vertexData.push_back(vertex);
        vertex.nz = nb; vertex.z = b; vertexData.push_back(vertex);

        short index = 5;
        indexData.push_back(index); indexData.push_back(1 + index); indexData.push_back(2 + index);
        indexData.push_back(index); indexData.push_back(2 + index); indexData.push_back(3 + index);
        indexData.push_back(index); indexData.push_back(3 + index); indexData.push_back(4 + index);
        indexData.push_back(index); indexData.push_back(4 + index); indexData.push_back(1 + index);

        vertex.u = vertex.v = 0.9f;
        vertex.nx = vertex.ny = 0; vertex.nz = 1.f;
        vertex.x = vertex.y = 0; vertex.z = 1.f; vertexData.push_back(vertex);
        vertex.nz = 0.f; vertex.nx = vertex.ny = nb;
        vertex.z = 0.f; vertex.x = vertex.y = b; vertexData.push_back(vertex);
        vertex.nx = -nb; vertex.x = -b; vertexData.push_back(vertex);
        vertex.ny = -nb; vertex.y = -b; vertexData.push_back(vertex);
        vertex.nx = nb; vertex.x = b; vertexData.push_back(vertex);

        index = 10;
        indexData.push_back(index); indexData.push_back(1 + index); indexData.push_back(2 + index);
        indexData.push_back(index); indexData.push_back(2 + index); indexData.push_back(3 + index);
        indexData.push_back(index); indexData.push_back(3 + index); indexData.push_back(4 + index);
        indexData.push_back(index); indexData.push_back(4 + index); indexData.push_back(1 + index);

        meshData.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(sizeof(MeshVertexDataDefinition) * vertexData.size());
        meshData.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(indexData.size() * sizeof(uint16_t));
        meshData.m_static_mesh_data.m_vertex_buffer->copyData((void*)vertexData.data());
        meshData.m_static_mesh_data.m_index_buffer->copyData((void*)indexData.data());
    }

}