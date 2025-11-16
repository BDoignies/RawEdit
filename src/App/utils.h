#pragma once

#include <filesystem>
#include <GL/gl.h>
#include "raylib.h"

void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* data);

std::filesystem::path exeDirectory();
