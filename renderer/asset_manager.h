
#pragma  once

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <iostream>

#include "render_type.h"

namespace Light
{
    class AssetManager
    {
    public:
        std::filesystem::path getFullPath(const std::string& relative_path) const;
        static StaticMeshData loadStaticMesh(std::string filename);
    };
}