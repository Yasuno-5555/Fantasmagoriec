#define NOMINMAX
#include <windows.h>

// Undefine Windows macros to prevent pollution
#ifdef DrawText
#undef DrawText
#endif
#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#ifndef FANTA_ENABLE_TEXT
#define FANTA_ENABLE_TEXT // Stable vtable requires text support always linked
#endif

#include <glad/gl.h>
#include "opengl.hpp"
#include "core/shader.hpp" // Moved from below and path adjusted

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

#include "../core/types.hpp"
#include "../core/paint_list.hpp"
#include "../core/render_context.hpp"
#include "../core/font_manager.hpp"
#include "../core/shader.hpp"

// No fanta types should be touched by macros
#undef list
#undef text
#undef rect
#undef circle
#undef line
#undef TypeId 

#include <iostream>

namespace fanta {
    // Manual definitions removed. GLAD handles everything.


// ============================================================================
// Internal GL Resource Wrappers (RAII - Rule of 5)
// ============================================================================

class GLBuffer {
    GLuint id_ = 0;
    GLenum type_ = 0;
public:
    GLBuffer(GLenum t) : type_(t) { glGenBuffers(1, &id_); }
    ~GLBuffer() { if(id_) glDeleteBuffers(1, &id_); }
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    GLBuffer(GLBuffer&& other) noexcept : id_(other.id_), type_(other.type_) { other.id_ = 0; }
    GLBuffer& operator=(GLBuffer&& other) noexcept { 
        if(this != &other) { if(id_) glDeleteBuffers(1, &id_); id_ = other.id_; type_ = other.type_; other.id_ = 0; } 
        return *this; 
    }
    void bind() const { glBindBuffer(type_, id_); }
};

class GLVertexArray {
    GLuint id_ = 0;
public:
    GLVertexArray() { glGenVertexArrays(1, &id_); }
    ~GLVertexArray() { if(id_) glDeleteVertexArrays(1, &id_); }
    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;
    GLVertexArray(GLVertexArray&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    GLVertexArray& operator=(GLVertexArray&& other) noexcept { 
        if(this != &other) { if(id_) glDeleteVertexArrays(1, &id_); id_ = other.id_; other.id_ = 0; } 
        return *this; 
    }
    void bind() const { glBindVertexArray(id_); }
};

class GLTexture {
    GLuint id_ = 0;
public:
    GLTexture() { glGenTextures(1, &id_); }
    ~GLTexture() { if(id_) glDeleteTextures(1, &id_); }
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    GLTexture(GLTexture&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    GLTexture& operator=(GLTexture&& other) noexcept { 
        if(this != &other) { if(id_) glDeleteTextures(1, &id_); id_ = other.id_; other.id_ = 0; } 
        return *this; 
    }
    void bind() const { glBindTexture(GL_TEXTURE_2D, id_); }
    GLuint get() const { return id_; }
};

// ============================================================================
// Internal Typography Backend
// ============================================================================

struct InternalTextVertex { Vec2 pos; Vec2 uv; };
struct GLGlyph { FantaGlyph glyph_metrics; int page_index = 0; };

class GLTextBackend {
public:
    std::vector<std::unique_ptr<GLTexture>> atlas_textures;
    GlyphAtlas atlas;
    // Using std::list because std::deque iterators are invalidated by push_front.
    // std::list iterators remain valid (except the erased one).
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> lru_map;
    std::list<uint64_t> lru_ids;
    std::unordered_map<uint64_t, GLGlyph> cache;
    TypographyStats stats;
    static constexpr std::size_t MAX_CACHE_SIZE = 4096;

    void add_page() {
        auto tex = std::make_unique<GLTexture>(); tex->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.width(), atlas.height(), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        atlas_textures.push_back(std::move(tex)); stats.atlas_pages = (int)atlas_textures.size();
    }

