/* Scores user-word models with a Viterbi search and backtrace. */
#include "gssdk/blocks.h"
#include "gssdk/heap.h"
#include "types.h"

#include <stddef.h>
#include <string.h>

#include "gssdk/engine.h"
#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Candidate scoring state, its distribution and its backtrace. */
typedef struct _SCOREUWACTIVEDPSTATE {
    s16 Score; /* Score of the candidate path. */
    _SCOREUWBACKTRACE *pBacktrace; /* Path that reached this state. */
    u16 State; /* Distribution at the end of the path. */
} _SCOREUWACTIVEDPSTATE;

/* Forward declaration for the user-word scoring block. */
typedef struct _DPSCOREUW _DPSCOREUW;

/* User-word scoring state and the queues used to process its frames. */
struct _DPSCOREUW {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    struct _INPUTQUEUE *InQueue; /* Signed distances received from the distance calculator. */
    /* Active distribution IDs and flags sent to the distance calculator. */
    struct _OUTPUTQUEUE *ActDistQueue;
    _SCOREUWACTIVEDPSTATE *pDynProg; /* Current paths through the word and silence states. */
    u16 AllocSize; /* Size in bytes of a backtrace allocation. */
    u32 NbrBacktraceArrs; /* Number of allocated chunks in pBacktraceArrs. */
    _SCOREUWBACKTRACE **pBacktraceArrs; /* Allocated backtrace chunks. */
    _SCOREUWBACKTRACE *BacktraceFreelist; /* Nodes available for another scoring path. */
    u16 NbrPhenoms; /* Number of phonemes in the word model. */
    u16 TotNbrPhenoms; /* Total number of states, including leading and trailing silence. */
    u16 NbrSilPhenoms; /* Number of silence phonemes in the initialization record. */
    u16 NbrPhenomsInLang; /* Number of language distributions addressed by the activity flags. */
    u16 NbrErgodicPhenoms; /* Number of ergodic phonemes in the language. */
    u16 *pErgodicPhenoms; /* The game's code never reads this field. */
    u16 *pPhenoms; /* Distribution selected for each word or silence state. */
    u16 SilPhenom; /* Distribution used for the leading and trailing silence states. */
    u16 singleWordGarbPhenom; /* Distribution scored on each frame for the garbage path. */
    u32 AccumulatedScore; /* Sum of the score floors subtracted from the word paths. */
    u32 AccumulatedGarbScore; /* Sum of the garbage distribution scores. */
    s16 UserWordScore; /* Current score of the final word state. */
    s16 sMin; /* Minimum score used to normalize the paths. */
    u8 optionalPaths; /* Allows a transition from the last silence state when enabled. */
    u8 EndOfSpeech; /* The game's code never reads this field. */
    u32 FrameCounter; /* Current frame index. */
    u32 TimeOut; /* Frame count limit for scoring the current utterance. */
    /* Writes the active-distribution packet. */
    u16 (*MakeActiveDist)(_DPSCOREUW *This, u16 *ActiveDist, u8 *ActiveBoolList);
    enum _ACTDISTSTATE ActDistState; /* Current active-distribution mode. */
    void (*SendResult)(_DPSCOREUW *This, u32 CParam, u32 DParam); /* Reports the completed path. */
};

/* Initializes the backtrace pool and returns its first node. */
static _SCOREUWBACKTRACE *InitBacktrace(_DPSCOREUW *This)
{
    u32 backtracesPerChunk = This->AllocSize / sizeof(_SCOREUWBACKTRACE);
    u32 lastBacktraceIndex = backtracesPerChunk - 1;
    _SCOREUWBACKTRACE *pOld = NULL;
    u32 i;

    for (i = 0; i < This->NbrBacktraceArrs; i++) {
        _SCOREUWBACKTRACE *pState = This->pBacktraceArrs[i];
        u32 j;

        if (pOld != NULL) {
            pOld->Previous = pState;
        }

        for (j = 0; j < lastBacktraceIndex; j++) {
            pState[j].Previous = &pState[j + 1];
        }
        pOld = &pState[j];
    }

    if (pOld != NULL) {
        pOld->Previous = NULL;
    }

    pOld = This->pBacktraceArrs[0];
    pOld->Score = 0;
    pOld->ExitTime = 0;
    pOld->Count = 0;
    pOld->State = 0;
    This->BacktraceFreelist = pOld->Previous;
    return pOld;
}

