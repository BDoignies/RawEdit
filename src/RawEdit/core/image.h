#pragma once

#include <string>
#include <memory>
#include <cstring>
#include <iostream>
#include <variant>
#include "error.h"

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
            CPU        = 1,
            GPU_OPENGL,
            GPU_CUDA
        };
        
        // Deleter for data will be provided with the returned
        // shared_ptr. 
        // This allows for data to hold completly different 
        // ressources that are dealocated whenever needed
        struct Image
        {
            static constexpr uint32_t MAX_BYTES_PER_COMP = 4;
            using MAX_COMP_TYPE = uint64_t;
            using AnyInt = std::variant<uint8_t, uint16_t, uint32_t, uint64_t>;

            Metadata metadata;

            uint32_t width    = 0;
            uint32_t height   = 0;
            uint8_t  channels = 0;
            uint8_t  bytes    = 0;

            Device device;
            uint8_t* data = nullptr;

            inline uint32_t GetIndex(uint32_t i, uint32_t j, uint32_t c) const 
            {
                return (c + (j + i * width) * channels) * bytes;
            }
            
            template<typename T>
            inline void SetValue(uint32_t i, uint32_t j, uint32_t c, T val)
            {
                AnyInt iVal;
                switch(bytes)
                {
                    case 1: iVal = std::uint8_t(val);  break;
                    case 2: iVal = std::uint16_t(val); break;
                    case 3:
                    case 4: iVal = std::uint32_t(val); break;
                    default:
                        iVal = std::uint64_t(val);
                        break;

                }
                std::visit([&](const auto& v) {
                    memcpy(data + GetIndex(i, j, c), &v, bytes);
                }, iVal);
            }

            inline MAX_COMP_TYPE GetValue(uint32_t i, uint32_t j, uint32_t c) const
            {
                MAX_COMP_TYPE value;
                memcpy(&value, data + GetIndex(i, j, c), bytes);
                return value;
            }
        };
        using ImagePtr = std::shared_ptr<Image>;

        Failable<ImagePtr> Transfer(ImagePtr source, Device device);
        Failable<ImagePtr> OpenImage(const char* path, Device device = Device::CPU);
        
    }
}
