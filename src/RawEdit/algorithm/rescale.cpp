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

            Image* newIm = new Image;
            newIm->metadata = im->metadata;
            newIm->width  = w;
            newIm->height = h;
            newIm->data = new std::float16_t[w * h * 3];

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
                        sizeof(std::float16_t) * 3
                    );
                }
            }
            
            return ImagePtr(newIm);
        }

        ImagePtr BilinearCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            const float wratio = im->width  / w;
            const float hratio = im->height / h;

            Image* newIm = new Image;
            newIm->metadata = im->metadata;
            newIm->width  = w;
            newIm->height = h;
            newIm->data = new std::float16_t[w * h * 3];
            
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

                    for (uint32_t ch = 0; ch < 3; ++ch)
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

                        newIm->SetValue(i, j, ch, (std::float16_t)val);
                    }
                }
            }
            
            return ImagePtr(newIm);
        }

        ImagePtr BicubicCPU(ImagePtr im, uint32_t w, uint32_t h)
        {
            constexpr auto kernel = [](float t, float a, float b, float c, float d) {
                const float t2 = t * t;
                const float t3 = t2 * t;

                const float e = 2 * b;
                const float f = -a + c;
                const float g = 2 * a - 5 * b + 4 * c - d;
                const float h = -1 + 3 * b - 3 * c + d;
                return 0.5 * (e + t * f + t2 * g + t3 * h);
            };
            const float wratio = im->width  / w;
            const float hratio = im->height / h;

            Image* newIm = new Image;
            newIm->metadata = im->metadata;
            newIm->width  = w;
            newIm->height = h;
            newIm->data = new std::float16_t[w * h * 3];
            
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < h; ++i)
            {
                for (uint32_t j = 0; j < w; ++j)
                {
                    const float x = (j * wratio);
                    const float y = (i * hratio);
                    
                    const uint32_t xim1 = (uint32_t)std::max(x - 1.f, 0.f);
                    const uint32_t yim1 = (uint32_t)std::max(y - 1.f, 0.f);
                    const uint32_t xi0  = std::min((uint32_t)x, im->width  - 1);
                    const uint32_t yi0  = std::min((uint32_t)y, im->height - 1);
                    const uint32_t xi1  = std::min(xi0  + 1, im->width  - 1);
                    const uint32_t yi1  = std::min(yi0  + 1, im->height - 1);
                    const uint32_t xi2  = std::min(xi0  + 2, im->width  - 1);
                    const uint32_t yi2  = std::min(yi0  + 2, im->height - 1);

                    const float dx = x - xi0;
                    const float dy = y - yi0;

                    for (uint32_t ch = 0; ch < 3; ++ch)
                    {
                        const float bm1 = kernel(dx, 
                            im->GetValue(yim1, xim1, ch), im->GetValue(yim1, xi0, ch), 
                            im->GetValue(yim1, xi1 , ch), im->GetValue(yim1, xi2, ch)
                        );
                        const float b0 = kernel(dx, 
                            im->GetValue(yi0, xim1, ch), im->GetValue(yi0, xi0, ch), 
                            im->GetValue(yi0, xi1 , ch), im->GetValue(yi0, xi2, ch)
                        );
                        const float b1 = kernel(dx, 
                            im->GetValue(yi1, xim1, ch), im->GetValue(yi1, xi0, ch), 
                            im->GetValue(yi1, xi1 , ch), im->GetValue(yi1, xi2, ch)
                        );
                        const float b2 = kernel(dx, 
                            im->GetValue(yi2, xim1, ch), im->GetValue(yi2, xi0, ch), 
                            im->GetValue(yi2, xi1 , ch), im->GetValue(yi2, xi2, ch)
                        );
                        const float val = kernel(dy, bm1, b0, b1, b2);

                        newIm->SetValue(i, j, ch, (std::float16_t)val);
                    }
                }
            }
            
            return ImagePtr(newIm);
        }

        ImagePtr Rescale(ImagePtr im, uint32_t w, uint32_t h, RescaleMethod method, Device device)
        {
            if (device == Device::CPU)
            {
                switch (method)
                {
                case RescaleMethod::NEAREST:  return NearestCPU(im, w, h);
                case RescaleMethod::BILINEAR: return BilinearCPU(im, w, h);
                case RescaleMethod::BICUBIC:  return BicubicCPU(im, w, h);
                default:
                    return nullptr;
                };
            }
            else if (device == Device::GPU_OPENGL)
            {
                // TODO: Do it on texture directly
                auto newIm = Rescale(im, w, h, method, Device::CPU);
                im->gpuImage.UploadData(newIm->width, newIm->height, newIm->data);
                return im;
            }
            return nullptr;
        }
    }
}