/* Allocates a chunk of backtrace nodes and appends it to the free list. */
static _SCOREUWBACKTRACE *AllocateBacktrace(_DPSCOREUW *This, _SCOREUWBACKTRACE *pEndFree)
{
    u32 NbrOfArrays = This->NbrBacktraceArrs;
    _SCOREUWBACKTRACE **pNewArrs;
    _SCOREUWBACKTRACE *pExtra;
    u32 backtracesPerChunk;
    u32 Max;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    pNewArrs = heap_Alloc(pTos->hHeap, (NbrOfArrays + 1) * sizeof(_SCOREUWBACKTRACE *));
    if (pNewArrs == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return NULL;
    }

    if (NbrOfArrays != 0) {
        memcpy(pNewArrs, This->pBacktraceArrs, NbrOfArrays * sizeof(_SCOREUWBACKTRACE *));
        heap_Free(pTos->hHeap, This->pBacktraceArrs);
    }

    This->pBacktraceArrs = pNewArrs;

    backtracesPerChunk = This->AllocSize / sizeof(_SCOREUWBACKTRACE);
    pExtra = heap_Alloc(pTos->hHeap, backtracesPerChunk * sizeof(_SCOREUWBACKTRACE));
    if (pExtra == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return NULL;
    }

    if (pEndFree != NULL) {
        u32 i;

        backtracesPerChunk = This->AllocSize / sizeof(_SCOREUWBACKTRACE);
        Max = backtracesPerChunk - 1;
        pEndFree->Previous = pExtra;
        for (i = 0; i < Max; i++) {
            pExtra[i].Previous = &pExtra[i + 1];
        }
        pExtra[i].Previous = NULL;
    }

    This->pBacktraceArrs[NbrOfArrays++] = pExtra;
    This->NbrBacktraceArrs = NbrOfArrays;
    return pExtra;
}

/* Frees the chunks of backtrace nodes owned by this block. */
static void FreeBacktrace(_DPSCOREUW *This)
{
    struct _TOS *context = This->BaseBlock.Object.pTos;
    u32 chunkIndex;

    for (chunkIndex = 0; chunkIndex < This->NbrBacktraceArrs; chunkIndex++) {
        heap_Free(context->hHeap, This->pBacktraceArrs[chunkIndex]);
    }

    if (This->pBacktraceArrs != NULL) {
        heap_Free(context->hHeap, This->pBacktraceArrs);
    }
    This->pBacktraceArrs = NULL;
    This->NbrBacktraceArrs = 0;
}

/* Returns the final backtrace, adding a closing node when silence states are disabled. */
static _SCOREUWBACKTRACE *GetBacktrace(_DPSCOREUW *This)
{
    _SCOREUWACTIVEDPSTATE *pDpOld = &This->pDynProg[This->TotNbrPhenoms - 1];
    _SCOREUWBACKTRACE *Bt = pDpOld->pBacktrace;
    s16 PrevScore;

    if (This->NbrSilPhenoms == 0) {
        _SCOREUWBACKTRACE *NewBacktrace = This->BacktraceFreelist;
        PrevScore = This->UserWordScore;

        if (NewBacktrace->Previous == NULL) {
            This->BacktraceFreelist = AllocateBacktrace(This, NewBacktrace);
        } else {
            This->BacktraceFreelist = NewBacktrace->Previous;
        }

        NewBacktrace->Previous = Bt;
        Bt->Count++;
        Bt = NewBacktrace;
        NewBacktrace->State = pDpOld->State;
        NewBacktrace->Count = 0;
        NewBacktrace->ExitTime = This->FrameCounter;
        NewBacktrace->Score = PrevScore + This->AccumulatedScore;
        NewBacktrace->GarbScore = This->AccumulatedGarbScore;
        NewBacktrace->ExitTime++;
    }

    return Bt;
}

