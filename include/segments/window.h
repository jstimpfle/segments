#ifndef SEGMENTS_WINDOW_H_INCLUDED
#define SEGMENTS_WINDOW_H_INCLUDED

enum {
        KEY_ENTER,
        KEY_ESCAPE,
        KEY_BACKSPACE,
        KEY_SPACE,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_UP,
        KEY_DOWN,
        NUM_KEY_KINDS
};

enum {
        MODIFIER_SHIFT,
        MODIFIER_CONTROL,
        MODIFIER_ALT,
        NUM_MODIFIER_KINDS,
};

enum {
        MOUSEBUTTON_1,
        MOUSEBUTTON_2,
        MOUSEBUTTON_3,
        MOUSEBUTTON_4,
        MOUSEBUTTON_5,
        MOUSEBUTTON_6,
        MOUSEBUTTON_7,
        MOUSEBUTTON_8,
        NUM_MOUSEBUTTON_KINDS
};

enum {
        MOUSEBUTTONEVENT_PRESS,
        MOUSEBUTTONEVENT_RELEASE,
        NUM_MOUSEBUTTONEVENT_KINDS
};

enum {
        EVENT_KEY,
        EVENT_MOUSEBUTTON,
        EVENT_MOUSEMOVE,
        EVENT_SCROLL,
        EVENT_WINDOWRESIZE,
        NUM_EVENT_KINDS
};

struct KeyEvent {
        int keyKind;
};

struct MousebuttonEvent {
        int mousebuttonKind;
        int mousebuttoneventKind;
};

struct MousemoveEvent {
        float x;
        float y;
};

struct ScrollEvent {
        float amount;  // unknown unit. Assume 1 scroll == 1 "click" in the scroll wheel
};

struct WindowresizeEvent {
        int w;
        int h;
};

struct Event {
        int eventKind;
        union {
                struct KeyEvent tKey;
                struct MousebuttonEvent tMousebutton;
                struct MousemoveEvent tMousemove;
                struct ScrollEvent tScroll;
                struct WindowresizeEvent tWindowresize;
        };
};

void send_windowresize_event(int w, int h);
void send_key_event(int keyKind);
void send_mousebutton_event(int mousebuttonKind, int mousebuttoneventKind);
void send_mousemove_event(int x, int y);
void send_scroll_event(float amount);
void fetch_all_pending_events(void);
int have_events(void);
void dequeue_event(struct Event *event);

void close_window(void);

#endif
