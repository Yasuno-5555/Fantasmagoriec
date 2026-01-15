#define NOMINMAX
#include <cstdio>
#include <cstdint>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstring>

#include "backend_factory.hpp"
#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"
#include "text/glyph_cache.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "stb_image_write.h"

namespace fanta {

class OpenGLTexture : public internal::GpuTexture {
public:
    OpenGLTexture(int w, int h, internal::TextureFormat fmt) : w_(w), h_(h), format_(fmt) {
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        GLint internal_fmt = GL_RGBA8;
        GLenum external_fmt = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        if (fmt == internal::TextureFormat::R8) { internal_fmt = GL_R8; external_fmt = GL_RED; }
        else if (fmt == internal::TextureFormat::RGBA16F) { internal_fmt = GL_RGBA16F; type = GL_HALF_FLOAT; }
        glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, w_, h_, 0, external_fmt, type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    ~OpenGLTexture() { if (id_) glDeleteTextures(1, &id_); }
    void upload(const void* data, int w, int h) override {
        glBindTexture(GL_TEXTURE_2D, id_);
        GLenum ext = (format_ == internal::TextureFormat::R8) ? GL_RED : GL_RGBA;
        GLenum type = (format_ == internal::TextureFormat::RGBA16F) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, ext, type, data);
    }
    uint64_t get_native_handle() const override { return id_; }
    int width() const override { return w_; }
    int height() const override { return h_; }
private:
    GLuint id_ = 0; int w_, h_; internal::TextureFormat format_;
};

class OpenGLBackend : public Backend {
    GLFWwindow* window = nullptr;
    float dpr = 1.0f;
    int lw = 0, lh = 0;
    GLuint sdf_prog, text_prog;
    GLint loc_pos, loc_size, loc_color, loc_radius, loc_shape, loc_is_squircle, loc_blur, loc_wobble;
    GLint loc_backdrop, loc_blur_sigma;
    GLuint quad_vao = 0, quad_vbo = 0, text_vao = 0, text_vbo = 0;
    GLuint grab_tex = 0; int grab_w = 0, grab_h = 0;
    float scroll_accum = 0;

    bool compile_shaders();
    void setup_buffers();

public:
    bool init(int w, int h, const char* title) override;
    void shutdown() override;
    bool is_running() override;
    void begin_frame(int w, int h) override;
    void end_frame() override;
    void render(const internal::DrawList& draw_list) override;
    void get_mouse_state(float& mx, float& my, bool& mdown, float& mwheel) override;
    void get_keyboard_events(std::vector<uint32_t>& chars, std::vector<int>& keys) override;
    
#ifndef NDEBUG
    bool capture_screenshot(const char* filename) override;
#endif
    internal::GpuTexturePtr create_texture(int w, int h, internal::TextureFormat fmt) override {
        return std::make_shared<OpenGLTexture>(w, h, fmt);
    }
    Capabilities capabilities() const override { return { true }; }
};

std::unique_ptr<Backend> CreateOpenGLBackend() { return std::make_unique<OpenGLBackend>(); }

// --- Shader Logic ---
static const char* vs_sdf = R"(#version 330 core
layout(location=0) in vec2 aPos;
uniform vec2 uViewportSize;
out vec2 vCoord;
void main(){gl_Position=vec4(aPos,0,1);vCoord=(aPos*0.5+0.5)*uViewportSize;vCoord.y=uViewportSize.y-vCoord.y;})";

