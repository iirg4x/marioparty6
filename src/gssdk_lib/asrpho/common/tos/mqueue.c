#include "types.h"

#include <stddef.h>
#include <string.h>

#include "gssdk/fastallo.h"
#include "gssdk/mqueue.h"

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void *heap_Alloc(void *heap, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern u8 DestructQueue(TosQueue *queue);

enum {
    QUEUE_STATE_ACTIVE = 1,
    QUEUE_STATE_IDLE = 2
};

#define QUEUE_READER_WAITING(queue) ((TosQueueElement *)(queue))
#define QUEUE_READER_DISABLED(queue) \
    ((TosQueueElement *)&(queue)->head)
static inline TosQueueElement *AllocateQueueElement(TosQueue *queue)
{
    u32 size = queue->elementSize + sizeof(TosQueueElement);
    TosQueueElement *element;

    if (size < 0x20) {
        FastAllocator *allocator = &queue->context->queueAllocator;
        u32 words = (size + 3) >> 2;

        element = allocator->freeLists[words];
        if (element != NULL) {
            allocator->freeLists[words] = element->next;
        } else {
            element = fastallo_AllocateMemoryFromChunk(allocator, words);
        }
    } else {
        element = heap_Alloc(queue->context->heap, size);
    }
    return element;
}

static inline void FreeQueueElement(
    TosQueue *queue, TosQueueElement *element)
{
    u32 size = queue->elementSize + sizeof(TosQueueElement);

    if (size < 0x20) {
        void **freeList = &queue->context->queueAllocator.freeLists[(size + 3) >> 2];

        element->next = *freeList;
        *freeList = element;
    } else {
        heap_Free(queue->context->heap, element);
    }
}

static void qUpdateReadPtrsAndIncNbrOfEnqueues(
    TosQueue *queue, TosQueueElement *firstNewElement);

u8 qQueueControl(
    TosQueue *queue, u32 command, u32 value, void *argument)
{
    TosQueueElement *reader;
    u32 i;
    u32 inactiveReaders;
    u8 success = 1;

    switch ((u8)command) {
    case 1:
    case 11:
        queue->state = QUEUE_STATE_ACTIVE;
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = QUEUE_READER_WAITING(queue);
        }
        break;
    case 2:
        queue->state = QUEUE_STATE_ACTIVE;
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = QUEUE_READER_WAITING(queue);
        }
        success = qQueueJumpBack(queue, queue->maxElements);
        break;
    case 3:
        queue->state = QUEUE_STATE_ACTIVE;
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = QUEUE_READER_WAITING(queue);
        }
        if (queue->maxElements != 0) {
            success = qQueueJumpBack(queue, value);
        }
        break;
    case 7:
        queue->state = QUEUE_STATE_ACTIVE;
        reader = queue->readPointers[value];
        if (reader == NULL || reader == QUEUE_READER_DISABLED(queue)) {
            queue->readPointers[value] = QUEUE_READER_WAITING(queue);
            if (queue->maxElements != 0) {
                success = qQueueJumpBackOne(
                    queue, value, (u16)(u32)argument);
            }
        }
        break;
    case 5:
    case 10:
        queue->state = QUEUE_STATE_ACTIVE;
        reader = queue->readPointers[value];
        if (reader == NULL || reader == QUEUE_READER_DISABLED(queue)) {
            queue->readPointers[value] = QUEUE_READER_WAITING(queue);
        }
        break;
    case 4:
        queue->state = QUEUE_STATE_IDLE;
        for (i = 0; i < queue->readerCount; i++) {
            reader = queue->readPointers[i];
            if (reader != NULL &&
                reader != QUEUE_READER_DISABLED(queue)) {
                queue->readPointers[i] = NULL;
            }
        }
        break;
    case 9:
        queue->state = QUEUE_STATE_IDLE;
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = QUEUE_READER_DISABLED(queue);
        }
        break;
    case 6:
        queue->readPointers[value] = NULL;
        for (inactiveReaders = 0; inactiveReaders < queue->readerCount;
             inactiveReaders++) {
            reader = queue->readPointers[inactiveReaders];
            if (reader != NULL && reader != QUEUE_READER_DISABLED(queue)) {
                break;
            }
        }
        if (inactiveReaders == queue->readerCount) {
            queue->state = QUEUE_STATE_IDLE;
        }
        break;
    case 8:
        queue->readPointers[value] = QUEUE_READER_DISABLED(queue);
        for (inactiveReaders = 0; inactiveReaders < queue->readerCount;
             inactiveReaders++) {
            reader = queue->readPointers[inactiveReaders];
            if (reader != NULL && reader != QUEUE_READER_DISABLED(queue)) {
                break;
            }
        }
        if (inactiveReaders == queue->readerCount) {
            queue->state = QUEUE_STATE_IDLE;
        }
        break;
    case 248:
        break;
    case 255:
        if (DestructQueue(queue) != 0) {
            success = 0;
        }
        break;
    default:
        success = 0;
    }
    return success;
}

