/* Constructs and runs the recognizer processing graph and its queues. */
#include "types.h"

#include <string.h>

#include "gssdk/heap.h"
#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Verifies that each queue has a consistent element size across its connected ports. */
static u8 SetAndControlQSizes(struct _TOS *This)
{
    unsigned long q;
    unsigned long p;
    struct _QUEUE *QuePtr;
    unsigned char FoundOne;
    unsigned short Size;
    unsigned long pq;
    struct _BLOCK *BlPtr;
    struct _INPUTQUEUE *pInQ;
    struct _OUTPUTQUEUE *pOutQ;

    for (q = 0; q < This->NrQueues; q++) {
        QuePtr = This->AllQueues[q];
        if (QuePtr == NULL) {
            continue;
        }
        Size = 0xFFFF;
        FoundOne = 0;
        for (p = 0; p < This->NrProces; p++) {
            BlPtr = This->AllProcesses[p];
            if (BlPtr == NULL) {
                continue;
            }
            for (pq = 0; pq < BlPtr->NrInputs; pq++) {
                pInQ = &BlPtr->InputQueues[pq];
                if (pInQ->pQ == QuePtr) {
                    if (FoundOne) {
                        if (pInQ->Size != Size) {
                            return 1;
                        }
                    } else {
                        FoundOne = 1;
                        Size = pInQ->Size;
                    }
                }
            }
            for (pq = 0; pq < BlPtr->NrOutputs; pq++) {
                pOutQ = &BlPtr->OutputQueues[pq];
                if (pOutQ->pQ == QuePtr) {
                    if (FoundOne) {
                        if (pOutQ->Size != Size) {
                            return 1;
                        }
                    } else {
                        FoundOne = 1;
                        Size = pOutQ->Size;
                    }
                }
            }
        }
        if (!FoundOne) {
            return 1;
        }
        QuePtr->Size = Size;
    }
    return 0;
}

/* Attaches a queue reader to a block input or to the architecture callback. */
static u8 ConnectInQtoBlock(
    struct _TOS *This, struct _BLOCK *BlPtr, struct _QUEUE *QPtr,
    u16 ReadPos)
{
    s8 inputNumber;

    if (BlPtr != NULL) {
        /* Block ID zero uses the scheduler address and architecture callbacks. */
        if (BlPtr == (struct _BLOCK *)This) {
            if (This->TosData.cbRegInput(
                    This, QPtr, ReadPos) != 0) {
                return 1;
            }
        } else {
            for (inputNumber = 0; inputNumber < BlPtr->NrInputs;
                 inputNumber++) {
                if (BlPtr->InputQueues[inputNumber].pQ == NULL) {
                    BlPtr->InputQueues[inputNumber].pQ = QPtr;
                    BlPtr->InputQueues[inputNumber].ReadPos = ReadPos;
                    break;
                }
            }
            if (QPtr != NULL &&
                qQueueControl(QPtr, 0xA, ReadPos, 0) == 0) {
                return 1;
            }
        }
    } else if (QPtr != NULL &&
               qQueueControl(QPtr, 0x8, ReadPos, 0) == 0) {
        return 1;
    }
    return 0;
}

/* Connects the writer to the requested output port. */
static u8 ConnectOutQtoBlockPos(
    struct _TOS *This, struct _BLOCK *BlPtr, struct _QUEUE *QPtr, u8 OutPos)
{
    if (BlPtr == (struct _BLOCK *)This) {
        if (This->TosData.cbRegOutput(This, QPtr) != 0) {
            return 1;
        }
    } else if (BlPtr != NULL && QPtr != NULL) {
        BlPtr->OutputQueues[OutPos].pQ = QPtr;
        if (QPtr->Size == 0) {
            QPtr->Size = BlPtr->OutputQueues[OutPos].Size;
        }
    }
    return 0;
}

/* Connects a reader at the requested input port and read position. */
static u8 ConnectInQtoBlockPos(
    struct _TOS *This, struct _BLOCK *BlPtr, struct _QUEUE *QPtr,
    u16 ReadPos, u16 QueInPos)
{
    if (BlPtr != NULL) {
        if (BlPtr == (struct _BLOCK *)This) {
            if (This->TosData.cbRegInput(This, QPtr, ReadPos) != 0) {
                return 1;
            }
        } else {
            BlPtr->InputQueues[QueInPos].pQ = QPtr;
            BlPtr->InputQueues[QueInPos].ReadPos = ReadPos;
            if (QPtr != NULL &&
                qQueueControl(QPtr, 0xA, ReadPos, 0) == 0) {
                return 1;
            }
        }
    } else if (QPtr != NULL &&
               qQueueControl(QPtr, 0x8, ReadPos, 0) == 0) {
        return 1;
    }
    return 0;
}