/* Resets the user-word scoring states for a new utterance. */
static void InitViterbi(_DPSCOREUW *This)
{
    _SCOREUWBACKTRACE *backtrace;
    _SCOREUWACTIVEDPSTATE *state;
    u32 stateIndex;
    u32 referenceCount = 0;

    This->FrameCounter = -1;
    This->sMin = 0;
    This->AccumulatedScore = 0;
    This->AccumulatedGarbScore = 0;

    backtrace = InitBacktrace(This);
    backtrace->Previous = NULL;
    backtrace->State = 0;
    backtrace->Count = 0;

    state = This->pDynProg;
    for (stateIndex = 0; stateIndex < This->NbrSilPhenoms; stateIndex++) {
        state->State = This->SilPhenom;
        if (stateIndex == 0) {
            state->Score = 0;
        } else {
            state->Score = 0x77FF;
        }
        state->pBacktrace = backtrace;
        state++;
        referenceCount++;
    }

    for (stateIndex = 0; stateIndex < This->NbrPhenoms; stateIndex++) {
        state->State = This->pPhenoms[stateIndex];
        state->Score = 0x77FF;
        state->pBacktrace = backtrace;
        state++;
        referenceCount++;
    }

    for (stateIndex = 0; stateIndex < This->NbrSilPhenoms; stateIndex++) {
        state->State = This->SilPhenom;
        state->Score = 0x77FF;
        state->pBacktrace = backtrace;
        state++;
        referenceCount++;
    }

    if (This->NbrSilPhenoms == 0) {
        This->pDynProg[0].Score = 0;
    }
    backtrace->Count = referenceCount;
}

/* Initializes the user-word scoring block and its queues. */
static u8 InitDpScoreUw(struct _BLOCK *_This)
{
    _DPSCOREUW *block = (_DPSCOREUW *) _This;
    u16 distributionCount = _tosGetProfileU32(&_This->Object, 0x13, 1);

    block->InQueue = _This->InputQueues;
    block->ActDistQueue = _This->OutputQueues;
    block->InQueue->Size = distributionCount * sizeof(s16);
    /* Each distribution contributes a two-byte ID and a one-byte activity flag. */
    block->ActDistQueue->Size = distributionCount * 3;

    block->AllocSize = _tosGetProfileU32(&_This->Object, 0x11, 400);
    if (block->AllocSize > 65000) {
        block->AllocSize = 65000;
    } else if (block->AllocSize < 200) {
        block->AllocSize = 200;
    }

    block->TimeOut = 0;
    block->optionalPaths = 0;
    return 0;
}

/* Scores the selected backtrace and reports its speech span and frame counts. */
static void SendResultScoreUserWords(_DPSCOREUW *block, u32 CParam, u32 DParam)
{
    u32 lastScore;
    u32 firstFrame;
    u32 lastFrame;
    u32 frameCount;
    u32 backtraceCount = 0;
    _SCOREUSERWORDRES result;
    _SCOREUWBACKTRACE *backtrace = GetBacktrace(block);
    _SCOREUWBACKTRACE *currentBacktrace = backtrace;
    s32 speechScore = 0;
    s32 speechFrames = 0;
    _SCOREUWBACKTRACE *lastBacktrace;
    _SCOREUWBACKTRACE *nextBacktrace;
    u8 inSilence = FALSE;
    f32 averageScore;

    if (block->NbrSilPhenoms == 0) {
        lastFrame = block->FrameCounter + 1;
        firstFrame = 0;
        lastScore = block->AccumulatedScore + block->UserWordScore;
    } else {
        lastFrame = backtrace->ExitTime;
        lastScore = backtrace->Score;
        lastBacktrace = backtrace;
        inSilence = TRUE;
        backtraceCount = 1;

        while ((nextBacktrace = currentBacktrace->Previous) != NULL) {
            if (currentBacktrace->State == block->SilPhenom && !inSilence) {
                inSilence = TRUE;
                speechScore -= currentBacktrace->Score - currentBacktrace->GarbScore;
                speechFrames -= currentBacktrace->ExitTime;
            } else if (currentBacktrace->State != block->SilPhenom && inSilence) {
                inSilence = FALSE;
                speechScore += currentBacktrace->Score - currentBacktrace->GarbScore;
                speechFrames += currentBacktrace->ExitTime;
            }

            lastBacktrace = currentBacktrace;
            currentBacktrace = nextBacktrace;
            backtraceCount++;
        }

        lastScore -= lastBacktrace->Score;
        firstFrame = lastBacktrace->ExitTime;
    }

    frameCount = lastFrame - firstFrame;
    result.SpeechLength = frameCount;
    result.SpeechScore = lastScore;
    if (frameCount != 0) {
        averageScore = (f32) lastScore / (f32) frameCount;
    } else {
        averageScore = 0.0f;
    }
    result.NormScore = averageScore;

    result.UWScore = block->AccumulatedScore + block->UserWordScore;
    result.StartIdx = firstFrame;
    result.StopIdx = lastFrame;
    result.NbrPhenoms = backtraceCount;
    result.pBackTrace = backtrace;
    if (speechFrames == 0) {
        result.ConfValue = 100;
    } else {
        result.ConfValue = speechScore / speechFrames;
    }

    if (_tosControl(&block->BaseBlock.Object, 0x0C, (u32) &result, 2)) {
        _tosErrorLog(&block->BaseBlock.Object, 1);
    }
}

