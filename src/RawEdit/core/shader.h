#pragma once

#include <map>
#include "error.h"
#include "openglimage.h"

namespace RawEdit
{
    namespace core
    {
        class ComputeShader
        {
        public:
            ComputeShader();

            Error Compile(const char* source);

            Error AddUniform(const std::string& name);
            void ComputeDispatchSizes(uint32_t sizeX, uint32_t sizeY);
            
            void Bind();
            void SetUniform(const std::string& name, const OpenGLImage& im, bool read = true, bool write = false);
            void SetUniform(const std::string& name, float value);
            void RunAndWait();
            
            ~ComputeShader();
        private:
            std::map<std::string, int> locationMap;
            uint32_t dispatchX;
            uint32_t dispatchY;

            int workGroupSize[3];
            unsigned int id;
        };
        
        Failable<ComputeShader> ShaderFromFile(const std::string& path);
        Failable<ComputeShader> ShaderFromSource(const char* data);
    }
};
