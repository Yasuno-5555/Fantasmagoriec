// Fantasmagorie v3 - Common SDF Functions
// Include in other shaders: #include "sdf_common.glsl"
// Note: GLSL doesn't have #include, this is for reference/copy

// ============================================================================
// Basic SDF Primitives
// ============================================================================

// Circle
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

// Rounded Rectangle
float sdRoundedBox(vec2 p, vec2 b, vec4 r) {
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

// Line Segment
float sdSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

// Capsule (rounded line)
float sdCapsule(vec2 p, vec2 a, vec2 b, float r) {
    return sdSegment(p, a, b) - r;
}

// Triangle
float sdTriangle(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
    vec2 e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
    vec2 v0 = p  - p0, v1 = p  - p1, v2 = p  - p2;
    vec2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0, 1.0);
    vec2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0, 1.0);
    vec2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0, 1.0);
    float s = sign(e0.x * e2.y - e0.y * e2.x);
    vec2 d = min(min(vec2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                     vec2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                     vec2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
    return -sqrt(d.x) * sign(d.y);
}

// Polygon (regular)
float sdPolygon(vec2 p, float r, int n) {
    float an = 3.141593 / float(n);
    vec2 acs = vec2(cos(an), sin(an));
    float bn = mod(atan(p.x, p.y), 2.0 * an) - an;
    p = length(p) * vec2(cos(bn), abs(sin(bn)));
    p -= r * acs;
    p.y += clamp(-p.y, 0.0, r * acs.y);
    return length(p) * sign(p.x);
}

// ============================================================================
// SDF Operations
// ============================================================================

// Union
float opUnion(float d1, float d2) {
    return min(d1, d2);
}

// Subtraction
float opSubtraction(float d1, float d2) {
    return max(-d1, d2);
}

// Intersection
float opIntersection(float d1, float d2) {
    return max(d1, d2);
}

// Smooth Union
float opSmoothUnion(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}

// Smooth Subtraction
float opSmoothSubtraction(float d1, float d2, float k) {
    float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
    return mix(d2, -d1, h) + k * h * (1.0 - h);
}

// Smooth Intersection
float opSmoothIntersection(float d1, float d2, float k) {
    float h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) + k * h * (1.0 - h);
}

// ============================================================================
// Transformations
// ============================================================================

// Rotate
vec2 opRotate(vec2 p, float a) {
    float c = cos(a), s = sin(a);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

// Scale (remember to multiply result by scale)
vec2 opScale(vec2 p, float s) {
    return p / s;
}

// Repeat
vec2 opRepeat(vec2 p, vec2 s) {
    return mod(p + s * 0.5, s) - s * 0.5;
}

// Round (thicken)
float opRound(float d, float r) {
    return d - r;
}

// Annular (ring)
float opAnnular(float d, float r) {
    return abs(d) - r;
}
