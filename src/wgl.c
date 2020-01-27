#include <segments/logging.h>
#include <segments/opengl.h>
#include <segments/window.h>
#include <assert.h>
#include <Windows.h>
#include <windowsx.h>  // GET_X_LPARAM(), GET_Y_LPARAM()


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

void create_opengl_context(void)
{
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

        // this could (or should?) go in a callback for the WM_CREATE message
        PIXELFORMATDESCRIPTOR pfd =
        {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
                32,                   // Colordepth of the framebuffer.
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                24,                   // Number of bits for the depthbuffer
                8,                    // Number of bits for the stencilbuffer
                0,                    // Number of Aux buffers in the framebuffer.
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
        };

        globalDC = GetDC(globalWND);
        int pixelFormat = ChoosePixelFormat(globalDC, &pfd);
        if (!pixelFormat)
                fatal_f("Failed to choose pixel format");
        if (!SetPixelFormat(globalDC, pixelFormat, &pfd))
                fatal_f("Failed to set pixel format");
        globalGLRC = wglCreateContext(globalDC);
        if (globalGLRC == NULL)
                fatal_f("Failed to create opengl context");

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