/* Constructs a block, connects its existing queues and calls its initializer. */
static u8 ConstructBlock(struct _TOS *This, u8 BlockID, int Index)
{
    const struct _PROCDESCRIP *Pd;
    const struct _QDESCRIP *Qd;
    const struct _QTARGETDESCRIP *Qt;
    struct _BLOCK *BlkPtr;
    u8 *pNbrReaders;
    int WritePos;
    int QueOutPos;
    u8 QIdx;
    int ReadPos;
    int i;

    i = 0;
    Pd = This->pInitialBlocks;
    while (Pd->BlockID != BlockID) {
        Pd++;
    }
    BlkPtr = Pd->Constructor(This, BlockID);
    if (BlkPtr == NULL) {
        return 1;
    }
    This->AllProcesses[Index] = BlkPtr;
    QueOutPos = 0;
    Qd = This->pInitialQueues;
    while (Qd->BlockID != 0xFF) {
        if (Qd->SourceID == BlockID) {
            if (ConnectOutQtoBlockPos(This, BlkPtr,
                    tosGetQueuePtr(This, Qd->BlockID), QueOutPos++) != 0) {
                return 1;
            }
        }
        Qd++;
    }
    pNbrReaders = heap_Calloc(This->hHeap, This->NrQueues, sizeof(u8));
    if (pNbrReaders == NULL) {
        return 1;
    }
    Qt = This->pQueueReaders;
    while (Qt->BlockID != 0xFF) {
        pNbrReaders[Qt->BlockID - This->QueueOffset]++;
        Qt++;
    }
    Qt = This->pQueueReaders;
    while (Qt->BlockID != 0xFF) {
        QIdx = Qt->BlockID - This->QueueOffset;
        ReadPos = --pNbrReaders[QIdx];
        if (Qt->TargetID == BlockID) {
            if (ConnectInQtoBlockPos(This, BlkPtr,
                    This->AllQueues[QIdx], ReadPos, i++) != 0) {
                heap_Free(This->hHeap, pNbrReaders);
                return 1;
            }
        }
        Qt++;
    }
    heap_Free(This->hHeap, pNbrReaders);
    if (BlkPtr->Init(BlkPtr) != 0) {
        return 1;
    }
    return 0;
}

/* Constructs a queue and connects its writers and readers. */
static u8 ConstructQueue(
    struct _TOS *This, u8 BlockID, int Index)
{
    u8 *pNbrOutputs;
    const struct _QTARGETDESCRIP *Qt;
    const struct _QDESCRIP *Qd;
    struct _BLOCK *BlPtrTarget;
    struct _BLOCK *BlPtrSource;
    struct _QUEUE *QuePtr;
    int NbrReaders = 0;

    pNbrOutputs = heap_Calloc(
        This->hHeap, This->NrProces, sizeof(u8));
    if (pNbrOutputs == NULL) {
        return 1;
    }

    Qd = This->pInitialQueues;
    while (Qd->BlockID != BlockID) {
        pNbrOutputs[Qd->SourceID]++;
        Qd++;
    }

    Qt = This->pQueueReaders;
    while (Qt->BlockID != 0xFF) {
        if (Qt->BlockID == BlockID) {
            NbrReaders++;
        }
        Qt++;
    }

    QuePtr = qQueueConstruct(This, BlockID, NbrReaders);
    if (QuePtr == NULL) {
        heap_Free(This->hHeap, pNbrOutputs);
        return 1;
    }
    This->AllQueues[Index] = QuePtr;
    Index = BlockID;

    while (Qd->BlockID == (u8)Index) {
        BlPtrSource = Qd->SourceID != 0
            ? This->AllProcesses[Qd->SourceID - This->BlockOffset]
            : (struct _BLOCK *)This;

        if (ConnectOutQtoBlockPos(This, BlPtrSource, QuePtr,
                                  pNbrOutputs[Qd->SourceID]++) != 0) {
            heap_Free(This->hHeap, pNbrOutputs);
            return 1;
        }
        Qd++;
    }

    heap_Free(This->hHeap, pNbrOutputs);

    Qt = This->pQueueReaders;
    while (Qt->BlockID != 0xFF) {
        if (BlockID == Qt->BlockID) {
            u8 QIdx = Index - This->QueueOffset;
            BlPtrTarget = Qt->TargetID != 0
                ? This->AllProcesses[Qt->TargetID - This->BlockOffset]
                : (struct _BLOCK *)This;
            NbrReaders--;
            if (ConnectInQtoBlock(
                    This, BlPtrTarget, This->AllQueues[QIdx],
                    NbrReaders) != 0) {
                return 1;
            }
        }
        Qt++;
    }

    if (qQueueInit(QuePtr) == NULL) {
        return 1;
    }
    return 0;
}

