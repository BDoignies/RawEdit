#pragma once

#include <cstdint>
#include "libraw.h"
#include "error.h"

namespace RawEdit
{
    namespace core
    {
        struct Image
        {
        public:
            Image();

            Error open(const char* filepath);

            std::string str() const;
            void release();

            const uint16_t* buffer() const;
            uint32_t width() const;
            uint32_t height() const;

            ~Image();
        private:
            bool loaded = false;

            LibRaw rawProcessor;
            libraw_processed_image_t* im;

            uint16_t* data = nullptr;
        };
    }
}
