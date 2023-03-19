#pragma once

#include <cstddef>

namespace ktl::detail
{
    constexpr size_t ALIGNMENT = 8;
    constexpr size_t ALIGNMENT_MASK = ALIGNMENT - 1;

    constexpr inline size_t align_to_architecture(size_t n) noexcept
    {
        size_t align = n & ALIGNMENT_MASK;
        if (align != 0)
            return ALIGNMENT - align;

        return 0;
    }
}