/* Clears output ports that reference the queue. */
static u8 DisconnectOutQtoBlock(struct _TOS *This, struct _BLOCK *BlPtr, struct _QUEUE *QPtr)
{
    s8 k;
    if (BlPtr != (struct _BLOCK *)This && BlPtr != NULL) {
        for (k = 0; k < BlPtr->NrOutputs; k++) {
            if (BlPtr->OutputQueues[k].pQ == QPtr) {
                BlPtr->OutputQueues[k].pQ = NULL;
                break;
            }
        }
    }
    return 0;
}

/* Clears input ports that reference the queue. */
static u8 DisconnectInQtoBlock(struct _TOS *This, struct _BLOCK *BlPtr, struct _QUEUE *QPtr)
{
    s8 k;
    if (BlPtr != NULL && BlPtr != (struct _BLOCK *)This) {
        for (k = 0; k < BlPtr->NrInputs; k++) {
            if (BlPtr->InputQueues[k].pQ == QPtr) {
                BlPtr->InputQueues[k].pQ = NULL;
                break;
            }
        }
    }
    return 0;
}

/* Disconnects a queue from block ports and frees it. */
u8 DestructQueue(struct _QUEUE *QuePtr)
{
    struct _TOS *This = QuePtr->Object.pTos;
    struct _BLOCK *BlPtrSource;
    struct _BLOCK *BlPtrTarget;
    u8 QIdx;
    u8 BlockID = QuePtr->Object.BlockID;
    const struct _QDESCRIP *Qd;
    const struct _QTARGETDESCRIP *Qt;

    qQueueReset(QuePtr);

    if (BlockID != 0xFF) {
        Qd = This->pInitialQueues;
        while (Qd->BlockID != BlockID) {
            Qd++;
        }
        while (Qd->BlockID == BlockID) {
            BlPtrSource = Qd->SourceID != 0
                ? This->AllProcesses[Qd->SourceID - This->BlockOffset]
                : (struct _BLOCK *)This;
            DisconnectOutQtoBlock(This, BlPtrSource, QuePtr);
            Qd++;
        }
    }

    Qt = This->pQueueReaders;
    while (Qt->BlockID != 0xFF) {
        if (BlockID == Qt->BlockID) {
            QIdx = BlockID - This->QueueOffset;
            BlPtrTarget = Qt->TargetID != 0
                ? This->AllProcesses[Qt->TargetID - This->BlockOffset]
                : (struct _BLOCK *)This;
            DisconnectInQtoBlock(This, BlPtrTarget,
                This->AllQueues[QIdx]);
        }
        Qt++;
    }

    BlockID = QuePtr->Object.BlockID;
    if (BlockID != 0xFF) {
        This->AllQueues[BlockID - This->QueueOffset] = NULL;
    }
    heap_Free(This->hHeap, QuePtr);
    return 0;
}

