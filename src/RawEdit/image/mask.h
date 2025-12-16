#pragma one

#include <cmath>
#include <algorithm>
#include "image.h"

namespace RawEdit
{
    using MaskDataType = uint8_t;
    class Mask : public CPUImage<MaskDataType>
    {
    public:
        static constexpr unsigned int MAX_MASK_COUNT = sizeof(MaskDataType) * 8;

        Mask()
        { }

        Mask(uint32_t w, uint32_t h, bool on = true) 
        {
            FillData(w, h, 1, on);
            updated = true;
            currentMaskCount = 1;
        }

        uint32_t GetMaskCount() const 
        {
            return currentMaskCount;
        }

        void NewMask()
        {
            currentMaskCount ++;
        }

        void Circle(MaskDataType mId, uint32_t x, uint32_t y, float radius)
        {
            if (radius < 0) return;

            const uint32_t b1 = std::round(radius);
            const uint32_t bound = b1 + !(b1 & 1);

            for (int i = -bound; i <= bound; ++i)
            {
                for (int j = -bound; j <= bound; ++j)
                {
                    if ((i * i + j * j) < bound * bound)
                    {
                        const uint32_t xi = std::clamp(x + j, 0u, width);
                        const uint32_t yi = std::clamp(y + i, 0u, height);
                        Set(mId, xi, yi, true);
                    }
                }
            }
        }

        void Set(MaskDataType mId, uint32_t x, uint32_t y, bool value)
        {
            SetData(x, y, 1, GetData(x, y, 1) & ~((MaskDataType)1 << mId) | ((MaskDataType)value << mId));
        }

        bool Updated()
        {
            if (updated)
            {
                updated = false;
                return true;
            }
            return false;
        }

        ~Mask()
        { }

    private:
        uint32_t currentMaskCount = 0;
        bool updated = false;
    };
}

