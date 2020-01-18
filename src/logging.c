#include <segments/logging.h>
#include <stdio.h>
#include <stdlib.h>

void message_fv(const char *fmt, va_list ap)
{
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
}

void message_f(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        message_fv(fmt, ap);
        va_end(ap);
}

void fatal_f(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        message_fv(fmt, ap);
        va_end(ap);
        abort();
}
