#pragma once

#include <cstdint>
#include "error.h"
#include "GL/gl.h"

namespace RawEdit
{
    namespace core
    {
        class OpenGLImage
        {
        public:
            OpenGLImage();

            OpenGLImage(const OpenGLImage& other) = delete;
            OpenGLImage(OpenGLImage&& other) = delete;
            OpenGLImage& operator=(const OpenGLImage& other) = delete;

            Error UploadData(uint32_t width, uint32_t height, void* data);

            ~OpenGLImage();

            bool Loaded() const { return isLoaded; }

            uint32_t DataSize();
        public:
            GLuint id;
            GLint internalFormat;
            GLint format;
            GLint type;

            uint32_t width;
            uint32_t height;
        private:
            bool isLoaded = false;
        };
    }
}
