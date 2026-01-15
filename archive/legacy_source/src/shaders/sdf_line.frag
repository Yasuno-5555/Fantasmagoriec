// Fantasmagorie v3 - SDF Line Shader
// Anti-aliased line segment with caps
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;      // Viewport size (physical pixels)
uniform float u_dpr;            // Device pixel ratio
uniform vec2 u_from;            // Start point (logical pixels)
uniform vec2 u_to;              // End point (logical pixels)
uniform float u_thickness;      // Line thickness
uniform vec4 u_color;           // Line color

// Cap style: 0 = butt, 1 = round, 2 = square
uniform int u_cap_style;

out vec4 FragColor;

// ============================================================================
// SDF Functions
// ============================================================================

// Signed Distance Field: Line Segment
float sdSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

// Signed Distance Field: Capsule (round caps)
float sdCapsule(vec2 p, vec2 a, vec2 b, float r) {
    return sdSegment(p, a, b) - r;
}

// Signed Distance Field: Box-extended Line (square caps)
float sdBoxLine(vec2 p, vec2 a, vec2 b, float r) {
    vec2 ba = b - a;
    float len = length(ba);
    vec2 dir = ba / len;
    vec2 perp = vec2(-dir.y, dir.x);
    
    // Extend endpoints
    vec2 extA = a - dir * r;
    vec2 extB = b + dir * r;
    
    // Project point onto line
    vec2 pa = p - extA;
    float t = dot(pa, dir) / (len + 2.0 * r);
    float s = dot(pa, perp);
    
    // Box distance
    float dx = max(0.0, abs(t - 0.5) * (len + 2.0 * r) - (len + 2.0 * r) * 0.5);
    float dy = max(0.0, abs(s) - r);
    
    return length(vec2(dx, dy));
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Convert to logical pixels
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    float halfThickness = u_thickness * 0.5;
    float d;
    
    // Calculate SDF based on cap style
    if (u_cap_style == 1) {
        // Round caps (capsule)
        d = sdCapsule(fragCoord, u_from, u_to, halfThickness);
    } else if (u_cap_style == 2) {
        // Square caps
        d = sdBoxLine(fragCoord, u_from, u_to, halfThickness);
    } else {
        // Butt caps (basic segment)
        d = sdSegment(fragCoord, u_from, u_to) - halfThickness;
    }
    
    // Anti-aliasing
    float aa = 1.0 / u_dpr;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    FragColor = u_color * alpha;
}
