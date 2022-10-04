#pragma once

#include <cstddef>

namespace ktl
{
    constexpr size_t ALIGNMENT = sizeof(void*);

    constexpr inline size_t align_to_architecture(size_t n) noexcept
    {
        size_t align = n % ALIGNMENT;
        if (align != 0)
            return ALIGNMENT - align;

        return 0;
    }
}