#include <segments/defs.h>
#include <segments/window.h>
#include <string.h>

static struct Event queue[16];
static int numEvents;

void send_event(struct Event event)
{
        if (numEvents < sizeof queue / sizeof queue[0]) {
                queue[numEvents++] = event;
        }
}

void send_key_event(int keyKind)
{
        struct Event event;
        event.eventKind = EVENT_KEY;
        event.tKey.keyKind = keyKind;
        send_event(event);
}

void send_mousebutton_event(int mousebuttonKind, int mousebuttoneventKind)
{
        struct Event event;
        event.eventKind = EVENT_MOUSEBUTTON;
        event.tMousebutton.mousebuttonKind = mousebuttonKind;
        event.tMousebutton.mousebuttoneventKind = mousebuttoneventKind;
        send_event(event);
}

void send_mousemove_event(int x, int y)
{
        struct Event event;
        event.eventKind = EVENT_MOUSEMOVE;
        event.tMousemove.x = x;
        event.tMousemove.y = y;
        send_event(event);
}

int have_events(void)
{
        return numEvents > 0;
}

void dequeue_event(struct Event *event)
{
        ENSURE(numEvents > 0);
        *event = queue[0];
        numEvents --;
        memmove(queue, queue + 1, numEvents * sizeof *queue);
}
