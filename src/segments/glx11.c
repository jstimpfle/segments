#include <segments/defs.h>
#include <segments/logging.h>
#include <segments/opengl.h>
#include <segments/window.h>

#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#define LENGTH(a) (sizeof (a) / sizeof (a)[0])

static const int initialWindowWidth = 800;
static const int initialWindowHeight = 600;

static GLint att[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        GLX_SAMPLE_BUFFERS, 1, // <-- MSAA
        GLX_SAMPLES, 4, // <-- MSAA
        None,
};

static Display *display;
static Window rootWin;
static Window window;
static XVisualInfo *visualInfo;
static Colormap colormap;
static GLXContext contextGlx;

void create_opengl_context(void)
{
        display = XOpenDisplay(NULL);
        if (display == NULL)
                fatal_f("Failed to XOpenDisplay()");

        rootWin = DefaultRootWindow(display);

        visualInfo = glXChooseVisual(display, 0, att);
        if (visualInfo == NULL)
                fatal_f("No appropriate visual found");

        colormap = XCreateColormap(display, rootWin,
                                   visualInfo->visual, AllocNone);

        XSetWindowAttributes wa;
        wa.colormap = colormap;
        wa.event_mask = ExposureMask
                | KeyPressMask | KeyReleaseMask
                | ButtonPressMask | ButtonReleaseMask
                | PointerMotionMask;

        window = XCreateWindow(display, rootWin, 0, 0,
                               initialWindowWidth, initialWindowHeight,
                               0, visualInfo->depth,
                               InputOutput, visualInfo->visual,
                               CWColormap | CWEventMask, &wa);

        XMapWindow(display, window);
        XStoreName(display, window, "segments");

        contextGlx = glXCreateContext(display, visualInfo, NULL, GL_TRUE);
        glXMakeCurrent(display, window, contextGlx);
}

void close_window(void)
{
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, contextGlx);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
}

static const struct {
        int xkeysym;
        int keyKind;
} keymap[] = {
        { XK_Escape, KEY_ESCAPE },
        { XK_space, KEY_SPACE },
        { XK_BackSpace, KEY_BACKSPACE },
        { XK_Left, KEY_LEFT },
        { XK_Right, KEY_RIGHT },
        { XK_Up, KEY_UP },
        { XK_Down, KEY_DOWN },
};

static int xbutton_to_mousebuttonKind(int xbutton)
{
        if (xbutton == 1)
                return MOUSEBUTTON_1;
        if (xbutton == 2)
                return MOUSEBUTTON_2;
        if (xbutton == 3)
                return MOUSEBUTTON_3;
        return -1;
}

static void handle_x11_button_press_or_release(XButtonEvent *xbutton, int mousebuttoneventKind)
{
        if (xbutton->button == 4 || xbutton->button == 5) {
                if (mousebuttoneventKind == MOUSEBUTTONEVENT_PRESS) {
                        float amount = xbutton->button == 4 ? 1.f : -1.f;
                        send_scroll_event(amount);
                }
                else {
                        // should not happen normally
                }
        }
        else {
                int mousebuttonKind = xbutton_to_mousebuttonKind(xbutton->button);
                send_mousebutton_event(mousebuttonKind, mousebuttoneventKind);
        }
}

void fetch_all_pending_events(void)
{
        while (XPending(display)) {
                XEvent event;
                XNextEvent(display, &event);
                if (event.type == KeyPress) {
                        XKeyEvent *key = &event.xkey;
                        int keysym = XkbKeycodeToKeysym(display, key->keycode, 0, 0);
                        for (int i = 0; i < LENGTH(keymap); i++) {
                                if (keymap[i].xkeysym == keysym) {
                                        send_key_event(keymap[i].keyKind);
                                        break;
                                }
                        }
                }
                else if (event.type == MotionNotify) {
                        XMotionEvent *motion = &event.xmotion;
                        int x = motion->x;
                        int y = motion->y;
                        send_mousemove_event(x, y);
                }
                else if (event.type == ButtonPress) {
                        XButtonEvent *button = &event.xbutton;
                        handle_x11_button_press_or_release(button, MOUSEBUTTONEVENT_PRESS);
                }
                else if (event.type == ButtonRelease) {
                        XButtonEvent *button = &event.xbutton;
                        handle_x11_button_press_or_release(button, MOUSEBUTTONEVENT_RELEASE);
                }
        }
}

void *load_opengl_pointer(const char *name)
{
        return glXGetProcAddress((const GLubyte *) name);
}

void swap_buffers(void)
{
        glXSwapBuffers(display, window);
}
