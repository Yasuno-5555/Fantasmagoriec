#pragma once
#include "../core/types_internal.hpp"
#include "../core/contexts_internal.hpp"
#include "../backend/drawlist.hpp"
#include <map>
#include <vector>

namespace fanta {
namespace internal {

    // Main Entry Point for Rendering the tree
    void RenderTree(
        const ElementState& el,
        const std::map<ID, ComputedLayout>& layout_results,
        const std::vector<ElementState>& all_elements,
        const std::map<ID, size_t>& id_map,
        DrawList& dl,
        RuntimeState& runtime,
        InputContext& input,
        StateStore& store,
        float world_tx = 0, float world_ty = 0, float world_scale = 1.0f,
        bool is_overlay = false
    );

} // namespace internal
} // namespace fanta