/* Allocates the scheduler tables and connects the blocks and queues from the descriptors. */
static u8 ConstructArchitecture(
    struct _TOS *This,
    const struct _PROCDESCRIP *InitialProcs,
    const struct _QDESCRIP *InitialQueues,
    const struct _QTARGETDESCRIP *QueueReaders)
{
    const struct _PROCDESCRIP *Pd;
    const struct _QDESCRIP *Qd;
    const struct _QTARGETDESCRIP *Qt;
    u8 *pNbrOutputs;
    u8 *pNbrInputs;
    u8 *pNbrReaders;
    u8 BlockID;
    struct _QUEUE *QuePtr;
    int MaxNrInputs;
    u32 NrProces;
    u32 NrQueues;
    struct _BLOCK *BlkPtr;
    struct _BLOCK *BlPtrSource;
    u8 QIdx;
    struct _BLOCK *BlPtrTarget;

    MaxNrInputs = 0;
    NrProces = 0;
    NrQueues = 0;
    This->pInitialBlocks = InitialProcs;
    This->pInitialQueues = InitialQueues;
    This->pQueueReaders = QueueReaders;

    Pd = InitialProcs;
    while (Pd->BlockID != 0xFF) {
        NrProces++;
        Pd++;
    }

    BlockID = 0xFF;
    Qd = InitialQueues;
    while (Qd->BlockID != 0xFF) {
        if (BlockID != Qd->BlockID) {
            BlockID = Qd->BlockID;
            NrQueues++;
        }
        Qd++;
    }

    This->BlockOffset = InitialProcs[1].BlockID;
    This->QueueOffset = InitialQueues[0].BlockID;

    This->AllProcesses = heap_Alloc(This->hHeap, NrProces * sizeof(struct _BLOCK *));
    if (This->AllProcesses == NULL) {
        return 1;
    }
    This->AllQueues = heap_Alloc(This->hHeap, NrQueues * sizeof(struct _QUEUE *));
    if (This->AllQueues == NULL) {
        return 1;
    }

    pNbrOutputs = heap_Calloc(This->hHeap, NrQueues + NrProces * 2, sizeof(u8));
    if (pNbrOutputs == NULL) {
        return 1;
    }
    pNbrInputs = pNbrOutputs + NrProces;
    pNbrReaders = pNbrInputs + NrProces;

    Qd = InitialQueues;
    while (Qd->BlockID != 0xFF) {
        pNbrOutputs[Qd->SourceID]++;
        Qd++;
    }

    Qt = QueueReaders;
    while (Qt->BlockID != 0xFF) {
        pNbrReaders[Qt->BlockID - This->QueueOffset]++;
        pNbrInputs[Qt->TargetID]++;
        Qt++;
    }

    This->NrProces = 0;
    Pd = InitialProcs;
    while (Pd->BlockID != 0xFF) {
        if (pNbrInputs[Pd->BlockID] > MaxNrInputs) {
            MaxNrInputs = pNbrInputs[Pd->BlockID];
        }
        if (Pd->Constructor != NULL) {
            BlkPtr = NULL;
            if (Pd->Enabled != 0) {
                BlkPtr = Pd->Constructor(This, Pd->BlockID);
            }
            This->AllProcesses[This->NrProces++] = BlkPtr;
            if (This->NrProces != Pd->BlockID) {
                heap_Free(This->hHeap, pNbrOutputs);
                return 1;
            }
        } else if (Pd->BlockID == 0) {
            if (This->TosData.cbInitQueues(This, pNbrInputs[0], pNbrOutputs[0]) != 0) {
                heap_Free(This->hHeap, pNbrOutputs);
                return 1;
            }
        }
        Pd++;
    }

    memset(pNbrOutputs, 0, NrProces);
    This->NrQueues = 0;
    BlockID = 0xFF;
    Qd = InitialQueues;
    while (Qd->BlockID != 0xFF) {
        BlPtrSource = Qd->SourceID != 0 ?
            This->AllProcesses[Qd->SourceID - This->BlockOffset] :
            (struct _BLOCK *)This;
        if (BlockID != Qd->BlockID) {
            QuePtr = NULL;
            if (Qd->Enabled != 0) {
                QuePtr = qQueueConstruct(This, Qd->BlockID,
                    pNbrReaders[Qd->BlockID - This->QueueOffset]);
                if (QuePtr == NULL) {
                    heap_Free(This->hHeap, pNbrOutputs);
                    return 1;
                }
            }
            This->AllQueues[This->NrQueues++] = QuePtr;
            BlockID = Qd->BlockID;
        }
        if (ConnectOutQtoBlockPos(This, BlPtrSource, QuePtr,
                pNbrOutputs[Qd->SourceID]++) != 0) {
            heap_Free(This->hHeap, pNbrOutputs);
            return 1;
        }
        Qd++;
    }

    Qt = QueueReaders;
    while (Qt->BlockID != 0xFF) {
        QIdx = Qt->BlockID - This->QueueOffset;
        BlPtrTarget = Qt->TargetID != 0 ?
            This->AllProcesses[Qt->TargetID - This->BlockOffset] :
            (struct _BLOCK *)This;
        if (ConnectInQtoBlock(This, BlPtrTarget, This->AllQueues[QIdx],
                --pNbrReaders[QIdx]) != 0) {
            heap_Free(This->hHeap, pNbrOutputs);
            return 1;
        }
        Qt++;
    }

    heap_Free(This->hHeap, pNbrOutputs);
    This->pInputsToBlocks = heap_Alloc(This->hHeap, MaxNrInputs * sizeof(void *));
    if (This->pInputsToBlocks == NULL) {
        return 1;
    }
    return 0;
}

/* Destroys the blocks and queues and frees the scheduler tables. */
static u8 DestructArchitecture(struct _TOS *This)
{
    struct _BLOCK *block;
    struct _QUEUE *queue;
    s32 blockNumber;
    s32 queueNumber;
    u8 failed = 0;
    u8 operationFailed;

    for (blockNumber = 0; blockNumber < This->NrProces;
         blockNumber++) {
        block = This->AllProcesses[blockNumber];
        if (block != NULL) {
            if (block->Control != NULL) {
                operationFailed = !(u8)block->Control(
                    block, 0xFF, 0, 0);
                failed |= operationFailed;
            }
        }
    }

    for (queueNumber = 0; queueNumber < This->NrQueues;
         queueNumber++) {
        queue = This->AllQueues[queueNumber];
        if (queue != NULL) {
            operationFailed = !qQueueControl(
                queue, 0xFF, 0, 0);
            failed |= operationFailed;
        }
    }

    heap_Free(This->hHeap, This->AllProcesses);
    heap_Free(This->hHeap, This->AllQueues);
    heap_Free(This->hHeap, This->pInputsToBlocks);
    return failed;
}

