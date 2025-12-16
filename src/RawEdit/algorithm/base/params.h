#pragma once

#include <array>
#include <vector>
#include "utils/cache.h"
#include "image/image.h"
#include "image/mask.h"

namespace RawEdit
{
    struct EnumType
    {
        using Value = std::string;
        using ValueList = std::vector<std::string>;

        EnumType() {}
        EnumType(std::initializer_list<Value> values) {
            possibleValues = std::make_shared<ValueList>(values);
        }
        EnumType(std::initializer_list<Value> values, uint32_t idx) {
            possibleValues = std::make_shared<ValueList>(values);
            value = possibleValues->at(idx);
        }

        bool operator!=(const EnumType& other) const
        { return possibleValues != other.possibleValues && value != other.value; }

        Value value;
        std::shared_ptr<ValueList> possibleValues;
    };
    
    

    // Using aliases here because it would otherwise be interpreted 
    // as parameter to X macro
    using __Color = std::array<float, 3>;
    #define PARAM_TYPES_LIST(...) \
        __RAWEDIT_PARAM_X(Int, int, __VA_ARGS__) \
        __RAWEDIT_PARAM_X(Enum, EnumType, __VA_ARGS__) \
        __RAWEDIT_PARAM_X(Float, float, __VA_ARGS__) \
        __RAWEDIT_PARAM_X(Color, __Color, __VA_ARGS__) 
    
    #define __RAWEDIT_PARAM_X(name, type, ...) type,
    using AnyParamType = std::variant<PARAM_TYPES_LIST() std::nullptr_t>;
    #undef __RAWEDIT_PARAM_X

    enum class ParamType
    {
        #define __RAWEDIT_PARAM_X(name, type, ...) name,
            PARAM_TYPES_LIST()
        #undef __RAWEDIT_PARAM_X
        __UNKNOWN_TYPE
    };

    inline std::string ParamTypeToString(ParamType t)
    {
        #define __RAWEDIT_PARAM_X(name, type, x,...) case ParamType :: name: return #name;
        switch(t)
        {
            PARAM_TYPES_LIST(t)
            default:
                return "";
        }
        #undef __RAWEDIT_PARAM_X
    };

    struct Param
    {
    public:
        Param() : type(ParamType::__UNKNOWN_TYPE), multiMask(false)         { values.push_back(Cached<AnyParamType>(nullptr)); }
        Param(ParamType t, bool mmask = false) : type(t), multiMask(mmask) { values.push_back(Cached<AnyParamType>(nullptr)); }

        template<typename T>
        Param(const T& initValue, bool mmask = true)  : multiMask(mmask)
        {
            *this = initValue;
        }

        template<typename T>
        const T& GetValue(uint32_t idx = 0) const
        { 
            if (!multiMask) idx = 0;
            if (idx >= Mask::MAX_MASK_COUNT) idx = Mask::MAX_MASK_COUNT - 1;
            return std::get<T>(values[idx].value()); 
        }

        template<typename T>
        T& GetValue(uint32_t idx = 0) 
        { 
            if (!multiMask) idx = 0;
            if (idx >= Mask::MAX_MASK_COUNT) idx = Mask::MAX_MASK_COUNT - 1;

            if (idx >= values.size()) 
                values.resize(idx + 1);
            return std::get<T>(values[idx].value()); 
        }

        template<typename T>
        Param& Set(const T& other)
        {
            #define __RAWEDIT_PARAM_X(n, t, ...) if constexpr (std::is_convertible_v<T, t>) if (type == ParamType :: n) { values[0] = static_cast<t>(other); }
            PARAM_TYPES_LIST()
            #undef __RAWEDIT_PARAM_X
            
            return *this;
        }

        #define __RAWEDIT_PARAM_X(name, type, ...) type & As ## name (uint32_t i = 0) { return GetValue<type>(i); }
            PARAM_TYPES_LIST()
        #undef __RAWEDIT_PARAM_X

        #define __RAWEDIT_PARAM_X(name, type, ...) const type & As ## name (uint32_t i = 0) const { return GetValue<type>(i); }
            PARAM_TYPES_LIST()
        #undef __RAWEDIT_PARAM_X

        bool dirty() const 
        { 
            bool dirtyval = values[0].dirty();

            for (uint32_t i = 1; i < values.size(); ++i)
                dirtyval = dirtyval || values[i].dirty();
            return dirtyval;
        }
        
        // Warning: Changes the type of parameter !
        // But this is deliberate, for algorithm to declare them
        // "simply"
        template<typename T>
        Param& operator=(const T& other)
        {
            #define __RAWEDIT_PARAM_X(n, t, ...) if constexpr (std::is_same_v<T, t>) { values[0] = other; type = ParamType :: n; }
            PARAM_TYPES_LIST()
            #undef __RAWEDIT_PARAM_X

                return *this;
        }

        template<typename T>
        Param& operator=(const std::vector<T>& other)
        {
            #define __RAWEDIT_PARAM_X(n, t, ...) if constexpr (std::is_same_v<T, t>) { values.resize(other.size()); for (uint32_t i = 0; i < values.size(); ++i) {values[i] = other[i];} type = ParamType :: n; }
            PARAM_TYPES_LIST()
            #undef __RAWEDIT_PARAM_X

                return *this;
        }

        ParamType type;
        bool multiMask = false;
        std::vector<Cached<AnyParamType>> values;
    };
}
