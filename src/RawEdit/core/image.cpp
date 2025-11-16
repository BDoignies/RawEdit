#include "image.h"
#include "libraw.h"

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "common.h"


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

        Image::Image(uint32_t w, uint32_t h)
        {
            width  = w;
            height = h;
            data   = new DataType[width * height * IMG_CHANNELS]{};
        }

        Image::~Image()
        {
            delete[] data;
        }

        Error Image::UploadGPU(bool wkcopy)
        {
            Error gpu = gpuImage.UploadData(width, height, (void*)data);
            if (gpu) return gpu;
            
            if (wkcopy)
            {
                Error wkc = workingCopy.UploadData(width, height, (void*)data);
                if (wkc) return wkc;
            }
            return NoError();
        }

        Failable<ImagePtr> LoadImage(const char* path)
        {

            int width, height, channels;
            uint8_t* data = stbi_load(path, &width, &height, &channels, 0);

            if (data == nullptr)
                return std::unexpected(IOError(std::format("[Image Loader] - Can not load '{}': {}", path, stbi_failure_reason())));

            ImagePtr im = std::make_shared<Image>(width, height);
            im->metadata.path   = path;
            im->metadata.source = "PC";
            
            #pragma omp parallel for collapse(2)
            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    uint32_t idx = j + i * width;

                    #pragma unroll
                    for (int c = 0; c < channels; ++c)
                        im->SetValue(i, j, c, data[c + idx * channels]);
                    
                    #pragma unroll
                    for (int c = channels; c < IMG_ALPHA; ++c)
                        im->SetValue(i, j, c, im->GetValue(i, j, 0));

                    #pragma unroll
                    for (int c = IMG_ALPHA; c < IMG_CHANNELS; ++c)
                        im->SetValue(i, j, c, IMG_MAX);
                }
            }
            return im;
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
            ImagePtr im = std::make_shared<Image>(rawimage->width, rawimage->height);
            im->metadata.path   = path;
            im->metadata.source = std::format("{} - {}", processor.imgdata.idata.make, processor.imgdata.idata.model);
            
            const std::uint8_t* data = (std::uint8_t*)rawimage->data;
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < im->height; ++i)
            {
                for (uint32_t j = 0; j < im->width; ++j)
                {
                    uint32_t idx = j + i * im->width;

                    #pragma unroll
                    for (int c = 0; c < rawimage->colors; ++c)
                        im->SetValue(i, j, c, data[c + idx * rawimage->colors] / 255.f16);

                    #pragma unroll
                    for (int c = rawimage->colors; c < IMG_ALPHA; ++c)
                        im->SetValue(i, j, c, im->GetValue(i, j, 0));

                    #pragma unroll
                    for (int c = IMG_ALPHA; c < IMG_CHANNELS; ++c)
                        im->SetValue(i, j, c, IMG_MAX);
                }
            }
            LibRaw::dcraw_clear_mem(rawimage);
            return im;
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
