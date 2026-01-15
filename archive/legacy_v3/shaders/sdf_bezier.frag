// Fantasmagorie v3 - SDF Bezier Shader
// Anti-aliased quadratic Bezier curve
// Uses analytical SDF for performance
#version 330 core

// ============================================================================
// Uniforms
// ============================================================================
uniform vec2 u_resolution;
uniform float u_dpr;
uniform vec2 u_p0;              // Start point
uniform vec2 u_p1;              // Control point
uniform vec2 u_p2;              // End point
uniform float u_thickness;
uniform vec4 u_color;

out vec4 FragColor;

// ============================================================================
// SDF: Quadratic Bezier
// Based on Inigo Quilez's implementation
// ============================================================================

float dot2(vec2 v) { return dot(v, v); }

float cross2(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

// Returns signed distance to quadratic Bezier
float sdBezier(vec2 pos, vec2 A, vec2 B, vec2 C) {
    vec2 a = B - A;
    vec2 b = A - 2.0 * B + C;
    vec2 c = a * 2.0;
    vec2 d = A - pos;
    
    float kk = 1.0 / dot(b, b);
    float kx = kk * dot(a, b);
    float ky = kk * (2.0 * dot(a, a) + dot(d, b)) / 3.0;
    float kz = kk * dot(d, a);
    
    float res = 0.0;
    float p = ky - kx * kx;
    float p3 = p * p * p;
    float q = kx * (2.0 * kx * kx - 3.0 * ky) + kz;
    float h = q * q + 4.0 * p3;
    
    if (h >= 0.0) {
        h = sqrt(h);
        vec2 x = (vec2(h, -h) - q) / 2.0;
        vec2 uv = sign(x) * pow(abs(x), vec2(1.0 / 3.0));
        float t = clamp(uv.x + uv.y - kx, 0.0, 1.0);
        res = dot2(d + (c + b * t) * t);
    } else {
        float z = sqrt(-p);
        float v = acos(q / (p * z * 2.0)) / 3.0;
        float m = cos(v);
        float n = sin(v) * 1.732050808;
        vec3 t = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0, 1.0);
        res = min(dot2(d + (c + b * t.x) * t.x),
                  dot2(d + (c + b * t.y) * t.y));
    }
    
    return sqrt(res);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    vec2 fragCoord = gl_FragCoord.xy / u_dpr;
    fragCoord.y = u_resolution.y / u_dpr - fragCoord.y;
    
    float d = sdBezier(fragCoord, u_p0, u_p1, u_p2) - u_thickness * 0.5;
    
    float aa = 1.0 / u_dpr;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    FragColor = u_color * alpha;
}