static const char* fs_sdf = R"(#version 330 core
in vec2 vCoord;
uniform vec2 uShapePos, uShapeSize, uWobble;
uniform vec4 uColor;
uniform float uRadius, uBlurSigma, uDevicePixelRatio;
uniform int uShapeType;
uniform bool uIsSquircle;
uniform sampler2D uBackdrop;
out vec4 oColor;
float sdf_rr(vec2 p, vec2 b, float r){
    if(uIsSquircle){r=min(r,min(b.x,b.y));vec2 d=abs(p)-(b-r);d=max(d,0.)/r;return(pow(d.x,2.4)+pow(d.y,2.4)-1.)*r;}
    r=min(r,min(b.x,b.y));vec2 d=abs(p)-b+r;return min(max(d.x,d.y),0.)+length(max(d,0.))-r;
}
void main(){
    float aa = 1.0/uDevicePixelRatio;
    vec2 p = vCoord - (uShapePos + uShapeSize*0.5);
    float d = 0.0;
    if(uShapeType==0) { vec2 q=abs(p)-uShapeSize*0.5; d=length(max(q,0.))+min(max(q.x,q.y),0.); }
    else if(uShapeType==1) d=sdf_rr(p, uShapeSize*0.5, uRadius);
    else if(uShapeType==2) d=length(p)-uRadius;
    else if(uShapeType==3) { vec2 ba=uShapeSize-uShapePos,pa=vCoord-uShapePos;float h=clamp(dot(pa,ba)/dot(ba,ba),0.,1.);d=length(pa-ba*h)-uRadius; }
    float a = 1.0-smoothstep(-aa,aa,d);
    vec4 final = uColor;
    if(uBlurSigma>0.0){
        vec2 uv = gl_FragCoord.xy/textureSize(uBackdrop,0);
        float s=uBlurSigma; vec2 ts=1.0/textureSize(uBackdrop,0);
        vec4 b = (texture(uBackdrop,uv+vec2(s,s)*ts)+texture(uBackdrop,uv+vec2(-s,s)*ts)+texture(uBackdrop,uv+vec2(s,-s)*ts)+texture(uBackdrop,uv+vec2(-s,-s)*ts))*0.25;
        final=mix(b*1.1,uColor,uColor.a);
    }
    float accent=1.-smoothstep(0.,2./uDevicePixelRatio,abs(d+0.5));
    final.rgb=mix(final.rgb, (uColor.r>0.5?vec3(1.,.2,.8):vec3(0.,1.,1.)), accent*0.4);
    oColor=vec4(final.rgb,final.a*a);
})";

static const char* vs_text = R"(#version 330 core
layout(location=0) in vec4 aData;
uniform vec2 uViewportSize;
out vec2 vUV;
void main(){gl_Position=vec4((aData.x/uViewportSize.x)*2.-1.,(1.-aData.y/uViewportSize.y)*2.-1.,0,1);vUV=aData.zw;})";

static const char* fs_text = R"(#version 330 core
in vec2 vUV;
uniform sampler2D uAtlas;
uniform vec4 uColor;
out vec4 oColor;
void main(){float d=texture(uAtlas,vUV).r;float w=fwidth(d);float a=smoothstep(0.5-w,0.5+w,d);if(a<0.01)discard;oColor=vec4(uColor.rgb,uColor.a*a);})";

static GLuint compile(GLenum t, const char* s){
    GLuint h=glCreateShader(t);glShaderSource(h,1,&s,0);glCompileShader(h);
    GLint ok;glGetShaderiv(h,GL_COMPILE_STATUS,&ok);
    if(!ok){char err[512];glGetShaderInfoLog(h,512,0,err);std::cerr<<err<<std::endl;return 0;}
    return h;
}
bool OpenGLBackend::compile_shaders(){
    GLuint vs=compile(GL_VERTEX_SHADER,vs_sdf), fs=compile(GL_FRAGMENT_SHADER,fs_sdf);
    sdf_prog=glCreateProgram();glAttachShader(sdf_prog,vs);glAttachShader(sdf_prog,fs);glLinkProgram(sdf_prog);
    vs=compile(GL_VERTEX_SHADER,vs_text); fs=compile(GL_FRAGMENT_SHADER,fs_text);
    text_prog=glCreateProgram();glAttachShader(text_prog,vs);glAttachShader(text_prog,fs);glLinkProgram(text_prog);
    return true;
}

void OpenGLBackend::setup_buffers(){
    float q[]={-1,-1,1,-1,-1,1,1,1};
    glGenVertexArrays(1,&quad_vao);glGenBuffers(1,&quad_vbo);
    glBindVertexArray(quad_vao);glBindBuffer(GL_ARRAY_BUFFER,quad_vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(q),q,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,0,8,0);glEnableVertexAttribArray(0);
    glGenVertexArrays(1,&text_vao);glGenBuffers(1,&text_vbo);
    glBindVertexArray(text_vao);glBindBuffer(GL_ARRAY_BUFFER,text_vbo);
    glVertexAttribPointer(0,4,GL_FLOAT,0,16,0);glEnableVertexAttribArray(0);
}

static std::vector<uint32_t> g_chars; static std::vector<int> g_keys;
void char_cb(GLFWwindow*,unsigned int c){g_chars.push_back(c);}
void key_cb(GLFWwindow*,int k,int,int a,int){if(a==GLFW_PRESS||a==GLFW_REPEAT)g_keys.push_back(k);}

