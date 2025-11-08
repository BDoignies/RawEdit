#pragma once

#include <string>

namespace RawEdit
{
    namespace core 
    {
        struct Error
        {
            enum class Code
            {
                NO_ERROR = 0,
                UK_ERROR = 1,
                IO_ERROR = 2, 
            };

            Error(Code _code) : code(_code) 
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
    }
}
