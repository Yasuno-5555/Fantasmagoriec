#include "timeline.hpp"

namespace fanta {
namespace internal {

static Timeline g_timeline;

Timeline& GetTimeline() {
    return g_timeline;
}

} // namespace internal
} // namespace fanta
