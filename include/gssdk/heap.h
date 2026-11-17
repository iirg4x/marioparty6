#ifndef GSSDK_HEAP_H
#define GSSDK_HEAP_H

#include "types.h"

void *heap_Calloc(void *heap, u32 count, u32 size);
void *heap_Alloc(void *heap, u32 size);
void heap_Free(void *heap, void *ptr);

#endif
