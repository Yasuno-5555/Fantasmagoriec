#pragma once
#include <cstdint>

enum class RenderLayer {
    Background,
    Content,
    Overlay,
    Tooltip,
    Count
};

enum class DrawType : uint32_t {
    Rect = 0,
    Text = 1,
    Image = 2,
    Line = 10,
    Bezier = 11,
    Arc = 12,
    Scissor = 13,
    Custom = 99
};

// 64 bytes for GPU alignment
struct DrawCmd {
    DrawType type;          // 0  (4)
    float x, y, w, h;       // 4  (16)
    uint32_t color;         // 20 (4)
    float p1, p2, p3, p4;   // 24 (16)
    float cx, cy, cw, ch;   // 40 (16) - Clip Rect
    float texture_id;       // 56 (4)
    float skew;             // 60 (4)
    const char* text_ptr;   // 64 (8) - For CPU-side text storage
                            // Total 64 bytes
};

struct Transform {
    float m[6]; // [0] [2] [4]  |  a c tx
                // [1] [3] [5]  |  b d ty
    
    static Transform Identity() {
        return {{1,0,0, 0,1,0}};
    }
    
    static Transform Translate(float x, float y) {
        return {{1,0, 0,1, x,y}};
    }
    
    static Transform Scale(float s) {
        return {{s,0, 0,s, 0,0}};
    }
    
    Transform operator*(const Transform& rhs) const {
        Transform r;
        r.m[0] = m[0]*rhs.m[0] + m[2]*rhs.m[1];
        r.m[1] = m[1]*rhs.m[0] + m[3]*rhs.m[1];
        r.m[2] = m[0]*rhs.m[2] + m[2]*rhs.m[3];
        r.m[3] = m[1]*rhs.m[2] + m[3]*rhs.m[3];
        r.m[4] = m[0]*rhs.m[4] + m[2]*rhs.m[5] + m[4];
        r.m[5] = m[1]*rhs.m[4] + m[3]*rhs.m[5] + m[5];
        return r;
    }

    void apply(float& x, float& y) const {
        float nx = m[0]*x + m[2]*y + m[4];
        float ny = m[1]*x + m[3]*y + m[5];
        x = nx; y = ny;
    }

    // Phase 2: Inverse for Interaction
    void apply_inverse(float& x, float& y) const {
        // Inverse of affine [a c tx]
        //                  [b d ty]
        //                  [0 0 1 ]
        // det = ad - bc
        float det = m[0]*m[3] - m[1]*m[2];
        if(det == 0.0f) return; // Non-invertible
        float invDet = 1.0f / det;
        
        float nx = x - m[4];
        float ny = y - m[5];
        
        // A^-1 * (P - T)
        float ix = (m[3] * nx - m[2] * ny) * invDet;
        float iy = (-m[1] * nx + m[0] * ny) * invDet;
        
        x = ix; y = iy;
    }
};

