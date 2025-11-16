#include "shadermanager.h"
#include <iostream>

namespace RawEdit
{
    namespace core
    {
        namespace fs = std::filesystem;
        
        Error ShaderManager::lastError = NoError();
        fs::path ShaderManager::shaderPath;
        std::map<std::string, ComputeShader> ShaderManager::shaderList;

        void ShaderManager::SetShaderPath(const std::string& path)
        {
            shaderPath = path;
        }

        Error ShaderManager::GetError() 
        {
            return lastError;
        }

        ComputeShader* ShaderManager::GetShader(const std::string& name, bool reload)
        {
            auto it = shaderList.find(name);
            lastError = NoError();
            if (it == shaderList.end() || reload)
            {
                const std::string path = shaderPath / (name + ".glsl");
                auto shader = ShaderFromFile(path);
                if (shader)
                {
                    shaderList[name] = *shader;
                    return &shaderList[name];
                }
                lastError = shader.error(); 
                return nullptr;
            }
            return &it->second;
        }
    }
}
