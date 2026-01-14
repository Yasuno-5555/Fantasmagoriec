---
globs: |-
  **/*.tsx
  **/*.ts
  **/*.js
description: "Phase 3 – Shadow/Elevation: The sole focus is to give UI elements
  elevation via shadows without altering the public API, introducing new shapes,
  or using blur/glass. This rule governs all shadow‑related changes in this
  phase."
alwaysApply: false
---

Only add elevation through ElementState (internal); extend DrawList with a ShadowCommand or by augmenting Rect+params; implement a two‑pass shadow (ambient wide & thin, key directional); do NOT add glass effects, backdrop blur, text styling, or animations.