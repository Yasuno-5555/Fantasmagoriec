#define NOMINMAX
// --- Standard Headers First ---
#include <cstdio>
#include <cstdint>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

#include "backend/opengl_backend.hpp"
#include "core/types_internal.hpp"
#include "backend/drawlist.hpp"
#include "text/glyph_cache.hpp" // Phase 4

// Phase 5.2: Screenshot support
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fanta {

// Phase 2: SDF Shader Source (embedded)
// ... (sdf_vert_src, sdf_frag_src) ...

// Phase 4: Text Shader
static const char* text_vert_src = R"(
#version 330 core
layout(location = 0) in vec4 aData; // xy=pos, zw=uv
uniform vec2 uViewportSize;
out vec2 vUV;
void main() {
    // Map Logic Coords (0..W, 0..H) to NDC (-1..1)
    // Y-down (0 at top) -> Y-up (1 at top, -1 at bottom)
    float x = (aData.x / uViewportSize.x) * 2.0 - 1.0;
    float y = (1.0 - aData.y / uViewportSize.y) * 2.0 - 1.0; 
    gl_Position = vec4(x, y, 0.0, 1.0);
    vUV = aData.zw;
}
)";

static const char* text_frag_src = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uAtlas; 
uniform vec4 uColor;
out vec4 FragColor;

void main() {
    float dist = texture(uAtlas, vUV).r;
    // Standard derivatives for SDF AA
    // Using fwidth ensures sharpness at any scale
    float width = fwidth(dist);
    // Standard SDF edge is at 0.5 (assuming FreeType generated it well)
    // 0.5 - wa/2 to 0.5 + wa/2
    float alpha = smoothstep(0.5 - width, 0.5 + width, dist);
    
    // Discard fully transparent pixels?
    if (alpha <= 0.01) discard; 
    
    FragColor = vec4(uColor.rgb, uColor.a * alpha);
}
)";

static const char* sdf_vert_src = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

uniform vec2 uShapePos;
uniform vec2 uShapeSize;
uniform vec2 uViewportSize;
uniform float uDevicePixelRatio;

out vec2 vFragCoord;

void main() {
    // Fullscreen quad
    vec2 ndc = aPos;
    gl_Position = vec4(ndc, 0.0, 1.0);
    // Convert to screen-space coords where Y=0 is at TOP (like layout coords)
    vec2 raw_coords = (ndc * 0.5 + 0.5) * uViewportSize;
    vFragCoord = vec2(raw_coords.x, uViewportSize.y - raw_coords.y);
}
)";

static const char* sdf_frag_src = R"(
#version 330 core
in vec2 vFragCoord;

uniform vec2 uShapePos;
uniform vec2 uShapeSize;
uniform vec4 uColor;
uniform float uRadius;
uniform int uShapeType;
uniform vec2 uViewportSize;
uniform float uDevicePixelRatio;

out vec4 FragColor;

float sdf_rounded_rect(vec2 p, vec2 size, float radius) {
    // Clamp radius to prevent corner inversion
    radius = min(radius, min(size.x, size.y) * 0.5);
    vec2 d = abs(p) - size + radius;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;
}

float sdf_circle(vec2 p, float r) {
    return length(p) - r;
}

float sdf_rect(vec2 p, vec2 size) {
    vec2 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float smooth_edge(float dist, float width) {
    return 1.0 - smoothstep(-width, width, dist);
}

float sdf_segment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

void main() {
    float dist = 0.0;
    
    // Edge AA: 1 logical pixel
    float aa_width = 1.0 / uDevicePixelRatio;
    
    if (uShapeType == 3) {
        // Line Segment
        vec2 p = vFragCoord; 
        vec2 p0 = uShapePos;
        vec2 p1 = uShapeSize;
        float half_width = uRadius;
        
        dist = sdf_segment(p, p0, p1) - half_width;
        
    } else {
        vec2 shape_pos = uShapePos;
        vec2 shape_size = uShapeSize;
        float shape_radius = uRadius;
        
        vec2 center = shape_pos + shape_size * 0.5;
        vec2 p = vFragCoord - center;
        
        if (uShapeType == 0) {
            dist = sdf_rect(p, shape_size * 0.5);
        } else if (uShapeType == 1) {
            dist = sdf_rounded_rect(p, shape_size * 0.5, shape_radius);
        } else if (uShapeType == 2) {
            dist = sdf_circle(p, shape_radius);
        } else {
            dist = sdf_rect(p, shape_size * 0.5);
        }
    }
    
    float alpha = smooth_edge(dist, aa_width);
    FragColor = vec4(uColor.rgb, uColor.a * alpha);

    
    // NOTE: This shader renders ONE shape.
    // To do shadow, we need to render the shape TWICE: once for shadow, once for body.
    // The backend Loop handles this.
}
)";

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error: " << info << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, 512, nullptr, info);
        std::cerr << "Shader link error: " << info << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

