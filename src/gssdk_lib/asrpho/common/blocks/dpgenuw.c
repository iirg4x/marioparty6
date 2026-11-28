/* Generates user-word models with a Viterbi search and backtrace. */
#include "gssdk/engine_params.h"
#include "gssdk/blocks.h"
#include "gssdk/heap.h"
#include "types.h"

#include <stddef.h>
#include <string.h>

#include "gssdk/engine.h"
#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Forward declaration for the path node. */
typedef struct _GENUWBACKTRACE _GENUWBACKTRACE;

/* One backtrace node with a distribution, frame and reference count. */
struct _GENUWBACKTRACE {
    _GENUWBACKTRACE *pPreviousBt; /* Earlier path node or next node in the free list. */
    u16 State; /* Distribution at the end of the path. */
    s16 Count; /* References from live paths and newer nodes. */
    u32 ExitTime; /* Frame when the path left the distribution. */
};

/* Candidate user-word state and the path that reached it. */
typedef struct _GENUWACTIVEDPSTATE {
    u16 State; /* Distribution at the end of the path. */
    s16 Score; /* Score of the candidate path. */
    _GENUWBACKTRACE *pBacktrace; /* Path that reached this state. */
} _GENUWACTIVEDPSTATE;

/* Forward declaration for the user-word generation block. */
typedef struct _DPGENUW _DPGENUW;

/* Dynamic-programming state and backtrace storage for user-word models. */
struct _DPGENUW {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    struct _INPUTQUEUE *InQueue; /* Signed distances received from the distance calculator. */
    /* Active distribution IDs and flags sent to the distance calculator. */
    struct _OUTPUTQUEUE *ActDistQueue;
    u16 uCountSilence; /* The game's code never reads this field. */
    u16 uNbrWords; /* The game's code never reads this field. */
    s16 WordTransPenal; /* Score penalty for leaving the silence path. */
    s16 PhenTransPenal; /* Score penalty for leaving a phoneme path. */
    /* Current scores and backtraces for two silence paths and twenty candidates. */
    _GENUWACTIVEDPSTATE *pDynProg;
    _GENUWBACKTRACE **pBacktraceArrs; /* Allocated backtrace chunks. */
    _GENUWBACKTRACE *BacktraceFreelist; /* Nodes available for another backtrace. */
    u32 uCountSilenceThreshold; /* The game's code never reads this field. */
    long OldWord; /* The game's code never reads this field. */
    u16 NbrErgodicPhenoms; /* Number of ergodic phonemes in the language. */
    u16 *pErgodicPhenoms; /* The game's code never reads this field. */
    s16 OffsetPhenToStat; /* The game's code never reads this field. */
    u32 NbrPhenoms; /* Number of distributions reported by the active language. */
    u32 NbrPhenInUW; /* The game's code never reads this field. */
    u16 UWNbrPhenoms; /* Number of candidate distributions copied from the initialization record. */
    u16 *UWpPhenoms; /* Phoneme candidates copied from the initialization record. */
    u16 UWSilPhenom; /* Silence distribution copied from the initialization record. */
    u32 AccumulatedScore; /* Sum of the score floors subtracted from the paths. */
    u32 UserWordScore; /* Score of the completed word path. */
    long OldSize; /* The game's code never reads this field. */
    s16 sMin; /* Minimum score used to normalize the paths. */
    _GENUWACTIVEDPSTATE *pStsMin; /* The game's code never reads this field. */
    u8 EndOfSpeech; /* The game's code never reads this field. */
    u32 FrameCounter; /* Current frame index. */
    u32 TimeOut; /* Maximum number of frames before generation stops. */
    /* Writes the active-distribution packet. */
    u16 (*MakeActiveDist)(_DPGENUW *This, u16 *ActiveDist, u8 *ActiveBoolList);
    enum _ACTDISTSTATE ActDistState; /* Current active-distribution mode. */
    void (*SendResult)(_DPGENUW *This, u32 CParam, u32 DParam); /* Reports the completed path. */
    s16 BestScores[20]; /* Scratch scores used to rank the candidate states. */
    u16 BestStates[20]; /* State numbers paired with the candidate scores. */
    u32 NbrBacktraceNodes; /* Counts live backtrace nodes. */
    u32 MaxNbrBacktraceNodes; /* The game's code never reads this field. */
    u16 NbrBacktraceArrs; /* Number of allocated chunks in pBacktraceArrs. */
    u16 AllocSize; /* Size in bytes of a backtrace allocation. */
};

