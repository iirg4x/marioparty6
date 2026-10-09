/* Manages the game's scheduled processes, their child trees, and per-process heaps. */
#include "game/process.h"
#include "game/memory.h"
#include "dolphin/os.h"

#define FAKE_RETADDR 0xA5A5A5A5
#define DEFAULT_STACK_SIZE 4096
#define PROCESS_HEAP_VALIDITY_MARKER 165

static jmp_buf processjmpbuf;
static HUPROCESS *processtop;
static HUPROCESS *processcur;
static u16 processcnt;
u32 procfunc;

/* Called during game startup to clear the scheduler's count and active-process list. */
void HuPrcInit(void)
{
    processcnt = 0;
    processtop = NULL;
}

/* HuPrcCreate calls this to insert a process; equal priorities retain creation order. */
static void LinkProcess(HUPROCESS **processList, HUPROCESS *newProcess) {
    HUPROCESS *currentProcess = *processList;

    if (currentProcess && (currentProcess->prio >= newProcess->prio)) {
        while (currentProcess->next && currentProcess->next->prio >= newProcess->prio) {
            currentProcess = currentProcess->next;
        }

        newProcess->next = currentProcess->next;
        newProcess->prev = currentProcess;
        currentProcess->next = newProcess;
        if (newProcess->next) {
            newProcess->next->prev = newProcess;
        }
    } else {
        newProcess->next = (*processList);
        newProcess->prev = NULL;
        *processList = newProcess;
        if (currentProcess) {
            currentProcess->prev = newProcess;
        }
    }
}
/* gcTerminateProcess calls this to remove a process from the scheduler list. */
static void UnlinkProcess(HUPROCESS **processList, HUPROCESS *targetProcess) {
    if (targetProcess->next) {
        targetProcess->next->prev = targetProcess->prev;
    }
    if (targetProcess->prev) {
        targetProcess->prev->next = targetProcess->next;
    } else {
        *processList = targetProcess->next;
    }
}

/* Game systems call this to allocate a process, its stack, and its private heap. */
HUPROCESS *HuPrcCreate(void (*entryFunc)(void), u16 priority, u32 stackBytes, s32 extraHeapBytes)
{
    HUPROCESS *process;
    s32 allocationBytes;
    void *processHeap;
    /* A zero stack size selects DEFAULT_STACK_SIZE (4096 bytes). */
    if(stackBytes == 0) {
        stackBytes = DEFAULT_STACK_SIZE;
    }
    allocationBytes = HuMemMemoryAllocSizeGet(sizeof(HUPROCESS))
                    +HuMemMemoryAllocSizeGet(stackBytes)
                    +HuMemMemoryAllocSizeGet(extraHeapBytes);
    if(!(processHeap = HuMemDirectMalloc(HEAP_HEAP, allocationBytes))) {
        OSReport("process> malloc error size %d\n", allocationBytes);
        return NULL;
    }
    HuMemHeapInit(processHeap, allocationBytes);
    process = HuMemMemoryAlloc(processHeap, sizeof(HUPROCESS), FAKE_RETADDR);
    process->heap = processHeap;
    process->exec = HUPRC_EXEC_NORMAL;
    process->stat = 0;
    process->prio = priority;
    process->sleep = 0;
    /* The saved stack pointer starts eight bytes below the allocated stack's end. */
    process->spBase = ((u32)HuMemMemoryAlloc(processHeap, stackBytes, FAKE_RETADDR))+stackBytes-8;
    gcsetjmp(&process->jump);
    /* Set the saved link register to entryFunc so the scheduler's first resume starts the process
     * there. */
    process->jump.lr = (u32)entryFunc;
    process->jump.sp = process->spBase;
    process->destructor = NULL;
    process->property = NULL;
    LinkProcess(&processtop, process);
    process->child = NULL;
    process->parent = NULL;
    processcnt++;
    return process;
}

/* HuPrcChildCreate uses this to attach a process beneath its parent in the child list. */
void HuPrcChildLink(HUPROCESS *parentProcess, HUPROCESS *childProcess)
{
    HuPrcChildUnlink(childProcess);
    if(parentProcess->child) {
        parentProcess->child->firstChild = childProcess;
    }
    /* nextChild points toward older siblings; firstChild links back toward newer ones. */
    childProcess->nextChild = parentProcess->child;
    childProcess->firstChild = NULL;
    parentProcess->child = childProcess;
    childProcess->parent = parentProcess;
}

/* HuPrcKill, HuPrcEnd, and HuPrcChildLink use this to detach a process from its parent. */
void HuPrcChildUnlink(HUPROCESS *childProcess)
{
    if(childProcess->parent) {
        if(childProcess->nextChild) {
            childProcess->nextChild->firstChild = childProcess->firstChild;
        }
        if(childProcess->firstChild) {
            childProcess->firstChild->nextChild = childProcess->nextChild;
        } else {
            childProcess->parent->child = childProcess->nextChild;
        }
        childProcess->parent = NULL;
    }
}

