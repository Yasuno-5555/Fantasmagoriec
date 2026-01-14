// Fantasmagorie v3 - SDF Circle Shader
// Anti-aliased circle with multi-layer shadow
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;      // Viewport size (physical pixels)
uniform float u_dpr;            // Device pixel ratio
uniform vec2 u_center;          // Circle center (logical pixels)
uniform float u_radius;         // Circle radius

// Fill
uniform vec4 u_fill_color;

// Border
uniform vec4 u_border_color;
uniform float u_border_width;

// Multi-layer shadow
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

// Signed Distance Field: Circle
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

// ============================================================================
// Shadow Calculation
// ============================================================================

float computeShadowLayer(vec2 p, float radius, vec2 offset, float blur, float spread) {
    vec2 shadowP = p - offset;
    float d = sdCircle(shadowP, radius + spread);
    float inner = -blur * 0.3;
    float outer = blur * 0.7;
    return 1.0 - smoothstep(inner, outer, d);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Convert to logical pixels
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    // Local position
    vec2 p = fragCoord - u_center;
    
    // SDF distance
    float d = sdCircle(p, u_radius);
    
    // Anti-aliasing
    float aa = 1.0 / u_dpr;
    
    // ========================================================================
    // Layer 1: Shadows
    // ========================================================================
    vec4 color = vec4(0.0);
    
    // Ambient shadow
    if (u_shadow_ambient_color.a > 0.001) {
        float ambient = computeShadowLayer(p, u_radius,
            u_shadow_ambient_offset, u_shadow_ambient_blur, u_shadow_ambient_spread);
        color = u_shadow_ambient_color * ambient;
    }
    
    // Key shadow
    if (u_shadow_key_color.a > 0.001) {
        float key = computeShadowLayer(p, u_radius,
            u_shadow_key_offset, u_shadow_key_blur, u_shadow_key_spread);
        vec4 keyColor = u_shadow_key_color * key;
        color.rgb += keyColor.rgb * keyColor.a;
        color.a = max(color.a, keyColor.a);
    }
    
    // ========================================================================
    // Layer 2: Fill
    // ========================================================================
    float fillMask = 1.0 - smoothstep(-aa, aa, d);
    
    if (fillMask > 0.001) {
        vec4 fill = u_fill_color * fillMask;
        color.rgb = fill.rgb + color.rgb * (1.0 - fill.a);
        color.a = fill.a + color.a * (1.0 - fill.a);
    }
    
    // ========================================================================
    // Layer 3: Border
    // ========================================================================
    if (u_border_width > 0.0 && u_border_color.a > 0.001) {
        float innerD = sdCircle(p, u_radius - u_border_width);
        float borderMask = smoothstep(-aa, aa, innerD) * fillMask;
        
        vec4 border = u_border_color * borderMask;
        color.rgb = border.rgb * border.a + color.rgb * (1.0 - border.a);
        color.a = max(color.a, border.a);
    }
    
    FragColor = color;
}