/* Initializes the configured blocks and queues. */
static u8 InitializeArchitecture(struct _TOS *This)
{
    struct _BLOCK *block;
    struct _QUEUE *queue;
    u8 initResult;
    s32 blockNumber;
    s32 queueNumber;

    for (blockNumber = 0; blockNumber < This->NrProces;
         blockNumber++) {
        block = This->AllProcesses[blockNumber];
        if (block != NULL && block->Init != NULL) {
            initResult = block->Init(block);
            if (initResult != 0) {
                return 1;
            }
        }
    }

    if (SetAndControlQSizes(This) != 0) {
        return 1;
    }

    for (queueNumber = 0; queueNumber < This->NrQueues;
         queueNumber++) {
        queue = This->AllQueues[queueNumber];
        if (queue != NULL && qQueueInit(queue) == NULL) {
            return 1;
        }
    }
    return 0;
}

/* Looks up a queue by its block ID. */
struct _QUEUE *tosGetQueuePtr(struct _TOS *This, u8 BlockID)
{
    s32 queueNumber = BlockID - (s32)This->QueueOffset;

    if (queueNumber >= 0 && queueNumber < This->NrQueues) {
        return This->AllQueues[queueNumber];
    }
    return NULL;
}

/* Looks up a process block in the configured block ID range. */
static inline struct _BLOCK *tosGetBlockPtr(struct _TOS *This, u8 BlockID)
{
    int Index = BlockID - (s32)This->BlockOffset;

    if (Index >= 0 && Index < This->NrProces) {
        return This->AllProcesses[Index];
    }
    return NULL;
}

/* Dispatches the requested control ID to its block or queue. */
u8 tosCallControlFunc(
    struct _TOS *This, const struct _CTRL *pControl,
    u32 CParam, u32 DParam)
{
    struct _BLOCK *pBlock;
    struct _QUEUE *pQ;
    s32 blockNumber;
    u8 WasEnabled;
    u8 RetVal = 0;

    blockNumber = pControl->BlockID - (s32)This->BlockOffset;
    pBlock = tosGetBlockPtr(This, pControl->BlockID);
    if (pBlock != NULL) {
        WasEnabled = pBlock->Enabled;
        if (WasEnabled != 0 ||
            pControl->ControlID == 0xFF) {
            RetVal |= pBlock->Control(
                pBlock, pControl->ControlID, CParam,
                DParam);
        }
        if (pControl->ControlID >= 0xF8) {
            RetVal |= tosBaseBlockControl(
                pBlock, pControl->ControlID, CParam,
                DParam);
        }
        if (WasEnabled == 0 && RetVal == 0) {
            RetVal = 1;
        }
    } else {
        pQ = tosGetQueuePtr(This, pControl->BlockID);

        if (pQ != NULL) {
            RetVal = qQueueControl(
                pQ, pControl->ControlID, CParam,
                DParam);
        } else {
            if (pControl->ControlID == 0xF8) {
                if (blockNumber >= 0 &&
                    blockNumber < This->NrProces) {
                    if (This->AllProcesses[blockNumber] == NULL) {
                        ConstructBlock(
                            This, pControl->BlockID, blockNumber);
                    }
                } else {
                    if (!((pControl->BlockID - This->QueueOffset) >= 0 &&
                          (pControl->BlockID - This->QueueOffset) < This->NrQueues &&
                          This->AllQueues[(pControl->BlockID - This->QueueOffset)] != NULL)) {
                        ConstructQueue(
                            This, pControl->BlockID,
                            (pControl->BlockID - This->QueueOffset));
                    }
                }
            }
            RetVal = 1;
        }
    }
    return RetVal;
}

/* Constructs and initializes the scheduler from its architecture description. */
u8 tosInit(
    struct _TOS *pTos, void *hHeap,
    const struct _TOSDATA *pTosData,
    const struct _KEYVALS *IniArray,
    const struct _PROCDESCRIP *InitialProcs,
    const struct _QDESCRIP *InitialQueues,
    const struct _QTARGETDESCRIP *QueueReaders,
    const struct _INBLOCK *const *const *ControlTable,
    u16 InitialMode, u16 InitialState)
{
    pTos->hHeap = hHeap;
    fastallo_Init(&pTos->FastAllocHeap, hHeap, 0x1000);
    pTos->IniArray = IniArray;
    pTos->ControlTable = ControlTable;
    pTos->ErrorLog = 0;
    pTos->TosData = *pTosData;
    pTos->ActiveMode = InitialMode;
    pTos->ActiveState = InitialState;

    if (ConstructArchitecture(
            pTos, InitialProcs, InitialQueues,
            QueueReaders) == 0 &&
        InitializeArchitecture(pTos) == 0) {
        return 0;
    }

    fastallo_Terminate(&pTos->FastAllocHeap);
    return 1;
}

/* Releases the scheduler architecture. */
u8 tosDestruct(struct _TOS *pTos)
{
    if (DestructArchitecture(pTos) != 0) {
        return 1;
    }
    fastallo_Terminate(&pTos->FastAllocHeap);
    return 0;
}

