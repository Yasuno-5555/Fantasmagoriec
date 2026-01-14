#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <string_view>
#include <span>
#include <vector>
#include <iostream>

namespace fanta {
namespace internal {

    // V5: Frame-Local Linear Allocator
    // Design: Fixed capacity, reset every frame. Zero allocation cost.
    // Safety: Fail-fast on overflow.
    class FrameArena {
    public:
        // Default 4MB per frame (plenty for 10k+ widgets)
        static constexpr size_t DEFAULT_CAPACITY = 4 * 1024 * 1024; 

        FrameArena(size_t capacity = DEFAULT_CAPACITY) 
            : m_capacity(capacity) 
        {
            m_buffer = new uint8_t[m_capacity];
            m_offset = 0;
            std::cout << "[FrameArena] Initialized with " << (capacity / 1024) << "KB" << std::endl;
        }

        ~FrameArena() {
            delete[] m_buffer;
        }

        void reset() {
            m_offset = 0;
        }

        // Allocate POD type T
        template<typename T, typename... Args>
        T* alloc(Args&&... args) {
            static_assert(std::is_trivially_destructible_v<T>, "Arena objects must be trivially destructible! (No non-trivial destructors allowed)");
            
            void* ptr = alloc_raw(sizeof(T), alignof(T));
            if (!ptr) return nullptr; // Should crash in alloc_raw if strict
            return new(ptr) T{std::forward<Args>(args)...};
        }

        // Allocate array of POD type T
        template<typename T>
        std::span<T> alloc_array(size_t count) {
            static_assert(std::is_trivially_destructible_v<T>, "Arena objects must be trivially destructible!");
            
            if (count == 0) return {};

            void* ptr = alloc_raw(sizeof(T) * count, alignof(T));
            T* t_ptr = static_cast<T*>(ptr);
            
            // Default construct (zero init or default)
            for (size_t i = 0; i < count; ++i) {
                new(t_ptr + i) T{};
            }
            
            return std::span<T>(t_ptr, count);
        }
        
        // Copy a string into the arena and return a string_view
        std::string_view push_string(std::string_view str) {
            if (str.empty()) return {};
            
            char* ptr = static_cast<char*>(alloc_raw(str.size() + 1, 1));
            std::memcpy(ptr, str.data(), str.size());
            ptr[str.size()] = '\0'; // Null-terminate for safety (though string_view handles length)
            
            return std::string_view(ptr, str.size());
        }

        // Raw allocation
        void* alloc_raw(size_t size, size_t alignment) {
            // Align offset
            size_t aligned_offset = (m_offset + (alignment - 1)) & ~(alignment - 1);
            
            if (aligned_offset + size > m_capacity) {
                std::cerr << "FATAL: FrameArena Overflow! Capacity: " << m_capacity << ", Requested: " << size << std::endl;
                std::abort();
                return nullptr;
            }
            
            void* ptr = m_buffer + aligned_offset;
            m_offset = aligned_offset + size;
            return ptr;
        }

        size_t used() const { return m_offset; }
        size_t capacity() const { return m_capacity; }

    private:
        uint8_t* m_buffer = nullptr;
        size_t m_capacity = 0;
        size_t m_offset = 0;
    };

    // V5: Arena-safe String Helper
    using ArenaString = std::string_view;

    // V5: Arena-safe Array Helper (Span)
    // T must be POD
    template<typename T>
    using ArenaSpan = std::span<T>;

} // namespace internal
} // namespace fanta
