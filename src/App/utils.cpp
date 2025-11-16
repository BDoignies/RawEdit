#include "utils.h"
#include "spdlog/spdlog.h"

void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* data)
{
    spdlog::error("OPENGL ERROR message {}: {}", id, message);
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             spdlog::error("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   spdlog::error("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: spdlog::error("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     spdlog::error("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     spdlog::error("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           spdlog::error("Source: Other"); break;
        default: spdlog::error("Source: N.A");
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               spdlog::error("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: spdlog::error("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  spdlog::error("Type: Undefined Behaviour"); break;
        case GL_DEBUG_TYPE_PORTABILITY:         spdlog::error("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         spdlog::error("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              spdlog::error("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          spdlog::error("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           spdlog::error("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               spdlog::error("Type: Other"); break;
        default: spdlog::error("Type: N.A");
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         spdlog::error("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       spdlog::error("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          spdlog::error("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: spdlog::error("Severity: notification"); break;
        default: spdlog::error("Severity: N.A");
    }
}

#if defined(_MSC_VER)
#include <windows.h> // TODO: Test this include and the function below
#else
#include <unistd.h>
#endif
// https://gist.github.com/Jacob-Tate/7b326a086cf3f9d46e32315841101109
std::filesystem::path exeDirectory()
{
    return std::filesystem::path(GetApplicationDirectory());
}


