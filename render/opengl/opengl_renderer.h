#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "../../core.h"
#include "../../platform/platform.h"
#include "../render.h"

// ============================================================================
// OpenGL Standard Type Definitions
// ============================================================================
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef char GLchar;
typedef __PTRDIFF_TYPE__ GLsizeiptr;
typedef __PTRDIFF_TYPE__ GLintptr;

// ============================================================================
// OpenGL Standard Constants
// ============================================================================
#define GL_FALSE                          0
#define GL_TRUE                           1

#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_BLEND                          0x0BE2
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303

#define GL_TRIANGLES                      0x0004

#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

#define GL_RGBA8                          0x8058
#define GL_RGBA                           0x1908
#define GL_BGRA                           0x80E1
#define GL_UNSIGNED_BYTE                  0x1401

#define GL_ARRAY_BUFFER                   0x8892
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FLOAT                          0x1406

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C

// ============================================================================
// OpenGL X-Macro Procedure List
// ============================================================================
#define OPENGL_PROC_LIST \
    X(glGenBuffers, void, (GLsizei n, GLuint* buffers)) \
    X(glBindBuffer, void, (GLenum target, GLuint buffer)) \
    X(glDeleteBuffers, void, (GLsizei n, GLuint* buffers)) \
    X(glBufferData, void, (GLenum target, GLsizeiptr size, const void* data, GLenum usage)) \
    X(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data)) \
    X(glGenVertexArrays, void, (GLsizei n, GLuint* arrays)) \
    X(glBindVertexArray, void, (GLuint array)) \
    X(glDeleteVertexArrays, void, (GLsizei n, GLuint* arrays)) \
    X(glCreateShader, GLuint, (GLenum type)) \
    X(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)) \
    X(glCompileShader, void, (GLuint shader)) \
    X(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint* params)) \
    X(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(glCreateProgram, GLuint, (void)) \
    X(glAttachShader, void, (GLuint program, GLuint shader)) \
    X(glLinkProgram, void, (GLuint program)) \
    X(glGetProgramiv, void, (GLuint program, GLenum pname, GLint* params)) \
    X(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(glUseProgram, void, (GLuint program)) \
    X(glDeleteShader, void, (GLuint shader)) \
    X(glDeleteProgram, void, (GLuint program)) \
    X(glEnableVertexAttribArray, void, (GLuint index)) \
    X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)) \
    X(glVertexAttribDivisor, void, (GLuint index, GLuint divisor)) \
    X(glGetUniformLocation, GLint, (GLuint program, const GLchar* name)) \
    X(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
    X(glActiveTexture, void, (GLenum texture)) \
    X(glDrawArraysInstanced, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X(glClearColor, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X(glClear, void, (GLbitfield mask)) \
    X(glGenTextures, void, (GLsizei n, GLuint* textures)) \
    X(glBindTexture, void, (GLenum target, GLuint texture)) \
    X(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)) \
    X(glTexParameteri, void, (GLenum target, GLenum pname, GLint param)) \
    X(glDeleteTextures, void, (GLsizei n, const GLuint* textures)) \
    X(glEnable, void, (GLenum cap)) \
    X(glBlendFunc, void, (GLenum sfactor, GLenum dfactor)) \
    X(glGetError, GLenum, (void)) \
    X(glGetIntegerv, void, (GLenum pname, GLint* data))

// Declare function pointer typedefs and extern variables
#define X(name, ret, params) \
    typedef ret (__stdcall *PFN_##name) params; \
    extern PFN_##name name;
OPENGL_PROC_LIST
#undef X

// ============================================================================
// OpenGL Graphics Context Structure
// ============================================================================
struct GraphicsContext {
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLuint texture_id;
    i32 cached_width;
    i32 cached_height;
#if defined(OS_WINDOWS)
    void* hwnd;
    void* hdc;
    void* hglrc;
#elif defined(OS_LINUX)
    void* display;
    unsigned long window;
    void* glx_context;
#endif
};

// ============================================================================
// Platform-Independent Rendering Functions
// ============================================================================
EXTERN_C GraphicsContext* __opengl_init_graphics(PlatformWindow* window) noexcept;
EXTERN_C b8 __opengl_upload_texture(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept;
EXTERN_C void __opengl_submit_frame(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept;
EXTERN_C void __opengl_destroy_graphics(GraphicsContext* context) noexcept;

#endif // OPENGL_RENDERER_H
