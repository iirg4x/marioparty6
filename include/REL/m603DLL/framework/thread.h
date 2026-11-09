#ifndef THREAD_H
#define THREAD_H

#include "dolphin/os.h"

/* Reconstructed native interfaces. Original class and inline accessor names
 * remain unknown. OSThread's natural alignment puts the embedded thread at
 * +8 in ThreadEntry, the semaphore at +0x328 and the argument at +0x334.
 * The allocating constructor owns storage; the external-stack form stores 0. */
class NativeThread {
public:
    OSThread thread;
    unsigned long *storage;
    NativeThread(void *(*)(void *),void *,unsigned long,OSPriority,unsigned short);
    NativeThread(void *(*)(void *),void *,void *,unsigned long,OSPriority,unsigned short);
    ~NativeThread();
    void fn_1_3AB58(void *(*)(void *),void *,unsigned long *,unsigned long,OSPriority,unsigned short);
    unsigned long fn_1_3AD6C();
    operator OSThread *() { return &thread; }
    static OSPriority currentPriority() { return OSGetThreadPriority(OSGetCurrentThread()); }
};

class Semaphore {
public:
    OSSemaphore value;
    Semaphore(int count) { OSInitSemaphore(&value,count); }
    operator OSSemaphore *() { return &value; }
};

class ThreadEntry {
public:
    virtual ~ThreadEntry();
    virtual void run(void *)=0;
    NativeThread thread;
    Semaphore semaphore;
    void *argument;
    ThreadEntry(unsigned long);
    void *resume(void *);
    void *suspend(void *);
};

#endif