/* Returns unused elements from the block input queues. */
static void blockFreeUnusedElementsInputQueues(struct _BLOCK *pBlock)
{
    u32 inputNumber;
    u32 inputCount;
    struct _INPUTQUEUE *input;

    inputNumber = 0;
    inputCount = pBlock->NrInputs;
    input = pBlock->InputQueues;
    while (inputNumber < inputCount) {
        if (input->pQ != NULL) {
            qFreeUnusedElemements(input->pQ);
        }
        input++;
        inputNumber++;
    }
}

/* Dequeues one set of inputs and calls the block process function. */
static void CallBlockProcessFunction(struct _BLOCK *pBlock)
{
    void **pInputs;
    int NbrInputs;

    if (pBlock != NULL) {
        pInputs = pBlock->Object.pTos->pInputsToBlocks;
        NbrInputs = pBlock->NrInputs;
        qCheckDeQueueOne(pBlock->InputQueues, NbrInputs, pInputs);
        pBlock->Process(pBlock, pInputs, NbrInputs);
        blockFreeUnusedElementsInputQueues(pBlock);
    }
}

/* Runs ready blocks and drains unused input queue elements. */
u8 tosRun(struct _TOS *pTos, const u8 *pBlockIDs)
{
    if (pBlockIDs == NULL) {
        unsigned int NrProces;
        u8 p;
        struct _BLOCK **ppBlock;

        NrProces = pTos->NrProces;
        do {
            ppBlock = pTos->AllProcesses;
            pTos->NbrOfEnqueues = 0;
            for (p = 0; p < NrProces; p++) {
                CallBlockProcessFunction(*ppBlock++);
            }
        } while (pTos->NbrOfEnqueues != 0);
    } else {
        const u8 *pIDs;
        struct _BLOCK **ppBlock;

        do {
            pIDs = pBlockIDs;
            ppBlock = pTos->AllProcesses - pTos->BlockOffset;
            pTos->NbrOfEnqueues = 0;
            while (*pIDs != 0) {
                CallBlockProcessFunction(ppBlock[*pIDs++]);
            }
        } while (pTos->NbrOfEnqueues != 0);
    }
    return 0;
}

/* Finds the current state command, changes state and invokes its targets and callback. */
u8 _tosControl(
    struct _OBJECT *pObject, u32 ControlID, u32 CParam1, u32 CParam2)
{
    struct _TOS *pTos = pObject->pTos;
    u8 BlockID = pObject->BlockID;
    const struct _INBLOCK *pInBlock;
    const struct _CNTRL0 *pInControl;
    const struct _CNTRL0 *const *ppInControl;
    const struct _CTRL *OutputControl;
    u8 RetVal = 0;

    pInBlock =
        pTos->ControlTable[pTos->ActiveMode][pTos->ActiveState];
    while (pInBlock->ppInControl != NULL) {
        if (pInBlock->BlockID == BlockID) {
            break;
        }
        pInBlock++;
    }
    ppInControl = pInBlock->ppInControl;
    if (ppInControl == NULL) {
        return _tosErrorLog(pObject, 5);
    }

    while (*ppInControl != NULL) {
        if ((*ppInControl)->ControlID == (u8)ControlID) {
            break;
        }
        ppInControl++;
    }
    pInControl = *ppInControl;
    if (pInControl == NULL) {
        return _tosErrorLog(pObject, 5);
    }

    if (pInControl->NextState != 0xFFFF &&
        pInControl->NextState != pTos->ActiveState) {
        if (pInControl->NextState == 0xFFFE) {
            pTos->ActiveState++;
        } else {
            pTos->ActiveState = pInControl->NextState;
        }
        RetVal = pTos->TosData.cbState(pTos, pTos->ActiveState);
        if (RetVal != 0) {
            return RetVal;
        }
    }

    /* The destination table ends at the record whose ControlID is 0. */
    OutputControl = pInControl->OutputControl;
    if (pInControl->SyncBlock == 0) {
        while (OutputControl->ControlID != 0) {
            RetVal |= (u8)!tosCallControlFunc(
                pTos, OutputControl, CParam1, CParam2);
            OutputControl++;
        }
    } else {
        while (OutputControl->ControlID != 0) {
            RetVal |= (u8)!tosCallControlFunc(
                pTos, OutputControl, CParam1, CParam2);
            OutputControl++;
            tosRun(pTos, NULL);
        }
    }

    if (pInControl->CallBackFunc != NULL) {
        RetVal |= pInControl->CallBackFunc(
            pTos, BlockID, ControlID,
            (void *)CParam1, CParam2);
    }
    return RetVal;
}

