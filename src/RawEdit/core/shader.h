#pragma once

#include "error.h"

namespace RawEdit
{
    namespace core
    {
        class ComputeShader
        {
        public:
            ComputeShader();

            Error Compile(const char* source);
            
            virtual ~ComputeShader();
        public:
            unsigned int id;
        };
        
        Failable<ComputeShader> ShaderFromFile(const std::string& path);
        Failable<ComputeShader> ShaderFromSource(const char* data);
    }
};
