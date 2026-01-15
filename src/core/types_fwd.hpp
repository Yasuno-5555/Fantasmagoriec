#pragma once
// L0: Forward Declarations Only
// NO INCLUDES. NO DEFINITIONS. DECLARATIONS ONLY.

namespace fanta {

class Backend;
struct Capabilities;

namespace internal {
    struct DrawList;
    struct DrawCmd;
    struct TextRun;
    struct Path;
    struct Vec2;
    struct Rectangle;
    struct ColorF;
    struct ComputedLayout;
    struct ElementState;
    struct ID;
    struct PersistentData;
    struct StateStore;
    struct InputState;
    struct InputContext;
    struct Theme;
    struct TypographyRule;
    struct WorldState;
    struct FrameState;
    struct RuntimeState;
    struct EngineContext;
    class FrameArena;
    class GpuTexture;
} // namespace internal

} // namespace fanta
