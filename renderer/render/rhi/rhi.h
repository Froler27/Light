
#pragma  once

#include "render/render_type.h"
#include "core/math/math_headers.h"

#include <memory>

namespace Light
{
    class WindowSystem;
    class RenderCamera;

    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
    };

    class RHIMesh {};

    class RHI
    {
    public:
        virtual ~RHI() = 0;

        virtual void initialize(RHIInitInfo initialize_info) = 0;

        // destroy
        virtual void clear() = 0;

        virtual void bindCamera(std::shared_ptr<RenderCamera> camera) = 0;

        virtual void bindMesh(size_t meshId) = 0;
        virtual void drawMesh(size_t meshId, const Matrix4x4& matrix) = 0;
        virtual void drawMesh(size_t meshId, uint32_t instance_count) = 0;
        virtual bool updateRHIMesh(const RenderMeshData& meshData, size_t meshId) = 0;

        virtual void bindMaterial(size_t materialId) = 0;
        virtual bool updateMaterial(const RenderMaterialData& materailData, size_t materialId) = 0;

        virtual bool updateImage(const std::shared_ptr<TextureData> texture) = 0;
    };

    inline RHI::~RHI() = default;
}