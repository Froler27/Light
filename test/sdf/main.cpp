#include <memory>

#include "engine.h"

const std::string SdfShaderPathPrefix{"../resources/sdf_shaders/"};
const std::string TexturePathPrefix{"../resources/textures/"};

int main()
{
    std::shared_ptr<Light::Engine> engine = std::make_shared<Light::Engine>();
    engine->startEngine("");
    engine->initialize();

    engine->run();

    engine->clear();
    engine->shutdownEngine();

//     Light::SdfRenderer renderer(windowSystem);
//     renderer.setSdfShader(SdfShaderPathPrefix+"sdf.fs");
//     renderer.run();

    return 0;
}