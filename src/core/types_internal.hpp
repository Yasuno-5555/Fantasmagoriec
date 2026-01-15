#pragma once
// DEPRECATED: types_internal.hpp
// This file now serves as a compatibility shim.
// All types are defined in the layered headers (L0-L5).
// New code should include contexts_internal.hpp directly.

#include "core/types_core.hpp"
#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"

// Legacy includes for backward compat
#include "fanta.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <optional>
#include <array>

namespace fanta {
namespace internal {

    // Re-export ID hash helper
    inline ID Hash(const char* str) { return ID(str); }

    // Legacy using declarations
    using ::fanta::LayoutMode;
    using ::fanta::Align;
    using ::fanta::Justify;
    using ::fanta::Wrap;

} // namespace internal
} // namespace fanta
