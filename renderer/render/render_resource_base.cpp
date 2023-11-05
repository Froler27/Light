#include "render/render_resource_base.h"

#include "core/base/macro.h"
#include "core/math/math_headers.h"

#include "asset_manager.h"

#include "global/global_context.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "render/rhi/rhi.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Light
{
    std::shared_ptr<TextureData> GetSingleColorTexture(Vector4 color)
    {
        std::shared_ptr<TextureData> texture = std::make_shared<TextureData>(32);

        auto col = (unsigned char*)(texture->m_pixels);
        col[0] = ((color.x) * 255);
        col[1] = ((color.y) * 255);
        col[2] = ((color.z) * 255);
        col[3] = ((color.w) * 255);
        texture->m_format = RHI_FORMAT_R8G8B8A8_UNORM;
        texture->m_channelCount = 4;
        texture->m_width = 1;
        texture->m_height = 1;
        return texture;
    }


    std::unordered_map<std::string, RenderMaterialData> SimpleMaterials{
        {"phong_silver", {GetSingleColorTexture(Vector4(Vector3(0.50754),1.))}}
    };

    RenderResourceBase::RenderResourceBase()
    {
        stbi_set_flip_vertically_on_load(true);
    }

    void RenderResourceBase::initScene(std::shared_ptr<RHI> rhi)
    {
        AxisAlignedBox aabb;
        MeshSourceDesc meshDesc{ "../resources/models/sphere.obj" };
        RenderMeshData meshData = loadMeshData(meshDesc, aabb);
        auto mesh_asset_id = m_mesh_asset_id_allocator.allocGuid(meshDesc);
        rhi->updateRHIMesh(meshData, mesh_asset_id);
    }

    std::shared_ptr<Light::TextureData> RenderResourceBase::loadTextureHDR(std::string file, int desired_channels /*= 4*/)
    {
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

        int iw, ih, n;
        texture->m_pixels =
            stbi_loadf(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, desired_channels);

        if (!texture->m_pixels)
            return nullptr;

        texture->m_width = iw;
        texture->m_height = ih;
        switch (desired_channels)
        {
        case 2:
            texture->m_format = RHIFormat::RHI_FORMAT_R32G32_SFLOAT;
            break;
        case 4:
            texture->m_format = RHIFormat::RHI_FORMAT_R32G32B32A32_SFLOAT;
            break;
        default:
            // three component format is not supported in some vulkan driver implementations
            throw std::runtime_error("unsupported channels number");
            break;
        }

        return texture;
    }

    std::shared_ptr<Light::TextureData> RenderResourceBase::loadTexture(std::string file, bool is_srgb /*= false*/)
    {
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

        int iw, ih, n;
        texture->m_pixels = stbi_load(asset_manager->getFullPath(file).generic_string().c_str(), &iw, &ih, &n, 4);

        if (!texture->m_pixels)
            return nullptr;



        texture->m_width = iw;
        texture->m_height = ih;
        texture->m_channelCount = n;
        texture->m_format = (is_srgb) ? RHIFormat::RHI_FORMAT_R8G8B8A8_SRGB :
            RHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;

        char* ttt = (char*)texture->m_pixels;
        bool kkkk = false;
        int i = 3;
        for (i = 3; i < texture->m_height * texture->m_width; i += 4) {
            if (ttt[i] != 0) {
                kkkk = true;
                break;
            }
        }

        return texture;
    }

    RenderMeshData RenderResourceBase::loadMeshData(const MeshSourceDesc& source, AxisAlignedBox& bounding_box)
    {
        RenderMeshData ret;

        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        if (std::filesystem::path(source.m_mesh_file).extension() == ".obj")
        {
            ret.m_static_mesh_data = loadStaticMesh(source.m_mesh_file, bounding_box);
        }

        m_bounding_box_cache_map.insert(std::make_pair(source, bounding_box));

        return ret;
    }

    RenderMaterialData RenderResourceBase::loadMaterialData(const MaterialSourceDesc& source)
    {

        if (SimpleMaterials.count(source.m_base_color_file) > 0) {
            return SimpleMaterials.find(source.m_base_color_file)->second;
        }
        RenderMaterialData ret;
        ret.m_base_color_texture = loadTexture(source.m_base_color_file);
        ret.m_metallic_roughness_texture = loadTexture(source.m_metallic_roughness_file);
        ret.m_normal_texture = loadTexture(source.m_normal_file);
        ret.m_occlusion_texture = loadTexture(source.m_occlusion_file);
        ret.m_emissive_texture = loadTexture(source.m_emissive_file);

        ret.m_shader.m_vertex_shader_code = loadFile(source.m_vertex_shader_file);
        ret.m_shader.m_fragment_shader_code = loadFile(source.m_fragment_shader_file);

        return ret;
    }

    AxisAlignedBox RenderResourceBase::getCachedBoudingBox(const MeshSourceDesc& source) const
    {
        auto find_it = m_bounding_box_cache_map.find(source);
        if (find_it != m_bounding_box_cache_map.end())
        {
            return find_it->second;
        }
        return AxisAlignedBox();
    }

    StaticMeshData RenderResourceBase::loadStaticMesh(const std::string& filename, AxisAlignedBox& bounding_box)
    {
        StaticMeshData mesh_data;

        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.vertex_color = false;
        if (!reader.ParseFromFile(filename, reader_config))
        {
            if (!reader.Error().empty())
            {
                LOG_ERROR("loadMesh {} failed, error: {}", filename, reader.Error());
            }
            assert(0);
        }

        if (!reader.Warning().empty())
        {
            LOG_WARN("loadMesh {} warning, warning: {}", filename, reader.Warning());
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        std::vector<MeshVertexDataDefinition> mesh_vertices;

        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                bool with_normal = true;
                bool with_texcoord = true;

                Vector3 vertex[3];
                Vector3 normal[3];
                Vector2 uv[3];

                // only deals with triangle faces
                if (fv != 3)
                {
                    continue;
                }

                // expanding vertex data is not efficient and is for testing purposes only
                for (size_t v = 0; v < fv; v++)
                {
                    auto idx = shapes[s].mesh.indices[index_offset + v];
                    auto vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    auto vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    auto vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    vertex[v].x = static_cast<float>(vx);
                    vertex[v].y = static_cast<float>(vy);
                    vertex[v].z = static_cast<float>(vz);

                    bounding_box.merge(Vector3(vertex[v].x, vertex[v].y, vertex[v].z));

                    if (idx.normal_index >= 0)
                    {
                        auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                        normal[v].x = static_cast<float>(nx);
                        normal[v].y = static_cast<float>(ny);
                        normal[v].z = static_cast<float>(nz);
                    }
                    else
                    {
                        with_normal = false;
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                        uv[v].x = static_cast<float>(tx);
                        uv[v].y = static_cast<float>(ty);
                    }
                    else
                    {
                        with_texcoord = false;
                    }
                }
                index_offset += fv;

                if (!with_normal)
                {
                    Vector3 v0 = vertex[1] - vertex[0];
                    Vector3 v1 = vertex[2] - vertex[1];
                    normal[0] = v0.crossProduct(v1).normalisedCopy();
                    normal[1] = normal[0];
                    normal[2] = normal[0];
                }

                if (!with_texcoord)
                {
                    uv[0] = Vector2(0.5f, 0.5f);
                    uv[1] = Vector2(0.5f, 0.5f);
                    uv[2] = Vector2(0.5f, 0.5f);
                }

                Vector3 tangent{ 1, 0, 0 };
                {
                    Vector3 edge1 = vertex[1] - vertex[0];
                    Vector3 edge2 = vertex[2] - vertex[1];
                    Vector2 deltaUV1 = uv[1] - uv[0];
                    Vector2 deltaUV2 = uv[2] - uv[1];

                    auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                    if (divide >= 0.0f && divide < 0.000001f)
                        divide = 0.000001f;
                    else if (divide < 0.0f && divide > -0.000001f)
                        divide = -0.000001f;

                    float df = 1.0f / divide;
                    tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    tangent = (tangent).normalisedCopy();
                }

                for (size_t i = 0; i < 3; i++)
                {
                    MeshVertexDataDefinition mesh_vert{};

                    mesh_vert.x = vertex[i].x;
                    mesh_vert.y = vertex[i].y;
                    mesh_vert.z = vertex[i].z;

                    mesh_vert.nx = normal[i].x;
                    mesh_vert.ny = normal[i].y;
                    mesh_vert.nz = normal[i].z;

                    mesh_vert.u = uv[i].x;
                    mesh_vert.v = uv[i].y;

                    mesh_vert.tx = tangent.x;
                    mesh_vert.ty = tangent.y;
                    mesh_vert.tz = tangent.z;

                    mesh_vertices.push_back(mesh_vert);
                }
            }
        }

        uint32_t stride = sizeof(MeshVertexDataDefinition);
        mesh_data.m_vertex_buffer = std::make_shared<BufferData>(mesh_vertices.size() * stride);
        mesh_data.m_index_buffer = std::make_shared<BufferData>(mesh_vertices.size() * sizeof(uint16_t));

        assert(mesh_vertices.size() <= std::numeric_limits<uint16_t>::max()); // take care of the index range, should be
                                                                              // consistent with the index range used by
                                                                              // rhi

        uint16_t* indices = (uint16_t*)mesh_data.m_index_buffer->m_data;
        for (size_t i = 0; i < mesh_vertices.size(); i++)
        {
            ((MeshVertexDataDefinition*)(mesh_data.m_vertex_buffer->m_data))[i] = mesh_vertices[i];
            indices[i] = static_cast<uint16_t>(i);
        }

        return mesh_data;
    }

    std::string RenderResourceBase::loadFile(const std::string& filename)
    {
        std::string res;
        std::ifstream ifs;
        ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            ifs.open(filename);
            std::stringstream ss;
            ss << ifs.rdbuf();
            ifs.close();
            res = ss.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            LOG_ERROR("loadFile {} failed, error: {}", filename, e.what());
        }
        return res;
    }

}