/* Allocates a chunk of backtrace nodes and appends it to the free list. */
static _GENUWBACKTRACE *AllocateBacktrace(_DPGENUW *This, _GENUWBACKTRACE *pEndFree)
{
    u16 NbrOfArrays = This->NbrBacktraceArrs;
    _GENUWBACKTRACE **pNewArrs;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _GENUWBACKTRACE *pExtra;

    pNewArrs = heap_Alloc(pTos->hHeap, (NbrOfArrays + 1) * sizeof(_GENUWBACKTRACE *));

    if (pNewArrs == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return NULL;
    }

    if (NbrOfArrays != 0) {
        memcpy(pNewArrs, This->pBacktraceArrs, NbrOfArrays * sizeof(_GENUWBACKTRACE *));
        heap_Free(pTos->hHeap, This->pBacktraceArrs);
    }
    This->pBacktraceArrs = pNewArrs;

    pExtra = heap_Alloc(pTos->hHeap,
                        (This->AllocSize / sizeof(_GENUWBACKTRACE)) * sizeof(_GENUWBACKTRACE));
    if (pExtra == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return NULL;
    }

    if (pEndFree != NULL) {
        unsigned int Max = This->AllocSize / sizeof(_GENUWBACKTRACE) - 1;
        unsigned int i;

        pEndFree->pPreviousBt = pExtra;
        for (i = 0; i < Max; i++) {
            pExtra[i].pPreviousBt = &pExtra[i + 1];
        }
        pExtra[i].pPreviousBt = NULL;
    }

    This->pBacktraceArrs[NbrOfArrays++] = pExtra;
    This->NbrBacktraceArrs = NbrOfArrays;
    return pExtra;
}

/* Frees the allocated chunks of backtrace nodes. */
static void FreeBacktrace(_DPGENUW *This)
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

/* Initializes the backtrace pool and returns its first node. */
static _GENUWBACKTRACE *InitBacktrace(_DPGENUW *This)
{
    u16 i;
    u32 Max = This->AllocSize / sizeof(_GENUWBACKTRACE);
    u32 lastBacktraceIndex = Max - 1;
    _GENUWBACKTRACE *pOld = NULL;

    for (i = 0; i < This->NbrBacktraceArrs; i++) {
        _GENUWBACKTRACE *pBt = This->pBacktraceArrs[i];
        s32 j;

        if (pOld != NULL) {
            pOld->pPreviousBt = pBt;
        }

        for (j = 0; j < lastBacktraceIndex; j++) {
            pBt->pPreviousBt = pBt + 1;
            pBt++;
        }
        pOld = pBt;
    }

    if (pOld != NULL) {
        pOld->pPreviousBt = NULL;
    }

    pOld = This->pBacktraceArrs[0];
    pOld->Count = 0;
    pOld->State = 0xFFFF;
    This->BacktraceFreelist = pOld->pPreviousBt;
    return pOld;
}

