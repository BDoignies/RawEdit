#include "openglimage.h"
#include <iostream>

namespace RawEdit
{
    namespace core
    {
        OpenGLImage::OpenGLImage()
        {
            glBindTexture(GL_TEXTURE_2D, 0);

            id = 0;
            width = height = 0;
        }

        Error OpenGLImage::UploadData(uint32_t w, uint32_t h, void* data)
        {
            if (id == 0)
                glGenTextures(1, &id);

            isLoaded = (data != nullptr);
            if (isLoaded)
            {

                width = w; 
                height = h;
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_2D, id);
                
                // TODO: SubImage when width/height are the same
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_HALF_FLOAT, data);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            return NoError();
        }

        OpenGLImage::~OpenGLImage()
        {
            glDeleteTextures(1, &id);
        }
    }
}