bool OpenGLBackend::compile_sdf_shader() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, sdf_vert_src);
    if (!vs) return false;
    
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, sdf_frag_src);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }
    
    sdf_program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return sdf_program != 0;
}

bool OpenGLBackend::compile_text_shader() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, text_vert_src);
    if (!vs) return false;
    
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, text_frag_src);
    if (!fs) { glDeleteShader(vs); return false; }
    
    text_program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return text_program != 0;
}

void OpenGLBackend::setup_text_renderer() {
    // Dynamic VAO/VBO for text batches
    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    
    // Layout: 0 = vec4(pos.xy, uv.zw)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void OpenGLBackend::setup_quad() {
    // Fullscreen quad for instanced rendering
    float quad[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

bool OpenGLBackend::init(int w, int h, const char* title) {
    if (!glfwInit()) return false;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!window) { glfwTerminate(); return false; }
    
    glfwMakeContextCurrent(window);
    
    // Phase 6.1: Set this instance as user pointer for callbacks
    glfwSetWindowUserPointer(window, this);

    // Phase 8: Scroll Callback
    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoff, double yoff) {
        auto* b = static_cast<OpenGLBackend*>(glfwGetWindowUserPointer(w));
        if (b) b->scroll_accum += (float)yoff;
    });

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) return false;
    
    // Basic setup
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (!compile_sdf_shader()) {
        std::cerr << "Failed to compile SDF shader" << std::endl;
        return false;
    }
    
    // Phase 4: Text Shader
    if (!compile_text_shader()) {
        std::cerr << "Failed to compile Text shader" << std::endl;
        return false;
    }
    
    setup_quad();
    setup_text_renderer();
    
    // Phase 4: Init Glyph Cache GPU resources
    internal::GlyphCache::Get().init();
    
    logical_width = w;
    logical_height = h;
    
    return true;
}

