
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
    class RenderCamera;

    class RenderEntity {
    public:
        explicit RenderEntity(uint32_t id): m_instance_id(id){}
        RenderEntity(uint32_t id, const Matrix4x4& mat): m_instance_id(id), m_model_matrix(mat) {}

        uint32_t  m_instance_id{ 0 };
        Matrix4x4 m_model_matrix{ Matrix4x4::IDENTITY };
    };

    enum class Shape : uint32_t
    {
        Sphere,
        Plane,
    };
    extern const std::vector<std::string> ShapeString;


    enum class LightType : uint32_t
    {
        Direction,
        Point,
    };
    class PointLikeLight
    {
    public:
        explicit PointLikeLight(LightType type = LightType::Direction);
        explicit PointLikeLight(std::shared_ptr<RenderCamera> camera, LightType type = LightType::Direction) :
            m_shadow_camera{ camera }, m_type{ type } {}

        void setDirectionl(const Vector3& forward);

        LightType m_type;
        std::shared_ptr<RenderCamera> m_shadow_camera;
        Vector4 m_pos_or_dir{ 0.f, 1.f, -1.f, 0.f };
        Vector4 m_color_and_intensity{ Vector3(1.f), Math_PI };
        
    };

    class RenderScene
    {
        friend class RenderSystem;
    public:
        explicit RenderScene(std::shared_ptr<RHI> rhi);

        void addLight(std::shared_ptr<PointLikeLight> light);

        void addMesh(const MeshSourceDesc& meshDesc, const Matrix4x4& mat = Matrix4x4::IDENTITY, const MaterialSourceDesc& material = MaterialSourceDesc());

        void addShape(Shape shape = Shape::Sphere, const Matrix4x4& mat = Matrix4x4::IDENTITY, const MaterialSourceDesc& material = MaterialSourceDesc());


    private:
        static void getShapeData(Shape shape, RenderMeshData& meshData);
        static void getSphereData(RenderMeshData& meshData);
        static void getPlaneData(RenderMeshData& meshData);
        void getTestData(RenderMeshData& meshData);
        void getAxisData(RenderMeshData& meshData);

    private:
        GuidAllocator<MeshSourceDesc> m_mesh_asset_id_allocator;
        GuidAllocator<MaterialSourceDesc> m_material_asset_id_allocator;
        std::shared_ptr<RenderResourceBase> m_render_resource;
        std::shared_ptr<RHI> m_rhi;
        std::vector<std::shared_ptr<PointLikeLight>> m_lights;
        std::unordered_multimap<size_t, size_t> m_material_to_meshs;
        std::unordered_multimap<size_t, RenderEntity> m_render_entities;
        std::unordered_map<size_t, AxisAlignedBox> m_aabbs;
    };
}