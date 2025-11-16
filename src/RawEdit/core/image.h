#pragma once

#include <string>
#include <memory>
#include <cstring>
#include <iostream>
#include <variant>
#include <stdfloat>
#include "error.h"
#include "openglimage.h"

#include "GL/gl.h"

namespace RawEdit
{
    namespace core
    {
        struct Metadata
        {
            std::string path;
            std::string source;
        };

        enum class Device
        {
            CPU = 0, 
            GPU_OPENGL = 1
        };

        struct Image
        {
            Metadata metadata;

            uint32_t width    = 0;
            uint32_t height   = 0;
            DataType* data  = nullptr;
            
            OpenGLImage gpuImage;
            OpenGLImage workingCopy;

            Image(uint32_t w, uint32_t h);
            Image(const Image& other) = delete;
            Image(Image&& other)      = delete;
            Image& operator=(const Image& other) = delete;
            ~Image();

            Error UploadGPU(bool wcopy = false);

            inline uint32_t GetIndex(uint32_t i, uint32_t j, uint32_t c) const 
            {
                return c + (j + i * width) * IMG_CHANNELS;
            }

            inline uint64_t DataSize() const 
            {
                return width * height * IMG_CHANNELS * sizeof(DataType);
            }

            inline uint64_t GPUDataSize() const 
            {
                return (gpuImage.width * gpuImage.height + workingCopy.width * workingCopy.height) * IMG_CHANNELS * sizeof(DataType);
            }
            
            inline void SetValue(uint32_t i, uint32_t j, uint32_t c, DataType val)
            {
                data[GetIndex(i, j, c)] = val;
            }

            inline DataType GetValue(uint32_t i, uint32_t j, uint32_t c) const
            {
                return data[GetIndex(i, j, c)];
            }
        };

        using ImagePtr = std::shared_ptr<Image>;
        Failable<ImagePtr> OpenImage(const char* path);
    }
}
