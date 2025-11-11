#include "image.h"
#include "libraw.h"

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

        Failable<ImagePtr> Transfer(ImagePtr im, Device device)
        {
            if (im->device != Device::CPU)
                return std::unexpected(NotImplemented("Only transfer from CPU images are supported for now"));

            if (device != Device::CPU)
                return std::unexpected(NotImplemented("Only CPU images are supported for now"));
            
            return im;
        }

        Failable<ImagePtr> LoadImage(const char* path, Device device)
        {
            // Copy data
            Image* im = new Image;
            im->metadata.path   = path;
            im->metadata.source = "PC";
            im->device = Device::CPU;

            int width, height, channels;
            if (stbi_is_16_bit(path)) 
            {
                im->bytes = 2;
                im->data = (uint8_t*)stbi_load(path, &width, &height, &channels, 0);
            }
            else 
            {
                im->bytes = 1;
                im->data = (uint8_t*)stbi_load(path, &width, &height, &channels, 0);
            }
            im->width = width;
            im->height = height;
            im->channels = channels;

            if (im->data == nullptr)
            {
                delete im;
                return std::unexpected(IOError(std::format("[Image Loader] - Can not load '{}': {}", path, stbi_failure_reason())));
            }
            
            auto deleter = [](Image* ptr) {
                stbi_image_free(ptr->data); 
                delete ptr;
            };
            return Transfer(ImagePtr(im, deleter), device);
        }

        Failable<ImagePtr> LoadRAWImage(const char* path, Device device)
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
            Image* im = new Image;
            im->metadata.path   = path;
            im->metadata.source = std::format("{} - {}", processor.imgdata.idata.make, processor.imgdata.idata.model);
            im->width    = rawimage->width;
            im->height   = rawimage->height;
            im->bytes    = rawimage->bits / 8;
            im->channels = rawimage->colors;
            im->device   = Device::CPU;
            im->data     = new uint8_t[rawimage->data_size];
            memcpy(im->data, rawimage->data, rawimage->data_size);
            
            auto deleter = [](Image* ptr) {
                delete[] ptr->data; 
                delete   ptr;
            };
            LibRaw::dcraw_clear_mem(rawimage);
            return Transfer(ImagePtr(im, deleter), device);
        }

        Failable<ImagePtr> OpenImage(const char* path, Device device)
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
                return LoadImage(path, device);
            else 
                return LoadRAWImage(path, device);
        }
    }
}