/* Game systems call this to create a scheduled process beneath a parent. */
HUPROCESS *HuPrcChildCreate(void (*entryFunc)(void), u16 priority, u32 stackBytes,
                            s32 extraHeapBytes, HUPROCESS *parent)
{
    HUPROCESS *child = HuPrcCreate(entryFunc, priority, stackBytes, extraHeapBytes);
    /* Allocation failure is passed to the link operation without a NULL check. */
    HuPrcChildLink(parent, child);
    return child;
}

/* Yields until this process has no children left in its child list. */
void HuPrcChildWatch()
{
    HUPROCESS *currentProcess = HuPrcCurrentGet();
    if(currentProcess->child) {
        currentProcess->exec = HUPRC_EXEC_CHILDWATCH;
        if(!gcsetjmp(&currentProcess->jump)) {
            gclongjmp(&processjmpbuf, 1);
        }
    }
}

/* Process code calls this to get the process currently running in the scheduler. */
HUPROCESS *HuPrcCurrentGet()
{
    return processcur;
}

/* HuPrcKill and HuPrcChildKill use this to clear sleep before marking a process killed. */
static s32 SetKillStatusProcess(HUPROCESS *process)
{
    if(process->exec != HUPRC_EXEC_KILLED) {
        HuPrcWakeup(process);
        process->exec = HUPRC_EXEC_KILLED;
        return 0;
    } else {
        return -1;
    }
}

/* Game code uses this to kill a process and its descendants; NULL selects the current process. */
s32 HuPrcKill(HUPROCESS *process)
{
    if(process == NULL) {
        process = HuPrcCurrentGet();
    }
    HuPrcChildKill(process);
    HuPrcChildUnlink(process);
    return SetKillStatusProcess(process);
}

/* HuPrcKill and HuPrcEnd call this to mark descendants killed and clear the process's child-list
 * head. */
void HuPrcChildKill(HUPROCESS *process)
{
    HUPROCESS *childProcess = process->child;
    while(childProcess) {
        if(childProcess->child) {
            HuPrcChildKill(childProcess);
        }
        SetKillStatusProcess(childProcess);
        childProcess = childProcess->nextChild;
    }
    process->child = NULL;
}

/* HuPrcEnd calls this to run cleanup, remove the process, and return to the scheduler. */
static void gcTerminateProcess(HUPROCESS *process)
{
    if(process->destructor) {
        process->destructor();
    }
    UnlinkProcess(&processtop, process);
    processcnt--;
    gclongjmp(&processjmpbuf, 2);
}

/* A running process enters here, or the scheduler redirects a killed process's saved link register
 * here, to kill children, detach the process, and terminate. */
void HuPrcEnd()
{
    HUPROCESS *process = HuPrcCurrentGet();
    HuPrcChildKill(process);
    HuPrcChildUnlink(process);
    gcTerminateProcess(process);
}

/* A nonzero argument sets sleep ticks unless the process is already killed; positive ticks count
 * down, while negative ticks sleep indefinitely. Zero only yields. */
void HuPrcSleep(s32 sleepTicks)
{
    HUPROCESS *process = HuPrcCurrentGet();
    if(sleepTicks != 0 && process->exec != HUPRC_EXEC_KILLED) {
        process->exec = HUPRC_EXEC_SLEEP;
        process->sleep = sleepTicks;
    }
    if(!gcsetjmp(&process->jump)) {
        gclongjmp(&processjmpbuf, 1);
    }
}

/* Yields for one scheduler pass without setting a positive sleep countdown. */
void HuPrcVSleep()
{
    HuPrcSleep(0);
}

/* Clears the countdown; SetKillStatusProcess calls this before switching to killed state. */
void HuPrcWakeup(HUPROCESS *process)
{
    process->sleep = 0;
}

/* Process owners call this to set cleanup that runs when the supplied process ends. */
void HuPrcDestructorSet2(HUPROCESS *targetProcess, void (*destructorFunc)(void))
{
    targetProcess->destructor = destructorFunc;
}

/* A process calls this to set cleanup that runs when it ends. */
void HuPrcDestructorSet(void (*destructorFunc)(void))
{
    HUPROCESS *currentProcess = HuPrcCurrentGet();
    currentProcess->destructor = destructorFunc;
}

