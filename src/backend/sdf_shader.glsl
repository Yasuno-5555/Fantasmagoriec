// Phase 2: SDF Shader (Edge AA, Rounded Corner, Circle)
// DPI変換はshader内で行う

#ifdef VERTEX_SHADER
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aSize;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aRadius;
layout(location = 4) in int aShapeType; // 0=Rect, 1=RoundedRect, 2=Circle, 3=Line

uniform vec2 uViewportSize;      // 物理ピクセルサイズ
uniform float uDevicePixelRatio; // devicePixelRatio

out vec2 vPos;
out vec2 vSize;
out vec4 vColor;
out float vRadius;
out int vShapeType;

void main() {
    // 論理px → 物理px変換（shader内で）
    vec2 physical_pos = aPos * uDevicePixelRatio;
    vec2 physical_size = aSize * uDevicePixelRatio;
    
    // NDC座標変換: 論理座標(aPos)からNDC座標へ
    // viewportSizeは物理ピクセルなので、物理座標を使う
    vec2 ndc = physical_pos / uViewportSize * 2.0 - 1.0;
    ndc.y = -ndc.y; // Flip Y
    
    gl_Position = vec4(ndc, 0.0, 1.0);
    
    // Pass to fragment shader (物理ピクセル単位)
    vPos = physical_pos;
    vSize = physical_size;
    vColor = aColor;
    vRadius = aRadius * uDevicePixelRatio;
    vShapeType = aShapeType;
}
#endif

#ifdef FRAGMENT_SHADER
#version 330 core

in vec2 vPos;
in vec2 vSize;
in vec4 vColor;
in float vRadius;
in int vShapeType;

uniform vec2 uViewportSize;
uniform float uDevicePixelRatio;

out vec4 FragColor;

// SDF for rounded rectangle
float sdf_rounded_rect(vec2 p, vec2 size, float radius) {
    vec2 d = abs(p) - size + radius;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;
}

// SDF for circle
float sdf_circle(vec2 p, float r) {
    return length(p) - r;
}

// SDF for rectangle
float sdf_rect(vec2 p, vec2 size) {
    vec2 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// SDF for line (capsule)
float sdf_line(vec2 p, vec2 p0, vec2 p1, float thickness) {
    vec2 pa = p - p0;
    vec2 ba = p1 - p0;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - thickness * 0.5;
}

// Smooth step for edge AA
float smooth_edge(float dist, float width) {
    return 1.0 - smoothstep(-width, width, dist);
}

void main() {
    vec2 frag_coord = gl_FragCoord.xy;
    vec2 center = vPos + vSize * 0.5;
    vec2 p = frag_coord - center;
    
    float dist = 0.0;
    
    if (vShapeType == 0) {
        // Rect
        dist = sdf_rect(p, vSize * 0.5);
    } else if (vShapeType == 1) {
        // RoundedRect
        dist = sdf_rounded_rect(p, vSize * 0.5, vRadius);
    } else if (vShapeType == 2) {
        // Circle
        dist = sdf_circle(p, vRadius);
    } else if (vShapeType == 3) {
        // Line: vPosからvPos+vSizeへの線分、vRadiusを太さとして使用
        vec2 p0 = vPos;
        vec2 p1 = vPos + vSize;
        dist = sdf_line(frag_coord, p0, p1, vRadius);
    }
    
    // Edge AA (DPI対応: 物理1pxでスムージング)
    float edge_width = max(1.0, 1.0 / uDevicePixelRatio);
    float alpha = smooth_edge(dist, edge_width);
    
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
#endif
