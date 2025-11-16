#pragma once

#include <stdfloat>
#include "../externals/glad.h"

// Note: not all gpu supports RGB16f...
// We rely on RGBA instead...
using DataType = std::float16_t;
inline constexpr unsigned char IMG_CHANNELS = 4;
inline constexpr unsigned char IMG_ALPHA = 3;
inline constexpr DataType IMG_MIN = 0.f16;
inline constexpr DataType IMG_MAX = 1.f16;
inline constexpr GLint GLIMG_INTERNAL_FORMAT = GL_RGBA16F;
inline constexpr GLint GLIMG_FORMAT = GL_RGBA;
inline constexpr GLint GLIMG_TYPE = GL_HALF_FLOAT;
