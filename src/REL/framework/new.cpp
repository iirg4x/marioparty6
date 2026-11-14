// C++ objects and arrays use the minigame framework's shared memory manager.
/* Retail fixes the next assert's line; the earlier lines are unknown: 14 lines are left blank. */














static const char rcsid[] = "$Id: new.cpp,v 1.4 2004/08/12 01:32:17 saf Exp $";

#include <stddef.h>
#include "REL/framework/heap.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

// C++ new expressions request storage in bytes, creating the manager on first use.
void *operator new(unsigned long byteCount) {
    void *ptr;
    if (!g_memman) {
        initializeMemoryManager();
    }
    ptr = g_memman->allocate(byteCount);
    // Exhaustion reports the assertion and aborts instead of returning null.
    assert(ptr != NULL);
    return ptr;
}
// Array new delegates to scalar new; scalar delete releases storage through the existing manager.
void *operator new[](unsigned long byteCount) { return ::operator new(byteCount); }
void operator delete(void *allocation) {
    assert(g_memman != NULL);
    g_memman->release(allocation);
}

// C++ delete[] expressions return array storage through the scalar delete path.
void operator delete[](void *allocation) { ::operator delete(allocation); }
