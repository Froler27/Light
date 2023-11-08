#include "render/render_system.h"

#include "core/base/macro.h"

//#include "asset_manager.h"

#include "render/render_camera.h"
#include "window_system.h"
#include "input/input_system.h"
#include "render/pipelines/render_pipeline_base.h"
#include "render/render_resource_base.h"
#include "render/render_scene.h"

#include "render/rhi/opengl/opengl_rhi.h"

namespace Light
{
    RenderSystem::~RenderSystem()
    {
        clear();
    }

    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // render context initialize
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;

        m_rhi = std::make_shared<OpenglRHI>();
        m_rhi->initialize(rhi_init_info);

        //m_render_resource = std::make_shared<RenderResourceBase>();
        m_render_scene = std::make_shared<RenderScene>(m_rhi);
       
        m_render_camera = std::make_shared<RenderCamera>();
        m_render_camera->lookAt(Vector3(0, -10, 0), Vector3(0.f), RenderCamera::Z);
       // m_render_camera->move(Vector3(0, -10, 0));
        m_render_camera->m_zfar  = 1000;
        m_render_camera->m_znear = 0.1f;
        m_render_camera->setAspect(init_info.window_system->getAspect());
        auto windowSize = init_info.window_system->getWindowSize();
        m_render_camera->setViewport({ 0, 0, windowSize[0], windowSize[1] });
        init_info.window_system->registerOnWindowSizeFunc(windowSizeChanged);


        // initialize render pipeline
        RenderPipelineInitInfo pipeline_init_info;
        pipeline_init_info.enable_fxaa     = false;

        m_render_pipeline        = std::make_shared<RenderPipelineBase>();
        m_render_pipeline->m_rhi = m_rhi;
        m_render_pipeline->initialize(pipeline_init_info);

