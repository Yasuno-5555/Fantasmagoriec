#include "renderer.hpp"
#include "../core/utf8_decode.hpp"
#include <cmath>

// ========================
// DrawList Implementation
// ========================

void DrawList::clear() { 
    for(int i=0; i<4; ++i) cmds[i].clear();
    clip_stack.clear(); 
    current_clip = {-10000, -10000, 20000, 20000};
    current_layer = RenderLayer::Content;
}

void DrawList::push_clip_rect(float x, float y, float w, float h) {
    float nx = std::max(x, current_clip.x);
    float ny = std::max(y, current_clip.y);
    float nw = std::min(x+w, current_clip.x+current_clip.w) - nx;
    float nh = std::min(y+h, current_clip.y+current_clip.h) - ny;
    if(nw < 0) nw = 0;
    if(nh < 0) nh = 0;
    
    clip_stack.push_back(current_clip);
    current_clip = {nx, ny, nw, nh};
    
    // Core Polish: Sync Hardware Scissor
    add_scissor(nx, ny, nw, nh);
}

void DrawList::pop_clip_rect() {
    if(!clip_stack.empty()) {
        current_clip = clip_stack.back();
        clip_stack.pop_back();
        
        // Restore Hardware Scissor
        // If we are back to root (large clip), disable it
        if (current_clip.w >= 10000) {
            add_scissor(0, 0, -1, 0); // Disable
        } else {
            add_scissor(current_clip.x, current_clip.y, current_clip.w, current_clip.h);
        }
    }
}

void DrawList::add_rect(float x, float y, float w, float h, uint32_t c, float r) { 
    DrawCmd cmd;
    cmd.type = DrawType::Rect;
    cmd.x=x; cmd.y=y; cmd.w=w; cmd.h=h;
    cmd.color=c;
    cmd.p1=r; cmd.p2=0; cmd.p3=0; cmd.p4=0;
    cmd.cx=current_clip.x; cmd.cy=current_clip.y; cmd.cw=current_clip.w; cmd.ch=current_clip.h;
    cmd.texture_id = 0; 
    cmd.skew = 0;
    cmds[(int)current_layer].push_back(cmd);
}

void DrawList::add_texture_rect(float x, float y, float w, float h, void* texture_id, float u0, float v0, float u1, float v1, uint32_t color) {
    DrawCmd cmd;
    cmd.type = DrawType::Image; // Use Image type or Custom? Image type expects params maybe?
    // Actually DrawType::Image usually means "Use Texture". 
    // In Renderer::render, DrawType::Image is handled... wait, render() logic doesn't switch on Type for "Draw". 
    // It relies on Shader `iMode`? 
    // Let's check vs_full. `iMode` (loc 4).
    // `DrawCmd.type` is mapped to `iMode` (loc 4). as float.
    // In VS: `vMode = iMode`.
    // In FS: `if(vMode > 0.5) ... texture ... else ... sdf`.
    // So if `type` > 0.5 (Text=1, Image=2), it samples texture?
    // Wait, Text=1. Text consumes a texture (Atlas).
    // Rect=0. Rect uses SDF or plain color (vMode <= 0.5).
    // So Image (Type=2) > 0.5 is correct.
    
    cmd.type = DrawType::Image;
    cmd.x=x; cmd.y=y; cmd.w=w; cmd.h=h;
    cmd.color=color;
    cmd.p1=u0; cmd.p2=v0; cmd.p3=u1; cmd.p4=v1;
    cmd.cx=current_clip.x; cmd.cy=current_clip.y; cmd.cw=current_clip.w; cmd.ch=current_clip.h;
    
    // safe cast void* to float (assuming 32bit or handle handle)
    // For now cast to uintptr then float? Warning on 64bit.
    // This framework seems to use float texture_id?
    // Renderer::render uses `(GLuint)tid_f`.
    // If pointer is passed, we need mapping or strict ID usage.
    // For now assuming the user casts ID to void*.
    cmd.texture_id = (float)(uintptr_t)texture_id; 
    
    cmd.skew = 0;
    cmds[(int)current_layer].push_back(cmd);
}

