#include "image.h"

#include <iostream>
#include <format>

namespace RawEdit
{
    namespace core
    {
        Image::Image() 
        {
        }

        Error Image::open(const char* path) 
        {
            release();
            
            const int retio = rawProcessor.open_file(path);
            if (retio != LIBRAW_SUCCESS)
            {
                Error err(Error::Code::IO_ERROR);
                err.errorString = std::format("IO Error for {}: '{}'", path, libraw_strerror(retio));
                return err;
            }

            const int retunpack = rawProcessor.unpack();
            if (retunpack != LIBRAW_SUCCESS) 
            {
                Error err(Error::Code::IO_ERROR);
                err.errorString = std::format("IO Error for {}: '{}'", path, libraw_strerror(retunpack));
                return err;
            }
            
            // TODO: Custom bayer democaising (especially if GPU is involved)
            const int retconvert = rawProcessor.dcraw_process();
            if (retconvert != LIBRAW_SUCCESS) 
            {
                Error err(Error::Code::IO_ERROR);
                err.errorString = std::format("IO Error for {}: '{}'", path, libraw_strerror(retunpack));
                return err;
            }
            im = rawProcessor.dcraw_make_mem_image();
            if (!im) 
            {
                Error err(Error::Code::IO_ERROR);
                err.errorString = std::format("Can not unpack raw data for {}", path);
                return err;
            }

            loaded = true;
            return NoError();
        }

        const uint16_t* Image::buffer() const 
        {
            if (!loaded)
                return nullptr;
            
            return reinterpret_cast<uint16_t*>(im->data);
        }

        uint32_t Image::width() const
        {
            if (!loaded)
                return 0;

            return im->width;
        }

        uint32_t Image::height() const
        {
            if (!loaded)
                return 0;

            return im->height;
        }

        std::string Image::str() const 
        {
            if (!loaded) 
                return std::string{"[Image not loaded]"};

            return std::format(
                "Image ({}x{} px), from camera ({})", 
                rawProcessor.imgdata.sizes.width, 
                rawProcessor.imgdata.sizes.height, 
                std::format("{} - {}", rawProcessor.imgdata.idata.make, rawProcessor.imgdata.idata.model)
            );
        }

        void Image::release() 
        {
            if (!loaded) return;

            LibRaw::dcraw_clear_mem(im);
            rawProcessor.recycle();
            loaded = false;
        }
        
        Image::~Image()
        {
            release();
        }
        
    };
};