        initScene();
    }

    void RenderSystem::clear()
    {
        if (m_rhi)
        {
            m_rhi->clear();
        }
        m_rhi.reset();
        
        if (m_render_pipeline)
        {
            m_render_pipeline->clear();
        }
        m_render_pipeline.reset();
    }

    std::shared_ptr<Light::RenderCamera> RenderSystem::getRenderCamera() const
    {
        return m_render_camera;
    }

    std::shared_ptr<Light::RHI> RenderSystem::getRHI() const
    {
        return m_rhi;
    }

    void RenderSystem::initScene()
    {
//          MeshSourceDesc meshDesc{ "../resources/models/sphere.obj" };
//          m_render_scene->addMesh(meshDesc);

        std::shared_ptr<PointLikeLight> light = std::make_shared<PointLikeLight>();
        m_render_scene->addLight(light);

        MaterialSourceDesc materialDesc;
        materialDesc.m_fragment_shader_file = "../resources/shaders/blinn_phong.fs";
        //materialDesc.m_base_color_file = "";
        m_render_scene->addShape(Shape::Sphere, Matrix4x4(), materialDesc);
        materialDesc.m_base_color_file = "phong_silver";
        m_render_scene->addShape(Shape::Plane, Matrix4x4::getTrans(0, 0, -1.f), materialDesc);
    }

    void RenderSystem::windowSizeChanged(int w, int h)
    {
        g_runtime_global_context.m_render_system->getRenderCamera()->setAspect(
            g_runtime_global_context.m_window_system->getAspect());
    }

    void RenderSystem::tick(float delta_time)
    {
        // prepare render command context
        //m_rhi->prepareContext();

        // prepare pipeline's render passes data
        //m_render_pipeline->preparePassData(m_render_resource);

        // render one frame
        //m_render_pipeline->forwardRender(m_rhi, m_render_resource);

        m_render_camera->rotate(Vector2(g_runtime_global_context.m_input_system->m_cursor_delta_pitch.valueDegrees(),
            -g_runtime_global_context.m_input_system->m_cursor_delta_yaw.valueDegrees()));
        auto command = g_runtime_global_context.m_input_system->getGameCommand();
        if (command < (unsigned int)GameCommand::invalid) {
            bool has_move_command = ((unsigned int)GameCommand::forward | (unsigned int)GameCommand::backward |
                (unsigned int)GameCommand::left | (unsigned int)GameCommand::right |
                (unsigned int)GameCommand::up | (unsigned int)GameCommand::down) & command;
            if (has_move_command) {
                Vector3 move_direction = Vector3::ZERO;

                if ((unsigned int)GameCommand::forward & command) {
                    move_direction += m_render_camera->forward();
                }
                if ((unsigned int)GameCommand::backward & command) {
                    move_direction -= m_render_camera->forward();
                }
                if ((unsigned int)GameCommand::left & command) {
                    move_direction -= m_render_camera->right();
                }
                if ((unsigned int)GameCommand::right & command) {
                    move_direction += m_render_camera->right();
                }
                if ((unsigned int)GameCommand::up & command) {
                    move_direction += m_render_camera->up();
                }
                if ((unsigned int)GameCommand::down & command) {
                    move_direction -= m_render_camera->up();
                }
                m_render_camera->move(move_direction * 2.0f * delta_time);
            }
        }
        
        auto shadowCamera = m_render_scene->m_lights[0]->m_shadow_camera;
        if (m_render_scene->m_lights.size() > 0) {
            m_rhi->bindCamera(shadowCamera);

            for (auto itr_meshs = m_render_scene->m_material_to_meshs.begin();
                itr_meshs != m_render_scene->m_material_to_meshs.end();)
            {
                auto materialId = itr_meshs->first;
                //m_rhi->bindMaterial(materialId);

                auto range_meshs = m_render_scene->m_material_to_meshs.equal_range(materialId);
                for (auto itr_mesh = range_meshs.first; itr_mesh != range_meshs.second; ++itr_mesh) {
                    auto meshId = itr_mesh->second;
                    m_rhi->bindMesh(meshId);
                    auto range_entities = m_render_scene->m_render_entities.equal_range(meshId);
                    uint32_t instance_num = m_render_scene->m_render_entities.count(meshId);
                    if (instance_num == 1) {
                        m_rhi->drawMesh(meshId, range_entities.first->second.m_model_matrix);
                    }
                    else {
                        // TODO: 实例化绘制
                        for (auto itr_entity = range_entities.first; itr_entity != range_entities.second; ++itr_entity) {
                            const auto& entity = itr_entity->second;
                            m_rhi->drawMesh(meshId, entity.m_model_matrix);
                        }
                    }
                }
                itr_meshs = range_meshs.second;
            }

        }
        //shadowCamera->m_current_camera_type = RenderCameraType::Editor;
        m_rhi->bindCamera(m_render_camera);
        //m_rhi->bindCamera(shadowCamera);

        for (auto itr_meshs = m_render_scene->m_material_to_meshs.begin();
            itr_meshs != m_render_scene->m_material_to_meshs.end();)
        {
            auto materialId = itr_meshs->first;
            m_rhi->bindMaterial(materialId);

            auto range_meshs = m_render_scene->m_material_to_meshs.equal_range(materialId);
            for (auto itr_mesh = range_meshs.first; itr_mesh != range_meshs.second; ++itr_mesh) {
                auto meshId = itr_mesh->second;
                m_rhi->bindMesh(meshId);
                auto range_entities = m_render_scene->m_render_entities.equal_range(meshId);
                uint32_t instance_num = m_render_scene->m_render_entities.count(meshId);
                if (instance_num == 1) {
                    m_rhi->drawMesh(meshId, range_entities.first->second.m_model_matrix);
                }
                else {
                    // TODO: 实例化绘制
                    for (auto itr_entity = range_entities.first; itr_entity != range_entities.second; ++itr_entity) {
                        const auto& entity = itr_entity->second;
                        m_rhi->drawMesh(meshId, entity.m_model_matrix);
                    }
                }
            }
            itr_meshs = range_meshs.second;
        }

        //drawAxis();

    }

    void RenderSystem::drawAxis()
    {
        static auto mesh_asset_id = m_render_scene->m_mesh_asset_id_allocator.allocGuid(MeshSourceDesc{ "Axis" });

        MaterialSourceDesc material;
        auto material_asset_id = m_render_scene->m_material_asset_id_allocator.allocGuid(material);
        RenderMaterialData materialData = m_render_scene->m_render_resource->loadMaterialData(material);
        m_rhi->updateMaterial(materialData, material_asset_id);

        static RenderMeshData meshData;
        m_render_scene->getAxisData(meshData);
        m_rhi->updateRHIMesh(meshData, mesh_asset_id);

    }

}