void DrawList::add_text(Font& font, const std::string& text, float x, float y, uint32_t c, bool bold, bool italic) {
    float cx = x;
    float cy = y + font.size; 
    float skew_val = italic ? -0.25f : 0.0f; // Shear factor
    
    const char* ptr = text.c_str();
    while (*ptr) {
            uint32_t codepoint = DecodeNextUTF8(ptr);
            Glyph& g = font.get_glyph(codepoint);
            float xpos = cx + g.bearing_x;
            float ypos = cy - g.bearing_y;
            float w = g.width;
            float h = g.height;
            
            if(w > 0 && h > 0) {
                DrawCmd cmd;
                cmd.type = DrawType::Text; // Text
                cmd.x=xpos; cmd.y=ypos; cmd.w=w; cmd.h=h;
                cmd.color=c;
                cmd.p1=g.u0; cmd.p2=g.v0; cmd.p3=g.u1; cmd.p4=g.v1;
                cmd.cx=current_clip.x; cmd.cy=current_clip.y; cmd.cw=current_clip.w; cmd.ch=current_clip.h;
                cmd.texture_id = 0; 
                cmd.skew = skew_val;
                
                cmds[(int)current_layer].push_back(cmd);
                
                if (bold) {
                    cmd.x += 1.0f; // 1px smear
                    cmds[(int)current_layer].push_back(cmd);
                }
            }
            cx += g.advance;
    }
}

void DrawList::add_focus_ring(float x, float y, float w, float h, uint32_t c, float thickness) {
    add_rect(x - thickness, y - thickness, w + thickness*2, thickness, c);
    add_rect(x - thickness, y + h, w + thickness*2, thickness, c);
    add_rect(x - thickness, y, thickness, h, c);
    add_rect(x + w, y, thickness, h, c);
}


void DrawList::add_arc(float cx, float cy, float r, float min_a, float max_a, uint32_t col, float thickness) {
    int segments = 24;
    float step = (max_a - min_a) / segments;
    float angle = min_a;
    
    float x1 = cx + cos(angle) * r;
    float y1 = cy + sin(angle) * r;
    
    for(int i=0; i<segments; ++i) {
        angle += step;
        float x2 = cx + cos(angle) * r;
        float y2 = cy + sin(angle) * r;
        
        add_line(x1, y1, x2, y2, col, thickness);
        x1 = x2; y1 = y2;
    }
}

// ========================
// Utils
// ========================

TextMeasurement MeasureText(Font& font, const std::string& text) {
    float w = 0;
    float max_w = 0;
    float h = font.size; // Initial line
    
    const char* ptr = text.c_str();
    while (*ptr) {
        uint32_t cp = DecodeNextUTF8(ptr);
        
        if (cp == '\n') {
            max_w = std::max(max_w, w);
            w = 0;
            h += font.size; // Simple line height
            continue;
        }
        Glyph& g = font.get_glyph(cp); 
        w += g.advance;
    }
    max_w = std::max(max_w, w);
    return {max_w, h};
}

// ========================
// Renderer Implementation
// ========================

