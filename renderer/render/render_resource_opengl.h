
#pragma once

#include "render/render_type.h"
#include "core/math/axis_aligned.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Light
{
    class RHI;
    class RenderScene;
    class RenderCamera;

    class RenderResourceOpengl
    {
    public:
        virtual ~RenderResourceOpengl() {}


    private:
    };
}