// Fantasmagorie v3 - Path System
// SVG-like path builder with adaptive Bezier tessellation
#pragma once

#include "types.hpp"
#include <vector>
#include <cmath>

namespace fanta {

// ============================================================================
// Path Segment Types
// ============================================================================

enum class PathVerb {
    MoveTo,
    LineTo,
    QuadTo,      // Quadratic Bezier
    CubicTo,     // Cubic Bezier
    ArcTo,       // Elliptical arc
    Close,
};

struct PathSegment {
    PathVerb verb;
    Vec2 p1, p2, p3;  // Up to 3 control points
    float arc_rx, arc_ry, arc_rotation;  // For ArcTo
    bool arc_large, arc_sweep;
};

// ============================================================================
// Path Builder
// ============================================================================

class Path {
public:
    // Construction
    Path& move_to(float x, float y) {
        return move_to({x, y});
    }
    
    Path& move_to(Vec2 p) {
        segments_.push_back({PathVerb::MoveTo, p, {}, {}});
        current_ = p;
        start_ = p;
        return *this;
    }
    
    Path& line_to(float x, float y) {
        return line_to({x, y});
    }
    
    Path& line_to(Vec2 p) {
        segments_.push_back({PathVerb::LineTo, p, {}, {}});
        current_ = p;
        return *this;
    }
    
    Path& quad_to(Vec2 control, Vec2 end) {
        segments_.push_back({PathVerb::QuadTo, control, end, {}});
        current_ = end;
        return *this;
    }
    
    Path& quad_to(float cx, float cy, float ex, float ey) {
        return quad_to({cx, cy}, {ex, ey});
    }
    
    Path& cubic_to(Vec2 c1, Vec2 c2, Vec2 end) {
        segments_.push_back({PathVerb::CubicTo, c1, c2, end});
        current_ = end;
        return *this;
    }
    
    Path& cubic_to(float c1x, float c1y, float c2x, float c2y, float ex, float ey) {
        return cubic_to({c1x, c1y}, {c2x, c2y}, {ex, ey});
    }
    
    Path& arc_to(float rx, float ry, float rotation, bool large_arc, bool sweep, Vec2 end) {
        PathSegment seg;
        seg.verb = PathVerb::ArcTo;
        seg.p1 = end;
        seg.arc_rx = rx;
        seg.arc_ry = ry;
        seg.arc_rotation = rotation;
        seg.arc_large = large_arc;
        seg.arc_sweep = sweep;
        segments_.push_back(seg);
        current_ = end;
        return *this;
    }
    
    Path& close() {
        segments_.push_back({PathVerb::Close, start_, {}, {}});
        current_ = start_;
        return *this;
    }
    
    // Convenience: Rounded rectangle
    static Path rounded_rect(Rect r, CornerRadius cr) {
        Path p;
        float l = r.x, t = r.y, w = r.w, h = r.h;
        float right = l + w, bottom = t + h;
        
        // Clamp radii
        float maxR = std::min(w, h) * 0.5f;
        float tl = std::min(cr.tl, maxR);
        float tr = std::min(cr.tr, maxR);
        float br = std::min(cr.br, maxR);
        float bl = std::min(cr.bl, maxR);
        
        p.move_to(l + tl, t);
        p.line_to(right - tr, t);
        if (tr > 0) p.quad_to({right, t}, {right, t + tr});
        p.line_to(right, bottom - br);
        if (br > 0) p.quad_to({right, bottom}, {right - br, bottom});
        p.line_to(l + bl, bottom);
        if (bl > 0) p.quad_to({l, bottom}, {l, bottom - bl});
        p.line_to(l, t + tl);
        if (tl > 0) p.quad_to({l, t}, {l + tl, t});
        p.close();
        
        return p;
    }
    
    // Convenience: Circle
    static Path circle(Vec2 center, float radius) {
        Path p;
        // Approximate circle with 4 cubic beziers
        float k = 0.5522847498f; // (4/3)*tan(pi/8)
        float kr = k * radius;
        float cx = center.x, cy = center.y;
        
        p.move_to(cx + radius, cy);
        p.cubic_to({cx + radius, cy + kr}, {cx + kr, cy + radius}, {cx, cy + radius});
        p.cubic_to({cx - kr, cy + radius}, {cx - radius, cy + kr}, {cx - radius, cy});
        p.cubic_to({cx - radius, cy - kr}, {cx - kr, cy - radius}, {cx, cy - radius});
        p.cubic_to({cx + kr, cy - radius}, {cx + radius, cy - kr}, {cx + radius, cy});
        p.close();
        
        return p;
    }
    
    // Convenience: Regular polygon
    static Path polygon(Vec2 center, float radius, int sides) {
        Path p;
        for (int i = 0; i < sides; ++i) {
            float angle = (float)i / sides * 2.0f * 3.14159265f - 3.14159265f / 2;
            Vec2 pt = {
                center.x + radius * std::cos(angle),
                center.y + radius * std::sin(angle)
            };
            if (i == 0) p.move_to(pt);
            else p.line_to(pt);
        }
        p.close();
        return p;
    }
    
    // Convenience: Star
    static Path star(Vec2 center, float outerR, float innerR, int points) {
        Path p;
        for (int i = 0; i < points * 2; ++i) {
            float angle = (float)i / (points * 2) * 2.0f * 3.14159265f - 3.14159265f / 2;
            float r = (i % 2 == 0) ? outerR : innerR;
            Vec2 pt = {
                center.x + r * std::cos(angle),
                center.y + r * std::sin(angle)
            };
            if (i == 0) p.move_to(pt);
            else p.line_to(pt);
        }
        p.close();
        return p;
    }
    
