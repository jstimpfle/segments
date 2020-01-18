#include <segments/logging.h>
#include <segments/memoryalloc.h>
#include <stdlib.h>

void realloc_memory(void **ptr, int numElems, int elemSize)
{
        int numBytes = numElems * elemSize;
        void *p = realloc(*ptr, numBytes);
        if (p == NULL)
                fatal_f("OOM!\n");
        *ptr = p;
}

void alloc_memory(void **ptr, int numElems, int elemSize)
{
        *ptr = NULL;
        realloc_memory(ptr, numElems, elemSize);
}

void free_memory(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}
