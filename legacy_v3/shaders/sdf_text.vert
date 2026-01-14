#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 v_uv;

uniform vec2 u_resolution;

void main() {
    // Screen coords [0, resolution] -> Normalized Device Coordinates [-1, 1]
    vec2 ndc = (a_pos / u_resolution) * 2.0 - 1.0;
    
    // Invert Y because our screen origin is top-left
    gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
    v_uv = a_uv;
}
