#include "mask.h"
#include <iostream>

namespace RawEdit
{
    namespace algorithm
    {
        using namespace core;

        Mask::Mask(uint32_t w, uint32_t h)
        { 
            Reset(w, h);
        }

        void Mask::Reset(uint32_t w, uint32_t h)
        {
            maskCount = 0;
            mask = std::make_shared<Image>(w, h);
            mask->UploadGPU();

            // TODO:
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < mask->height; ++i)
            {
                for (uint32_t j = 0; j < mask->width; ++j)
                {
                    mask->SetValue(i, j, 3, 0.5f16);
                }
            }
        }

        void Mask::NewMask()
        {
            maskCount++;
            if (maskCount > MAX_MASK_COUNT)
                maskCount = MAX_MASK_COUNT;
        }

        uint32_t Mask::GetMaskCount() const
        {
            return maskCount;
        }

        void Mask::RemoveMask(uint32_t id)
        {
            for (uint32_t k = id; k < maskCount + 1; ++k)
            {
                #pragma omp parallel for collapse(2) 
                for (uint32_t i = 0; i < mask->height; ++i)
                {
                    for (uint32_t j = 0; j < mask->width; ++j)
                    {
                        const uint32_t val = mask->GetValue(i, j, 0);
                        const bool x = val & (1 << (k + 1));
                        const uint32_t newVal = (val & ~(1 << k)) | (x << k);
                        mask->SetValue(i, j, 0, newVal);
                    }
                }
            }
        }

        void Mask::Update()
        {
            mask->UploadGPU(false);
        }

        const Image* Mask::GetMask() const 
        {
            return mask.get();
        }

        void Mask::Inverse(uint32_t id)
        {
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < mask->height; ++i)
            {
                for (uint32_t j = 0; j < mask->width; ++j)
                {
                    const uint32_t val = mask->GetValue(i, j, 0);
                    const bool x = val & (1 << id);
                    const uint32_t newVal = (val & ~(1 << id)) | (!x << id);
                    mask->SetValue(i, j, 0, newVal);
                }
            }
        }

        void Mask::Circle(uint32_t id, uint32_t x, uint32_t y, double radius)
        {
            const uint32_t minX = (uint32_t)std::max(x - radius, 0.);
            const uint32_t minY = (uint32_t)std::max(y - radius, 0.);
            const uint32_t maxX = (uint32_t)std::min(x + radius, (double)mask->width);
            const uint32_t maxY = (uint32_t)std::min(y + radius, (double)mask->height);

            radius = radius * radius;
            for (uint32_t i = minY; i <= maxY; ++i)
            {
                for (uint32_t j = minX; j <= maxX; ++j)
                {
                    const double dist = (i - y) * (i - y) + (j - x) * (j - x);
                    if (dist < radius)
                    {
                        const uint32_t val = mask->GetValue(i, j, 0);
                        const uint32_t newVal = (val & ~(1 << id)) | (1 << id);
                        mask->SetValue(i, j, 0, newVal);
                    }
                }
            }
        }
    }
}