/* Resets the Viterbi states and scores for a new utterance. */
static void InitViterbi(_DPGENUW *This)
{
    _GENUWBACKTRACE *backtrace;
    _GENUWACTIVEDPSTATE *state;
    u32 stateIndex;

    This->uCountSilence = 0;
    This->OldWord = 0;
    This->FrameCounter = -1;
    This->sMin = 0;

    backtrace = InitBacktrace(This);
    backtrace->pPreviousBt = NULL;
    backtrace->State = 0;
    backtrace->ExitTime = 0;

    state = This->pDynProg;
    for (stateIndex = 0; stateIndex < 20; stateIndex++, state++) {
        state->Score = 31622;
        state->State = This->UWpPhenoms[stateIndex];
        state->pBacktrace = backtrace;
    }

    state->Score = 0;
    state->State = This->UWSilPhenom;
    state->pBacktrace = backtrace;
    state++;

    state->Score = 31622;
    state->State = This->UWSilPhenom;
    state->pBacktrace = backtrace;

    backtrace->Count = 22;
    This->AccumulatedScore = 0;
    This->NbrBacktraceNodes = 0;
}

/* Initializes the user-word generation block and its queues. */
static u8 InitDpGenUw(struct _BLOCK *_This)
{
    _DPGENUW *block = (_DPGENUW *) _This;
    u16 distributionCount;

    block->uCountSilenceThreshold = (u16) ((s32) _tosGetProfileU32(&_This->Object, 2, 1) / 10);

    distributionCount = _tosGetProfileU32(&_This->Object, 0x13, 1);
    block->InQueue = _This->InputQueues;
    block->ActDistQueue = _This->OutputQueues;
    block->InQueue->Size = distributionCount * sizeof(s16);
    /* Each distribution contributes a two-byte ID and a one-byte activity flag. */
    block->ActDistQueue->Size = distributionCount * 3;

    block->AllocSize = _tosGetProfileU32(&_This->Object, 0x11, 1500);
    if (block->AllocSize > 65000) {
        block->AllocSize = 65000;
    } else if (block->AllocSize < 200) {
        block->AllocSize = 200;
    }

    block->TimeOut = (u16) _tosGetProfileU32(&_This->Object, 0x0B, 0) * 100;
    block->PhenTransPenal = (s16) _tosGetProfileU32(&_This->Object, 0x14, 120);
    block->WordTransPenal = (s16) _tosGetProfileU32(&_This->Object, 0x23, 900);
    block->NbrBacktraceArrs = 0;
    return 0;
}

/* Reports the selected user-word distributions and their speech duration. */
static void SendResultUserWords(_DPGENUW *block, u32 CParam, u32 DParam)
{
    struct _TOS *context = block->BaseBlock.Object.pTos;
    _GENUWBACKTRACE *currentBacktrace;
    _USERWORDRES *result;
    int backtraceIndex;
    u16 backtraceCount;
    u32 lastFrame;
    u32 speechDuration;

    currentBacktrace = block->pDynProg[21].pBacktrace;
    lastFrame = currentBacktrace->ExitTime;
    backtraceCount = 0;
    speechDuration = 0;

    while (currentBacktrace->pPreviousBt != NULL) {
        if (currentBacktrace->State == block->UWSilPhenom) {
            speechDuration += lastFrame - currentBacktrace->ExitTime;
            lastFrame = 0;
        } else if (lastFrame == 0) {
            lastFrame = currentBacktrace->ExitTime;
        }

        currentBacktrace = currentBacktrace->pPreviousBt;
        backtraceCount++;
    }

    if (backtraceCount != 0) {
        backtraceCount--;
    }

    result = heap_Alloc(context->hHeap, sizeof(_USERWORDRES) + (backtraceCount - 1) * sizeof(u16));
    if (result == NULL) {
        _tosErrorLog(&block->BaseBlock.Object, 1);
        return;
    }

    result->SpeechFrames = speechDuration;
    result->NbrPhenoms = backtraceCount;
    if (block->FrameCounter != 0) {
        result->NormScore =
            (f32) (block->UserWordScore + block->AccumulatedScore) / (f32) block->FrameCounter;
    } else {
        result->NormScore = 0.0f;
    }

    currentBacktrace = block->pDynProg[21].pBacktrace;
    for (backtraceIndex = backtraceCount - 1; backtraceIndex >= 0;) {
        result->pPhenoms[backtraceIndex--] = currentBacktrace->State;
        currentBacktrace = currentBacktrace->pPreviousBt;
    }

    if (_tosControl(&block->BaseBlock.Object, 0x0C, (u32) result, 1)) {
        _tosErrorLog(&block->BaseBlock.Object, 1);
    }
    heap_Free(context->hHeap, result);
}