    const GLGlyph* get_or_load_glyph(uint32_t font_id, uint32_t cp, float dpr) {
        uint64_t key = ((uint64_t)font_id << 32) | cp; auto it = cache.find(key);
        if (it != cache.end()) { 
            // Move to front (splice is O(1) and keeps iterators valid)
            lru_ids.splice(lru_ids.begin(), lru_ids, lru_map[key]);
            stats.lru_hits++; return &it->second; 
        }
        stats.lru_misses++;
        
        auto gd = FontManager::instance().get_glyph(font_id, cp, 64.0f, dpr);
        if (!gd.glyph_metrics.valid) return nullptr;
        
        if (cache.size() >= MAX_CACHE_SIZE) { 
            uint64_t ek = lru_ids.back(); 
            lru_ids.pop_back(); 
            lru_map.erase(ek); 
            cache.erase(ek); 
        }
        
        AtlasRect r; int pw = (int)(gd.glyph_metrics.size.x * dpr), ph = (int)(gd.glyph_metrics.size.y * dpr);
        if (!atlas.allocate(pw, ph, r)) { add_page(); atlas.clear(); if (!atlas.allocate(pw, ph, r)) return nullptr; }
        
        atlas_textures.back()->bind(); glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, GL_RED, GL_UNSIGNED_BYTE, gd.pixels.data());
        
        GLGlyph& g = cache[key]; g.glyph_metrics = gd.glyph_metrics;
        g.glyph_metrics.uv_min = {(float)r.x / atlas.width(), (float)r.y / atlas.height()};
        g.glyph_metrics.uv_max = {(float)(r.x + r.w) / atlas.width(), (float)(r.y + r.h) / atlas.height()};
        g.page_index = (int)atlas_textures.size() - 1;
        
        lru_ids.push_front(key); 
        lru_map[key] = lru_ids.begin();
        
        stats.total_glyphs = (int)cache.size(); stats.atlas_utilization = atlas.utilization(); return &g;
    }
};

// ============================================================================
// RenderContextImplementation (Pimpl details)
// ============================================================================

class RenderContextImplementation : public RenderContext {
public:
    GLVertexArray vao_fullscreen;
    GLTextBackend text_backend;
    GLVertexArray text_vao;
    std::unique_ptr<GLBuffer> text_vbos[3];
    int current_vbo_idx = 0;

    ShaderProgram* shader_rect = nullptr;
    ShaderProgram* shader_circle = nullptr;
    ShaderProgram* shader_line = nullptr;
    ShaderProgram* shader_text = nullptr;

    bool init_resources() {
        std::cout << "RenderContextImplementation::init_resources start" << std::endl;
        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
             std::cerr << "Failed to initialize GLAD" << std::endl;
             return false;
        }
        std::cout << "GLAD Initialized" << std::endl;

        // Load Shaders (Development Mode: Load from files relative to binary)
        // Only loading correct files to avoid build errors if files missing
        auto& sm = ShaderManager::instance();
        std::cout << "Loading shaders..." << std::endl;
        sm.load_file("sdf_rect",   "../../shaders/sdf_rect.vert", "../../shaders/sdf_rect.frag");
        sm.load_file("sdf_circle", "../../shaders/sdf_rect.vert", "../../shaders/sdf_circle.frag");
        sm.load_file("sdf_line",   "../../shaders/sdf_rect.vert", "../../shaders/sdf_line.frag");
        sm.load_file("sdf_text",   "../../shaders/sdf_text.vert", "../../shaders/sdf_text.frag");
        std::cout << "Shaders loaded request sent" << std::endl;

        if (sm.has_errors()) {
            std::cerr << "Shader compilation failed!" << std::endl;
            for (const auto& [name, err] : sm.errors()) {
                std::cerr << name << ": " << err.message << std::endl;
            }
            return false;
        }

