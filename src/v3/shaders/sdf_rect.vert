// Fantasmagorie v3 - Full-screen Quad Vertex Shader
// Used for SDF rendering (covers entire viewport, fragment shader does the work)
#version 330 core

// Full-screen triangle (no vertex buffer needed)
// Uses gl_VertexID to generate positions
void main() {
    // Generate full-screen triangle vertices
    // ID 0: (-1, -1), ID 1: (3, -1), ID 2: (-1, 3)
    vec2 pos = vec2(
        float((gl_VertexID & 1) * 4 - 1),
        float((gl_VertexID >> 1) * 4 - 1)
    );
    gl_Position = vec4(pos, 0.0, 1.0);
}
