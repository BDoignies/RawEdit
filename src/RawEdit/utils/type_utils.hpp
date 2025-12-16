#pragma once

#include <type_traits>

namespace RawEdit
{
    namespace utils
    {
        template<typename From, typename To, typename T>
        decltype(auto) map(T&& x)
        {
            using F = std::remove_cv_t<From>;
            using U = std::remove_cv_t<T>;
            if constexpr (std::is_same_v<U, F>)
            {
                return static_cast<std::copy_cv_t<F, To>>(x);
            }

            return std::forward(x);
        }
    }
}
