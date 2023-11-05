
#pragma once

#include "render/render_type.h"
#include "render/render_guid_allocator.h"
#include "core/math/axis_aligned.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Light
{
    class RHI;
    class RenderScene;
    class RenderCamera;

    class RenderResourceBase
    {
    public:
        RenderResourceBase();
        virtual ~RenderResourceBase() {}

        void initScene(std::shared_ptr<RHI> rhi);

        // TODO: data caching
        std::shared_ptr<TextureData> loadTextureHDR(std::string file, int desired_channels = 4);
        std::shared_ptr<TextureData> loadTexture(std::string file, bool is_srgb = false);
        RenderMeshData               loadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box);
        RenderMaterialData           loadMaterialData(const MaterialSourceDesc& source);
        AxisAlignedBox               getCachedBoudingBox(const MeshSourceDesc& source) const;

    private:
        StaticMeshData loadStaticMesh(const std::string& mesh_file, AxisAlignedBox& bounding_box);

        std::string loadFile(const std::string& filename);;

        std::unordered_map<MeshSourceDesc, AxisAlignedBox> m_bounding_box_cache_map;

        GuidAllocator<MeshSourceDesc> m_mesh_asset_id_allocator;
    };
}