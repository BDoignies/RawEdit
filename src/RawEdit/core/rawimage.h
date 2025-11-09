#pragma once

#include <cstdint>
#include <memory>
#include <format>

#include "libraw.h"
#include "error.h"

namespace RawEdit
{
    namespace core
    {
        struct Image
        {
            uint16_t width = 0;
            uint16_t height = 0;
            uint8_t channels = 0;
            uint8_t bits = 0;
            char* data = nullptr;
        };

        class RawImage
        {
        public:
            RawImage() {}
            ~RawImage() {}

            static Failable<RawImage> open(const char* path, bool loadDisplay = false, bool loadThumb = false);
            
            std::string getCamera() const;
            Failable<Image> GetDisplay();
            Failable<Image> GetThumbnail();

            Error LoadDisplay();
            Error LoadThumbnail();
        private:
            // LibRaw doesn't have proper cpy/move constructor
            // It has many nested structures with pointers...
            std::shared_ptr<LibRaw> processor;
            std::shared_ptr<libraw_processed_image_t> im;

            std::string path;
            Image display;
            Image thumbnail;
        };
    }
}