const char* vs_full = R"(#version 330 core
layout(location=0)in vec2 aPos;
layout(location=1)in vec4 iRect;
layout(location=2)in vec4 iColor;
layout(location=3)in vec4 iParams; // x=Radius, y=Angle(Rot)
layout(location=4)in float iMode;
layout(location=5)in vec4 iClip;
layout(location=6)in float iTexId; 
layout(location=7)in float iSkew; // New
uniform vec2 uScreen;
out vec2 vP; out vec2 vUV; out vec4 vC; out vec2 vS; out float vR; out float vMode;
out vec2 vScreenPos; out vec4 vClip; out float vT;
out float vSoftness;
void main(){
    vS = iRect.zw; vC = iColor; vR = iParams.x; vMode = iMode; vClip = iClip; vT=iTexId;
    vSoftness = iParams.z;
    vUV = mix(iParams.xy, iParams.zw, aPos); // Fixed mixed params
    
    // Scale
    vec2 pos = aPos * iRect.zw;
    
    // Skew (Italic)
    if (iSkew != 0.0) {
        // Skew top (y=0 in UV space? No, aPos y=0 is top? or bottom?)
        // Renderer quadV is 0,0 1,0 0,1 1,1
        // Usually 0,0 is Top-Left in UI.
        // If we want italic, we shift Top Right. 
        // Or Bottom Left. 
        // Let's try: x += (1.0 - aPos.y) * iSkew * height?
        // Or just x += pos.y * skew.
        // Let's use simple shear relative to height?
        // pos.x += (iRect.w - pos.y) * iSkew; // Skew top relative to bottom?
        // Let's stick to standard shear:
        // x' = x + y * tan(theta)
        // We pass tan(theta) as iSkew.
        // Note: Check coordinate system. If Y is down, Pos Y grows.
        // If we want top to move right... and Top has Y=0 locally?
        // Wait, aPos y goes 0 to 1. 0 is usually top in UI generation?
        // If 0 is top, we want 0 to be shifted.
        // so x += (1.0 - aPos.y) * iRect.w * iSkew?
        // Simpler: x -= aPos.y * iSkew * h; (shifts bottom left)
        // Let's just do: pos.x += (1.0 - aPos.y) * iRect.w * iSkew;
        // Actually, let's just use a fixed pixel shift passed as iSkew?
        // No, iSkew is typically a factor.
        // Let's rely on simple `pos.x += (1.0 - aPos.y) * 10.0 * iSkew;` where iSkew is 0 or 1.
        // Or pass the actual pixel offset in `skew`.
        
        // Let's assume input 'skew' is roughly -0.2f for italic.
        // And we want to skew relative to baseline.
        // Standard italic matrix: [1, k, 0, 1]. x' = x + ky.
        pos.x += pos.y * iSkew; 
    }
    
    // Rotate (around center of rect)
    float angle = iParams.y;
    if(angle != 0.0) {
        vec2 center = iRect.zw * 0.5;
        pos -= center;
        float c = cos(angle); float s = sin(angle);
        pos = vec2(pos.x*c - pos.y*s, pos.x*s + pos.y*c);
        pos += center;
    }
    
    vP = pos;
    vec2 sp = iRect.xy + vP;
    vScreenPos = sp;
    
    vec2 n = (sp/uScreen)*2.0-1.0; n.y = -n.y;
    gl_Position = vec4(n,0,1);
}
)";
const char* fs_full = R"(#version 330 core
in vec2 vP; in vec2 vUV; in vec4 vC; in vec2 vS; in float vR; in float vMode;
in vec2 vScreenPos; in vec4 vClip; in float vT;
in float vSoftness; // from iParams.z
uniform sampler2D uTex; // Assuming single atlas for now
out vec4 FragColor;
float sdf(vec2 p,vec2 b,float r){vec2 d=abs(p-b)-b+r;return min(max(d.x,d.y),0.0)+length(max(d,0.0))-r;}
void main(){
    if (vScreenPos.x < vClip.x || vScreenPos.y < vClip.y ||
        vScreenPos.x > vClip.x + vClip.z || vScreenPos.y > vClip.y + vClip.w) {
        discard;
    }
    if(vMode > 0.5) {
        float alpha = texture(uTex, vUV).r;
        FragColor = vec4(vC.rgb, vC.a * alpha);
    } else {
        vec2 c=vS*0.5;
        float d=sdf(vP-c,c,vR);
        float s = max(0.5, vSoftness);
        float a=1.0-smoothstep(-s,s,d);
        FragColor=vec4(vC.rgb,vC.a*a);
    }
}
)";

static float quadV[] = {0,0,1,0,0,1,1,1};

