#include "layout/flex_layout.hpp"

// Dummy UIContext to satisfy Linker if needed (declarations only usually fine for compile checks)
// But flex_layout.hpp includes ui_context.hpp which defines struct UIContext.
// We just need to check if the header + cpp logic parses.

int main() {
    // Just instantiate something or call something to ensure code generation
    return 0;
}
