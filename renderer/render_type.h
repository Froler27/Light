
#pragma  once

#include <cstdint>
#include <memory>
#include <string_view>

namespace Light
{

    class BufferData
    {
    public:
        size_t m_size{ 0 };
        void* m_data{ nullptr };

        BufferData() = delete;
        BufferData(size_t size)
        {
            m_size = size;
            m_data = malloc(size);
        }
        ~BufferData()
        {
            if (m_data)
            {
                free(m_data);
            }
        }
        bool isValid() const { return m_data != nullptr; }
    };

    class TextureData
    {
    public:
        uint32_t m_id{ 0 };
        uint32_t m_width{ 0 };
        uint32_t m_height{ 0 };
        uint32_t m_channelCount{ 0 };
        void* m_pixels{ nullptr };

        TextureData() = default;
        ~TextureData()
        {
            if (m_pixels)
            {
                free(m_pixels);
            }
        }
        bool isValid() const { return m_pixels != nullptr; }
    };

    struct FrameBuffer {
        uint32_t fb_id{ 0 };
        uint32_t texture_id{ 0 };
    };

    struct MeshVertexDataDefinition
    {
        float x, y, z;    // position
        float nx, ny, nz; // normal
        float tx, ty, tz; // tangent
        float u, v;       // UV coordinates
    };

    struct MeshSourceDesc
    {
        std::string m_mesh_file;

        bool   operator==(const MeshSourceDesc& rhs) const { return m_mesh_file == rhs.m_mesh_file; }
        size_t getHashValue() const { return std::hash<std::string> {}(m_mesh_file); }
    };

    struct StaticMeshData
    {
        std::shared_ptr<BufferData> m_vertex_buffer;
        std::shared_ptr<BufferData> m_index_buffer;
    };
} // namespace Light

template<>
struct std::hash<Light::MeshSourceDesc>
{
    size_t operator()(const Light::MeshSourceDesc& rhs) const noexcept { return rhs.getHashValue(); }
};