#include "rescale.h"

#include <cstring>
#include <iostream>
#include <format>

namespace RawEdit
{
    namespace algorithm
    {
        ImagePtr NearestCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            const float wratio = im->width  / w;
            const float hratio = im->height / h;

            Image* newIm = new Image(*im);
            newIm->width  = w;
            newIm->height = h;
            newIm->data = new uint8_t[w * h * im->channels * im->bytes];

            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < h; ++i)
            {
                for (uint32_t j = 0; j < w; ++j)
                {
                    const uint32_t srcX = std::min((uint32_t)(j * wratio), im->width  - 1);
                    const uint32_t srcY = std::min((uint32_t)(i * hratio), im->height - 1);

                    memcpy(
                        newIm->data + newIm->GetIndex(   i,    j, 0),
                           im->data +    im->GetIndex(srcY, srcX, 0),
                        im->bytes * im->channels
                    );
                }
            }
            
            auto deleter = [](Image* ptr) {
                delete[] ptr->data;
                delete ptr;
            };
            return ImagePtr(newIm, deleter);
        }

        ImagePtr BilinearCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            const float wratio = im->width  / w;
            const float hratio = im->height / h;

            Image* newIm = new Image(*im);
            newIm->width  = w;
            newIm->height = h;
            newIm->data = new uint8_t[w * h * im->channels * im->bytes];
            
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < h; ++i)
            {
                for (uint32_t j = 0; j < w; ++j)
                {
                    const float x = (j * wratio);
                    const float y = (i * hratio);

                    const uint32_t xi = std::min((uint32_t)x, im->width  - 1);
                    const uint32_t yi = std::min((uint32_t)y, im->height - 1);
                    const uint32_t xj = std::min(xi + 1, im->width  - 1);
                    const uint32_t yj = std::min(yi + 1, im->height - 1);

                    const float xw = x - xi;
                    const float yw = y - yi;

                    for (uint32_t ch = 0; ch < im->channels; ++ch)
                    {
                        const float a = (float)im->GetValue(yi, xi, ch);
                        const float b = (float)im->GetValue(yj, xi, ch);
                        const float c = (float)im->GetValue(yi, xj, ch);
                        const float d = (float)im->GetValue(yj, xj, ch);

                        const float val = 
                            a * (1 - xw) * (1 - yw) +
                            b *      xw  * (1 - yw) +
                            c * (1 - xw) *      yw  +
                            d *      xw  *      yw;

                        newIm->SetValue(i, j, ch, val);
                    }
                }
            }
            
            auto deleter = [](Image* ptr) {
                delete[] ptr->data;
                delete ptr;
            };
            return ImagePtr(newIm, deleter);
        }

        ImagePtr BicubicCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            return nullptr;
        }

        ImagePtr LanczosCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            return nullptr;
        }


        ImagePtr Rescale(ImagePtr im, uint32_t w, uint32_t h, RescaleMethod method)
        {
            if (im->device == Device::CPU)
            {
                switch (method)
                {
                case RescaleMethod::NEAREST:  return NearestCPU(im, w, h);
                case RescaleMethod::BILINEAR: return BilinearCPU(im, w, h);
                case RescaleMethod::BICUBIC:  return BicubicCPU(im, w, h);
                case RescaleMethod::LANCZOS:  return LanczosCPU(im, w, h);
                default:
                    return nullptr;
                };
            }
            return nullptr;
        }
    }
}
