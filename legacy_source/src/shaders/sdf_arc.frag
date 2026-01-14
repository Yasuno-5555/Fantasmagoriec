// Fantasmagorie v3 - SDF Arc Shader
// Anti-aliased arc/pie slice
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;
uniform float u_dpr;
uniform vec2 u_center;          // Arc center
uniform float u_radius;         // Outer radius
uniform float u_inner_radius;   // Inner radius (0 for filled arc)
uniform float u_start_angle;    // Start angle in radians
uniform float u_end_angle;      // End angle in radians
uniform vec4 u_fill_color;
uniform float u_thickness;      // For stroke only (when inner_radius > 0)

out vec4 FragColor;

// ============================================================================
// SDF Functions
// ============================================================================

// Signed Distance Field: Arc
// Based on Inigo Quilez's arc SDF
float sdArc(vec2 p, vec2 sc1, vec2 sc2, float ra, float rb) {
    // sc1 = vec2(sin, cos) of start angle
    // sc2 = vec2(sin, cos) of end angle
    // ra = outer radius, rb = thickness
    
    p.x = abs(p.x);
    float k = (sc2.y * p.x > sc2.x * p.y) ? dot(p, sc2) : length(p);
    return sqrt(dot(p, p) + ra * ra - 2.0 * ra * k) - rb;
}

// Signed Distance Field: Pie slice
float sdPie(vec2 p, vec2 c, float r) {
    // c = vec2(sin, cos) of half angle
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p, c), 0.0, r));
    return max(l, m * sign(c.y * p.x - c.x * p.y));
}

// ============================================================================
// Main
// ============================================================================

void main() {
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    vec2 p = fragCoord - u_center;
    
    // Rotate to align with start angle
    float midAngle = (u_start_angle + u_end_angle) * 0.5;
    float halfAngle = (u_end_angle - u_start_angle) * 0.5;
    
    float c = cos(-midAngle);
    float s = sin(-midAngle);
    p = vec2(c * p.x - s * p.y, s * p.x + c * p.y);
    
    // SDF
    float d;
    if (u_inner_radius > 0.0) {
        // Ring arc
        vec2 sc = vec2(sin(halfAngle), cos(halfAngle));
        float thickness = (u_radius - u_inner_radius) * 0.5;
        float midRadius = (u_radius + u_inner_radius) * 0.5;
        d = sdArc(p, sc, sc, midRadius, thickness);
    } else {
        // Pie slice
        vec2 sc = vec2(sin(halfAngle), cos(halfAngle));
        d = sdPie(p, sc, u_radius);
    }
    
    // Anti-aliasing
    float aa = 1.0 / u_dpr;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    FragColor = u_fill_color * alpha;
}
