#pragma once

#include "raylib.h"
#include "RawEdit/RawEdit.h"

int RaylibFormatFromImage(const RawEdit::core::Image* img);
Texture2D ConvertToRaylibTexture(const RawEdit::core::Image* img);