void Renderer::init() {
    GLuint vs=glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs,1,&vs_full,NULL); glCompileShader(vs);
    GLuint fs=glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs,1,&fs_full,NULL); glCompileShader(fs);
    sp=glCreateProgram(); glAttachShader(sp,vs); glAttachShader(sp,fs); glLinkProgram(sp);
    
    GLuint qvbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&qvbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,qvbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quadV),quadV,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,0x1406,0,2*sizeof(float),(void*)0);
    
    // Instance Data
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    // DrawCmd stride is 64
    // DrawCmd stride is 64
    GLsizei stride = sizeof(DrawCmd);
    // Adjusted offsets for new DrawCmd layout:
    // Type (0), Rect (4), Color (20), Params (24), Clip (40), Tex (56)
    
    // Loc 1: Rect (4 floats) -> Offset 4
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,0x1406,0,stride,(void*)4); glVertexAttribDivisor(1,1); 
    // Loc 2: Color (4 bytes, normalized) -> Offset 20
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,4,0x1401,1,stride,(void*)20); glVertexAttribDivisor(2,1); 
    // Loc 3: Params (4 floats) -> Offset 24
    glEnableVertexAttribArray(3); glVertexAttribPointer(3,4,0x1406,0,stride,(void*)24); glVertexAttribDivisor(3,1); 
    // Loc 4: Mode/Type (1 float/int) -> Offset 0
    // Shader expects float at loc 4 currently. Casting enum (uint32) to float works if we use glVertexAttribPointer(GL_FLOAT).
    // It reads 4 bytes. 
    glEnableVertexAttribArray(4); glVertexAttribPointer(4,1,0x1406,0,stride,(void*)0); glVertexAttribDivisor(4,1); 
    // Loc 5: Clip (4 floats) -> Offset 40
    glEnableVertexAttribArray(5); glVertexAttribPointer(5,4,0x1406,0,stride,(void*)40); glVertexAttribDivisor(5,1); 
    // Loc 6: TexId (1 float) -> Offset 56
    glEnableVertexAttribArray(6); glVertexAttribPointer(6,1,0x1406,0,stride,(void*)56); glVertexAttribDivisor(6,1);
    // Loc 7: Skew (1 float) -> Offset 60
    glEnableVertexAttribArray(7); glVertexAttribPointer(7,1,0x1406,0,stride,(void*)60); glVertexAttribDivisor(7,1); 
}

void DrawList::add_line(float x1, float y1, float x2, float y2, uint32_t color, float thickness) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = std::sqrt(dx*dx + dy*dy);
    if(len == 0) return;
    float angle = std::atan2(dy, dx);
    
    DrawCmd cmd;
    cmd.type = DrawType::Line;
    cmd.x=x1; cmd.y=y1; cmd.w=len; cmd.h=thickness;
    cmd.color=color;
    cmd.p1=0; // Radius=0 (Sharp)
    cmd.p2=angle; // Rotation
    cmd.p3=0; cmd.p4=0;
    cmd.cx=current_clip.x; cmd.cy=current_clip.y; cmd.cw=current_clip.w; cmd.ch=current_clip.h;
    cmd.texture_id = 0; 
    cmd.skew = 0;
    cmds[(int)current_layer].push_back(cmd);
}

void DrawList::add_bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, uint32_t color, float thickness) {
    // Simple iterative caching
    const int segments = 20;
    float px = x1, py = y1;
    for(int i=1; i<=segments; ++i) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;
        float tt = t*t;
        float uu = u*u;
        float uuu = uu * u;
        float ttt = tt * t;
        
        // Cubic Bezier
        float x = uuu * x1 + 3 * uu * t * x2 + 3 * u * tt * x3 + ttt * x4;
        float y = uuu * y1 + 3 * uu * t * y2 + 3 * u * tt * y3 + ttt * y4;
        
        add_line(px, py, x, y, color, thickness);
        px = x; py = y;
    }
}