bool OpenGLBackend::init(int w, int h, const char* title){
    if(!glfwInit())return 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    window=glfwCreateWindow(w,h,title,0,0);if(!window)return 0;
    glfwMakeContextCurrent(window);glfwSetWindowUserPointer(window,this);
    glfwSetCharCallback(window,char_cb);glfwSetKeyCallback(window,key_cb);
    glfwSetScrollCallback(window,[](GLFWwindow* win,double,double y){static_cast<OpenGLBackend*>(glfwGetWindowUserPointer(win))->scroll_accum+=(float)y;});
    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress))return 0;
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    compile_shaders();setup_buffers();
    internal::GlyphCache::Get().init(this);
    lw=w;lh=h;return 1;
}

void OpenGLBackend::shutdown(){if(sdf_prog)glDeleteProgram(sdf_prog);glfwTerminate();}
bool OpenGLBackend::is_running(){return !glfwWindowShouldClose(window);}
void OpenGLBackend::begin_frame(int w, int h){
    glfwPollEvents();int dw,dh;glfwGetFramebufferSize(window,&dw,&dh);
    glViewport(0,0,dw,dh);dpr=(float)dw/(float)w;lw=w;lh=h;
    glClearColor(0.1f,0.1f,0.3f,1);glClear(GL_COLOR_BUFFER_BIT);
}
void OpenGLBackend::end_frame(){glfwSwapBuffers(window);}