/* Puts the phoneme count in the active-distribution output. */
static u16 MakeUserWordsActiveDist(_DPGENUW *This, u16 *ActiveDist, u8 *active)
{
    ActiveDist[0] = This->NbrPhenoms;
    return ActiveDist[0];
}

/* Returns unshared backtrace nodes to the free list. */
static void CleanUp(_DPGENUW *This, _GENUWBACKTRACE *pFirst)
{
    _GENUWBACKTRACE *pPrev = pFirst;
    _GENUWBACKTRACE *pBt = pFirst->pPreviousBt;

    This->NbrBacktraceNodes--;
    while (pBt->Count <= 1) {
        pPrev = pBt;
        pBt = pBt->pPreviousBt;
        This->NbrBacktraceNodes--;
    }
    pBt->Count--;
    pPrev->pPreviousBt = This->BacktraceFreelist;
    This->BacktraceFreelist = pFirst;
}

/* Caps the live state scores at the maximum. */
static void FloorStates(_DPGENUW *This)
{
    _GENUWACTIVEDPSTATE *pDp1 = This->pDynProg;
    unsigned short j;

    for (j = 1; j < 22; j++, pDp1++) {
        if (pDp1->Score > 31622) {
            pDp1->Score = 31622;
        }
    }
}

/* Finds the lowest-scoring candidate, keeping the first state on a tie. */
static _GENUWACTIVEDPSTATE *GetMinimalState(_DPGENUW *This)
{
    _GENUWACTIVEDPSTATE *pDp1;
    _GENUWACTIVEDPSTATE *pMin;
    unsigned short j;

    pMin = This->pDynProg;
    pDp1 = This->pDynProg + 1;
    for (j = 1; j < 20; j++, pDp1++) {
        if (pDp1->Score < pMin->Score) {
            pMin = pDp1;
        }
    }
    return pMin;
}

/* Finds the highest-scoring candidate, keeping the last state on a tie. */
static _GENUWACTIVEDPSTATE *GetMaximalState(_DPGENUW *This)
{
    _GENUWACTIVEDPSTATE *pDp1;
    _GENUWACTIVEDPSTATE *pMax;
    unsigned short j;

    pMax = This->pDynProg;
    pDp1 = This->pDynProg + 1;
    for (j = 1; j < 20; j++, pDp1++) {
        if (pDp1->Score >= pMax->Score) {
            pMax = pDp1;
        }
    }
    return pMax;
}

/* Stores a state score, distribution and backtrace, and retains the backtrace. */
static void UpdateState(_GENUWACTIVEDPSTATE *pState, unsigned short State, int Score,
                        _GENUWBACKTRACE *pBt)
{
    pState->Score = Score;
    pState->State = State;
    pState->pBacktrace = pBt;
    pBt->Count++;
}