/* Collects the scored states' distinct active distributions. */
static u16 MakeScoreUserWordsActiveDist(_DPSCOREUW *This, u16 *ActiveDist, u8 *ActiveBoolList)
{
    _SCOREUWACTIVEDPSTATE *state = This->pDynProg;
    u32 activeCount;
    u32 stateIndex;

    memset(ActiveBoolList, 0, This->NbrPhenomsInLang);
    activeCount = This->NbrErgodicPhenoms;
    memset(ActiveBoolList, 1, activeCount);

    for (stateIndex = 0; stateIndex < This->TotNbrPhenoms; stateIndex++, state++) {
        u16 distribution = state->State;

        if (ActiveBoolList[distribution] == 0) {
            ActiveBoolList[distribution] = 1;
            ActiveDist[++activeCount] = distribution;
        }
    }

    ActiveDist[0] = activeCount;
    return (u16) activeCount;
}

/* Discards backtrace nodes that are no longer referenced. */
static inline void CleanUp(_DPSCOREUW *This, _SCOREUWBACKTRACE *FirstPtr)
{
    _SCOREUWBACKTRACE *PrevPtr = FirstPtr;
    _SCOREUWBACKTRACE *ThisPtr = FirstPtr->Previous;
    while (ThisPtr->Count <= 1) {
        PrevPtr = ThisPtr;
        ThisPtr = ThisPtr->Previous;
    }
    ThisPtr->Count--;
    PrevPtr->Previous = This->BacktraceFreelist;
    This->BacktraceFreelist = FirstPtr;
}

/* Advances the user-word and garbage paths using the current frame's distances. */
static int DynProgScoreUserWords(_DPSCOREUW *This, s16 *pDistances)
{
    int dbg_bSubModActive;
    void *dbg_pInst;
    _SCOREUWACTIVEDPSTATE *pDp1;
    _SCOREUWACTIVEDPSTATE *pDpOld;
    u32 St;
    s32 sNewMin = 0x7FFF;
    s32 sOldMin = This->sMin;
    _SCOREUWBACKTRACE *PrevBt;
    _SCOREUWBACKTRACE *pLastSilBt;
    s32 PrevScore;
    s32 LocScore;
    s32 LastSilScore;
    s32 NewScore;
    _SCOREUWBACKTRACE *NewBt;
    _SCOREUWBACKTRACE *NewBacktrace;

    This->AccumulatedScore += sOldMin;
    This->AccumulatedGarbScore += pDistances[This->singleWordGarbPhenom];

    pDp1 = This->pDynProg;
    for (St = 0; St < This->TotNbrPhenoms; St++) {
        pDp1->Score -= sOldMin;
        if (pDp1->Score > 0x77FF) {
            pDp1->Score = 0x77FF;
        }
        pDp1++;
    }

    pDp1 = This->pDynProg;
    PrevScore = pDp1->Score;
    PrevBt = pDp1->pBacktrace;
    pDp1->Score = PrevScore + pDistances[pDp1->State];
    if (pDp1->Score < sNewMin) {
        sNewMin = pDp1->Score;
    }
    PrevBt->Count++;

    pDpOld = pDp1;
    LastSilScore = pDp1->Score;
    pLastSilBt = pDp1->pBacktrace;
    pDp1++;

    for (St = 1; St < This->TotNbrPhenoms; St++) {
        NewScore = PrevScore;
        NewBt = PrevBt;
        PrevScore = pDp1->Score;

        PrevBt = pDp1->pBacktrace;
        PrevBt->Count++;
        LocScore = pDistances[pDp1->State];

        if (This->optionalPaths && pDp1->State == This->SilPhenom &&
            LastSilScore < NewScore && LastSilScore < PrevScore) {
            pDp1->Score = LastSilScore + LocScore;
            if (pDp1->Score < sNewMin) {
                sNewMin = pDp1->Score;
            }

            if (--pDp1->pBacktrace->Count <= 0) {
                CleanUp(This, pDp1->pBacktrace);
            }
            pDp1->pBacktrace = pLastSilBt;
            pLastSilBt->Count++;
        } else if (NewScore < PrevScore) {
            pDp1->Score = NewScore + LocScore;
            if (pDp1->Score < sNewMin) {
                sNewMin = pDp1->Score;
            }

            NewBacktrace = This->BacktraceFreelist;
            if (NewBacktrace->Previous == NULL) {
                This->BacktraceFreelist = AllocateBacktrace(This, NewBacktrace);
            } else {
                This->BacktraceFreelist = NewBacktrace->Previous;
            }

            NewBacktrace->Previous = NewBt;
            NewBt->Count++;
            NewBacktrace->State = pDpOld->State;
            NewBacktrace->Count = 0;
            NewBacktrace->ExitTime = This->FrameCounter;
            NewBacktrace->Score = NewScore + This->AccumulatedScore;
            NewBacktrace->GarbScore = This->AccumulatedGarbScore;
            NewBacktrace->Count++;

            if (--pDp1->pBacktrace->Count <= 0) {
                CleanUp(This, pDp1->pBacktrace);
            }
            pDp1->pBacktrace = NewBacktrace;
        } else {
            pDp1->Score += LocScore;
            if (pDp1->Score < sNewMin) {
                sNewMin = pDp1->Score;
            }
        }

        if (pDp1->State == This->SilPhenom) {
            LastSilScore = PrevScore;
            pLastSilBt = pDp1->pBacktrace;
        }

        if (--NewBt->Count <= 0) {
            CleanUp(This, NewBt);
        }
        pDpOld = pDp1;
        pDp1++;
    }

    if (--PrevBt->Count <= 0) {
        CleanUp(This, PrevBt);
    }
    This->sMin = sNewMin;
    This->UserWordScore = pDpOld->Score;
    return 0;
}