/* Records the first error and notifies the configured error callback. */
u8 _tosErrorLog(struct _OBJECT *pObject, u16 ErrorCode)
{
    struct _TOS *pTos = pObject->pTos;
    u8 BlockID = pObject->BlockID;

    if (pTos->ErrorLog == 0) {
        pTos->ErrorLog =
            ((u32)BlockID << 16) | ErrorCode;
    }
    if (pTos->TosData.cbError != NULL) {
        pTos->TosData.cbError(pTos, pTos->ErrorLog);
    }
    return (u8)ErrorCode;
}

/* Records the first warning and notifies the configured warning callback. */
u8 _tosWarningLog(struct _OBJECT *pObject, u16 WarningCode)
{
    struct _TOS *pTos = pObject->pTos;
    u8 BlockID = pObject->BlockID;

    if (pTos->WarningLog == 0) {
        pTos->WarningLog =
            ((u32)BlockID << 16) | WarningCode;
    }
    if (pTos->TosData.cbWarning != NULL) {
        pTos->TosData.cbWarning(pTos, pTos->WarningLog);
    }
    return (u8)WarningCode;
}

/* Looks up a floating-point setting, falling back to an integer setting or the default. */
f32 _tosGetProfileFloat(
    struct _OBJECT *pObject, u16 KeyID, f32 RetVal)
{
    struct _TOS *pTos = pObject->pTos;
    u8 BlockID = pObject->BlockID;
    const struct _KEYVAL_FLT *pKeyValFLT;
    const struct _KEYVAL_U32 *pKeyValU32;
    f32 DefVal = RetVal;
    int Found = 0;

    pKeyValFLT =
        pTos->IniArray[BlockID].pKeyValFLT;
    pKeyValU32 =
        pTos->IniArray[BlockID].pKeyValU32;
    if (pKeyValFLT != NULL) {
        while (pKeyValFLT->KeyID != KeyID &&
               pKeyValFLT->KeyID != 0) {
            pKeyValFLT++;
        }
        if (pKeyValFLT->KeyID != 0) {
            DefVal = pKeyValFLT->Val;
            Found = 1;
        }
    }

    if (Found == 0 && pKeyValU32 != NULL) {
        while (pKeyValU32->KeyID != KeyID && pKeyValU32->KeyID != 0) {
            pKeyValU32++;
        }
        if (pKeyValU32->KeyID != 0) {
            DefVal = (f32)pKeyValU32->Val;
        }
    }
    return DefVal;
}

/* Looks up an integer setting, falling back to a floating-point setting or the default. */
u32 _tosGetProfileU32(
    struct _OBJECT *pObject, u16 KeyID, u32 RetVal)
{
    const struct _KEYVAL_FLT *pKeyValFLT;
    const struct _KEYVAL_U32 *pKeyValU32;
    u32 DefVal = RetVal;
    int Found = 0;

    pKeyValFLT =
        pObject->pTos->IniArray[pObject->BlockID].pKeyValFLT;
    pKeyValU32 =
        pObject->pTos->IniArray[pObject->BlockID].pKeyValU32;
    if (pKeyValU32 != NULL) {
        while (pKeyValU32->KeyID != KeyID && pKeyValU32->KeyID != 0) {
            pKeyValU32++;
        }
        if (pKeyValU32->KeyID != 0) {
            DefVal = pKeyValU32->Val;
            Found = 1;
        }
    }

    if (Found == 0 && pKeyValFLT != NULL) {
        while (pKeyValFLT->KeyID != KeyID &&
               pKeyValFLT->KeyID != 0) {
            pKeyValFLT++;
        }
        if (pKeyValFLT->KeyID != 0) {
            DefVal = (u32)pKeyValFLT->Val;
        }
    }
    return DefVal;
}

/* Allocates a block and its port descriptors and initializes its callbacks. */
struct _BLOCK *tosBaseBlockConstruct(
    struct _TOS *pTos, u8 BlockID, u8 NbrInputs,
    u8 NbrOutputs, void (*Proces)(struct _BLOCK *, void *, int),
    u8 (*Init)(struct _BLOCK *), u8 (*Control)(struct _BLOCK *, u8, u32, u32), u32 BlockSize)
{
    struct _OBJECT Object;
    struct _BLOCK *pBaseBlock;
    u8 *allocation;
    struct _INPUTQUEUE *input;

    Object.pTos = pTos;
    Object.BlockID = BlockID;

    if ((BlockSize & 3) != 0) {
        BlockSize += 4 - (BlockSize & 3);
    }

    allocation = heap_Calloc(
        pTos->hHeap, 1,
        BlockSize +
            NbrInputs * sizeof(struct _INPUTQUEUE) +
            NbrOutputs * sizeof(struct _OUTPUTQUEUE));
    if (allocation == NULL) {
        _tosErrorLog(&Object, 2);
        return NULL;
    }

    pBaseBlock = (struct _BLOCK *)allocation;

    /* The input queues follow the full, aligned size of the concrete block. */
    input = (struct _INPUTQUEUE *)(allocation + BlockSize);
    pBaseBlock->InputQueues = input;
    pBaseBlock->OutputQueues = (struct _OUTPUTQUEUE *)(input + NbrInputs);
    pBaseBlock->Object.pTos = pTos;
    pBaseBlock->Object.BlockID = BlockID;
    pBaseBlock->NrInputs = NbrInputs;
    pBaseBlock->NrOutputs = NbrOutputs;
    pBaseBlock->Init = Init;
    pBaseBlock->Control = Control;
    pBaseBlock->Process = Proces;
    pBaseBlock->Enabled = 1;
    return pBaseBlock;
}

