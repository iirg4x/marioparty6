// Cooperative worker threads that exchange arguments with their C++ minigame callers.
static const char rcsid[] = "$Id: subthread.cpp,v 1.6 2004/02/17 09:22:00 hanamasu Exp $";

#include "REL/framework/thread.h"

extern "C" {
#include "REL/framework/assertion.h"
}

void operator delete(void *);

// Called by MinigameCore during construction; this thread hook has no effect.
void emptyThreadHook() {}

// Worker construction uses the calling thread's base scheduling priority.
OSPriority queryCurrentThreadPriority()
{
    return OSGetThreadPriority(OSGetCurrentThread());
}

// Creates a suspended, detached worker at the caller's priority with no pending handoff.
// The first resume supplies its initial argument before the entry point runs.
ThreadEntry::ThreadEntry(unsigned long stackBytes)
    : thread(entryPoint, this, stackBytes, queryCurrentThreadPriority(), OS_THREAD_ATTR_DETACH),
      handoffSemaphore(0) {}

// When the owner deletes the worker, NativeThread cancels it if needed and frees its stack.
ThreadEntry::~ThreadEntry() {}

// Reports whether the worker's native thread is terminated.
bool threadHasTerminated(ThreadEntry *worker)
{
    return (bool)OSIsThreadTerminated(worker->thread);
}

// Called by the owner to pass an argument and wait for the worker's handoff signal.
// The worker signals before suspending or returning from its entry point.
// Returns the last exchanged argument immediately if the worker is already terminated.
void *ThreadEntry::resume(void *incomingArgument)
{
    if (OSIsThreadTerminated(thread)) {
        return exchangedArgument;
    }
    exchangedArgument = incomingArgument;
    OSResumeThread(thread);
    OSWaitSemaphore(handoffSemaphore);
    // The unused stack count is discarded, but the stack and OS thread checks still run.
    thread.stackUsage();
    OSCheckActiveThreads();
    return exchangedArgument;
}

// Called from the worker's run routine to hand an argument back and suspend itself.
// After the owner resumes it, returns the new argument supplied by that owner.
void *ThreadEntry::suspend(void *outgoingArgument)
{
    OSGetCurrentThread() == thread ? (void)0 :
        __msl_assertion_failed("::OSGetCurrentThread() == m_th", "subthread.cpp", 77);
    !OSIsThreadTerminated(thread) ? (void)0 :
        __msl_assertion_failed("!::OSIsThreadTerminated(m_th)", "subthread.cpp", 80);
    exchangedArgument = outgoingArgument;
    OSSignalSemaphore(handoffSemaphore);
    OSSuspendThread(thread);
    return exchangedArgument;
}

// The OS calls this on the first resume to run the worker's virtual routine.
// Finishing wakes the owner without replacing the last exchanged argument.
void *ThreadEntry::entryPoint(void *workerContext)
{
    ThreadEntry *worker = (ThreadEntry *)workerContext;
    worker->run(worker->exchangedArgument);
    OSSignalSemaphore(worker->handoffSemaphore);
    return 0;
}
