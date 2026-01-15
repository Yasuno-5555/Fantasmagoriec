#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_Texture;
uniform vec2 u_Offset;
uniform vec2 u_Resolution;

void main() {
    vec2 offset = u_Offset / u_Resolution;
    
    vec4 sum = texture(u_Texture, TexCoords) * 4.0;
    sum += texture(u_Texture, TexCoords + vec2( offset.x,  offset.y));
    sum += texture(u_Texture, TexCoords + vec2(-offset.x,  offset.y));
    sum += texture(u_Texture, TexCoords + vec2( offset.x, -offset.y));
    sum += texture(u_Texture, TexCoords + vec2(-offset.x, -offset.y));
    
    FragColor = sum / 8.0;
}
