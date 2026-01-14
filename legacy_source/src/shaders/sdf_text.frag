#version 330 core
out vec4 FragColor;

in vec2 v_uv;

uniform vec4 u_color;
uniform sampler2D u_Atlas;

void main() {
    float distance = texture(u_Atlas, v_uv).r;
    
    // SDF rendering: 0.5 is the edge.
    float smoothing = 0.02;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    
    FragColor = vec4(u_color.rgb, u_color.a * alpha);
}
