#pragma once
#include <vector>
#include <cmath>
#include "backend/drawlist.hpp"

namespace fanta {
namespace internal {

// De Casteljau's algorithm for cubic bezier
inline void de_casteljau(
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
    float t,
    float& out_x, float& out_y
) {
    float inv_t = 1.0f - t;
    float x01 = inv_t * x0 + t * x1;
    float y01 = inv_t * y0 + t * y1;
    float x12 = inv_t * x1 + t * x2;
    float y12 = inv_t * y1 + t * y2;
    float x23 = inv_t * x2 + t * x3;
    float y23 = inv_t * y2 + t * y3;
    
    float x012 = inv_t * x01 + t * x12;
    float y012 = inv_t * y01 + t * y12;
    float x123 = inv_t * x12 + t * x23;
    float y123 = inv_t * y12 + t * y23;
    
    out_x = inv_t * x012 + t * x123;
    out_y = inv_t * y012 + t * y123;
}

// Adaptive Tessellation logic
class Tessellator {
public:
    static void flatten_cubic_bezier(
        const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3,
        float scale,
        std::vector<Vec2>& out_points
    ) {
        // Tolerance determines smoothness. 
        // Higher scale -> needs smaller tolerance in local space, or we project flatness to screen space.
        // Let's use a simple distance checking or recursion limit.
        // For adaptive, we can check if control points are close to the line p0-p3.
        
        recursive_bezier(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, scale, out_points, 0);
        out_points.push_back(p3); // Always add end point
    }

private:
    static void recursive_bezier(
        float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
        float scale,
        std::vector<Vec2>& out_points, int depth
    ) {
        // Flatness check: max distance of p1 and p2 from line p0-p3
        float d03 = std::hypot(x3 - x0, y3 - y0);
        
        // If dist check is too complex, simpler heuristic:
        // dist(p0, p1) + dist(p1, p2) + dist(p2, p3) vs dist(p0, p3)
        // If diff is small, it is flat.
        
        float d_arc = std::hypot(x1 - x0, y1 - y0) + std::hypot(x2 - x1, y2 - y1) + std::hypot(x3 - x2, y3 - y2);
        
        // Threshold: 0.25 pixels in screen space is usually enough for high quality.
        // Since we are in logical space, we multiply by scale (DPR * ViewScale) for comparison, OR divide threshold.
        // Let's say threshold is 0.5px.
        float tolerance = 0.5f / scale; 
        
        if (depth > 10 || (d_arc - d03 < tolerance)) {
            out_points.push_back({x0, y0});
            return;
        }
        
        // Split at 0.5
        float x01 = (x0 + x1) * 0.5f, y01 = (y0 + y1) * 0.5f;
        float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
        float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
        float x012 = (x01 + x12) * 0.5f, y012 = (y01 + y12) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        float x0123 = (x012 + x123) * 0.5f, y0123 = (y012 + y123) * 0.5f;
        
        recursive_bezier(x0, y0, x01, y01, x012, y012, x0123, y0123, scale, out_points, depth + 1);
        recursive_bezier(x0123, y0123, x123, y123, x23, y23, x3, y3, scale, out_points, depth + 1);
    }
};

}
}
