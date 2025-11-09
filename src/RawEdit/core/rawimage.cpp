#include "rawimage.h"

#define IO_ERROR(path, code) \
    Error(Error::Code::IO_ERROR, std::format("IO Error ({}): {}", path, libraw_strerror(code)))


namespace RawEdit
{
    namespace core
    {
        Failable<RawImage> RawImage::open(const char* path, bool loadDisplay, bool loadThumbnail)
        {
            RawImage img;
            img.path = path;
            img.processor = std::make_shared<LibRaw>();

            const int retio = img.processor->open_file(path);
            if (retio != LIBRAW_SUCCESS)
                return std::unexpected(IO_ERROR(path, retio));

            if (loadDisplay)
            {
                Error err = img.LoadDisplay();
                if (err)
                    return std::unexpected(err);
            }

            if (loadThumbnail)
            {
                Error err = img.LoadThumbnail();
                if (err)
                    return std::unexpected(err);
            }

            return img;
        }

        std::string RawImage::getCamera() const
        {
            if (!processor)
                return std::string{};

            return std::format("{} - {}", processor->imgdata.idata.make, processor->imgdata.idata.model);
        }

        Failable<Image*> RawImage::GetDisplay()
        {
            if (display.data == nullptr)
            {
                Error err = LoadDisplay();
                if (err)
                    return std::unexpected(err);
            }
            return &display;
        }

        Failable<Image*> RawImage::GetThumbnail()
        {
            if (thumbnail.data == nullptr)
            {
                Error err = LoadThumbnail();
                if (err)
                    return std::unexpected(err);
            }
            return &thumbnail;
        }

        Error RawImage::LoadDisplay() 
        {
            // Already loaded
            if (display.data != nullptr)
                return NoError();
            
            const int retunpack = processor->unpack();
            if (retunpack != LIBRAW_SUCCESS)
                return IO_ERROR(path, retunpack);

            const int retconvert = processor->dcraw_process();
            if (retconvert != LIBRAW_SUCCESS)
                return IO_ERROR(path, retconvert);
            
            im = std::shared_ptr<libraw_processed_image_t>(
                    processor->dcraw_make_mem_image(), 
                    [](auto ptr) { LibRaw::dcraw_clear_mem(ptr); });

            if (!im)
                return Error(Error::Code::IO_ERROR, "Failed to extract image data");

            if (im->type != LIBRAW_IMAGE_BITMAP)
                return Error(Error::Code::IO_ERROR, "Can not display JPEG image");

            display.width = im->width; 
            display.height = im->height;
            display.bits = im->bits;
            display.channels = im->colors;
            display.data = reinterpret_cast<char*>(im->data);

            return NoError();
        }

        Error RawImage::LoadThumbnail()
        {
            return Error(Error::Code::NOT_IMPLEMENTED_ERROR, "Not implemented");
        }
    }
}
