#include "raweditraylib.h"
#include "spdlog/spdlog.h"

Texture2D ConvertToRaylibTexture(const RawEdit::core::Image* img)
{
    if (img->workingCopy.Loaded())
    {
        spdlog::info("Using direct conversion (working copy)");
        Texture2D texture = {
            .id = img->workingCopy.id,
            .width   = (int)img->workingCopy.width, 
            .height  = (int)img->workingCopy.height, 
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16
        };
        return texture;
    }
    else if(img->gpuImage.Loaded())
    {
        spdlog::info("Using direct conversion (gpuImage copy - {})", img->gpuImage.id);
        Texture2D texture = {
            .id      = img->gpuImage.id,
            .width   = (int)img->gpuImage.width, 
            .height  = (int)img->gpuImage.height, 
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16
        };
        return texture;
    }
    else
    {
        spdlog::warn("Conversion using upload copy");
        Image im = {
            .data    = img->data,
            .width   = (int)img->width, 
            .height  = (int)img->height, 
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16
        };

        return LoadTextureFromImage(im);
    }
    return {};
}
