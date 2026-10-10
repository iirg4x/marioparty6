// Native thread storage and cooperative worker handoffs for C++ minigame tasks.
#ifndef REL_FRAMEWORK_THREAD_H
#define REL_FRAMEWORK_THREAD_H

extern "C" {
#include "dolphin/os.h"
}

// Signature for a thread hook that takes no argument and returns no result.
typedef void (*ThreadProcedure)();

// Carries an embedded OS thread after a leading byte region.
struct ThreadOwner {
    // Leading bytes not accessed through this thread wrapper.
    unsigned char prefixBytes[8];
    // Native thread state returned to OS thread functions.
    OSThread thread;
    OSThread *get() { return &thread; }
};

// Owns a native thread and, when allocated by the wrapper, its stack buffer.
struct NativeThread {
    // OS scheduling state, saved context and stack bounds.
    OSThread thread;
    // Allocated stack to delete; null when the caller supplies the stack buffer.
    unsigned long *storage;
    NativeThread(void *(*entry)(void *), void *argument, unsigned long stackBytes,
                 OSPriority priority, unsigned short flags);
    NativeThread(void *(*entry)(void *), void *argument, void *stackBuffer,
                 unsigned long stackBytes, OSPriority priority, unsigned short flags);
    ~NativeThread();
    void initialize(void *(*entry)(void *), void *argument,
                    unsigned long *stackBuffer, unsigned long stackWords,
                    OSPriority priority, unsigned short flags);
    // Checks the stack guard and returns the byte length of consecutive stack-magic words.
    // Beyond the guard, this estimates unused space only if debug stack filling was enabled.
    unsigned long stackUsage();
    operator OSThread *() { return &thread; }
    static OSPriority currentPriority() {
        return OSGetThreadPriority(OSGetCurrentThread());
    }
};

// Wraps the OS semaphore used to wake a caller waiting for a worker handoff.
struct ThreadSemaphore {
    // Available signals and waiting threads; a zero count makes a wait block.
    OSSemaphore semaphore;
    ThreadSemaphore(int initialCount) { OSInitSemaphore(&semaphore, initialCount); }
    operator OSSemaphore *() { return &semaphore; }
};

// Worker whose run routine can exchange a pointer with its owner while suspended.
struct ThreadEntry {
    static void *entryPoint(void *workerContext);
    virtual ~ThreadEntry();
    // Entered by the OS on the first resume, with that resume's argument.
    virtual void run(void *initialArgument) = 0;
    // Native worker; starts suspended and uses the owner's base priority at creation.
    NativeThread thread;
    // Signaled before worker suspension or entry-point return, releasing the waiting owner.
    ThreadSemaphore handoffSemaphore;
    // Last pointer supplied by either participant; first set by the owner's resume.
    void *exchangedArgument;
    ThreadEntry(unsigned long stackBytes);
    void *resume(void *incomingArgument);
    void *suspend(void *outgoingArgument);
};

#endif
