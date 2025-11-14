#pragma once

#include "raylib.h"
#include "RawEdit/RawEdit.h"

int RaylibFormatFromImage(RawEdit::core::ImagePtr img);
Texture2D ConvertToRaylibTexture(RawEdit::core::ImagePtr img);
