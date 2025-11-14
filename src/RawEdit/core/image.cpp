#include "image.h"
#include "libraw.h"

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GL/gl.h"

#include <format>
#include <cstring>
#include <algorithm>
#include <filesystem>

namespace RawEdit
{
    namespace core
    {
        static const char* STB_IMAGE_FORMAT[] = {
            ".jpg", ".jpeg", ".png", 
        };
        bool stb_loadable(const std::string& ext) 
        {
            for (const char* format : STB_IMAGE_FORMAT)
            {
                if (ext == format)
                    return true;
            }
            return false;
        }

        Image::~Image()
        {
            delete[] data;
        }

        Error Image::UploadGPU()
        {
            return gpuImage.UploadData(width, height, (void*)data);
        }

        Failable<ImagePtr> LoadImage(const char* path)
        {
            // Copy data
            Image* im = new Image();
            im->metadata.path   = path;
            im->metadata.source = "PC";

            int width, height, channels;
            uint8_t* data = stbi_load(path, &width, &height, &channels, 0);
            
            if (data == nullptr)
            {
                delete im;
                return std::unexpected(IOError(std::format("[Image Loader] - Can not load '{}': {}", path, stbi_failure_reason())));
            }
            im->data = new std::float16_t[width * height * 3];
            
            #pragma omp parallel for collapse(2)
            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    uint32_t idx = j + i * width;
                    for (int c = 0; c < channels; ++c)
                        im->data[c + idx * 3] = data[c + idx * channels]/ 255.f16;

                    for (int c = channels; c < 3; ++c)
                        im->data[c + idx * 3] = im->data[0 + idx * 3];
                }
            }

            im->width = width;
            im->height = height;

            return ImagePtr(im);
        }

        Failable<ImagePtr> LoadRAWImage(const char* path)
        {
            LibRaw processor;

            const int retio = processor.open_file(path);
            if (retio != LIBRAW_SUCCESS)
                return std::unexpected(IOError(std::format("[RAW Loader] - Can not open '{}': {}", path, libraw_strerror(retio))));
           
            const int retunpack = processor.unpack();
            if (retunpack != LIBRAW_SUCCESS)
                return std::unexpected(IOError(std::format("[RAW Loader] - Can not extract data ('{}'): {}", path, libraw_strerror(retunpack))));

            const int retprocess = processor.dcraw_process();
            if (retprocess != LIBRAW_SUCCESS)
                return std::unexpected(IOError(std::format("[RAW Loader] - Can not extract data ('{}'): {}", path, libraw_strerror(retprocess))));

            auto rawimage = processor.dcraw_make_mem_image();
            if (!rawimage)
            {
                LibRaw::dcraw_clear_mem(rawimage);
                return std::unexpected(IOError(std::format("[RAW Loader] - Can not extract data ('{}'): {}", path, libraw_strerror(retprocess))));
            }

            if (rawimage->bits % 8 != 0)
                return std::unexpected(NotImplemented("RawEdit does not support non-byte value per component"));
            
            // Copy data
            Image* im = new Image();
            im->metadata.path   = path;
            im->metadata.source = std::format("{} - {}", processor.imgdata.idata.make, processor.imgdata.idata.model);
            im->width    = rawimage->width;
            im->height   = rawimage->height;
            
            const std::uint8_t* data = (std::uint8_t*)rawimage->data;
            im->data = new std::float16_t[im->width * im->height * 3];

            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < im->height; ++i)
            {
                for (uint32_t j = 0; j < im->width; ++j)
                {
                    uint32_t idx = j + i * im->width;

                    for (int c = 0; c < rawimage->colors; ++c)
                        im->data[c + idx * 3] = (std::float16_t)(data[c + idx * rawimage->colors]) / 255.f16; 

                    for (int c = rawimage->colors; c < 3; ++c)
                        im->data[c + idx * 3] = im->data[0 + idx * 3];
                }
            }
            LibRaw::dcraw_clear_mem(rawimage);
            return ImagePtr(im);
        }

        Failable<ImagePtr> OpenImage(const char* path)
        {
            namespace fs = std::filesystem;
            auto fpath = fs::path(path);

            if (!fs::is_regular_file(fpath))
                return std::unexpected(IOError(std::format("'{}' does not exist or is not a file", path)));

            auto ext = fpath.extension().native();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

            if (ext.empty())
                return std::unexpected(IOError(std::format("Can not deduce extension of '{}'", path)));
            
            if (stb_loadable(ext))
                return LoadImage(path);
            else 
                return LoadRAWImage(path);
        }
    }
}
