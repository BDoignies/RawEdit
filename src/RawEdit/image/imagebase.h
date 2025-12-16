#pragma once

#include <type_traits>
#include <stdfloat>
#include <variant>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>

namespace RawEdit
{
    #ifndef RAWEDIT_OPENGL
        #define RAWEDIT_OPENGL 0
    #endif 

    #ifndef RAWEDIT_CUDA
        #define RAWEDIT_CUDA 0
    #endif

    #define __RAWEDIT_BACKEND_IF(cond, ...)      __RAWEDIT_BACKEND_IF_IMPL(cond, __VA_ARGS__)
    #define __RAWEDIT_BACKEND_IF_IMPL(cond, ...) __RAWEDIT_BACKEND_IF_ ## cond (__VA_ARGS__)
    #define __RAWEDIT_BACKEND_IF_0(...)
    #define __RAWEDIT_BACKEND_IF_1(...) __VA_ARGS__
            
    #define BACKEND_LIST(...) \
        __RAWEDIT_BACKEND_X(CPU, CPUImage, __VA_ARGS__) \
        __RAWEDIT_BACKEND_IF(RAWEDIT_OPENGL, __RAWEDIT_BACKEND_X(OpenGL, OpenGLImage, __VA_ARGS__)) \
        __RAWEDIT_BACKEND_IF(RAWEDIT_CUDA,   __RAWEDIT_BACKEND_X(CUDA  , CudaImage, __VA_ARGS__))

    #define DATATYPE_LIST(...) \
        __RAWEDIT_DATATYPE_X(UINT8  , std::uint8_t, __VA_ARGS__)  \
        __RAWEDIT_DATATYPE_X(UINT16 , std::uint16_t, __VA_ARGS__) \
        __RAWEDIT_DATATYPE_X(UINT32 , std::uint32_t, __VA_ARGS__) \
        __RAWEDIT_DATATYPE_X(FLOAT16, std::float16_t, __VA_ARGS__) \
        __RAWEDIT_DATATYPE_X(FLOAT32, std::float32_t, __VA_ARGS__) \
        __RAWEDIT_DATATYPE_X(FLOAT64, std::float64_t, __VA_ARGS__) 

    enum class ImageBackend
    {
        #define __RAWEDIT_BACKEND_X(name, ...) name,
            BACKEND_LIST()
        #undef __RAWEDIT_BACKEND_X
        __INVALID_DEVICE
    };

    enum class ImageDataType
    {
        #define __RAWEDIT_DATATYPE_X(name, type, ...) name,
            DATATYPE_LIST()
        #undef __RAWEDIT_DATATYPE_X
        __INVALID_TYPE
    };

    inline const char* ImageBackendToString(const ImageBackend& back)
    {
        switch(back)
        {
            #define __RAWEDIT_BACKEND_X(name, ...) case ImageBackend :: name : return #name;
                BACKEND_LIST()
            #undef __RAWEDIT_BACKEND_X
            default:
                return "Unknown";
        };
    };

    inline const char* ImageDataTypeToString(const ImageDataType& type)
    {
        switch(type)
        {
            #define __RAWEDIT_DATATYPE_X(name, type, ...) case ImageDataType :: name : return #name;
                DATATYPE_LIST()
            #undef __RAWEDIT_DATATYPE_X
            default:
                return "Unknown";
        };
    };

    inline size_t SizeofType(const ImageDataType& type)
    {
        switch(type)
        {
            #define __RAWEDIT_DATATYPE_X(name, type, ...) case ImageDataType :: name : return sizeof(type);
                DATATYPE_LIST()
            #undef __RAWEDIT_DATATYPE_X
            default:
                return 0;
        }
    }   
    
    template<typename T>
    inline ImageDataType TypeToImageDataType()
    {
        #define __RAWEDIT_DATATYPE_X(name, type, ...) if constexpr (std::is_same_v<T, type>) return ImageDataType :: name;
            DATATYPE_LIST()
        #undef __RAWEDIT_DATATYPE_X
        return ImageDataType::__INVALID_TYPE;
    }
    
    // Leave a valid type here
    template<ImageDataType DT> struct ImageDataTypeConverter { using Type = char; };   
     
    #define __RAWEDIT_DATATYPE_X(tname, ttype, ...) template<> struct ImageDataTypeConverter<ImageDataType :: tname> { using Type = ttype; };
        DATATYPE_LIST()
    #undef __RAWEDIT_DATATYPE_X

    #define __RAWEDIT_DATATYPE_X(tname, ttype, ...) ImageDataTypeConverter<ImageDataType :: tname>,
        using AnyDataTypeConverter = std::variant<DATATYPE_LIST() ImageDataTypeConverter<ImageDataType::__INVALID_TYPE>>;
    #undef __RAWEDIT_DATATYPE_X

    inline AnyDataTypeConverter ConvertDataType(ImageDataType type) 
    {
        #define __RAWEDIT_DATATYPE_X(tname, ttype, ...) if (type == ImageDataType :: tname) return ImageDataTypeConverter<ImageDataType :: tname>{};
            DATATYPE_LIST();
        #undef __RAWEDIT_DATATYPE_X
        return ImageDataTypeConverter<ImageDataType::__INVALID_TYPE>{};
    }

    #define DISPATCH_DATATYPE(base, ...) std::visit([&](auto&& type) { using DataType = std::remove_cvref_t<decltype(type)>::Type; __VA_ARGS__;}, ConvertDataType(base))

    struct MetaData
    {
        std::string source = "";
        std::string path   = "";
    };

    struct ImageBase
    {
        ImageBase(ImageBackend b, ImageDataType t) : backend(b), type(t) 
        {
            assert(t != ImageDataType::__INVALID_TYPE && "Unsupported type");
        }

        virtual ImageBase* EmptyCopy(bool metadata) const = 0;
        virtual ImageBase* Copy() const = 0;

        virtual ~ImageBase() { }

        ImageBase(const ImageBase&) = delete;
        ImageBase& operator=(const ImageBase&) = delete;

        const ImageBackend backend;
        const ImageDataType type;

        MetaData metadata;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 0;

        virtual void SetData(
            uint32_t w, uint32_t h, uint32_t c, 
            ImageDataType type, const void* data
        ) = 0;
    };

};