/* Leaves input frames unprocessed while user-word scoring is idle. */
static void IdleDpScoreUwProcess(struct _BLOCK *baseBlock, void *inputs, int inputCount)
{
}

/* Consumes one score frame and advances the user-word backtrace. */
static void DpScoreUwProcess(struct _BLOCK *_This, void *pInputs, int inputCount)
{
    _DPSCOREUW *This = (_DPSCOREUW *) _This;
    void **pInputQueues = pInputs;
    s16 *pDistances = pInputQueues[0];

    if (pDistances != NULL) {
        This->FrameCounter++;
        if (This->FrameCounter == 0) {
            _SCOREUWACTIVEDPSTATE *state = This->pDynProg;

            state->Score = pDistances[state->State];
            This->sMin = state->Score;
        } else if (DynProgScoreUserWords(This, pDistances) != 0) {
            _tosErrorLog(&This->BaseBlock.Object, 0x77);
            This->BaseBlock.Process = IdleDpScoreUwProcess;
            if (_tosControl(&This->BaseBlock.Object, 0x0B, 0, 0)) {
                This->BaseBlock.Process = IdleDpScoreUwProcess;
                return;
            }
        }

        {
            u16 *ActiveDist = qEnQueueOne(This->ActDistQueue->pQ);

            /* The frame packet puts flags before the final distance-width slot. */
            This->MakeActiveDist(This, ActiveDist,
                                 (u8 *) ActiveDist + This->InQueue->Size - sizeof(u16));
        }

        if (This->TimeOut != 0 && This->FrameCounter > This->TimeOut) {
            This->BaseBlock.Process = IdleDpScoreUwProcess;
            if (_tosControl(&This->BaseBlock.Object, 8, This->FrameCounter, 0)) {
                _tosErrorLog(&This->BaseBlock.Object, 1);
            }
        }
    }
}

