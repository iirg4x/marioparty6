/*
 * Owns native minigame threads and checks the stack space they use.
 */
static const char rcsid[] = "$Id: thread.cpp,v 1.6 2004/02/17 09:22:00 hanamasu Exp $";

#include "REL/framework/thread.h"

extern "C" {
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
}

void initializeThreadStack(
    unsigned long *storage,
    unsigned long count
);

int checkThreadStack(
    unsigned long *begin,
    int mode
);

/* Nonzero fills new thread stacks with a pattern and reports how much of them a thread used. */
unsigned char threadStackDebugEnabled;

void *operator new[](unsigned long);

void operator delete[](void *);

extern "C" int printf(const char *,
    ...);

#define THREAD_STACK_DEBUG_BIT_PATTERN 3735927486UL
#define THREAD_STACK_GUARD_WORD_COUNT 8

/*
 * Called while constructing a worker that lets this wrapper allocate its stack.
 */
CThread::CThread(
                           void *(*entryFunction)(void *),
                           void *entryArgument,
                           unsigned long stackBytes,
                           OSPriority schedulingPriority,
                           unsigned short threadFlags
)
{
    unsigned long stackWords = stackBytes / sizeof(unsigned long);
    // Any trailing bytes smaller than one stack word are discarded.
    unsigned long *stackStorage = new unsigned long[stackWords];

    storage = stackStorage;

    initialize(entryFunction, entryArgument, stackStorage, stackWords, schedulingPriority,
               threadFlags);
}

// Called by an owner that supplies the memory used for the native
// thread's stack.
CThread::CThread(
                           void *(*entryFunction)(void *),
                           void *entryArgument,
                           void *stackBuffer,
                           unsigned long stackBytes,
                           OSPriority schedulingPriority,
                           unsigned short threadFlags
)
{
    storage = 0;

    unsigned long stackWords = stackBytes / sizeof(unsigned long);
    // Any trailing bytes smaller than one stack word are discarded.
    initialize(entryFunction, entryArgument, (unsigned long *)stackBuffer, stackWords,
               schedulingPriority, threadFlags);
}

/*
 * Called by either constructor to prepare the stack and create the suspended OS thread.
 */
void CThread::initialize(
                           void *(*entryFunction)(void *),
                           void *entryArgument,
                              unsigned long *stackStorage,
                              unsigned long stackWords,
                              OSPriority priority,
                              unsigned short threadFlags
)
{
    assert(priority >= OS_PRIORITY_MIN && priority <= OS_PRIORITY_MAX);

    initializeThreadStack(stackStorage,
        stackWords);

    checkThreadStack(stackStorage,
        0);

    bool threadCreated = OSCreateThread(&thread, entryFunction, entryArgument,
                                        stackStorage + stackWords,
                                        stackWords * sizeof(unsigned long), priority,
                                        threadFlags);
    if (!threadCreated)
    {
        OSPanic(__FILE__, __LINE__, "OSCreateThread() failed.");
    }
}

// Cancels a nonterminated thread without joining it, then frees any
// wrapper-owned stack.
CThread::~CThread()
{
    if (!OSIsThreadTerminated(&thread))
    {
        OSCancelThread(&thread);
    }
    if (threadStackDebugEnabled)
    {
        unsigned long totalStackBytes = thread.stackBase -
            reinterpret_cast<unsigned char *>(thread.stackEnd);
        unsigned long unusedStackBytes = stackUsage();
        printf("delete CThread@%p\nstack remain/total = 0x%08x/0x%08x (%f%%)\n", this,
               unusedStackBytes, totalStackBytes, 100.0 * unusedStackBytes / totalStackBytes);
    }
    delete[] storage;
}

// Called after the owner resumes a worker and it suspends or finishes;
// returns untouched stack
// bytes.
unsigned long CThread::stackUsage()
{
    unsigned long *stackStart = thread.stackEnd;
    if (checkThreadStack(stackStart, 1))
    {
        OSPanic(__FILE__, __LINE__, "stack overflow\n");
    }
    unsigned long *stackWord = stackStart;
    unsigned long *stackEnd = reinterpret_cast<unsigned long *>(thread.stackBase);
    for (; stackWord < stackEnd; ++stackWord) {
        if (*stackWord != OS_THREAD_STACK_MAGIC) {
            break;
        }
    }
    return reinterpret_cast<unsigned char *>(stackWord) -
        reinterpret_cast<unsigned char *>(stackStart);
}

// Fills or checks the eight guard words at the low end of a worker stack.
int checkThreadStack(unsigned long *stackBegin, int checkMode) {
    unsigned long *guardEnd = stackBegin + THREAD_STACK_GUARD_WORD_COUNT;
    switch (checkMode) {
    case 0:
        while (stackBegin != guardEnd) {
            *stackBegin++ = OS_THREAD_STACK_MAGIC;
        }
        break;
    case 1:
        while (stackBegin != guardEnd) {
            if (*stackBegin++ != OS_THREAD_STACK_MAGIC) {
                return 1;
            }
        }
        break;
    }
    return 0;
}

// When stack debugging is enabled, marks the full stack so stackUsage can find how far the worker
// has grown it.
void initializeThreadStack(unsigned long *stackStorage, unsigned long stackWordCount) {
    if (threadStackDebugEnabled) {
        unsigned long *stackEnd = stackStorage + stackWordCount;
        while (stackStorage != stackEnd) {
            *stackStorage++ = THREAD_STACK_DEBUG_BIT_PATTERN;
        }
    }
}