u8 qQueueJumpBack(TosQueue *queue, u16 count)
{
    TosQueueElement *element;
    u32 elementCount;
    u32 i;

    if (count != 0) {
        elementCount = 0;
        element = queue->head;
        while (element != NULL) {
            element = element->next;
            elementCount++;
        }
        if (elementCount < count) {
            return 0;
        }

        element = queue->head;
        elementCount -= count;
        for (i = 0; i < elementCount; i++) {
            element = element->next;
        }

        for (i = 0; i < queue->readerCount; i++) {
            if (queue->readPointers[i] != QUEUE_READER_DISABLED(queue) &&
                queue->readPointers[i] != NULL) {
                queue->readPointers[i] = element;
            }
        }
        qFreeUnusedElemements(queue);
    }
    return 1;
}

u8 qQueueJumpBackOne(TosQueue *queue, u16 reader, u16 count)
{
    TosQueueElement *element;
    u32 elementCount;
    u32 i;

    if (count != 0) {
        elementCount = 0;
        element = queue->head;
        while (element != NULL) {
            element = element->next;
            elementCount++;
        }
        if (elementCount < count) {
            return 0;
        }

        element = queue->head;
        elementCount -= count;
        for (i = 0; i < elementCount; i++) {
            element = element->next;
        }
        queue->readPointers[reader] = element;
    }
    return 1;
}

TosQueue *qQueueInit(TosQueue *queue)
{
    u32 profileElementSize;
    u32 initializeReaders;
    u32 i;

    profileElementSize = _tosGetProfileU32(queue, 2, -1);
    if (profileElementSize != (u32)-1 &&
        profileElementSize != queue->elementSize) {
        return NULL;
    }

    qQueueReset(queue);
    initializeReaders = _tosGetProfileU32(queue, 3, 1);
    if ((u8)initializeReaders != 0) {
        for (i = 0; i < queue->readerCount; i++) {
            if (queue->readPointers[i] != QUEUE_READER_DISABLED(queue)) {
                queue->readPointers[i] = QUEUE_READER_WAITING(queue);
            }
        }
    } else {
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = NULL;
        }
    }
    queue->maxElements = (u8)_tosGetProfileU32(queue, 4, 0);
    return queue;
}

TosQueue *qQueueInitEx(
    TosQueue *queue, u16 elementSize, u8 initializeReaders,
    u16 maxElements)
{
    u32 i;

    queue->elementSize = elementSize != 0
        ? elementSize : (u16)_tosGetProfileU32(queue, 2, -1);
    qQueueReset(queue);

    if (initializeReaders == 0xFF) {
        initializeReaders = (u8)_tosGetProfileU32(queue, 3, 1);
    }
    if (initializeReaders != 0) {
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = QUEUE_READER_WAITING(queue);
        }
    } else {
        for (i = 0; i < queue->readerCount; i++) {
            queue->readPointers[i] = NULL;
        }
    }

    if (maxElements != 0xFFFF) {
        queue->maxElements = maxElements;
    } else {
        queue->maxElements = _tosGetProfileU32(queue, 4, 0);
    }
    return queue;
}

