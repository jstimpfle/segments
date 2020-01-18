#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

#define NORETURN __attribute__((noreturn))

#define ENSURE assert


#define LENGTH(a) (sizeof (a) / sizeof (a)[0])