/* main.c calls this once per game loop to resume processes and advance their sleep timers. */
void HuPrcCall(s32 schedulerTicks)
{
    HUPROCESS *scheduledProcess;
    s32 dispatchResult;
    processcur = processtop;
    dispatchResult = gcsetjmp(&processjmpbuf);
    while(1) {
        switch(dispatchResult) {
            case 2:
                /* The process ended; release its outer heap before advancing the scheduler. */
                HuMemDirectFree(processcur->heap);
            case 1:
                /* The allocator marks a live heap block with byte 0xA5 at this offset. */
                if(((u8 *)(processcur->heap))[4] != PROCESS_HEAP_VALIDITY_MARKER) {
                    printf("stack overlap error.(process pointer %x)\n", processcur);
                    while(1);
                } else {
                    processcur = processcur->next;
                }
                break;
        }
        scheduledProcess = processcur;
        if(!scheduledProcess) {
            return;
        }
        procfunc = scheduledProcess->jump.lr;
        if ((scheduledProcess->stat & (HU_PRC_STAT_PAUSE | HU_PRC_STAT_UPAUSE)) &&
            scheduledProcess->exec != HUPRC_EXEC_KILLED) {
            /* Paused processes are skipped, while killed processes must still reach HuPrcEnd. */
            dispatchResult = 1;
            continue;
        }
        switch(scheduledProcess->exec) {
            case HUPRC_EXEC_SLEEP:
                if(scheduledProcess->sleep > 0) {
                    scheduledProcess->sleep -= schedulerTicks;
                    if(scheduledProcess->sleep <= 0) {
                        /* Clamp an expired countdown so the process resumes with zero sleep. */
                        scheduledProcess->sleep = 0;
                        scheduledProcess->exec = HUPRC_EXEC_NORMAL;
                    }
                }
                dispatchResult = 1;
                break;

            case HUPRC_EXEC_CHILDWATCH:
                if(scheduledProcess->child) {
                    dispatchResult = 1;
                } else {
                    scheduledProcess->exec = HUPRC_EXEC_NORMAL;
                    dispatchResult = 0;
                }
                break;

            case HUPRC_EXEC_KILLED:
                /* Replace the saved entry point so the resumed process terminates through
                 * HuPrcEnd. */
                scheduledProcess->jump.lr = (u32)HuPrcEnd;
            case HUPRC_EXEC_NORMAL:
                gclongjmp(&scheduledProcess->jump, 1);
                break;
        }
    }
}

/* Allocates memory from the calling process's private heap. */
void *HuPrcMemAlloc(s32 allocationBytes)
{
    HUPROCESS *process = HuPrcCurrentGet();
    return HuMemMemoryAlloc(process->heap, allocationBytes, FAKE_RETADDR);
}

/* Process owners call this to free a block allocated from the process's private heap. */
void HuPrcMemFree(void *allocation)
{
    HuMemMemoryFree(allocation, FAKE_RETADDR);
}

/* Sets the selected pause or process status bits. */
void HuPrcSetStat(HUPROCESS *process, u16 statusMask)
{
    process->stat |= statusMask;
}

/* Clears the selected pause or process status bits. */
void HuPrcResetStat(HUPROCESS *process, u16 statusMask)
{
    process->stat &= ~statusMask;
}

/* Pause controls call this to pause processes that have not opted out, or clear the pause bit. */
void HuPrcAllPause(s32 enablePause)
{
    HUPROCESS *scheduledProcess = processtop;
    if(enablePause) {
        while(scheduledProcess != NULL) {
            if(!(scheduledProcess->stat & HU_PRC_STAT_PAUSE_ON)) {
                HuPrcSetStat(scheduledProcess, HU_PRC_STAT_PAUSE);
            }

            scheduledProcess = scheduledProcess->next;
        }
    } else {
        while(scheduledProcess != NULL) {
            if(scheduledProcess->stat & HU_PRC_STAT_PAUSE) {
                HuPrcResetStat(scheduledProcess, HU_PRC_STAT_PAUSE);
            }

            scheduledProcess = scheduledProcess->next;
        }
    }
}

/* Pause controls call this to set or clear the independent UPAUSE state. */
void HuPrcAllUPause(s32 enableUPause)
{
    HUPROCESS *scheduledProcess = processtop;
    if(enableUPause) {
        while(scheduledProcess != NULL) {
            if(!(scheduledProcess->stat & HU_PRC_STAT_UPAUSE_ON)) {
                HuPrcSetStat(scheduledProcess, HU_PRC_STAT_UPAUSE);
            }

            scheduledProcess = scheduledProcess->next;
        }
    } else {
        while(scheduledProcess != NULL) {
            if(scheduledProcess->stat & HU_PRC_STAT_UPAUSE) {
                HuPrcResetStat(scheduledProcess, HU_PRC_STAT_UPAUSE);
            }

            scheduledProcess = scheduledProcess->next;
        }
    }
}