TosQueue *qQueueReDimension(TosQueue *queue, u16 elementSize)
{
    TosQueueElement *element;
    TosQueueElement *old;
    u16 newElementSize;

    element = queue->head;
    newElementSize = elementSize != 0
        ? elementSize : (u16)_tosGetProfileU32(queue, 2, -1);
    while (element != NULL) {
        old = element;
        element = element->next;
        FreeQueueElement(queue, old);
    }
    queue->head = NULL;
    queue->elementSize = newElementSize;
    return queue;
}

static void qUpdateReadPtrsAndIncNbrOfEnqueues(
    TosQueue *queue, TosQueueElement *firstNewElement)
{
    TosQueueElement *element;
    TosQueueElement *next;
    u32 elementCount;
    u32 i;
    u32 reader;

    if (queue->maxElements != 0) {
        {
            TosQueueElement *scan;
            for (scan = element = queue->head, elementCount = 0;
                 scan != NULL; elementCount++) {
                scan = scan->next;
            }
        }

        if (elementCount > queue->maxElements) {
            elementCount -= queue->maxElements;
            for (i = 0; i < elementCount; i++) {
                next = element;
                for (reader = 0; reader < queue->readerCount; reader++) {
                    if (queue->readPointers[reader] == element) {
                        break;
                    }
                }
                if (reader < queue->readerCount) {
                    break;
                }
                element = element->next;
                FreeQueueElement(queue, next);
            }
            queue->head = element;
        }
    }

    for (i = 0; i < queue->readerCount; i++) {
        if (queue->readPointers[i] == QUEUE_READER_WAITING(queue)) {
            queue->readPointers[i] = firstNewElement;
        }
    }
    queue->context->enqueuesThisPass++;
}

u32 qEnQueue(TosQueue *queue, void *elements, u32 count)
{
    TosQueueElement *element;
    TosQueueElement *tail;
    TosQueueElement *firstNewElement = NULL;
    u8 *source = (u8 *)elements;
    u32 i;

    if (queue->state == QUEUE_STATE_IDLE && queue->maxElements == 0) {
        return 0;
    }

    tail = queue->head;
    if (tail != NULL) {
        while (tail->next != NULL) {
            tail = tail->next;
        }
    }

    for (i = 0; i < count; i++) {
        element = AllocateQueueElement(queue);
        if (firstNewElement == NULL) {
            firstNewElement = element;
        }
        if (tail != NULL) {
            tail->next = element;
        } else {
            queue->head = element;
        }
        tail = element;
        memcpy(element->data, source, queue->elementSize);
        source += queue->elementSize;
    }
    if (tail != NULL) {
        tail->next = NULL;
    }
    qUpdateReadPtrsAndIncNbrOfEnqueues(queue, firstNewElement);
    return 1;
}

void *qEnQueueOne(TosQueue *queue)
{
    TosQueueElement *element;
    TosQueueElement *tail;

    if (queue->state == QUEUE_STATE_IDLE && queue->maxElements == 0) {
        return NULL;
    }

    element = AllocateQueueElement(queue);
    tail = queue->head;
    if (tail != NULL) {
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = element;
    } else {
        queue->head = element;
    }
    element->next = NULL;
    qUpdateReadPtrsAndIncNbrOfEnqueues(queue, element);
    return element->data;
}

void *qDeQueueOne(TosQueue *queue, u32 reader)
{
    TosQueueElement *element = queue->readPointers[reader];

    if (element == (TosQueueElement *)&queue->head || element == NULL ||
        element == (TosQueueElement *)queue) {
        return NULL;
    }

    if (element->next != NULL) {
        queue->readPointers[reader] = element->next;
    } else {
        queue->readPointers[reader] = (TosQueueElement *)queue;
    }
    return element->data;
}

