#ifndef ALLOC_H
#define ALLOC_H

#include "int.h"

extern char _heap;
static char *hbreak = &_heap;

static void *sbrk(size_t size)
{
    void *ptr = hbreak;
    hbreak += size;
    return ptr;
}

#endif