void DrawList::add_image(uint32_t tex_id, float x, float y, float w, float h) {
    DrawCmd cmd;
    cmd.type = DrawType::Image;
    cmd.x=x; cmd.y=y; cmd.w=w; cmd.h=h;
    cmd.color=0xFFFFFFFF; // White tint
    cmd.p1=0; cmd.p2=0; cmd.p3=1; cmd.p4=1; // UVs (0,0) to (1,1)
    cmd.cx=current_clip.x; cmd.cy=current_clip.y; cmd.cw=current_clip.w; cmd.ch=current_clip.h;
    cmd.texture_id = (float)tex_id; 
    cmd.skew = 0;
    cmds[(int)current_layer].push_back(cmd);
}

void Renderer::render(const DrawList& l, float w, float h, GLuint default_tex) {
    glUseProgram(sp);
    glUniform2f(glGetUniformLocation(sp,"uScreen"),w,h);
    glUniform1i(glGetUniformLocation(sp,"uTex"), 0);
    glActiveTexture(GL_TEXTURE0);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);

    // Render Layers
    for (int i=0; i<4; ++i) {
        const auto& cmds = l.cmds[i];
        if (cmds.empty()) continue;
        
        // Full Upload (Stream) - Simplest strategy for now
        // If buffer is too small we might have issues but we assume sufficient VRAM/driver magic
        size_t total_size = cmds.size() * sizeof(DrawCmd);
        glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, total_size, cmds.data());
        
        // iterate and draw batches
        size_t batch_start = 0;
        GLuint current_tex = 0; // 0 means default_tex needs binding or unset
        
        // Initial state
        float first_id_f = cmds[0].texture_id;
        GLuint first_id = (GLuint)first_id_f;
        GLuint bind_id = (first_id == 0) ? default_tex : first_id;
        glBindTexture(GL_TEXTURE_2D, bind_id);
        current_tex = bind_id;
        
        bool scissor_enabled = false;
        
        for (size_t k = 0; k < cmds.size(); ++k) {
             const DrawCmd& cmd = cmds[k];
             
             // Check for Scissor Command
             if ((int)cmd.type == 13) { // Scissor
                 // Flush pending
                 if (k > batch_start) {
                     glDrawArraysInstanced(0x0005, (GLint)batch_start, 4, (GLsizei)(k - batch_start));
                 }
                 
                 // Apply Scissor
                 if (cmd.w < 0) {
                     glDisable(GL_SCISSOR_TEST);
                     scissor_enabled = false;
                 } else {
                     if (!scissor_enabled) { glEnable(GL_SCISSOR_TEST); scissor_enabled = true; }
                     // Convert Top-Left (x,y) to Bottom-Left (x, h-y-h_scis)
                     // Need integer coords
                     GLint sx = (GLint)cmd.x;
                     GLint sw = (GLint)cmd.w;
                     GLint sh = (GLint)cmd.h;
                     GLint sy = (GLint)(h - cmd.y - cmd.h); 
                     glScissor(sx, sy, sw, sh);
                 }
                 
                 // Skip this command in drawing (it's a state command, not a quad)
                 batch_start = k + 1;
                 continue;
             }
             
             float tid_f = cmd.texture_id;
             GLuint tid = (GLuint)tid_f;
             GLuint resolved = (tid == 0) ? default_tex : tid;
             
             if (resolved != current_tex) {
                 // Flush
                 glDrawArraysInstanced(0x0005, (GLint)batch_start, 4, (GLsizei)(k - batch_start));
                 
                 // Bind new
                 glBindTexture(GL_TEXTURE_2D, resolved);
                 current_tex = resolved;
                 batch_start = k;
             }
        }
        
        // Flush remaining
        if (batch_start < cmds.size()) {
            glDrawArraysInstanced(0x0005, (GLint)batch_start, 4, (GLsizei)(cmds.size() - batch_start));
        }
        
        if (scissor_enabled) glDisable(GL_SCISSOR_TEST);
    }
}
