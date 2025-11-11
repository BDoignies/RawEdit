#pragma once

#include <expected>
#include <string>

namespace RawEdit
{
    namespace core 
    {
        struct Error
        {
            enum class Code
            {
                NO_ERROR              = 0,
                UK_ERROR              = 1,
                NOT_IMPLEMENTED_ERROR = 2,
                IO_ERROR              = 3 
            };

            Error(Code _code) : code(_code) 
            { }

            Error(Code _code, std::string _msg) : 
                code(_code), errorString(std::move(_msg))
            { }

            operator bool() const
            {
                return code != Code::NO_ERROR;
            }
            
            Code code;
            std::string errorString;
        };

        inline Error NoError() 
        {
            return Error(Error::Code::NO_ERROR);
        }

        inline Error IOError(const std::string& message)
        {
            return Error(Error::Code::IO_ERROR, message);
        }

        inline Error NotImplemented(const std::string& message)
        {
            return Error(Error::Code::NOT_IMPLEMENTED_ERROR, message);
        }


        template <typename T>
        using Failable = std::expected<T, Error>;
    }
}
