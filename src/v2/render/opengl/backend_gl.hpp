// Fantasmagorie v2 - OpenGL ES 3.0 Backend
// Compatible with Desktop OpenGL 3.3+ and GLES 3.0
#pragma once

#include "../backend.hpp"

namespace fanta {

class OpenGLBackend : public RenderBackend {
public:
    OpenGLBackend() = default;
    ~OpenGLBackend() override { shutdown(); }
    
    bool init() override;
    void shutdown() override;
    
    void begin_frame(int width, int height) override;
    void end_frame() override;
    
    void render(const DrawList& list, Font* font = nullptr) override;
    
    void* create_texture(int w, int h, const void* pixels, int channels = 4) override;
    void update_texture(void* tex, int x, int y, int w, int h, const void* pixels) override;
    void destroy_texture(void* tex) override;
    
    bool supports_shadows() const override { return true; }
    
private:
    unsigned int shader_program_ = 0;
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int white_texture_ = 0;
    
    int width_ = 0;
    int height_ = 0;
    bool initialized_ = false;
    
    void compile_shaders();
    void setup_buffers();
    void render_layer(const std::vector<DrawCmd>& cmds, Font* font);
};

} // namespace fanta
