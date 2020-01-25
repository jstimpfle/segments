#include <segments/opengl.h>
#include <segments/gfx.h>
#include <shaders.h>

#define LENGTH(a) (sizeof (a) / sizeof (a)[0])

int main(void)
{
        create_opengl_context();
        setup_opengl();

        do_gfx();

        return 0;
}