        text_backend.add_page();
        for(int i=0; i<3; ++i) { 
            text_vbos[i] = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER); text_vbos[i]->bind();
            glBufferData(GL_ARRAY_BUFFER, 4096 * 6 * sizeof(InternalTextVertex), nullptr, GL_DYNAMIC_DRAW);
        }
        // IMPORTANT: Bind VBO[0] before setting up VAO vertex attributes
        // The currently bound VBO at glVertexAttribPointer time is captured by the VAO
        text_vao.bind();
        text_vbos[0]->bind();  // Must be VBO[0] because current_vbo_idx = 0
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(InternalTextVertex), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(InternalTextVertex), (void*)(sizeof(Vec2)));
        return true;
    }

    void draw_rect(const cmd::RectCmd& r) override {
        if (!shader_rect) return; shader_rect->use();
        shader_rect->set_vec2("u_resolution", (float)viewport_width, (float)viewport_height);
        shader_rect->set_float("u_dpr", device_pixel_ratio);
        shader_rect->set_vec4("u_rect", r.bounds.x, r.bounds.y, r.bounds.w, r.bounds.h);
        auto rad = r.radius.as_vec4(); shader_rect->set_vec4("u_radius", rad.x, rad.y, rad.z, rad.w);
        shader_rect->set_vec4("u_fill_color", r.fill.r, r.fill.g, r.fill.b, r.fill.a);
        shader_rect->set_vec4("u_border_color", r.border_color.r, r.border_color.g, r.border_color.b, r.border_color.a);
        shader_rect->set_float("u_border_width", r.border_width);
        shader_rect->set_vec4("u_shadow_ambient_color", r.shadow.ambient.color.r, r.shadow.ambient.color.g, r.shadow.ambient.color.b, r.shadow.ambient.color.a);
        shader_rect->set_float("u_shadow_ambient_blur", r.shadow.ambient.blur);
        shader_rect->set_float("u_shadow_ambient_spread", r.shadow.ambient.spread);
        shader_rect->set_vec2("u_shadow_ambient_offset", r.shadow.ambient.offset.x, r.shadow.ambient.offset.y);
        shader_rect->set_vec4("u_shadow_key_color", r.shadow.key.color.r, r.shadow.key.color.g, r.shadow.key.color.b, r.shadow.key.color.a);
        shader_rect->set_float("u_shadow_key_blur", r.shadow.key.blur);
        shader_rect->set_float("u_shadow_key_spread", r.shadow.key.spread);
        shader_rect->set_vec2("u_shadow_key_offset", r.shadow.key.offset.x, r.shadow.key.offset.y);
        vao_fullscreen.bind(); glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void draw_circle(const cmd::CircleCmd& c) override {
        if (!shader_circle) return; shader_circle->use();
        shader_circle->set_vec2("u_resolution", (float)viewport_width, (float)viewport_height);
        shader_circle->set_float("u_dpr", device_pixel_ratio);
        shader_circle->set_vec2("u_center", c.center.x, c.center.y);
        shader_circle->set_float("u_radius", c.radius);
        shader_circle->set_vec4("u_fill_color", c.fill.r, c.fill.g, c.fill.b, c.fill.a);
        shader_circle->set_vec4("u_shadow_ambient_color", c.shadow.ambient.color.r, c.shadow.ambient.color.g, c.shadow.ambient.color.b, c.shadow.ambient.color.a);
        shader_circle->set_float("u_shadow_ambient_blur", c.shadow.ambient.blur);
        shader_circle->set_float("u_shadow_ambient_spread", c.shadow.ambient.spread);
        shader_circle->set_vec2("u_shadow_ambient_offset", c.shadow.ambient.offset.x, c.shadow.ambient.offset.y);
        shader_circle->set_vec4("u_shadow_key_color", c.shadow.key.color.r, c.shadow.key.color.g, c.shadow.key.color.b, c.shadow.key.color.a);
        shader_circle->set_float("u_shadow_key_blur", c.shadow.key.blur);
        shader_circle->set_float("u_shadow_key_spread", c.shadow.key.spread);
        shader_circle->set_vec2("u_shadow_key_offset", c.shadow.key.offset.x, c.shadow.key.offset.y);
        vao_fullscreen.bind(); glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void draw_line(const cmd::LineCmd& l) override {
        if (!shader_line) return; shader_line->use();
        shader_line->set_vec2("u_resolution", (float)viewport_width, (float)viewport_height);
        shader_line->set_float("u_dpr", device_pixel_ratio);
        shader_line->set_vec2("u_from", l.from.x, l.from.y);
        shader_line->set_vec2("u_to", l.to.x, l.to.y);
        shader_line->set_float("u_thickness", l.thickness);
        shader_line->set_vec4("u_color", l.color.r, l.color.g, l.color.b, l.color.a);
        shader_line->set_int("u_cap_style", 1);
        vao_fullscreen.bind(); glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void draw_text(const cmd::TextCmd& t) override {
        if (!shader_text || t.content.empty()) return;
        auto shape = FontManager::instance().shape_simple(t.font_id, t.content, t.font_size, device_pixel_ratio);
        if (shape.glyphs.empty()) return;
        shader_text->use(); shader_text->set_vec2("u_resolution", (float)viewport_width, (float)viewport_height);
        shader_text->set_float("u_dpr", device_pixel_ratio); shader_text->set_vec4("u_color", t.color.r, t.color.g, t.color.b, t.color.a);
        shader_text->set_int("u_Atlas", 0); glActiveTexture(GL_TEXTURE0);
        static std::vector<InternalTextVertex> vertices; vertices.clear();
        float bx = t.pos.x, by = t.pos.y, s = t.font_size / 64.0f; int last_page = -1;
        auto flush_batch = [&]() {
            if (vertices.empty()) return; 
            // Use buffer orphaning to prevent GPU stalls and flickering
            // This tells the driver we're done with old data and it can allocate new storage
            text_vbos[current_vbo_idx]->bind(); 
            glBufferData(GL_ARRAY_BUFFER, 4096 * 6 * sizeof(InternalTextVertex), nullptr, GL_DYNAMIC_DRAW); // Orphan
            glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(vertices.size() * sizeof(InternalTextVertex)), vertices.data());
            text_vao.bind(); glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size()); vertices.clear();
        };
        for (const auto& sg : shape.glyphs) {
            const GLGlyph* gl = text_backend.get_or_load_glyph(t.font_id, sg.codepoint, device_pixel_ratio);
            if (!gl) continue;
            if (gl->page_index != last_page && last_page != -1) flush_batch();
            if (gl->page_index != last_page) { text_backend.atlas_textures[gl->page_index]->bind(); last_page = gl->page_index; }
            float gx = bx + sg.pos.x + gl->glyph_metrics.bearing.x * s, gy = by + sg.pos.y - gl->glyph_metrics.bearing.y * s, gw = gl->glyph_metrics.size.x * s, gh = gl->glyph_metrics.size.y * s;
            Vec2 v0 = {gx, gy}, v1 = {gx + gw, gy}, v2 = {gx, gy + gh}, v3 = {gx + gw, gy + gh};
            Vec2 u0 = gl->glyph_metrics.uv_min, u1 = {gl->glyph_metrics.uv_max.x, gl->glyph_metrics.uv_min.y}, u2 = {gl->glyph_metrics.uv_min.x, gl->glyph_metrics.uv_max.y}, u3 = gl->glyph_metrics.uv_max;
            vertices.push_back({v0, u0}); vertices.push_back({v1, u1}); vertices.push_back({v2, u2});
            vertices.push_back({v1, u1}); vertices.push_back({v3, u3}); vertices.push_back({v2, u2});
        }
        flush_batch();
    }
};

