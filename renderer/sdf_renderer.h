
#pragma  once

#include <glad/glad.h>
#include <string>
#include <memory>

namespace Light
{

    class Shader;
    class WindowSystem;

    class SdfRenderer
    {
    public:
        explicit SdfRenderer(std::shared_ptr<WindowSystem> windowSystem);
        void setSdfShader(const char* shader);
        void setSdfShader(const std::string& shader);
        void run();

    private:
        std::unique_ptr<Shader> m_shader;
        std::shared_ptr<WindowSystem> m_windowSystem;
    };

}