void qFreeUnusedElemements(TosQueue *queue)
{
    TosQueueElement *element = queue->head;
    TosQueueElement *old;
    u32 reader;

    if (element == NULL) {
        return;
    }

    if (queue->maxElements != 0) {
        for (reader = 0; reader < queue->readerCount; reader++) {
            if (queue->readPointers[reader] !=
                    QUEUE_READER_DISABLED(queue) &&
                queue->readPointers[reader] == NULL) {
                return;
            }
        }
    }

    while (element != NULL) {
        for (reader = 0; reader < queue->readerCount; reader++) {
            if (queue->readPointers[reader] == element) {
                break;
            }
        }
        if (reader != queue->readerCount) {
            break;
        }
        old = element;
        element = element->next;
        FreeQueueElement(queue, old);
    }
    if (element == NULL) {
        queue->head = NULL;
    } else {
        queue->head = element;
    }
}

u8 qCheckDeQueueOne(
    TosQueuePort *inputs, u32 inputCount, void **elements)
{
    u32 i;

    if (qCheckInputQueues(inputs, inputCount) == 0) {
        memset(elements, 0, inputCount * sizeof(void *));
        return 0;
    }

    for (i = 0; i < inputCount; i++) {
        void *element = NULL;

        if (inputs->queue != NULL) {
            TosQueue *queue = inputs->queue;
            TosQueueElement **readPointer = &queue->readPointers[inputs->outputSize];
            TosQueueElement *node = *readPointer;

            if (node == QUEUE_READER_DISABLED(queue) || node == NULL ||
                node == QUEUE_READER_WAITING(queue)) {
                element = NULL;
            } else {
                if (node->next != NULL) {
                    *readPointer = node->next;
                } else {
                    *readPointer = QUEUE_READER_WAITING(queue);
                }
                element = node->data;
            }
        }
        *elements++ = element;
        inputs++;
    }
    return 1;
}

u8 qCheckInputQueues(TosQueuePort *inputs, u32 inputCount)
{
    TosQueuePort *end = inputs + inputCount;
    TosQueue *queue;
    TosQueueElement *reader;
    u8 waiting = 1;

    for (; inputs < end; inputs++) {
        queue = inputs->queue;
        if (queue != NULL) {
            reader = queue->readPointers[inputs->outputSize];
            if (reader != NULL && reader != QUEUE_READER_DISABLED(queue)) {
                waiting = queue->readPointers[inputs->outputSize] == QUEUE_READER_WAITING(queue);
                break;
            }
        }
    }

    for (inputs++; inputs < end; inputs++) {
        queue = inputs->queue;
        if (queue != NULL) {
            reader = queue->readPointers[inputs->outputSize];
            if (reader != NULL && reader != QUEUE_READER_DISABLED(queue) &&
                waiting != (u8)(reader == QUEUE_READER_WAITING(queue))) {
                break;
            }
        }
    }
    if (inputs != end) {
        return 0;
    }
    return !waiting;
}

void qQueueReset(TosQueue *queue)
{
    TosQueueElement *element;
    TosQueueElement *old;
    u32 i;

    element = queue->head;
    while (element != NULL) {
        old = element;
        element = element->next;
        FreeQueueElement(queue, old);
    }

    for (i = 0; i < queue->readerCount; i++) {
        if (queue->readPointers[i] != QUEUE_READER_DISABLED(queue)) {
            queue->readPointers[i] = QUEUE_READER_WAITING(queue);
        }
    }
    queue->head = NULL;
    queue->state = QUEUE_STATE_ACTIVE;
}

u32 qQueueNbrElements(TosQueue *queue)
{
    TosQueueElement *element = queue->head;
    u32 count = 0;

    while (element != NULL) {
        element = element->next;
        count++;
    }
    return count;
}

TosQueue *qQueueConstruct(
    TosContext *context, u8 queueIndex, u8 readerCount)
{
    TosQueue *queue;

    queue = heap_Calloc(
        context->heap, 1,
        sizeof(TosQueue) + (readerCount - 1) * sizeof(TosQueueElement *));
    if (queue == NULL) {
        struct TosQueueProfile {
            TosContext *context;
            u8 queueIndex;
        } profile;

        profile.context = context;
        profile.queueIndex = queueIndex;
        _tosErrorLog(&profile, 2);
        return NULL;
    }

    queue->readerCount = readerCount;
    queue->queueIndex = queueIndex;
    queue->context = context;
    return queue;
}
