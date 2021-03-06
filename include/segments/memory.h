void realloc_memory(void **ptr, int numElems, int elemSize);
void alloc_memory(void **ptr, int numElems, int elemSize);
void free_memory(void **ptr);

static inline int get_number_of_allocated_elems(void *ptr)
{
        if (!ptr)
                return 0;
        return *(int *) ((char *) ptr - 16);
}

static inline void realloc_memory_tophalf(void **ptr, int numElems, int elemSize)
{
        int cap = get_number_of_allocated_elems(*ptr);
        if (cap < numElems) \
                realloc_memory(ptr, numElems, elemSize);
}

#define REALLOC_MEMORY(ptr, numElems) realloc_memory_tophalf((void**)(ptr), (numElems), sizeof **(ptr))
#define ALLOC_MEMORY(ptr, numElems) alloc_memory((void**)(ptr), (numElems), sizeof **(ptr))
#define FREE_MEMORY(ptr) free_memory((void**)(ptr))

#include <string.h>
#define COPY_MEMORY(dst, src, numElems) copy_memory((dst), (src), (numElems), sizeof *(dst))

static inline void copy_memory(void *dst, const void *src, int numElems, int elemSize)
{
        int numBytes = numElems * elemSize;
        memcpy(dst, src, numBytes);
}
