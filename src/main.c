#include <segments/opengl.h>
#include <shaders.h>

#define LENGTH(a) (sizeof (a) / sizeof (a)[0])

int main(void)
{
        create_opengl_context();
        setup_opengl();

        just_do_all_gl_stuff();

        return 0;
}
