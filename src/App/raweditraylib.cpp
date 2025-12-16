#include "raweditraylib.h"
#include "spdlog/spdlog.h"

Texture2D ConvertToRaylibTexture(const RawEdit::Image* img)
{
    if (img->backend != RawEdit::ImageBackend::CPU && 
        img->type != RawEdit::ImageDataType::UINT8)
    {
        spdlog::error("Unsupported img type");
        return {};
    }

    spdlog::warn("Conversion using upload copy");

    using ImType = const RawEdit::CPUImage<uint8_t>*;
    ImType rawim = reinterpret_cast<ImType>(img);
    Image im = {
        .data    = const_cast<uint8_t*>(rawim->GetDataPtr()),
        .width   = (int)img->width, 
        .height  = (int)img->height, 
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8
    };

    if (img->channels == 1)
        im.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

    return LoadTextureFromImage(im);
}
