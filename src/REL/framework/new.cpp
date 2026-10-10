// C++ objects and arrays use the minigame framework's shared memory manager.
static const char rcsid[] = "$Id: new.cpp,v 1.4 2004/08/12 01:32:17 saf Exp $";

#include "REL/framework/heap.h"

extern "C" {
#include "REL/framework/assertion.h"
}

// C++ new expressions request object storage in bytes, creating the manager on first use.
void *operator new(unsigned long byteCount)
{
    void *allocation;
    if (!g_memman) {
        initializeMemoryManager();
    }
    allocation = g_memman->allocate(byteCount);
    // Exhaustion reports the assertion and aborts instead of returning null.
    allocation != 0 ? (void)0 : __msl_assertion_failed("ptr != NULL", "new.cpp", 32);
    return allocation;
}

// C++ new[] expressions request array storage in bytes through the same shared manager.
void *operator new[](unsigned long byteCount)
{
    void *allocation;
    if (!g_memman) {
        initializeMemoryManager();
    }
    allocation = g_memman->allocate(byteCount);
    // Array allocation has the same abort-on-exhaustion behavior as object allocation.
    allocation != 0 ? (void)0 : __msl_assertion_failed("ptr != NULL", "new.cpp", 32);
    return allocation;
}

// C++ delete expressions return object storage after destruction; the manager must exist.
void operator delete(void *allocation)
{
    g_memman != 0 ? (void)0 : __msl_assertion_failed("g_memman != NULL", "new.cpp", 38);
    // Only the manager is checked here; the allocation pointer is forwarded unchanged.
    g_memman->release(allocation);
}

// C++ delete[] expressions return array storage after destroying its elements.
void operator delete[](void *allocation)
{
    g_memman != 0 ? (void)0 : __msl_assertion_failed("g_memman != NULL", "new.cpp", 38);
    // Only the manager is checked here; the allocation pointer is forwarded unchanged.
    g_memman->release(allocation);
}
