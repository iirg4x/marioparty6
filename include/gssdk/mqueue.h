/* TOS queue records and queue operations. */
#ifndef GSSDK_MQUEUE_H
#define GSSDK_MQUEUE_H

#include "types.h"

#include "gssdk/tos.h"

/* One element in the queue's linked list. */
struct _QUEUEELEM {
    /* Next element in enqueue order. */
    struct _QUEUEELEM *pNext;
    /* First four bytes of the payload. Element allocations use the queue's payload size. */
    u8 Data[4];
};

/* Queue configuration and independently tracked reader positions. */
struct _QUEUE {
    /* Queue's identity in the TOS scheduler. */
    struct _OBJECT Object;
    /* Payload size in bytes. */
    u16 Size;
    /* Maximum number of older elements kept for rewinding. */
    u16 MaxJumpBack;
    /* Current queue mode. */
    u8 State;
    /* Number of reader cursors allocated with the queue. */
    u8 NbrReaders;
    /* First stored element. */
    struct _QUEUEELEM *pElements;
    /* Trailing array with one cursor per reader. The queue address marks a reader
       waiting for the next enqueue. The address of pElements marks a disabled reader. */
    struct _QUEUEELEM *pReadPtrs[1];
};

/* Allocates a queue and its reader cursors. */
struct _QUEUE *qQueueConstruct(struct _TOS *pTos, u8 BlockID, u8 NbrReaders);

/* Releases the queue and its stored elements. */
u8 DestructQueue(struct _QUEUE *QuePtr);

/* Changes the queue state or a reader position. */
u8 qQueueControl(struct _QUEUE *Q, u8 ControlID, u32 CParam, u32 DParam);

/* Positions active readers at the requested distance from the end of the queue. */
u8 qQueueJumpBack(struct _QUEUE *Q, u16 JumpBack);

/* Moves one reader back by the requested number of elements. */
u8 qQueueJumpBackOne(struct _QUEUE *Q, u16 ReadPos, u16 JumpBack);

/* Initializes the queue from its TOS profile. */
struct _QUEUE *qQueueInit(struct _QUEUE *Q);

/* Sets the element size, initial reader cursors, and look-back capacity. */
struct _QUEUE *qQueueInitEx(
    struct _QUEUE *Q, u16 NewSize, u8 PassThrough, u16 NumberToJumpBack);

/* Releases the queued elements and changes the payload size. */
struct _QUEUE *qQueueReDimension(struct _QUEUE *Q, u16 NewSize);

/* Copies a batch of elements into the queue and returns its enqueue status. */
u8 qEnQueue(struct _QUEUE *Q, void *pData, u32 NbrOfElements);

/* Appends one element and returns its writable payload. */
void *qEnQueueOne(struct _QUEUE *Q);

/* Returns the next payload for one reader and advances its cursor. */
void *qDeQueueOne(struct _QUEUE *Q, u32 Pos);

/* Frees leading elements until it reaches one referenced by a reader cursor. */
void qFreeUnusedElemements(struct _QUEUE *Q);

/* Returns one payload for each input when all inputs have an element. */
u8 qCheckDeQueueOne(
    const struct _INPUTQUEUE *InputQueues, u32 NbrQueues, void *ppData);

/* Checks whether the input queues are ready for a processing pass. */
u8 qCheckInputQueues(const struct _INPUTQUEUE *InputQueues, u32 NbrQueues);

/* Releases queued elements and resets the reader positions. */
void qQueueReset(struct _QUEUE *Q);

/* Counts the elements stored in a queue. */
u32 qQueueNbrElements(struct _QUEUE *Q);

#endif
