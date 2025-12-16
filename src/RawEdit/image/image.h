#pragma once

#include "imagebase.h"
#include "cpuimage.h"

#include <type_traits>
#include <variant>
#include <memory>

namespace RawEdit 
{
    using Image = ImageBase;
    using ImagePtr = std::shared_ptr<Image>;

    #define __RAWEDIT_BACKEND_X(bame, btype, tname, ttype, ...) btype<ttype>*,
    #define __RAWEDIT_DATATYPE_X(tname, ttype, ...) BACKEND_LIST(tname, ttype, __VA_ARGS__)
        using AnyBackend = std::variant<DATATYPE_LIST() std::nullptr_t>;
    #undef __RAWEDIT_BACKEND_X
    #undef __RAWEDIT_DATATYPE_X

    inline AnyBackend ConvertBackend(ImagePtr base)
    {
        #define __RAWEDIT_BACKEND_X(bname, btype, tname, ttype, pt, ...) if (pt->type == ImageDataType :: tname && pt->backend == ImageBackend :: bname) return static_cast<btype<ttype>*>(pt);
        #define __RAWEDIT_DATATYPE_X(tname, ttype, pt, ...) BACKEND_LIST(tname, ttype, pt, __VA_ARGS__)
            DATATYPE_LIST(base.get())
        #undef __RAWEDIT_BACKEND_X
        #undef __RAWEDIT_DATATYPE_X

        return nullptr;
    }

    #define DISPATCH_IMAGE_CALL(base, ...) std::visit([&](auto&& __dev) { using ImagePtr = decltype(__dev); __VA_ARGS__;}, ConvertBackend(base))
}
