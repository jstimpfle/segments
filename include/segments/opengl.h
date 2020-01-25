void create_opengl_context(void);
void *load_opengl_pointer(const char *name);
void setup_opengl(void);
void swap_buffers(void);

#include <GL/gl.h>
#define MAKE(type, name) extern type const name;
#include "openglprocs.h"
#undef MAKE
