// Fantasmagorie v3 - SDF Rounded Rectangle Shader
// Phase 1: Core SDF + Multi-Layer Shadow
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;      // Viewport size (physical pixels)
uniform float u_dpr;            // Device pixel ratio
uniform vec4 u_rect;            // x, y, w, h (logical pixels)
uniform vec4 u_radius;          // Corner radii: tl, tr, br, bl

// Fill
uniform vec4 u_fill_color;      // RGBA (premultiplied alpha)

// Border
uniform vec4 u_border_color;
uniform float u_border_width;

// Multi-layer shadow (Ambient + Key)
uniform vec4 u_shadow_ambient_color;
uniform float u_shadow_ambient_blur;
uniform float u_shadow_ambient_spread;
uniform vec2 u_shadow_ambient_offset;

uniform vec4 u_shadow_key_color;
uniform float u_shadow_key_blur;
uniform float u_shadow_key_spread;
uniform vec2 u_shadow_key_offset;

out vec4 FragColor;

// ============================================================================
// SDF Functions
// ============================================================================

// Signed Distance Field: Rounded Rectangle
// p: local position (relative to rect center)
// b: half-size of the rectangle
// r: corner radii (tl, tr, br, bl) - uses bottom-right logic for quadrant selection
float sdRoundedBox(vec2 p, vec2 b, vec4 r) {
    // Select radius based on quadrant
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    
    // Calculate distance
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

// ============================================================================
// Shadow Calculation
// ============================================================================

float computeShadowLayer(vec2 p, vec2 halfSize, vec4 radius, 
                         vec2 offset, float blur, float spread) {
    vec2 shadowP = p - offset;
    vec2 expandedSize = halfSize + spread;
    float d = sdRoundedBox(shadowP, expandedSize, radius);
    
    // Soft edge using smoothstep
    // Asymmetric for more natural falloff (sharper inner, softer outer)
    float inner = -blur * 0.3;
    float outer = blur * 0.7;
    return 1.0 - smoothstep(inner, outer, d);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Convert fragment coordinates to logical pixels
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    
    // Flip Y for top-left origin
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    // Rectangle center and half-size
    vec2 center = u_rect.xy + u_rect.zw * 0.5;
    vec2 halfSize = u_rect.zw * 0.5;
    
    // Local position relative to center
    vec2 p = fragCoord - center;
    
    // Compute SDF distance
    float d = sdRoundedBox(p, halfSize, u_radius);
    
    // Anti-aliasing width (1 physical pixel in logical space)
    float aa = 1.0 / u_dpr;
    
    // ========================================================================
    // Layer 1: Shadows (rendered behind everything)
    // ========================================================================
    vec4 color = vec4(0.0);
    
    // Ambient shadow (soft, omnidirectional)
    if (u_shadow_ambient_color.a > 0.001) {
        float ambient = computeShadowLayer(p, halfSize, u_radius,
            u_shadow_ambient_offset, u_shadow_ambient_blur, u_shadow_ambient_spread);
        vec4 ambientColor = u_shadow_ambient_color * ambient;
        color = ambientColor;
    }
    
    // Key shadow (directional, sharper)
    if (u_shadow_key_color.a > 0.001) {
        float key = computeShadowLayer(p, halfSize, u_radius,
            u_shadow_key_offset, u_shadow_key_blur, u_shadow_key_spread);
        vec4 keyColor = u_shadow_key_color * key;
        // Additive blend for shadow layers
        color.rgb += keyColor.rgb * keyColor.a;
        color.a = max(color.a, keyColor.a);
    }
    
    // ========================================================================
    // Layer 2: Fill
    // ========================================================================
    float fillMask = 1.0 - smoothstep(-aa, aa, d);
    
    if (fillMask > 0.001) {
        // Composite fill over shadows (Porter-Duff over)
        vec4 fill = u_fill_color * fillMask;
        color.rgb = fill.rgb + color.rgb * (1.0 - fill.a);
        color.a = fill.a + color.a * (1.0 - fill.a);
    }
    
    // ========================================================================
    // Layer 3: Border (inside the fill)
    // ========================================================================
    if (u_border_width > 0.0 && u_border_color.a > 0.001) {
        float innerD = sdRoundedBox(p, halfSize - u_border_width, u_radius - u_border_width);
        float borderMask = smoothstep(-aa, aa, innerD) * fillMask;
        
        vec4 border = u_border_color * borderMask;
        color.rgb = border.rgb * border.a + color.rgb * (1.0 - border.a);
        color.a = max(color.a, border.a);
    }
    
    FragColor = color;
}