void OpenGLBackend::render(const internal::DrawList& dl){
    glUseProgram(sdf_prog);
    loc_pos=glGetUniformLocation(sdf_prog,"uShapePos");loc_size=glGetUniformLocation(sdf_prog,"uShapeSize");
    loc_color=glGetUniformLocation(sdf_prog,"uColor");loc_radius=glGetUniformLocation(sdf_prog,"uRadius");
    loc_shape=glGetUniformLocation(sdf_prog,"uShapeType");loc_is_squircle=glGetUniformLocation(sdf_prog,"uIsSquircle");
    loc_blur_sigma=glGetUniformLocation(sdf_prog,"uBlurSigma");loc_backdrop=glGetUniformLocation(sdf_prog,"uBackdrop");
    glUniform2f(glGetUniformLocation(sdf_prog,"uViewportSize"),(float)lw,(float)lh);
    glUniform1f(glGetUniformLocation(sdf_prog,"uDevicePixelRatio"),dpr);
    glBindVertexArray(quad_vao);
    
    std::vector<float> ts_tx={0}, ts_ty={0}, ts_sc={1};
    auto push_t=[&](float tx,float ty,float s){float cx=ts_tx.back(),cy=ts_ty.back(),cs=ts_sc.back();ts_tx.push_back(tx*cs+cx);ts_ty.push_back(ty*cs+cy);ts_sc.push_back(s*cs);};
    auto pop_t=[&](){if(ts_tx.size()>1){ts_tx.pop_back();ts_ty.pop_back();ts_sc.pop_back();}};
    auto apply_t=[&](float& x,float& y,float& w,float& h){float cs=ts_sc.back();x=x*cs+ts_tx.back();y=y*cs+ts_ty.back();w*=cs;h*=cs;};
    auto apply_p=[&](float& x,float& y){float cs=ts_sc.back();x=x*cs+ts_tx.back();y=y*cs+ts_ty.back();};

    struct Clip{float x,y,w,h;}; std::vector<Clip> cs;
    auto apply_c=[&](){if(cs.empty())glDisable(GL_SCISSOR_TEST);else{glEnable(GL_SCISSOR_TEST);auto& c=cs.back();glScissor((int)(c.x*dpr),(int)((lh-(c.y+c.h))*dpr),(int)(c.w*dpr),(int)(c.h*dpr));}};
    auto push_c=[&](float x,float y,float w,float h){float nx=x,ny=y,nw=w,nh=h;apply_t(nx,ny,nw,nh);if(cs.empty())cs.push_back({nx,ny,nw,nh});else{auto& p=cs.back();float ix=std::max(nx,p.x),iy=std::max(ny,p.y);cs.push_back({ix,iy,std::max(0.0f,std::min(nx+nw,p.x+p.w)-ix),std::max(0.0f,std::min(ny+nh,p.y+p.h)-iy)});};apply_c();};
    auto pop_c=[&](){if(!cs.empty())cs.pop_back();apply_c();};

    // --- Passes ---
    for(int pass=0;pass<3;++pass){
        ts_tx={0};ts_ty={0};ts_sc={1};cs.clear();glDisable(GL_SCISSOR_TEST);
        for(const auto& cmd : dl.commands){
            if(cmd.type==internal::DrawCmdType::PushTransform)push_t(cmd.transform.tx,cmd.transform.ty,cmd.transform.scale);
            else if(cmd.type==internal::DrawCmdType::PopTransform)pop_t();
            else if(cmd.type==internal::DrawCmdType::PushClip)push_c(cmd.clip.x,cmd.clip.y,cmd.clip.w,cmd.clip.h);
            else if(cmd.type==internal::DrawCmdType::PopClip)pop_c();
            else{
                if(pass<2){ // Shadows
                    float el=0,px=0,py=0,sx=0,sy=0,cr=0; int ty=0;
                    if(cmd.type==internal::DrawCmdType::Rectangle){el=cmd.rectangle.elevation;px=cmd.rectangle.pos_x;py=cmd.rectangle.pos_y;sx=cmd.rectangle.size_x;sy=cmd.rectangle.size_y;}
                    else if(cmd.type==internal::DrawCmdType::RoundedRect){el=cmd.rounded_rect.elevation;px=cmd.rounded_rect.pos_x;py=cmd.rounded_rect.pos_y;sx=cmd.rounded_rect.size_x;sy=cmd.rounded_rect.size_y;cr=cmd.rounded_rect.radius;ty=1;}
                    else if(cmd.type==internal::DrawCmdType::Circle){el=cmd.circle.elevation;px=cmd.circle.center_x-cmd.circle.r;py=cmd.circle.center_y-cmd.circle.r;sx=cmd.circle.r*2;sy=sx;cr=cmd.circle.r;ty=2;}
                    else continue;
                    if(el>0){
                        apply_t(px,py,sx,sy); float cs_v=ts_sc.back(); cr*=cs_v; el*=cs_v;
                        float ex,oy,al; if(pass==0){ex=el*0.8f;oy=0;al=std::min(0.025f*el,0.15f);}else{ex=el*0.15f;oy=el*0.4f;al=std::min(0.05f*el,0.25f);}
                        glUniform2f(loc_pos,px-ex,py-ex+oy);glUniform2f(loc_size,sx+ex*2,sy+ex*2);
                        glUniform4f(loc_color,0,0,0,al);glUniform1f(loc_radius,cr+ex);glUniform1i(loc_shape,ty);
                        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                    }
                } else { // Body
                    if(cmd.type==internal::DrawCmdType::Rectangle){
                        float px=cmd.rectangle.pos_x,py=cmd.rectangle.pos_y,sx=cmd.rectangle.size_x,sy=cmd.rectangle.size_y;apply_t(px,py,sx,sy);
                        glUniform2f(loc_pos,px,py);glUniform2f(loc_size,sx,sy);glUniform4f(loc_color,cmd.rectangle.color_r,cmd.rectangle.color_g,cmd.rectangle.color_b,cmd.rectangle.color_a);
                        glUniform1f(loc_radius,0);glUniform1i(loc_shape,0);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                    } else if(cmd.type==internal::DrawCmdType::RoundedRect){
                        float px=cmd.rounded_rect.pos_x,py=cmd.rounded_rect.pos_y,sx=cmd.rounded_rect.size_x,sy=cmd.rounded_rect.size_y,cr=cmd.rounded_rect.radius;apply_t(px,py,sx,sy);float csv=ts_sc.back();cr*=csv;
                        glUniform2f(loc_pos,px,py);glUniform2f(loc_size,sx,sy);glUniform4f(loc_color,cmd.rounded_rect.color_r,cmd.rounded_rect.color_g,cmd.rounded_rect.color_b,cmd.rounded_rect.color_a);
                        glUniform1f(loc_radius,cr);glUniform1i(loc_is_squircle,cmd.rounded_rect.is_squircle);glUniform1i(loc_shape,1);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                    } else if(cmd.type==internal::DrawCmdType::Circle){
                        float cr=cmd.circle.r,px=cmd.circle.center_x-cr,py=cmd.circle.center_y-cr,s=cr*2;apply_t(px,py,s,s);float csv=ts_sc.back();cr*=csv;
                        glUniform2f(loc_pos,px,py);glUniform2f(loc_size,s,s);glUniform4f(loc_color,cmd.circle.color_r,cmd.circle.color_g,cmd.circle.color_b,cmd.circle.color_a);
                        glUniform1f(loc_radius,cr);glUniform1i(loc_shape,2);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                    } else if(cmd.type==internal::DrawCmdType::Line){
                        float px0=cmd.line.p0_x,py0=cmd.line.p0_y,px1=cmd.line.p1_x,py1=cmd.line.p1_y,th=cmd.line.thickness;
                        float csv=ts_sc.back();px0=px0*csv+ts_tx.back();py0=py0*csv+ts_ty.back();px1=px1*csv+ts_tx.back();py1=py1*csv+ts_ty.back();th*=csv;
                        glUniform2f(loc_pos,px0,py0);glUniform2f(loc_size,px1,py1);glUniform4f(loc_color,cmd.line.color_r,cmd.line.color_g,cmd.line.color_b,cmd.line.color_a);
                        glUniform1f(loc_radius,th*0.5f);glUniform1i(loc_shape,3);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                    }
                }
            }
        }
    }
    // Text Pass
    glUseProgram(text_prog); glActiveTexture(GL_TEXTURE0);
    auto atlas=internal::GlyphCache::Get().get_atlas_texture(); if(atlas){glBindTexture(GL_TEXTURE_2D,(GLuint)atlas->get_native_handle());internal::GlyphCache::Get().update_texture();}
    glUniform1i(glGetUniformLocation(text_prog,"uAtlas"),0);glUniform2f(glGetUniformLocation(text_prog,"uViewportSize"),(float)lw,(float)lh);
    GLint lc=glGetUniformLocation(text_prog,"uColor"); glBindVertexArray(text_vao);glBindBuffer(GL_ARRAY_BUFFER,text_vbo);
    ts_tx={0};ts_ty={0};ts_sc={1};cs.clear();glDisable(GL_SCISSOR_TEST);
    for(const auto& cmd : dl.commands){
        if(cmd.type==internal::DrawCmdType::PushTransform)push_t(cmd.transform.tx,cmd.transform.ty,cmd.transform.scale);
        else if(cmd.type==internal::DrawCmdType::PopTransform)pop_t();
        else if(cmd.type==internal::DrawCmdType::PushClip)push_c(cmd.clip.x,cmd.clip.y,cmd.clip.w,cmd.clip.h);
        else if(cmd.type==internal::DrawCmdType::PopClip)pop_c();
        else if(cmd.type==internal::DrawCmdType::Glyph){
            glUniform4f(lc,cmd.glyph.color_r,cmd.glyph.color_g,cmd.glyph.color_b,cmd.glyph.color_a);
            float gx=cmd.glyph.pos_x,gy=cmd.glyph.pos_y,gw=cmd.glyph.size_x,gh=cmd.glyph.size_y;apply_t(gx,gy,gw,gh);
            float vd[]={gx,gy,cmd.glyph.u0,cmd.glyph.v0,gx,gy+gh,cmd.glyph.u0,cmd.glyph.v1,gx+gw,gy,cmd.glyph.u1,cmd.glyph.v0,gx+gw,gy+gh,cmd.glyph.u1,cmd.glyph.v1};
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vd),vd);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
        }
    }
}
void OpenGLBackend::get_mouse_state(float& mx,float& my,bool& mdown,float& mwheel){
    if(!window){mx=my=0;mdown=0;mwheel=0;return;}
    double dx,dy;glfwGetCursorPos(window,&dx,&dy);mx=(float)dx;my=(float)dy;
    mdown=glfwGetMouseButton(window,0)==GLFW_PRESS;mwheel=scroll_accum;scroll_accum=0;
}
void OpenGLBackend::get_keyboard_events(std::vector<uint32_t>& c,std::vector<int>& k){c=g_chars;k=g_keys;g_chars.clear();g_keys.clear();}

#ifndef NDEBUG
bool OpenGLBackend::capture_screenshot(const char* filename) {
    if(!filename)return 0; int pw=(int)(lw*dpr), ph=(int)(lh*dpr);
    std::vector<unsigned char> px(pw*ph*4); glFinish(); glReadBuffer(GL_BACK);
    glReadPixels(0,0,pw,ph,GL_RGBA,GL_UNSIGNED_BYTE,px.data());
    std::vector<unsigned char> fl(px.size()); int rs=pw*4;
    for(int y=0;y<ph;++y)memcpy(fl.data()+(ph-1-y)*rs,px.data()+y*rs,rs);
    for(size_t i=0;i<fl.size();i+=4)std::swap(fl[i],fl[i+2]);
    return stbi_write_png(filename,pw,ph,4,fl.data(),rs)!=0;
}
#endif

} // namespace fanta
