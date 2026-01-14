#pragma once
#include "core/types_internal.hpp"

namespace fanta {
namespace internal {
    inline ID& GetGlobalIDCounter() {
        static ID counter = 1;
        return counter;
    }
}
}
