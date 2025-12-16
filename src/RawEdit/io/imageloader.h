#pragma once

#include <image/image.h>
#include <utils/error.h>

namespace RawEdit
{
  Failable<ImagePtr> Load(const char* path);
}