/* Disables the block readers and releases the block. */
void tosBaseBlockDestruct(struct _BLOCK *pBaseBlock)
{
    struct _TOS *This = pBaseBlock->Object.pTos;
    s32 NrInQueues = pBaseBlock->NrInputs;
    s32 i;

    i = 0;
    while (i < NrInQueues) {
        if (pBaseBlock->InputQueues[i].pQ != NULL) {
            qQueueControl(pBaseBlock->InputQueues[i].pQ, 0x8,
                          pBaseBlock->InputQueues[i].ReadPos, 0);
        }
        i++;
    }

    This->AllProcesses[
        pBaseBlock->Object.BlockID -
        This->BlockOffset] = NULL;
    heap_Free(
        pBaseBlock->Object.pTos->hHeap, pBaseBlock);
}

/* Disables each connected input reader and accumulates failures. */
u8 tosBaseBlockDisableInputQueues(struct _BLOCK *pBaseBlock)
{
    u32 inputNumber;
    struct _QUEUE *queue;
    u8 failed = 0;

    for (inputNumber = 0; inputNumber < pBaseBlock->NrInputs;
         inputNumber++) {
        queue = pBaseBlock->InputQueues[inputNumber].pQ;
        if (queue != NULL) {
            failed |= (u8)!qQueueControl(
                queue, 0x8, pBaseBlock->InputQueues[inputNumber].ReadPos, 0);
        }
    }
    return failed;
}

/* Starts connected input readers with the requested jump-back count. */
u8 tosBaseBlockStartInputQueues(
    struct _BLOCK *pBaseBlock, u32 JumpBack)
{
    u32 inputNumber;
    u32 queueCommand;
    u8 failed;

    queueCommand = JumpBack != 0 ? (u8)0x7 : (u8)0x5;
    failed = 0;

    for (inputNumber = 0; inputNumber < pBaseBlock->NrInputs;
         inputNumber++) {
        struct _QUEUE *queue = pBaseBlock->InputQueues[inputNumber].pQ;
        if (queue != NULL) {
            failed |= (u8)!qQueueControl(
                queue, queueCommand,
                pBaseBlock->InputQueues[inputNumber].ReadPos,
                JumpBack);
        }
    }
    return failed;
}

/* Leaves a disabled block idle when the scheduler calls its process slot. */
static void DisabledProcess(void)
{
}

/* Stops the readers connected to the input ports. */
static u8 tosBaseBlockStopInputQueues(struct _BLOCK *pBaseBlock)
{
    unsigned long i;
    unsigned char RetVal = 0;
    for (i = 0; i < pBaseBlock->NrInputs; i++) {
        RetVal |= (u8)!qQueueControl(pBaseBlock->InputQueues[i].pQ, 0x6,
            pBaseBlock->InputQueues[i].ReadPos, 0);
    }
    return RetVal;
}

/* Applies a scheduler control command to the block and its input readers. */
u8 tosBaseBlockControl(
    struct _BLOCK *pBaseBlock, u8 ControlId, u32 CParam,
    u32 DParam)
{
    u8 RetVal = 1;

    switch (ControlId) {
    case 0xFC:
        if (pBaseBlock->Enabled != 0) {
            pBaseBlock->Enabled = 0;
            pBaseBlock->SavedProcess = pBaseBlock->Process;
            pBaseBlock->Process = (void (*)(struct _BLOCK *, void *, int))DisabledProcess;
            return !tosBaseBlockDisableInputQueues(pBaseBlock);
        }
        return 0;
    case 0xFA:
        if (pBaseBlock->Enabled != 0) {
            return !tosBaseBlockStopInputQueues(pBaseBlock);
        }
        return 0;
    case 0xFB:
        pBaseBlock->Enabled = 1;
        CParam = 0;
        if (pBaseBlock->Process == (void (*)(struct _BLOCK *, void *, int))DisabledProcess) {
            pBaseBlock->Process = pBaseBlock->SavedProcess;
        }
        /* fall through */
    case 0xF9:
        if (pBaseBlock->Enabled != 0) {
            return !tosBaseBlockStartInputQueues(pBaseBlock, CParam);
        }
        return 0;
    case 0xF8:
        break;
    default:
        RetVal = 0;
        break;
    }
    return RetVal;
}
