#pragma once

#include <filesystem>
#include <map>
#include "shader.h"

namespace RawEdit
{
    namespace core
    {
        namespace fs = std::filesystem;

        class ShaderManager
        {
        public:
            static void  SetShaderPath(const std::string& path);
            static const ComputeShader* GetShader(const std::string& name, bool reload = false);
            static Error GetError();
        private:
            static Error lastError;
            static fs::path shaderPath;
            static std::map<std::string, ComputeShader> shaderList;
        };
    }
}
