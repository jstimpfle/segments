void create_opengl_context(void);
void *load_opengl_pointer(const char *name);
void setup_opengl(void);
void swap_buffers(void);

#ifdef _WIN32
#include <Windows.h>  // otherwise GL.h doesn't work
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#define MAKE(type, name) extern type const name;
#include "openglprocs.h"
#undef MAKE