void OpenGLBackend::shutdown() {
    if (sdf_program) {
        glDeleteProgram(sdf_program);
        sdf_program = 0;
    }
    if (quad_vao) {
        glDeleteVertexArrays(1, &quad_vao);
        quad_vao = 0;
    }
    if (quad_vbo) {
        glDeleteBuffers(1, &quad_vbo);
        quad_vbo = 0;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool OpenGLBackend::is_running() {
    return !glfwWindowShouldClose(window);
}

void OpenGLBackend::begin_frame(int w, int h) {
    glfwPollEvents();
    
    int dw, dh;
    glfwGetFramebufferSize(window, &dw, &dh);
    glViewport(0, 0, dw, dh);
    
    // Calculate device pixel ratio
    device_pixel_ratio = (float)dw / (float)w;
    logical_width = w;
    logical_height = h;
    
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::end_frame() {
    glfwSwapBuffers(window);
}

void OpenGLBackend::render(const internal::DrawList& draw_list) {
    glUseProgram(sdf_program);
    
    // Phase 5.1: Fix uniform names to match shader variables
    GLint loc_pos = glGetUniformLocation(sdf_program, "uShapePos");
    GLint loc_size = glGetUniformLocation(sdf_program, "uShapeSize");
    GLint loc_color = glGetUniformLocation(sdf_program, "uColor");
    GLint loc_radius = glGetUniformLocation(sdf_program, "uRadius");
    GLint loc_shape = glGetUniformLocation(sdf_program, "uShapeType");
    GLint loc_vp = glGetUniformLocation(sdf_program, "uViewportSize");
    GLint loc_dpr = glGetUniformLocation(sdf_program, "uDevicePixelRatio");

    // Set viewport-wide uniforms once (use LOGICAL size like text shader)
    if (loc_vp >= 0) glUniform2f(loc_vp, (float)logical_width, (float)logical_height);
    if (loc_dpr >= 0) glUniform1f(loc_dpr, device_pixel_ratio);

    // Bind quad VAO
    glBindVertexArray(quad_vao);
    
    // Phase 7: Transform Logic
    std::vector<float> tx_s, ty_s, sc_s;
    auto reset_stack = [&]() {
        tx_s.clear(); ty_s.clear(); sc_s.clear();
        tx_s.push_back(0); ty_s.push_back(0); sc_s.push_back(1);
    };
    auto push_t = [&](float tx, float ty, float s) {
        float cx = tx_s.back(), cy = ty_s.back(), cs = sc_s.back();
        // New = Local * Scale + Translate (Applied to world)
        // World = (Local * s + tx) * cs + cx
        // World = Local * (s * cs) + (tx * cs + cx)
        float nsc = s * cs;
        float ntx = tx * cs + cx;
        float nty = ty * cs + cy;
        tx_s.push_back(ntx); ty_s.push_back(nty); sc_s.push_back(nsc);
    };
    auto pop_t = [&]() {
        if(tx_s.size() > 1) { tx_s.pop_back(); ty_s.pop_back(); sc_s.pop_back(); }
    };
    auto apply_t = [&](float& x, float& y, float& w, float& h) {
        float cx = tx_s.back(), cy = ty_s.back(), cs = sc_s.back();
        x = x * cs + cx; y = y * cs + cy; w *= cs; h *= cs;
    };
    auto apply_s = [&](float& v) { v *= sc_s.back(); };
    auto apply_p = [&](float& x, float& y) {
        float cx = tx_s.back(), cy = ty_s.back(), cs = sc_s.back();
        x = x * cs + cx; y = y * cs + cy;
    };

    // Phase 8: Scissor Stack
    struct ClipRect { float x, y, w, h; };
    std::vector<ClipRect> clip_stack;
    auto apply_scissor = [&]() {
        if (clip_stack.empty()) {
            glDisable(GL_SCISSOR_TEST);
        } else {
            glEnable(GL_SCISSOR_TEST);
            const auto& c = clip_stack.back();
            float dpr = device_pixel_ratio;
            int sx = (int)(c.x * dpr);
            int sy = (int)((logical_height - (c.y + c.h)) * dpr);
            int sw = (int)(c.w * dpr);
            int sh = (int)(c.h * dpr);
            glScissor(sx, sy, sw, sh);
        }
    };
    auto push_clip = [&](float x, float y, float w, float h) {
        float nx = x, ny = y, nw = w, nh = h;
        apply_t(nx, ny, nw, nh);
        if (clip_stack.empty()) {
            clip_stack.push_back({nx, ny, nw, nh});
        } else {
            const auto& prev = clip_stack.back();
            float ix = std::max(nx, prev.x);
            float iy = std::max(ny, prev.y);
            float iw = std::min(nx + nw, prev.x + prev.w) - ix;
            float ih = std::min(ny + nh, prev.y + prev.h) - iy;
            clip_stack.push_back({ix, iy, std::max(0.0f, iw), std::max(0.0f, ih)});
        }
        apply_scissor();
    };
    auto pop_clip = [&]() {
        if (!clip_stack.empty()) clip_stack.pop_back();
        apply_scissor();
    };
    auto reset_clip = [&]() {
        clip_stack.clear();
        glDisable(GL_SCISSOR_TEST);
    };

    reset_stack();
    reset_clip();

    // --- Pass 1: Ambient Shadows (Broad, Non-directional) ---
    for (const auto& cmd : draw_list.commands) {
        if (cmd.type == internal::DrawCmdType::PushTransform) {
            push_t(cmd.transform.tx, cmd.transform.ty, cmd.transform.scale);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopTransform) {
            pop_t();
            continue;
        } else if (cmd.type == internal::DrawCmdType::PushClip) {
            push_clip(cmd.clip.x, cmd.clip.y, cmd.clip.w, cmd.clip.h);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopClip) {
            pop_clip();
            continue;
        }

        float elevation = 0;
        float pos_x=0, pos_y=0, size_x=0, size_y=0, radius=0;
        int shape_type = 0;
        
        // Extract parameters based on type
        if (cmd.type == internal::DrawCmdType::Rectangle) {
            elevation = cmd.rectangle.elevation;
            pos_x = cmd.rectangle.pos_x; pos_y = cmd.rectangle.pos_y;
            size_x = cmd.rectangle.size_x; size_y = cmd.rectangle.size_y;
            radius = 0; shape_type = 0;
        } else if (cmd.type == internal::DrawCmdType::RoundedRect) {
            elevation = cmd.rounded_rect.elevation;
            pos_x = cmd.rounded_rect.pos_x; pos_y = cmd.rounded_rect.pos_y;
            size_x = cmd.rounded_rect.size_x; size_y = cmd.rounded_rect.size_y;
            radius = cmd.rounded_rect.radius; shape_type = 1;
        } else if (cmd.type == internal::DrawCmdType::Circle) {
            elevation = cmd.circle.elevation;
            pos_x = cmd.circle.center_x - cmd.circle.r; 
            pos_y = cmd.circle.center_y - cmd.circle.r;
            size_x = cmd.circle.r * 2; size_y = cmd.circle.r * 2;
            radius = cmd.circle.r; shape_type = 2;
        } else { continue; } // Skip lines/others for shadow
        
        apply_t(pos_x, pos_y, size_x, size_y);
        apply_s(radius);
        apply_s(elevation);

        if (elevation > 0) {
            // Ambient Shadow: Broad, centered, subtle but visible
            float expand = elevation * 0.8f; 
            float shadow_alpha = std::min(0.025f * elevation, 0.15f); // Balanced
            
            // Expand rect and radius together
            float final_radius = radius + expand;
            
            if (loc_pos >= 0) glUniform2f(loc_pos, pos_x - expand, pos_y - expand);
            if (loc_size >= 0) glUniform2f(loc_size, size_x + expand*2, size_y + expand*2);
            // Pure Black Shadow
            if (loc_color >= 0) glUniform4f(loc_color, 0.0f, 0.0f, 0.0f, shadow_alpha);
            if (loc_radius >= 0) glUniform1f(loc_radius, final_radius);
            if (loc_shape >= 0) glUniform1i(loc_shape, shape_type);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    reset_stack();
    reset_clip(); 

    // --- Pass 2: Key Shadows (Directional, Offset Y) ---
    for (const auto& cmd : draw_list.commands) {
        if (cmd.type == internal::DrawCmdType::PushTransform) {
            push_t(cmd.transform.tx, cmd.transform.ty, cmd.transform.scale);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopTransform) {
            pop_t();
            continue;
        } else if (cmd.type == internal::DrawCmdType::PushClip) {
            push_clip(cmd.clip.x, cmd.clip.y, cmd.clip.w, cmd.clip.h);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopClip) {
            pop_clip();
            continue;
        }

        float elevation = 0;
        float pos_x=0, pos_y=0, size_x=0, size_y=0, radius=0;
        int shape_type = 0;
        
        // Extract parameters based on type
        if (cmd.type == internal::DrawCmdType::Rectangle) {
            elevation = cmd.rectangle.elevation;
            pos_x = cmd.rectangle.pos_x; pos_y = cmd.rectangle.pos_y;
            size_x = cmd.rectangle.size_x; size_y = cmd.rectangle.size_y;
            radius = 0; shape_type = 0;
        } else if (cmd.type == internal::DrawCmdType::RoundedRect) {
            elevation = cmd.rounded_rect.elevation;
            pos_x = cmd.rounded_rect.pos_x; pos_y = cmd.rounded_rect.pos_y;
            size_x = cmd.rounded_rect.size_x; size_y = cmd.rounded_rect.size_y;
            radius = cmd.rounded_rect.radius; shape_type = 1;
        } else if (cmd.type == internal::DrawCmdType::Circle) {
            elevation = cmd.circle.elevation;
            pos_x = cmd.circle.center_x - cmd.circle.r; 
            pos_y = cmd.circle.center_y - cmd.circle.r;
            size_x = cmd.circle.r * 2; size_y = cmd.circle.r * 2;
            radius = cmd.circle.r; shape_type = 2;
        } else { continue; } 
        
        apply_t(pos_x, pos_y, size_x, size_y);
        apply_s(radius);
        apply_s(elevation); 

        if (elevation > 0) {
            // Key Shadow: Offset downwards, balanced
            float expand = elevation * 0.15f; 
            float offset_y = elevation * 0.4f;
            float shadow_alpha = std::min(0.05f * elevation, 0.25f); // Balanced key light
            
            float final_radius = radius + expand;

            if (loc_pos >= 0) glUniform2f(loc_pos, pos_x - expand, pos_y - expand + offset_y);
            if (loc_size >= 0) glUniform2f(loc_size, size_x + expand*2, size_y + expand*2);
            // Pure Black Shadow
            if (loc_color >= 0) glUniform4f(loc_color, 0.0f, 0.0f, 0.0f, shadow_alpha);
            if (loc_radius >= 0) glUniform1f(loc_radius, final_radius);
            if (loc_shape >= 0) glUniform1i(loc_shape, shape_type);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    reset_stack();
    reset_clip();

    // --- Pass 3: Body (Actual Shapes) ---
    for (const auto& cmd : draw_list.commands) {
        if (cmd.type == internal::DrawCmdType::PushTransform) {
            push_t(cmd.transform.tx, cmd.transform.ty, cmd.transform.scale);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopTransform) {
            pop_t();
            continue;
        } else if (cmd.type == internal::DrawCmdType::PushClip) {
            push_clip(cmd.clip.x, cmd.clip.y, cmd.clip.w, cmd.clip.h);
            continue;
        } else if (cmd.type == internal::DrawCmdType::PopClip) {
            pop_clip();
            continue;
        } else if (cmd.type == internal::DrawCmdType::Bezier) {
             float bx0 = cmd.bezier.p0_x, by0 = cmd.bezier.p0_y;
             float bx1 = cmd.bezier.p1_x, by1 = cmd.bezier.p1_y;
             float bx2 = cmd.bezier.p2_x, by2 = cmd.bezier.p2_y;
             float bx3 = cmd.bezier.p3_x, by3 = cmd.bezier.p3_y;
             
             apply_p(bx0, by0); apply_p(bx1, by1); apply_p(bx2, by2); apply_p(bx3, by3);
             
             float thickness = cmd.bezier.thickness;
             apply_s(thickness);
             if (loc_color >= 0) glUniform4f(loc_color, cmd.bezier.color_r, cmd.bezier.color_g, cmd.bezier.color_b, cmd.bezier.color_a);
             if (loc_shape >= 0) glUniform1i(loc_shape, 3); // Line (using SDF capsule/line)
             if (loc_radius >= 0) glUniform1f(loc_radius, thickness * 0.5f);

             float px = bx0, py = by0;
             const int segs = 20;
             for(int i=1; i<=segs; ++i) {
                 float t = i/(float)segs;
                 float u = 1.0f-t;
                 float nx = u*u*u*bx0 + 3*u*u*t*bx1 + 3*u*t*t*bx2 + t*t*t*bx3;
                 float ny = u*u*u*by0 + 3*u*u*t*by1 + 3*u*t*t*by2 + t*t*t*by3;
                 
                 glUniform2f(loc_pos, px, py);
                 glUniform2f(loc_size, nx, ny);
                 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                 px = nx; py = ny;
             }
             continue;
        }

        if (cmd.type == internal::DrawCmdType::Rectangle) {
             float pos_x = cmd.rectangle.pos_x, pos_y = cmd.rectangle.pos_y;
             float size_x = cmd.rectangle.size_x, size_y = cmd.rectangle.size_y;
             apply_t(pos_x, pos_y, size_x, size_y);
             if (loc_pos >= 0) glUniform2f(loc_pos, pos_x, pos_y);
             if (loc_size >= 0) glUniform2f(loc_size, size_x, size_y);
             if (loc_color >= 0) glUniform4f(loc_color, cmd.rectangle.color_r, cmd.rectangle.color_g, cmd.rectangle.color_b, cmd.rectangle.color_a);
             if (loc_radius >= 0) glUniform1f(loc_radius, 0.0f);
             if (loc_shape >= 0) glUniform1i(loc_shape, 0); 
        } else if (cmd.type == internal::DrawCmdType::RoundedRect) {
             float pos_x = cmd.rounded_rect.pos_x, pos_y = cmd.rounded_rect.pos_y;
             float size_x = cmd.rounded_rect.size_x, size_y = cmd.rounded_rect.size_y;
             float radius = cmd.rounded_rect.radius;
             apply_t(pos_x, pos_y, size_x, size_y);
             apply_s(radius);
             if (loc_pos >= 0) glUniform2f(loc_pos, pos_x, pos_y);
             if (loc_size >= 0) glUniform2f(loc_size, size_x, size_y);
             if (loc_color >= 0) glUniform4f(loc_color, cmd.rounded_rect.color_r, cmd.rounded_rect.color_g, cmd.rounded_rect.color_b, cmd.rounded_rect.color_a);
             if (loc_radius >= 0) glUniform1f(loc_radius, radius);
             if (loc_shape >= 0) glUniform1i(loc_shape, 1); 
        } else if (cmd.type == internal::DrawCmdType::Circle) {
             float left = cmd.circle.center_x - cmd.circle.r;
             float top = cmd.circle.center_y - cmd.circle.r;
             float size = cmd.circle.r * 2.0f;
             apply_t(left, top, size, size); 
             float radius = cmd.circle.r; apply_s(radius);
             if (loc_pos >= 0) glUniform2f(loc_pos, left, top);
             if (loc_size >= 0) glUniform2f(loc_size, size, size);
             if (loc_color >= 0) glUniform4f(loc_color, cmd.circle.color_r, cmd.circle.color_g, cmd.circle.color_b, cmd.circle.color_a);
             if (loc_radius >= 0) glUniform1f(loc_radius, radius);
             if (loc_shape >= 0) glUniform1i(loc_shape, 2); 
        } else if (cmd.type == internal::DrawCmdType::Line) {
             float p0x = cmd.line.p0_x, p0y = cmd.line.p0_y;
             float p1x = cmd.line.p1_x, p1y = cmd.line.p1_y;
             float th = cmd.line.thickness;
             apply_p(p0x, p0y); apply_p(p1x, p1y); apply_s(th);
             if (loc_pos >= 0) glUniform2f(loc_pos, p0x, p0y);
             if (loc_size >= 0) glUniform2f(loc_size, p1x, p1y);
             if (loc_color >= 0) glUniform4f(loc_color, cmd.line.color_r, cmd.line.color_g, cmd.line.color_b, cmd.line.color_a);
             if (loc_radius >= 0) glUniform1f(loc_radius, th * 0.5f);
             if (loc_shape >= 0) glUniform1i(loc_shape, 3); 
        } else {
            continue; 
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    

    
    glBindVertexArray(0);
    glUseProgram(0);
    
    // --- Phase 4: Text Pass ---
    if (!draw_list.text_runs.empty() && text_program) {
        glUseProgram(text_program);
        
        // Bind Atlas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, internal::GlyphCache::Get().get_texture_id());
        internal::GlyphCache::Get().update_texture();
        
        GLint loc_atlas = glGetUniformLocation(text_program, "uAtlas");
        if (loc_atlas >= 0) glUniform1i(loc_atlas, 0);
        
        GLint loc_vp = glGetUniformLocation(text_program, "uViewportSize");
        if (loc_vp >= 0) glUniform2f(loc_vp, (float)logical_width, (float)logical_height);
        
        GLint loc_color = glGetUniformLocation(text_program, "uColor");
        
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        
        reset_stack();
        reset_clip();
        
        for (const auto& cmd : draw_list.commands) {
            if (cmd.type == internal::DrawCmdType::PushTransform) {
                push_t(cmd.transform.tx, cmd.transform.ty, cmd.transform.scale);
                continue;
            } else if (cmd.type == internal::DrawCmdType::PopTransform) {
                pop_t();
                continue;
            } else if (cmd.type == internal::DrawCmdType::PushClip) {
                push_clip(cmd.clip.x, cmd.clip.y, cmd.clip.w, cmd.clip.h);
                continue;
            } else if (cmd.type == internal::DrawCmdType::PopClip) {
                pop_clip();
                continue;
            }
            
            if (cmd.type == internal::DrawCmdType::Text) {
                const auto& run = draw_list.text_runs[cmd.text.run_index];
                if (run.empty()) continue;
                
                glUniform4f(loc_color, cmd.text.color_r, cmd.text.color_g, cmd.text.color_b, cmd.text.color_a);
                
                // Build dynamic buffer
                static std::vector<float> batch;
                batch.clear();
                batch.reserve(run.quads.size() * 6 * 4);
                
                for (const auto& q : run.quads) {
                    float x0 = q.x0, y0 = q.y0, x1 = q.x1, y1 = q.y1;
                    float tx0 = x0, ty0 = y0, tx1 = x1, ty1 = y1;
                    // Apply Transform to text quads
                    apply_p(tx0, ty0);
                    apply_p(tx1, ty1);
                    // Note: This applies uniform scale and translation to quads.
                    // This is sufficient for Pan/Zoom.
                    
                    // T1
                    batch.push_back(tx0); batch.push_back(ty0); batch.push_back(q.u0); batch.push_back(q.v0);
                    batch.push_back(tx0); batch.push_back(ty1); batch.push_back(q.u0); batch.push_back(q.v1);
                    batch.push_back(tx1); batch.push_back(ty0); batch.push_back(q.u1); batch.push_back(q.v0);
                    // T2
                    batch.push_back(tx1); batch.push_back(ty0); batch.push_back(q.u1); batch.push_back(q.v0);
                    batch.push_back(tx0); batch.push_back(ty1); batch.push_back(q.u0); batch.push_back(q.v1);
                    batch.push_back(tx1); batch.push_back(ty1); batch.push_back(q.u1); batch.push_back(q.v1);
                }
                
                glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(float), batch.data(), GL_STREAM_DRAW);
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)run.quads.size() * 6);
            }
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// Phase 6.1
void OpenGLBackend::get_mouse_state(float& x, float& y, bool& down, float& wheel) {
    if (!window) { x=0; y=0; down=false; wheel=0; return; }
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    x = (float)mx;
    y = (float)my;
    
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    down = (state == GLFW_PRESS);
    
    wheel = scroll_accum;
    scroll_accum = 0; // Reset after reading
}

// Phase 5.2: Screenshot capture implementation
bool OpenGLBackend::capture_screenshot(const char* filename) {
    // Get physical framebuffer size
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    
    // Allocate buffer (RGBA)
    std::vector<uint8_t> pixels(fb_width * fb_height * 4);
    
    // Ensure drawing is finished and read from back buffer
    glFinish();
    glReadBuffer(GL_BACK);
    
    // Read pixels from framebuffer
    glReadPixels(0, 0, fb_width, fb_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Debug: Check center pixel
    int cx = fb_width / 2;
    int cy = fb_height / 2;
    int idx = (cy * fb_width + cx) * 4;
    std::cout << "[Screenshot] Center Pixel: R=" << (int)pixels[idx] 
              << " G=" << (int)pixels[idx+1] 
              << " B=" << (int)pixels[idx+2] 
              << " A=" << (int)pixels[idx+3] << std::endl;
              
    // OpenGL reads from bottom-left, PNG expects top-left
    // Flip Y
    std::vector<uint8_t> flipped(fb_width * fb_height * 4);
    for (int y = 0; y < fb_height; ++y) {
        memcpy(
            flipped.data() + y * fb_width * 4,
            pixels.data() + (fb_height - 1 - y) * fb_width * 4,
            fb_width * 4
        );
    }
    
    // Write PNG
    int result = stbi_write_png(filename, fb_width, fb_height, 4, flipped.data(), fb_width * 4);
    
    if (result) {
        std::cout << "[Screenshot] Saved to: " << filename << " (" << fb_width << "x" << fb_height << ")" << std::endl;
    } else {
        std::cerr << "[Screenshot] Failed to save: " << filename << std::endl;
    }
    
    return result != 0;
}

} // namespace fanta
