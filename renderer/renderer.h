#pragma  once

#include <glad/glad.h>

namespace Light
{

    class Renderer
    {
    public:
        void initialize(GLADloadproc proc);
        void clear();
    };

}