// ============================================================================
// RenderBackend Facade Implementation
// ============================================================================

RenderBackend::RenderBackend() {
    // Initialize GLAD before creating implementation, because implementation members (GLVertexArray)
    // call OpenGL functions in their constructors.
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "[RenderBackend] Failed to initialize GLAD" << std::endl;
        // Proceeding will likely crash, but we can't return false from constructor.
        // In a real engine, we'd throw or use a factory method.
    }
    impl_ = std::make_unique<RenderContextImplementation>();
}
RenderBackend::~RenderBackend() {}
bool RenderBackend::init() { 
    if (!impl_->init_resources()) return false;
    if (!FontManager::instance().init()) {
        std::cerr << "Failed to init FontManager" << std::endl;
        return false;
    }
    return true;
}
void RenderBackend::shutdown() { 
    FontManager::instance().shutdown();
    impl_.reset(); 
}
void RenderBackend::begin_frame(int w, int h, float dpr) {
    if (!impl_) return;
    impl_->viewport_width = w; impl_->viewport_height = h; impl_->device_pixel_ratio = dpr;
    impl_->shader_rect = ShaderManager::instance().get("sdf_rect");
    impl_->shader_circle = ShaderManager::instance().get("sdf_circle");
    impl_->shader_line = ShaderManager::instance().get("sdf_line");
    impl_->shader_text = ShaderManager::instance().get("sdf_text");
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glViewport(0, 0, w, h);
}
void RenderBackend::end_frame() {}
void RenderBackend::draw_rect(const cmd::RectCmd& r) { if(impl_) impl_->draw_rect(r); }
void RenderBackend::draw_circle(const cmd::CircleCmd& c) { if(impl_) impl_->draw_circle(c); }
void RenderBackend::draw_line(const cmd::LineCmd& l) { if(impl_) impl_->draw_line(l); }
void RenderBackend::draw_text(const cmd::TextCmd& t) { if(impl_) impl_->draw_text(t); }
TypographyStats RenderBackend::get_stats() const { return impl_ ? impl_->text_backend.stats : TypographyStats(); }
void RenderBackend::execute(const PaintList& list) { if (impl_) list.execute_all(*impl_); }
void load_gl_functions() {}

} // namespace fanta
