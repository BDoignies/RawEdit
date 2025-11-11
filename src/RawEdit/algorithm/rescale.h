#pragma once

#include "../core/error.h"
#include "../core/image.h"

namespace RawEdit
{
    namespace algorithm
    {
        using namespace core;
        enum class RescaleMethod
        {
            NEAREST  = 0,
            BILINEAR,
            BICUBIC,
            LANCZOS
        };

        ImagePtr Rescale(ImagePtr im, uint32_t w, uint32_t h, RescaleMethod method = RescaleMethod::NEAREST);
    }
}