/* Advances the user-word Viterbi search using the current frame's distances. */
static int DynProgUserWords(_DPGENUW *This, s16 *pDistances)
{
    _GENUWACTIVEDPSTATE *pDp1;
    _GENUWBACKTRACE *pBPtr;
    _GENUWBACKTRACE *NewBacktrace;
    unsigned int St;
    int scoreMin;
    int MinScoreNS;
    int MinScoreAll;
    unsigned short *pPhenom;
    _GENUWACTIVEDPSTATE *pMinNSState;
    _GENUWACTIVEDPSTATE *pMinAllState;
    unsigned short MinNSStateNr;
    unsigned short MinAllStateNr;
    int LSLocScore;
    _GENUWACTIVEDPSTATE *pMaxAct;
    _GENUWBACKTRACE *pNSBt;
    _GENUWBACKTRACE *pAllBt;
    int iScore;

    FloorStates(This);
    pMinNSState = GetMinimalState(This);

    pDp1 = &This->pDynProg[20];
    scoreMin = pMinNSState->Score;
    MinNSStateNr = pMinNSState->State;
    MinScoreNS = scoreMin + This->PhenTransPenal;

    if (pDp1->Score < scoreMin) {
        scoreMin = pDp1->Score;
    }

    MinScoreAll = pDp1->Score + This->WordTransPenal;
    if (MinScoreAll < MinScoreNS) {
        MinAllStateNr = pDp1->State;
        pMinAllState = pDp1;
    } else {
        MinScoreAll = MinScoreNS;
        MinAllStateNr = MinNSStateNr;
        pMinAllState = pMinNSState;
    }

    LSLocScore = pDistances[pDp1->State];
    if (pDp1[1].Score < scoreMin) {
        scoreMin = pDp1[1].Score;
    }
    if (pDp1[1].Score + This->WordTransPenal < MinScoreAll) {
        MinAllStateNr = pDp1[1].State;
        MinScoreAll = pDp1[1].Score + This->WordTransPenal;
        pMinAllState = &pDp1[1];
    }

    pBPtr = pMinAllState->pBacktrace;
    NewBacktrace = This->BacktraceFreelist;
    if (NewBacktrace->pPreviousBt == NULL) {
        This->BacktraceFreelist = AllocateBacktrace(This, NewBacktrace);
    } else {
        This->BacktraceFreelist = NewBacktrace->pPreviousBt;
    }
    NewBacktrace->pPreviousBt = pBPtr;
    pBPtr->Count++;
    NewBacktrace->State = MinAllStateNr;
    NewBacktrace->Count = 0;
    NewBacktrace->ExitTime = This->FrameCounter;
    This->NbrBacktraceNodes++;
    pNSBt = NewBacktrace;
    pNSBt->Count++;

    if (MinNSStateNr != MinAllStateNr) {
        pBPtr = pMinNSState->pBacktrace;
        NewBacktrace = This->BacktraceFreelist;
        if (NewBacktrace->pPreviousBt == NULL) {
            This->BacktraceFreelist = AllocateBacktrace(This, NewBacktrace);
        } else {
            This->BacktraceFreelist = NewBacktrace->pPreviousBt;
        }
        NewBacktrace->pPreviousBt = pBPtr;
        pBPtr->Count++;
        NewBacktrace->State = MinNSStateNr;
        NewBacktrace->Count = 0;
        NewBacktrace->ExitTime = This->FrameCounter;
        This->NbrBacktraceNodes++;
        pAllBt = NewBacktrace;
        pAllBt->Count++;
    } else {
        pAllBt = pNSBt;
        pAllBt->Count++;
    }

    pDp1 = This->pDynProg;
    for (St = 0; St < 20; St++) {
        if (pDp1->Score < MinScoreAll) {
            pDp1->Score = pDp1->Score + pDistances[pDp1->State] - scoreMin;
        } else {
            pDp1->Score = MinScoreAll + pDistances[pDp1->State] - scoreMin;
            pBPtr = pDp1->pBacktrace;
            pDp1->pBacktrace = pNSBt;
            pNSBt->Count++;
            if (--pBPtr->Count <= 0) {
                CleanUp(This, pBPtr);
            }
        }

        pDistances[pDp1->State] = 1024;
        pDp1++;
    }

    pDp1->Score = pDp1->Score + LSLocScore - scoreMin;
    if (pDp1[1].Score <= MinScoreNS) {
        pDp1[1].Score = pDp1[1].Score + LSLocScore - scoreMin;
    } else {
        pDp1[1].Score = MinScoreNS + LSLocScore - scoreMin;
        pBPtr = pDp1[1].pBacktrace;
        pDp1[1].pBacktrace = pAllBt;
        pAllBt->Count++;
        if (--pBPtr->Count <= 0) {
            CleanUp(This, pBPtr);
        }
    }
    This->UserWordScore = (u16) pDp1[1].Score;

    pMaxAct = GetMaximalState(This);

    pPhenom = This->UWpPhenoms;
    for (St = 0; St < This->UWNbrPhenoms; St++, pPhenom++) {
        iScore = MinScoreAll + pDistances[*pPhenom] - scoreMin;

        if (iScore < pMaxAct->Score) {
            pBPtr = pMaxAct->pBacktrace;

            UpdateState(pMaxAct, *pPhenom, iScore, pNSBt);
            if (--pBPtr->Count <= 0) {
                CleanUp(This, pBPtr);
            }

            pMaxAct = GetMaximalState(This);
        }
    }

    if (--pNSBt->Count <= 0) {
        CleanUp(This, pNSBt);
    }
    if (--pAllBt->Count <= 0) {
        CleanUp(This, pAllBt);
    }
    This->AccumulatedScore += (u16) scoreMin;
    return 0;
}

