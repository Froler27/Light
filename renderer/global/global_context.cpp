#include "global_context.h"

#include "core/log/log_system.h"

#include "asset_manager.h"

#include "input/input_system.h"
#include "render/render_system.h"
#include "window_system.h"

namespace Light
{
    RuntimeGlobalContext g_runtime_global_context;

    void RuntimeGlobalContext::startSystems(const std::string& config_file_path)
    {
        m_logger_system = std::make_shared<LogSystem>();

        m_asset_manager = std::make_shared<AssetManager>();

        m_window_system = std::make_shared<WindowSystem>();
        WindowCreateInfo window_create_info;
        m_window_system->initialize(window_create_info);

        m_input_system = std::make_shared<InputSystem>();
        m_input_system->initialize();

        m_render_system = std::make_shared<RenderSystem>();
        RenderSystemInitInfo render_init_info;
        render_init_info.window_system = m_window_system;
        m_render_system->initialize(render_init_info);
    }

    void RuntimeGlobalContext::shutdownSystems()
    {
        m_window_system.reset();

        m_input_system->clear();
        m_input_system.reset();

        m_asset_manager.reset();

        m_logger_system.reset();
    }
}