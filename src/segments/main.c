#include <segments/opengl.h>
#include <segments/gfx.h>

int main(void)
{
        create_opengl_context();
        setup_opengl();
        do_gfx();
        return 0;
}