/* Leaves input frames unprocessed while user-word generation is idle. */
static void IdleDpGenUwProcess(struct _BLOCK *baseBlock, void *inputs, int inputCount)
{
}

/* Consumes one frame of distribution scores and advances user-word generation. */
static void DpGenUwProcess(struct _BLOCK *_This, void *pInputs, int inputCount)
{
    _DPGENUW *This = (_DPGENUW *) _This;
    void **pInputQueues = pInputs;
    s16 *pDistances = pInputQueues[0];

    if (pDistances != NULL) {
        s32 i;
        s16 *pDist = pDistances;

        for (i = 0; i < This->NbrErgodicPhenoms;) {
            if (i != This->UWSilPhenom) {
                *pDist = 1020;
            }
            i++;
            pDist++;
        }

        This->FrameCounter++;
        if (This->FrameCounter == 0) {
            _GENUWACTIVEDPSTATE *state = &This->pDynProg[20];

            state->Score = pDistances[This->UWSilPhenom];
            This->sMin = state->Score;
        } else if (DynProgUserWords(This, pDistances) != 0) {
            _tosErrorLog(&This->BaseBlock.Object, 0x77);
            This->BaseBlock.Process = IdleDpGenUwProcess;
            _tosControl(&This->BaseBlock.Object, 0x0B, 0, 0);
        }

        {
            u16 *ActiveDist = qEnQueueOne(This->ActDistQueue->pQ);

            /* The frame packet puts flags before the final distance-width slot. */
            This->MakeActiveDist(This, ActiveDist,
                                 (u8 *) ActiveDist + This->InQueue->Size - sizeof(u16));
        }

        if (This->TimeOut != 0 && This->FrameCounter > This->TimeOut) {
            This->BaseBlock.Process = IdleDpGenUwProcess;
            if (_tosControl(&This->BaseBlock.Object, 0x08, This->FrameCounter, 0)) {
                _tosErrorLog(&This->BaseBlock.Object, 1);
            }
        }
    }
}

/* Releases the allocated backtrace chunks and dynamic-programming states. */
static void ResetAllocations(_DPGENUW *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    FreeBacktrace(This);
    if (This->pDynProg != NULL) {
        heap_Free(pTos->hHeap, This->pDynProg);
    }
    This->pDynProg = NULL;
}