    // Accessors
    const std::vector<PathSegment>& segments() const { return segments_; }
    bool empty() const { return segments_.empty(); }
    void clear() { segments_.clear(); current_ = start_ = {}; }
    
private:
    std::vector<PathSegment> segments_;
    Vec2 current_, start_;
};

// ============================================================================
// Adaptive Bezier Tessellator
// ============================================================================

class BezierTessellator {
public:
    float tolerance = 0.5f;      // Max error in logical pixels
    float min_segment = 1.0f;    // Min segment length
    int max_subdivisions = 64;   // Prevent infinite recursion
    
    // Tessellate quadratic Bezier
    void tessellate_quad(Vec2 p0, Vec2 p1, Vec2 p2, float zoom, 
                         std::vector<Vec2>& out) {
        tessellate_quad_recursive(p0, p1, p2, zoom, 0, out);
        out.push_back(p2);
    }
    
    // Tessellate cubic Bezier
    void tessellate_cubic(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float zoom,
                          std::vector<Vec2>& out) {
        tessellate_cubic_recursive(p0, p1, p2, p3, zoom, 0, out);
        out.push_back(p3);
    }
    
private:
    // Distance from point to line
    float point_line_distance(Vec2 p, Vec2 a, Vec2 b) const {
        Vec2 ab = b - a;
        Vec2 ap = p - a;
        float len2 = ab.x * ab.x + ab.y * ab.y;
        if (len2 < 0.0001f) return ap.length();
        float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / len2, 0.0f, 1.0f);
        Vec2 proj = {a.x + t * ab.x, a.y + t * ab.y};
        return (p - proj).length();
    }
    
    // Quadratic subdivision
    void tessellate_quad_recursive(Vec2 p0, Vec2 p1, Vec2 p2, float zoom,
                                   int depth, std::vector<Vec2>& out) {
        // Flatness test: distance from control point to line
        float flatness = point_line_distance(p1, p0, p2);
        float adjusted_tolerance = tolerance / zoom;
        
        if (flatness < adjusted_tolerance || depth >= max_subdivisions) {
            return;
        }
        
        // De Casteljau split at t=0.5
        Vec2 m01 = (p0 + p1) * 0.5f;
        Vec2 m12 = (p1 + p2) * 0.5f;
        Vec2 mid = (m01 + m12) * 0.5f;
        
        tessellate_quad_recursive(p0, m01, mid, zoom, depth + 1, out);
        out.push_back(mid);
        tessellate_quad_recursive(mid, m12, p2, zoom, depth + 1, out);
    }
    
    // Cubic subdivision
    void tessellate_cubic_recursive(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, 
                                    float zoom, int depth, std::vector<Vec2>& out) {
        // Flatness: max distance of control points from chord
        float d1 = point_line_distance(p1, p0, p3);
        float d2 = point_line_distance(p2, p0, p3);
        float flatness = std::max(d1, d2);
        float adjusted_tolerance = tolerance / zoom;
        
        if (flatness < adjusted_tolerance || depth >= max_subdivisions) {
            return;
        }
        
        // De Casteljau split at t=0.5
        Vec2 m01 = (p0 + p1) * 0.5f;
        Vec2 m12 = (p1 + p2) * 0.5f;
        Vec2 m23 = (p2 + p3) * 0.5f;
        Vec2 m012 = (m01 + m12) * 0.5f;
        Vec2 m123 = (m12 + m23) * 0.5f;
        Vec2 mid = (m012 + m123) * 0.5f;
        
        tessellate_cubic_recursive(p0, m01, m012, mid, zoom, depth + 1, out);
        out.push_back(mid);
        tessellate_cubic_recursive(mid, m123, m23, p3, zoom, depth + 1, out);
    }
};

// ============================================================================
// Path Tessellation (convert Path to polyline)
// ============================================================================

struct TessellatedPath {
    std::vector<Vec2> points;
    std::vector<size_t> contour_starts;  // Index where each subpath starts
    bool closed = false;
};

inline TessellatedPath tessellate_path(const Path& path, float zoom = 1.0f) {
    TessellatedPath result;
    BezierTessellator tess;
    
    Vec2 current = {0, 0};
    Vec2 start = {0, 0};
    
    result.contour_starts.push_back(0);
    
    for (const auto& seg : path.segments()) {
        switch (seg.verb) {
            case PathVerb::MoveTo:
                if (!result.points.empty()) {
                    result.contour_starts.push_back(result.points.size());
                }
                current = seg.p1;
                start = seg.p1;
                result.points.push_back(current);
                break;
                
            case PathVerb::LineTo:
                result.points.push_back(seg.p1);
                current = seg.p1;
                break;
                
            case PathVerb::QuadTo:
                tess.tessellate_quad(current, seg.p1, seg.p2, zoom, result.points);
                current = seg.p2;
                break;
                
            case PathVerb::CubicTo:
                tess.tessellate_cubic(current, seg.p1, seg.p2, seg.p3, zoom, result.points);
                current = seg.p3;
                break;
                
            case PathVerb::Close:
                if (result.points.size() > result.contour_starts.back()) {
                    result.points.push_back(start);
                }
                result.closed = true;
                current = start;
                break;
                
            case PathVerb::ArcTo:
                // TODO: Convert arc to bezier segments
                result.points.push_back(seg.p1);
                current = seg.p1;
                break;
        }
    }
    
    return result;
}

} // namespace fanta
