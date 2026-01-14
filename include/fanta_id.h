#pragma once
#include <cstdint>
#include <string_view>

namespace fanta {

    // 64-bit FNV-1a Hash Constants
    constexpr uint64_t FNV1a_OFFSET = 14695981039346656037ULL;
    constexpr uint64_t FNV1a_PRIME  = 1099511628211ULL;

    struct ID {
        uint64_t value;

        constexpr ID() : value(0) {}
        constexpr ID(uint64_t v) : value(v) {}
        
        // Compile-time/Runtime hash from string
        constexpr ID(std::string_view str, uint64_t seed = FNV1a_OFFSET) : value(seed) {
            for (char c : str) {
                value ^= static_cast<uint64_t>(c);
                value *= FNV1a_PRIME;
            }
        }
        
        // Explicit const char* to avoid void* ambiguity
        constexpr ID(const char* str) : ID(std::string_view(str)) {}
        
        // Pointer hash (explicit to avoid string literal ambiguity)
        explicit ID(const void* ptr) {
            value = reinterpret_cast<uint64_t>(ptr); 
        }

        // Int constructor to disambiguate 0
        constexpr ID(int v) : value(static_cast<uint64_t>(v)) {}

        bool operator==(const ID& other) const { return value == other.value; }
        bool operator!=(const ID& other) const { return value != other.value; }
        bool operator<(const ID& other) const { return value < other.value; } // For map usage
    };

} // namespace fanta