/* Initializes the phoneme distributions used to train user words. */
static u8 InitUserWords(_DPGENUW *block, struct _INITUSERWORDDATA *pInitData)
{
    struct _TOS *context = block->BaseBlock.Object.pTos;

    block->UWNbrPhenoms = pInitData->NbrPhenoms;
    block->UWpPhenoms = pInitData->pPhenoms;
    block->UWSilPhenom = pInitData->SilPhenome;
    block->InQueue->Size = block->InQueue->pQ->Size;
    block->uNbrWords = block->NbrPhenoms;

    ResetAllocations(block);

    if (AllocateBacktrace(block, NULL) == NULL) {
        return _tosErrorLog(&block->BaseBlock.Object, 0x71);
    }
    block->pDynProg = heap_Alloc(context->hHeap, 22 * sizeof(_GENUWACTIVEDPSTATE));
    if (block->pDynProg == NULL) {
        return _tosErrorLog(&block->BaseBlock.Object, 0x71);
    }
    block->EndOfSpeech = 0;
    InitViterbi(block);
    return 0;
}

/* Loads the phoneme counts and pointers used for user-word generation. */
static inline u8 InitRecognitionLanguage(_DPGENUW *This, struct _LANGUAGE *pLang)
{
    This->NbrErgodicPhenoms = pLang->langGetNbrErgodicPhenomes(pLang);
    This->pErgodicPhenoms = pLang->langGetpErgodicPhenomes(pLang);
    This->OffsetPhenToStat = -This->NbrErgodicPhenoms;
    This->NbrPhenoms = pLang->langGetNbrPhenome(pLang);
    This->NbrPhenInUW = pLang->langGetNbrPhenUserWordTraining(pLang);
    if (This->ActDistQueue->pQ != NULL) {
        This->ActDistQueue->Size = This->ActDistQueue->pQ->Size;
    }
    return 0;
}

/* Handles initialization, generation and teardown commands for the block. */
static u8 ControlDpGenUw(struct _BLOCK *_This, u8 ControlId, u32 CParam1, u32 CParam2)
{
    _DPGENUW *block = (_DPGENUW *) _This;

    switch (ControlId) {
    case 4:
        block->BaseBlock.Process = IdleDpGenUwProcess;
        block->SendResult(block, CParam1, CParam2);
        /* Fall through. */
    case 0x15:
        break;
    case 3:
        InitViterbi(block);
        qQueueReset(block->ActDistQueue->pQ);
        {
            u16 *output = qEnQueueOne(block->ActDistQueue->pQ);

            /* The startup packet puts flags after the full distance extent. */
            block->MakeActiveDist(block, output, (u8 *) output + block->InQueue->Size);
        }
        block->BaseBlock.Process = DpGenUwProcess;
        break;
    case 5:
        block->BaseBlock.Process = IdleDpGenUwProcess;
        break;
    case 0x22:
        InitRecognitionLanguage(block, (struct _LANGUAGE *)CParam1);
        break;
    case 0x17:
        if (InitUserWords(block, (struct _INITUSERWORDDATA *) CParam1)) {
            return 0;
        }
        break;
    case 0xFF:
        ResetAllocations(block);
        tosBaseBlockDestruct(&block->BaseBlock);
        break;
    /* Transfers the phoneme and word transition penalties in TSCount and TSEnable. */
    case 0x1A:
        block->PhenTransPenal = ((struct _DYNPROG_PARAM *) CParam1)->TSCount;
        block->WordTransPenal = ((struct _DYNPROG_PARAM *) CParam1)->TSEnable;
        break;
    case 0x1B:
        ((struct _DYNPROG_PARAM *) CParam1)->TSCount = block->PhenTransPenal;
        ((struct _DYNPROG_PARAM *) CParam1)->TSEnable = block->WordTransPenal;
        break;
    default:
        return 0;
    }

    return 1;
}

/* Constructs a TOS block for user-word model generation. */
struct _BLOCK *ConstructDpGenUw(struct _TOS *pTos, u8 BlockID)
{
    _DPGENUW *block = (_DPGENUW *) tosBaseBlockConstruct(
        pTos, BlockID, 1, 1, IdleDpGenUwProcess, InitDpGenUw, ControlDpGenUw, sizeof(_DPGENUW));

    block->MakeActiveDist = MakeUserWordsActiveDist;
    block->SendResult = SendResultUserWords;
    return &block->BaseBlock;
}
