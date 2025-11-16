#pragma once

#include "../core/image.h"

namespace RawEdit
{
    namespace algorithm
    {
        using namespace core;

        // Red   is for mask
        class Mask
        {
        public:
            static constexpr uint32_t MAX_MASK_COUNT = 16;

            Mask(uint32_t width = 1, uint32_t height = 1);
            
            void Update();
            void Reset(uint32_t width, uint32_t height);

            const Image* GetMask() const;

            void NewMask();
            uint32_t GetMaskCount() const;
            void RemoveMask(uint32_t id);  
            
            void Inverse(uint32_t id);
            void Circle(uint32_t id, uint32_t x, uint32_t y, double radius);
        private:
            uint32_t maskCount;
            ImagePtr mask;
        };
    }
}
