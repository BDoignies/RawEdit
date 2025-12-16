#include "imageloader.h"
#include "stb_image.h"

namespace RawEdit 
{
    Failable<ImagePtr> LoadImage(const char* path)
    {
        int width, height, channels;
        uint8_t* data = stbi_load(path, &width, &height, &channels, 0);
    
        if (data == nullptr)
            return Failed("[Image Loader] - Can not load '{}': {}", path, stbi_failure_reason());
        
        ImagePtr image = std::make_shared<CPUImage<uint8_t>>();
        image->metadata.path   = path;
        image->metadata.source = "PC";
        image->SetData(width, height, channels, ImageDataType::UINT8, data);
    
        stbi_image_free(data);
        return image;
    }

    Failable<ImagePtr> Load(const char* path)
    {
        return LoadImage(path);
    }
}
