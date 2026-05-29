#ifndef COEP_GLX_LINUX_H
#define COEP_GLX_LINUX_H

#include "../../../core.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef void* GLXContext;
typedef unsigned long GLXDrawable;

#define GLX_RGBA            4
#define GLX_DOUBLEBUFFER    5
#define GLX_DEPTH_SIZE      12

#define GLX_PROC_LIST \
	X(glXChooseVisual, XVisualInfo*, (Display* dpy, int screen, int* attribList)) \
	X(glXCreateContext, GLXContext, (Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct)) \
	X(glXMakeCurrent, Bool, (Display* dpy, GLXDrawable drawable, GLXContext ctx)) \
	X(glXSwapBuffers, void, (Display* dpy, GLXDrawable drawable)) \
	X(glXDestroyContext, void, (Display* dpy, GLXContext ctx))

#define X(name, ret, params) typedef ret (*PFN_##name) params;
GLX_PROC_LIST
#undef X

typedef void* (*PFN_glXGetProcAddress)(const char*);

#define X(name, ret, params) extern PFN_##name name;
GLX_PROC_LIST
#undef X

extern PFN_glXGetProcAddress glXGetProcAddress_ptr;

b8 glx_load_symbols() noexcept;
void* glx_dlsym(const char* name) noexcept;
void glx_unload_symbols() noexcept;

#endif // COEP_GLX_LINUX_H
