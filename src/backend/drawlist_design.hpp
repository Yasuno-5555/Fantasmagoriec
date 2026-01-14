#pragma once

// Phase 2: DrawList 設計（実装は別ファイル）
// このファイルは設計仕様の凍結のみ。実装は drawlist.hpp/cpp で行う。

#include "core/types_internal.hpp"
#include <vector>

namespace fanta {
namespace internal {

// ============================================================================
// DrawList: Type-Erased Command Buffer
// ============================================================================

// DrawCommand の型（最小仕様）
enum class DrawCmdType {
    Rect,        // 矩形 (pos, size, color)
    RoundedRect, // 角丸矩形 (+ radius)
    Circle,      // 円 (center, r)
    Line         // 線 (p0, p1, thickness)
    // 禁止: Path, Bezier, Gradient, Text
};

// Type-Erased Command（virtual禁止、RTTI禁止）
struct DrawCmd {
    DrawCmdType type;
    
    // Union for type-erased data
    union {
        struct {
            Vec2 pos;      // 左上角
            Vec2 size;     // 幅・高さ
            ColorF color;
        } rect;
        
        struct {
            Vec2 pos;      // 左上角
            Vec2 size;     // 幅・高さ
            float radius;  // 角丸半径
            ColorF color;
        } rounded_rect;
        
        struct {
            Vec2 center;   // 中心座標
            float r;       // 半径
            ColorF color;
        } circle;
        
        struct {
            Vec2 p0;       // 始点
            Vec2 p1;       // 終点
            float thickness; // 線の太さ
            ColorF color;
        } line;
    };
};

// DrawList: Command Buffer
struct DrawList {
    std::vector<DrawCmd> commands;
    
    // 追加メソッド（実装は drawlist.cpp）
    void add_rect(const Vec2& pos, const Vec2& size, const ColorF& color);
    void add_rounded_rect(const Vec2& pos, const Vec2& size, float radius, const ColorF& color);
    void add_circle(const Vec2& center, float r, const ColorF& color);
    void add_line(const Vec2& p0, const Vec2& p1, float thickness, const ColorF& color);
    
    void clear() { commands.clear(); }
    size_t size() const { return commands.size(); }
};

// ============================================================================
// DPI 変換の唯一の真実
// ============================================================================

// ルール: APIは常にlogical px、shader内でdevicePixelRatioを掛ける
// - CPU側でスケールしない
// - レイアウトは常に論理座標
// - DrawListも論理座標
// - Backend: 論理px → 物理px変換（shader内）

struct DPIContext {
    float device_pixel_ratio = 1.0f;  // フレームバッファサイズ / ウィンドウサイズ
    int logical_width = 0;            // 論理幅（API座標）
    int logical_height = 0;           // 論理高さ（API座標）
    int physical_width = 0;           // 物理幅（フレームバッファ）
    int physical_height = 0;          // 物理高さ（フレームバッファ）
};

// ============================================================================
// SDF Shader の責務限定（Phase 2）
// ============================================================================

// Phase 2で実装する機能:
// - Edge AA（アンチエイリアス）
// - Rounded corner（角丸）
// - Circle（円）

// Phase 2で実装しない機能:
// - 影（Phase 3）
// - Blur（Phase 5）
// - Apple HIG（Phase 3以降）

// SDF Shader Uniforms（設計のみ）
struct SDFUniforms {
    Vec2 viewport_size;        // 物理ピクセルサイズ（shader内でDPI変換に使用）
    float device_pixel_ratio;  // devicePixelRatio
    // Phase 2ではこれだけ。影・BlurはPhase 3以降。
};

} // namespace internal
} // namespace fanta