/* Initializes scoring from the user-word phoneme and silence lists. */
static u8 InitScoreUserWords(_DPSCOREUW *This, _INITSCOREWUSERWORDDATA *pInitData)
{
    struct _TOS *context;
    u16 *pSilPhenoms;
    u8 error;

    This->NbrPhenoms = pInitData->NbrPhenoms;
    This->pPhenoms = pInitData->pPhenoms;
    This->NbrSilPhenoms = pInitData->NbrSilPhenoms;
    pSilPhenoms = pInitData->pSilPhenoms;

    if (This->NbrSilPhenoms > 1) {
        _tosErrorLog(&This->BaseBlock.Object, 1);
        return 1;
    }

    This->FrameCounter = -1;
    This->sMin = 0;
    This->AccumulatedScore = 0;
    This->AccumulatedGarbScore = 0;

    if (This->NbrSilPhenoms != 0) {
        This->SilPhenom = pSilPhenoms[0];
        This->TotNbrPhenoms = This->NbrPhenoms + 2;
    } else {
        This->SilPhenom = 0;
        This->TotNbrPhenoms = This->NbrPhenoms;
    }

    context = This->BaseBlock.Object.pTos;
    FreeBacktrace(This);
    if (This->pDynProg != NULL) {
        heap_Free(context->hHeap, This->pDynProg);
    }
    This->pDynProg = NULL;
    error = 0;

    if (AllocateBacktrace(This, NULL) == NULL) {
        return 1;
    }

    This->pDynProg = heap_Alloc(This->BaseBlock.Object.pTos->hHeap,
                                This->TotNbrPhenoms * sizeof(_SCOREUWACTIVEDPSTATE));
    if ((error = This->pDynProg == NULL ? _tosErrorLog(&This->BaseBlock.Object, 0x71) : error) !=
        0) {
        return 1;
    }

    This->EndOfSpeech = 0;
    This->InQueue->Size = This->InQueue->pQ->Size;
    This->ActDistQueue->Size = This->ActDistQueue->pQ->Size;
    return 0;
}

/* Handles setup, scoring and teardown commands for the block. */
static u8 ControlDpScoreUw(struct _BLOCK *_This, u8 ControlId, u32 CParam1, u32 CParam2)
{
    _DPSCOREUW *block = (_DPSCOREUW *) _This;

    switch (ControlId) {
    case 2:
        block->BaseBlock.Process = IdleDpScoreUwProcess;
        block->SendResult(block, CParam1, CParam2);
        break;
    case 1:
        InitViterbi(block);
        qQueueReset(block->ActDistQueue->pQ);
        {
            u16 *output = qEnQueueOne(block->ActDistQueue->pQ);

            /* The startup packet puts flags after the full distance extent. */
            block->MakeActiveDist(block, output, (u8 *) output + block->InQueue->Size);
        }
        block->BaseBlock.Process = DpScoreUwProcess;
        break;
    case 3:
        block->BaseBlock.Process = IdleDpScoreUwProcess;
        break;
    case 7:
        block->NbrPhenomsInLang =
            ((struct _LANGUAGE *) CParam1)->langGetNbrPhenome((struct _LANGUAGE *) CParam1);
        block->NbrErgodicPhenoms =
            ((struct _LANGUAGE *) CParam1)->langGetNbrErgodicPhenomes((struct _LANGUAGE *) CParam1);
        block->pErgodicPhenoms =
            ((struct _LANGUAGE *) CParam1)->langGetpErgodicPhenomes((struct _LANGUAGE *) CParam1);
        block->singleWordGarbPhenom =
            ((struct _LANGUAGE *) CParam1)
                ->langGetSingleWordGarbagePhenome((struct _LANGUAGE *) CParam1);
        if (block->ActDistQueue->pQ != NULL) {
            block->ActDistQueue->Size = block->ActDistQueue->pQ->Size;
        }
        break;
    case 4:
        if (InitScoreUserWords(block, (_INITSCOREWUSERWORDDATA *) CParam1)) {
            return 0;
        }
        break;
    case 0xFF: {
        struct _TOS *context = block->BaseBlock.Object.pTos;

        FreeBacktrace(block);
        if (block->pDynProg != NULL) {
            heap_Free(context->hHeap, block->pDynProg);
        }
        block->pDynProg = NULL;
        tosBaseBlockDestruct(&block->BaseBlock);
        break;
    }
    case 5:
        block->optionalPaths = CParam1;
        break;
    case 6:
        *(u8 *) CParam1 = block->optionalPaths;
        break;
    default:
        return 0;
    }

    return 1;
}

/* Constructs a TOS block for user-word scoring. */
struct _BLOCK *ConstructDpScoreUw(struct _TOS *pTos, u8 BlockID)
{
    _DPSCOREUW *block =
        (_DPSCOREUW *) tosBaseBlockConstruct(pTos, BlockID, 1, 1, IdleDpScoreUwProcess,
                                             InitDpScoreUw, ControlDpScoreUw, sizeof(_DPSCOREUW));

    block->MakeActiveDist = MakeScoreUserWordsActiveDist;
    block->SendResult = SendResultScoreUserWords;
    return &block->BaseBlock;
}
