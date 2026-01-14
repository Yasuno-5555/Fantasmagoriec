# Phase 2: SDF Core + DrawList 設計仕様書

**凍結日**: 2024
**目的**: 描画品質の証明（表現力競争ではない）

---

## 1. DrawList の最小仕様

### 許可される DrawCommand

```cpp
enum class DrawCmdType {
    Rect,        // (pos, size, color)
    RoundedRect, // (+ radius)
    Circle,      // (center, r)
    Line         // (p0, p1, thickness)
};
```

### 禁止事項

- ❌ Path
- ❌ Bezier
- ❌ Gradient
- ❌ Text

**理由**: Phase 2は描画品質の証明であって、表現力競争ではない。

---

## 2. Type-Erased の実装形式

### 基本構造

```cpp
struct DrawCmd {
    enum class Type { Rect, RoundedRect, Circle, Line };
    Type type;
    
    union {
        struct { Vec2 pos; Vec2 size; ColorF color; } rect;
        struct { Vec2 pos; Vec2 size; float radius; ColorF color; } rounded_rect;
        struct { Vec2 center; float r; ColorF color; } circle;
        struct { Vec2 p0; Vec2 p1; float thickness; ColorF color; } line;
    };
};
```

### 制約

- ❌ virtual禁止
- ❌ RTTI禁止
- ✅ backend側でswitch文で処理
- ✅ 後でVulkan/Metal/WebGLに投げるための形

**理由**: 堅実な実装。変態にならない。

---

## 3. DPI の唯一の真実

### ルール（一行で固定）

> **APIは常にlogical px、shader内でdevicePixelRatioを掛ける**

### 詳細

1. **CPU側でスケールしない**
   - レイアウトは常に論理座標
   - DrawListも論理座標

2. **GPU側でスケールする**
   - shader内で`devicePixelRatio`を掛ける
   - フレームバッファサイズとビューポートサイズの違いを吸収

3. **座標系の一貫性**
   - ElementState: 論理px
   - Layout Engine: 論理px
   - DrawList: 論理px
   - Backend: 論理px → 物理px変換（shader内）

**理由**: ここで逃げると死にます。

---

## 4. SDF Shader の責務限定

### Phase 2で実装する機能

- ✅ Edge AA（アンチエイリアス）
- ✅ Rounded corner（角丸）
- ✅ Circle（円）

### Phase 2で実装しない機能

- ❌ 影（Phase 3）
- ❌ Blur（Phase 5）
- ❌ Apple HIG（Phase 3以降）

**理由**: Phase 2は描画品質の証明。機能追加はPhase 3以降。

---

## 5. 既存コードからの移行

### 現在の構造
- `PaintList` (backend.hpp): virtual使用、Type-Erased不完全
- `Cmd::Rect`, `Cmd::Text`, `Cmd::LayerBegin` など

### 移行方針
1. `PaintList` → `DrawList` に置き換え
2. `Cmd::Rect` → `DrawCmd::RoundedRect` (radius=0で矩形)
3. `Cmd::Text` → Phase 4まで保留（Phase 2では描画しない）
4. `Cmd::LayerBegin/End` → Phase 5まで保留（BlurはPhase 5）

### 互換性
- `fanta.cpp`の`RenderTree`関数を修正して`DrawList`を使用
- `backend/opengl_backend.cpp`の`render()`メソッドを`DrawList`対応に変更

---

## 6. 実装進行順序

1. ✅ **設計凍結**（このドキュメント）
2. ✅ **DrawList設計の書き出し**（`drawlist_design.hpp`）
3. ⏳ `DrawList`の実装（`drawlist.hpp/cpp`）
4. ⏳ `fanta.cpp`の`RenderTree`を`DrawList`使用に変更
5. ⏳ `backend/opengl_backend.cpp` を 描画即時 → DrawList consume に書き換え
6. ⏳ SDF shader実装（edge AA、rounded corner、circle）
7. ⏳ 角丸1個を完璧に描く
8. ⏳ DPI 200% で破綻しないのを確認

**成功基準**: ここまで行ったら、Phase 2 成功。「Imgui以下」じゃなくなる。

---

## 7. 依存関係の維持

```
fanta_core (SDF, DrawList) ← Phase 2
  ↑
fanta_layout (Flex, HitTest) ← Phase 1（完了）
  ↑
fanta_compose (FBO, Blur, Shadow) ← Phase 3以降
  ↑
API / Widget DSL
```

**原則**: 逆向き参照は禁止。上位レイヤーは下位レイヤーに依存する。

---

## 8. API変更禁止

- `fanta.h`は一切変更しない
- 既存のAPIはそのまま
- 内部実装のみ差し替え

---

## 9. デグレード戦略

- SDF shaderが使えない環境 → フォールバック（角丸なし矩形）
- DPI変換が失敗 → 論理px = 物理px（1:1）
- エラー時はクラッシュせず、ログを出して続行

---

**この設計は凍結済み。変更する場合は必ず承認を得ること。**
