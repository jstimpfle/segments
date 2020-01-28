#include <segments/defs.h>
#include <segments/logging.h>
#include <segments/opengl.h>
#include <segments/window.h>
#include <assert.h>
#include <Windows.h>
#include <windowsx.h>  // GET_X_LPARAM(), GET_Y_LPARAM()
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>


static const struct {
        int code;
        int keyKind;
} keyMap[] = {
        { VK_RETURN, KEY_ENTER },
        { VK_BACK, KEY_BACKSPACE },
        { VK_ESCAPE, KEY_ESCAPE },
        { VK_LEFT, KEY_LEFT },
        { VK_RIGHT, KEY_RIGHT },
        { VK_UP, KEY_UP },
        { VK_DOWN, KEY_DOWN },
        { VK_SPACE, KEY_SPACE },
};

static const struct {
        int code;
        int mousebuttonKind;
} mouseMap[] = {
        { VK_LBUTTON, MOUSEBUTTON_1 },
        { VK_MBUTTON, MOUSEBUTTON_2 },
        { VK_RBUTTON, MOUSEBUTTON_3 },
};

static HWND globalWND;
static HDC globalDC;
static HGLRC globalGLRC;

LRESULT CALLBACK my_window_proc(
    _In_ HWND hWnd,
    _In_ UINT msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam)
{
        if (msg == WM_CREATE) {
                return TRUE;
        }
        else if (msg == WM_KEYDOWN) {
                int isRepeated = lParam & (1 << 30);
                for (int i = 0; i < sizeof keyMap / sizeof keyMap[0]; i++) {
                        if (wParam == keyMap[i].code) {
                                send_key_event(keyMap[i].keyKind);
                                return TRUE;
                        }
                }
                return FALSE;
        }
        else if (msg == WM_LBUTTONDOWN) {
                send_mousebutton_event(MOUSEBUTTON_1, MOUSEBUTTONEVENT_PRESS);
                return TRUE;
        }
        else if (msg == WM_LBUTTONUP) {
                send_mousebutton_event(MOUSEBUTTON_1, MOUSEBUTTONEVENT_RELEASE);
                return TRUE;
        }
        else if (msg == WM_MBUTTONDOWN) {
                send_mousebutton_event(MOUSEBUTTON_2, MOUSEBUTTONEVENT_PRESS);
                return TRUE;
        }
        else if (msg == WM_MBUTTONDOWN) {
                send_mousebutton_event(MOUSEBUTTON_2, MOUSEBUTTONEVENT_RELEASE);
                return TRUE;
        }
        else if (msg == WM_RBUTTONDOWN) {
                send_mousebutton_event(MOUSEBUTTON_3, MOUSEBUTTONEVENT_PRESS);
                return TRUE;
        }
        else if (msg == WM_RBUTTONDOWN) {
                send_mousebutton_event(MOUSEBUTTON_3, MOUSEBUTTONEVENT_RELEASE);
                return TRUE;
        }
        else if (msg == WM_MOUSEMOVE) {
                send_mousemove_event(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return TRUE;
        }
        else if (msg == WM_MOUSEWHEEL) {
                send_scroll_event(GET_WHEEL_DELTA_WPARAM(wParam) / (float) WHEEL_DELTA);
                return TRUE;
        }
        else if (msg == WM_WINDOWPOSCHANGED) {
                struct tagWINDOWPOS *pos = (void *)lParam;
                send_windowresize_event(pos->cx, pos->cy);
                return TRUE;
        }
        else if (msg == WM_CLOSE) {
                exit(0); //XXX
        }
        else {
                return DefWindowProc(hWnd, msg, wParam, lParam);
        }
}

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

/* This function copied from https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c */
static void init_opengl_extensions(void)
{
        // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
        // We use a dummy window because you can only set the pixel format for a window once. For the
        // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
        // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
        // have a context.
        WNDCLASSA window_class = {
            .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            .lpfnWndProc = DefWindowProcA,
            .hInstance = GetModuleHandle(0),
            .lpszClassName = "Dummy_WGL_djuasiodwa",
        };

        if (!RegisterClassA(&window_class))
                fatal_f("Failed to register dummy OpenGL window.");

        HWND dummy_window = CreateWindowExA(
                0,
                window_class.lpszClassName,
                "Dummy OpenGL Window",
                0,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                window_class.hInstance,
                0);

        if (!dummy_window)
                fatal_f("Failed to create dummy OpenGL window.");

        HDC dummy_dc = GetDC(dummy_window);

        PIXELFORMATDESCRIPTOR pfd = {
            .nSize = sizeof(pfd),
            .nVersion = 1,
            .iPixelType = PFD_TYPE_RGBA,
            .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .cColorBits = 32,
            .cAlphaBits = 8,
            .iLayerType = PFD_MAIN_PLANE,
            .cDepthBits = 24,
            .cStencilBits = 8,
        };

        int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
        if (!pixel_format)
                fatal_f("Failed to find a suitable pixel format.");
        if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
                fatal_f("Failed to set the pixel format.");

        HGLRC dummy_context = wglCreateContext(dummy_dc);
        if (!dummy_context)
                fatal_f("Failed to create a dummy OpenGL rendering context.");
        if (!wglMakeCurrent(dummy_dc, dummy_context))
                fatal_f("Failed to activate dummy OpenGL rendering context.");

        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        if (wglCreateContextAttribsARB == NULL)
                fatal_f("failed to load extension wglCreateContextAttribsARB()");
        if (wglChoosePixelFormatARB == NULL)
                fatal_f("failed to load extension arglChoosePixelFormatARB()");

        wglMakeCurrent(dummy_dc, 0);
        wglDeleteContext(dummy_context);
        ReleaseDC(dummy_window, dummy_dc);
        DestroyWindow(dummy_window);
}

void create_opengl_context(void)
{
        init_opengl_extensions();

        HMODULE hInstance = GetModuleHandle(NULL);
        if (hInstance == NULL)
                fatal_f("Failed to GetModuleHandle(NULL)");
        int nWidth = 640;
        int nHeight = 480;
	WNDCLASSA wc      = {0}; 
	wc.lpfnWndProc   = my_window_proc;
	wc.hInstance     = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = "myclass";
	wc.style = CS_OWNDC;
        if (!RegisterClass(&wc))
		fatal_f("Failed to register window class");
        HWND window = CreateWindowA(wc.lpszClassName, "My Window", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 100, 100, nWidth, nHeight, NULL, NULL, hInstance, NULL);
        if (window == NULL)
                fatal_f("Failed to create window");
        globalWND = window;


        globalDC = GetDC(globalWND);
        if (globalDC == NULL)
                fatal_f("Failed to GetDC() from HWND");

        /*
        FIND AND SET PIXEL FORMAT
        */

        int trysamplesettings[] = {
                16, 4, 1
        };
        int pixelFormat;
        int foundPixelFormat = 0;

        for (int i = 0; i < LENGTH(trysamplesettings); i++) {
                int numSamples = trysamplesettings[i];
                const float pfAttribFList[] = { 0, 0 };
                const int piAttribIList[] = {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_RED_BITS_ARB, 8,
                    WGL_GREEN_BITS_ARB, 8,
                    WGL_BLUE_BITS_ARB, 8,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 16,
                    WGL_STENCIL_BITS_ARB, 0,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    // suppport for MSAA. Is 16 too much? It works for me.
                    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                    WGL_SAMPLES_ARB, numSamples,
                    0, 0
                };

                UINT nMaxFormats = 1;
                UINT nNumFormats;
                if (!wglChoosePixelFormatARB(globalDC, piAttribIList, pfAttribFList, nMaxFormats, &pixelFormat, &nNumFormats))
                        continue;
                foundPixelFormat = 1;
                break;
        }

        if (!foundPixelFormat)
                fatal_f("Failed to ChoosePixelFormat()");

        /* Passing NULL as the PIXELFORMATDESCRIPTOR pointer. Does that work on all machines?
        The documentation is cryptic on the use of that value.
        In conjunction with wglChoosePixelFormatARB(), the method that you can find on the internet,
        which involves calling DescribePixelFormat() + passing non-NULL parameter here did not work
        on all machines for me. */
        if (!SetPixelFormat(globalDC, pixelFormat, NULL))
                fatal_f("failed to SetPixelFormat()");

        /* create opengl context */
        static const int gl30_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 0,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };
        globalGLRC = wglCreateContextAttribsARB(globalDC, 0, gl30_attribs);
        if (!globalGLRC)
                fatal_f("Failed to create OpenGL 3.0 context.");

        if (!wglMakeCurrent(globalDC, globalGLRC))
                fatal_f("Failed to wglMakeCurrent(globalDC, globalGLRC);");
}

void close_window(void)
{
        // TODO
}

void *load_opengl_pointer(const char *name)
{
        return wglGetProcAddress(name);
}

void fetch_all_pending_events(void)
{
        MSG msg;
        BOOL bRet;
        for (;;) {
                bRet = PeekMessageA(&msg, globalWND, 0, 0, PM_REMOVE);
                if (bRet == 0)
                        break;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}

void swap_buffers(void)
{
        if (!SwapBuffers(globalDC))
                fatal_f("Failed to SwapBuffers()");
}