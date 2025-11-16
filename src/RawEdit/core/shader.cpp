#include "shader.h"
#include "../externals/glad.h"

#include <fstream>
#include <sstream>
#include <format>

namespace RawEdit
{
    namespace core
    {
        ComputeShader::ComputeShader()
        {
            id = glCreateProgram();
            workGroupSize[0] = 0;
            workGroupSize[1] = 0;
            workGroupSize[2] = 0;
        }

        Error ComputeShader::Compile(const char* source)
        {
            unsigned int shaderId = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(shaderId, 1, &source, NULL);
            glCompileShader(shaderId);
            
            GLint status = 0;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
            
            if (status == GL_FALSE)
            {
                GLint bufsize = 0;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &bufsize);

                std::string error(bufsize, '\0');
                glGetShaderInfoLog(shaderId, bufsize, &bufsize, &error[0]);

                return ShaderError(error);
            }
            
            glAttachShader(id, shaderId);
            glLinkProgram(id);
            return NoError();
        }

        ComputeShader::~ComputeShader()
        {
            // glDeleteShader(id);
        }

        Error ComputeShader::AddUniform(const std::string& name)
        {
            int loc = glGetUniformLocation(id, name.c_str());
            if (loc != -1)
            {
                locationMap[name] = loc;
                return NoError();
            }
            return ShaderError(std::format("Can not find location of {} in shader {}", name, id));
        }

        void ComputeShader::ComputeDispatchSizes(uint32_t width, uint32_t height)
        {
            if (workGroupSize[0] == 0 &&
                workGroupSize[1] == 0 && 
                workGroupSize[2] == 0)
            {
                glGetProgramiv(id, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
            }

            dispatchX = (width  + workGroupSize[0] - 1) / workGroupSize[0];
            dispatchY = (height + workGroupSize[1] - 1) / workGroupSize[1];
        }

        void ComputeShader::Bind()
        {
            glUseProgram(id);
        }

        void ComputeShader::RunAndWait()
        {
            glDispatchCompute(dispatchX, dispatchY, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glUseProgram(0);
        }

        void ComputeShader::SetUniform(const std::string& name, const OpenGLImage& im, bool read, bool write)
        {
            GLenum access = GL_READ_ONLY;
            if (!read && write) access = GL_WRITE_ONLY;
            if ( read && write) access = GL_READ_WRITE;

            int loc = locationMap[name];
            glBindImageTexture(loc, im.id, 0, false, 0, access, GLIMG_INTERNAL_FORMAT);
        }

        void ComputeShader::SetUniform(const std::string& name, float value)
        {
            glUniform1f(locationMap[name], value);
        }

        Failable<ComputeShader> ShaderFromFile(const std::string& path)
        {
            std::ifstream file(path);

            if (!file)
                return std::unexpected(IOError(std::format("No such file or directory {}", path)));
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            return ShaderFromSource(buffer.str().c_str());
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
