#pragma once

#include <format>
#include <string>
#include <expected>

namespace RawEdit
{
    using Error = std::string;

    template<typename T>
    using Failable = std::expected<T, Error>;
    
    inline std::unexpected<Error> Failed(const Error& err) 
    { 
        return std::unexpected(err);
    }
    
    template<typename... Args>
    inline std::unexpected<Error> Failed(const char* fmt, Args... args) 
    { 
        return std::unexpected(std::vformat(fmt, std::make_format_args(args...))); 
    }

    inline Error Ok() { return ""; }
};
