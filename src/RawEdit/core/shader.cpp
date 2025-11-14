#include "shader.h"
#include "../externals/glad.h"

#include <fstream>
#include <format>

namespace RawEdit
{
    namespace core
    {
        ComputeShader::ComputeShader()
        {
            id = glCreateShader(GL_COMPUTE_SHADER);
        }

        Error ComputeShader::Compile(const char* source)
        {
            glShaderSource(id, 1, &source, NULL);
            glCompileShader(id);
            
            GLint status = 0;
            glGetShaderiv(id, GL_COMPILE_STATUS, &status);
            
            if (status == GL_FALSE)
            {
                GLint bufsize = 0;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &bufsize);

                std::string error(bufsize, '\0');
                glGetShaderInfoLog(id, bufsize, &bufsize, &error[0]);

                return ShaderError(error);
            }

            return NoError();
        }

        ComputeShader::~ComputeShader()
        {
            glDeleteShader(id);
        }


        Failable<ComputeShader> ShaderFromFile(const std::string& path)
        {
            std::ifstream file(path);

            if (!file)
                return std::unexpected(IOError(std::format("No such file or directory {}", path)));
            
            const uint32_t size = 4096;
            std::string src;
            std::string buffer(size, '\0');

            while(file.read(&buffer[0], size))
                src.append(buffer, 0, file.gcount());
            
            return ShaderFromSource(src.c_str());
        }
        
        Failable<ComputeShader> ShaderFromSource(const char* src)
        {
            ComputeShader shader;
            auto err = shader.Compile(src);

            if (err) return std::unexpected(err);
            return shader;
        }
    }
}
