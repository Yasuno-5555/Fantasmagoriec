#include <fanta.h>
#include "../core/contexts_internal.hpp"
#include "../core/contexts_internal.hpp"
#include <cmath> // for NAN

namespace fanta {

// Helper to access state safely
static size_t ToIndex(ElementHandle* h) { return reinterpret_cast<size_t>(h) - 1; }

static internal::ElementState& GetState(ElementHandle* h) {
    return internal::GetStore().frame_elements[ToIndex(h)];
}

Element& Element::absolute() {
    GetState(handle).is_absolute = true;
    return *this;
}

Element& Element::top(float v) {
    GetState(handle).pos_top = v;
    return *this;
}

Element& Element::bottom(float v) {
    GetState(handle).pos_bottom = v;
    return *this;
}

Element& Element::left(float v) {
    GetState(handle).pos_left = v;
    return *this;
}

Element& Element::right(float v) {
    GetState(handle).pos_right = v;
    return *this;
}

} // namespace fanta
