#define FANTA_ENABLE_TEXT
#include "v3/core/types.hpp"
#include "v3/core/animation.hpp"
#include "v3/core/paint_list.hpp"
#include "v3/core/font_manager.hpp"
#include "v3/platform/opengl.hpp"
#include <string>

int main() {
    fanta::PaintList list;
    list.text({0,0}, "hello");
    return 0;
}
