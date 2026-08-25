#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <cassert>

class Utils {
public:
    static std::size_t CalculatePadding(const std::size_t baseAddress, const std::size_t alignment) noexcept {
        assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "Alignment must be a non-zero power of 2.");

        const std::size_t aligned = (baseAddress + alignment - 1) & ~(alignment - 1);

        return aligned - baseAddress;
    }

    static std::size_t CalculatePaddingWithHeader(const std::size_t baseAddress, const std::size_t alignment, const std::size_t headerSize) noexcept {
        assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "Alignment must be a non-zero power of 2.");

        std::size_t padding = CalculatePadding(baseAddress, alignment);

        if (padding < headerSize) {
            std::size_t needed = headerSize - padding;
            padding += alignment * ((needed + alignment - 1) / alignment);
        }

        return padding;
    }
};

#endif /* UTILS_H */