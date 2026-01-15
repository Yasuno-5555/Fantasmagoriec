#pragma once
#include <windows.h>
#include <GLFW/glfw3.h> // Includes GL.h (1.1)

#include <stddef.h>

// Minimal GL definitions for 3.3 Core
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_STREAM_DRAW 0x88E0
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_TEXTURE0 0x84C0
#define GL_ActiveTexture 0x84E0

// Function pointer types
typedef void (APIENTRY *PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY *PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY *PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRY *PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (APIENTRY *PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRY *PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (APIENTRY *PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY *PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);

// Global Pointers
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM4FPROC glUniform4f;
// New
typedef void (APIENTRY *PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
extern PFNGLUNIFORM1IPROC glUniform1i;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

typedef void (APIENTRY *PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (APIENTRY *PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

// Textures
// Need defines if not in GL 1.1 (Windows gl.h usually has 1.1, missing these?)
// GL 1.1 has glTexImage2D, glBindTexture, glGenTextures?
// glGenTextures is 1.1. glBindTexture is 1.1.
// But we are using pointers for everything to be safe/modern core? Or because we don't link opengl32 functions directly (though now we do).
// If we link opengl32, we have 1.1 functions.
// But let's define PFNs for consistency or if we want to use extensions.
// Actually standard practice with loaders is to load everything.

// Textures
// GL 1.1 functions (glGenTextures, glBindTexture, etc) are linked via opengl32.lib
// We only need extended ones.
typedef void (APIENTRY *PFNGLACTIVETEXTUREPROC) (GLenum texture);

extern PFNGLACTIVETEXTUREPROC glActiveTexture;

// Defines if missing from 1.1 headers
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif

#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif

#ifndef GL_RED
#define GL_RED 0x1903
#endif

#ifndef GL_RGBA
#define GL_RGBA 0x1908
#endif

#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401
#endif

#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER 0x2801
#endif

#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER 0x2800
#endif

#ifndef GL_LINEAR
#define GL_LINEAR 0x2601
#endif

#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_S 0x2802
#endif

#ifndef GL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_T 0x2803
#endif





void init_gl_lite();
