// Fantasmagorie v3 - SDF Polyline Shader
// Renders tessellated path as thick anti-aliased lines
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;
uniform float u_dpr;
uniform float u_thickness;
uniform vec4 u_color;

// Segment uniforms (for instanced rendering, we'd use a buffer)
// For now, render one segment at a time
uniform vec2 u_from;
uniform vec2 u_to;
uniform vec2 u_prev;     // Previous point (for join)
uniform vec2 u_next;     // Next point (for join)
uniform int u_cap_start; // 0=none, 1=round, 2=square
uniform int u_cap_end;
uniform int u_join_type; // 0=miter, 1=round, 2=bevel

out vec4 FragColor;

// ============================================================================
// SDF Functions
// ============================================================================

float sdSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

float sdCapsule(vec2 p, vec2 a, vec2 b, float r) {
    return sdSegment(p, a, b) - r;
}

// Miter join SDF
float sdMiterJoin(vec2 p, vec2 a, vec2 b, vec2 c, float r, float miterLimit) {
    vec2 ba = normalize(a - b);
    vec2 bc = normalize(c - b);
    vec2 miter = normalize(ba + bc);
    float miterLen = r / max(dot(miter, vec2(-ba.y, ba.x)), 0.001);
    miterLen = min(miterLen, r * miterLimit);
    
    // Distance to miter region
    vec2 pb = p - b;
    float d1 = sdSegment(p, a, b) - r;
    float d2 = sdSegment(p, b, c) - r;
    return min(d1, d2);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    float r = u_thickness * 0.5;
    float d = sdCapsule(fragCoord, u_from, u_to, r);
    
    float aa = 1.0 / u_dpr;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    FragColor = u_color * alpha;
}
