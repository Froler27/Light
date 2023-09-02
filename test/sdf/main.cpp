#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <memory>

#include <window_system.h>
#include <sdf_renderer.h>

void framebuffer_size_callback(int width, int height);
void processInput(GLFWwindow* window);
void handleKeyEvent(int key, int scancode, int action, int mods);

std::shared_ptr<Light::WindowSystem> windowSystem;

const std::string SdfShaderPathPrefix{"../resources/sdf_shaders/"};
const std::string TexturePathPrefix{"../resources/textures/"};

int main()
{
    windowSystem = std::make_shared<Light::WindowSystem>();
    windowSystem->initialize(Light::WindowCreateInfo());
    
    windowSystem->registerOnWindowSizeFunc(framebuffer_size_callback);
    windowSystem->registerOnKeyFunc(handleKeyEvent);

    Light::SdfRenderer renderer(windowSystem);
    renderer.setSdfShader(SdfShaderPathPrefix+"sdf.fs");
    renderer.run();

    return 0;
}

void framebuffer_size_callback(int width, int height)
{
    glViewport(0, 0, width, height);
}

void handleKeyEvent(int key, int scancode, int action, int mods)
{
    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) {
            windowSystem->setShouldClose(true);
        }
        break;
    default:
        break;
    }
}