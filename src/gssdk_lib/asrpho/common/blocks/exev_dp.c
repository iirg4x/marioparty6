/* Scores extra-event state chains used by the rec1600 speech recognizer. */
#include "gssdk/blocks.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/secondpass.h"
#include "types.h"

#include "gssdk/tos.h"

/* One configured state ID and its accumulated path cost. */
struct _EXEVACTIVEDPSTATE {
    long Score; /* Accumulated path cost for this state. */
    unsigned long StateIndex; /* Dictionary state index used to look up frame costs. */
};

/* Paths and scoring callbacks for the extra-event recognizer. */
typedef struct {
    struct _BLOCK BaseBlock; /* TOS lifecycle and callback state for this block. */
    struct _INPUTQUEUE *LabelInQueue; /* Single input port whose frames drive scoring. */
    void *pDistCalc; /* Opaque data passed to the configured score callbacks. */
    short (*CalculateDistance)(void *, unsigned int); /* Computes a state ID's added cost. */
    long (*GetDistanceOffset)(void *); /* Computes an input frame's added cost. */
    short *(*GetpCachedDistances_DISTBLK)(void *); /* Supplies the cache indexed by state ID. */
    short *pCachedDistances_DISTBLK; /* Cached state costs; 32767 marks a cache miss. */
    unsigned int NbrPhenomes; /* Number of entries in the state table and dictionary. */
    struct _DICTSTATE *pStateInDict; /* State entries used to seed the scoring table. */
    long BestScore; /* Lower of the first and terminal state costs. */
    long AccumulatedScoreOffset; /* Sum of frame costs processed by this block. */
    /* Processed-frame counter; initialized to 65535 before the first frame. */
    unsigned short u16FrameCounter;
    long Rejection_NBS_penalty; /* Cost added when a transition chain advances. */
    long Rejection_NBS_path_penalty; /* Initial cost assigned to the first state. */
    unsigned long ExtraEventSilencePhenome; /* The game's code never reads this field. */
    unsigned long NbrOfPronunciations_RejectionWord; /* Number of configured transition chains. */
    /* State-table boundaries delimiting each transition chain. */
    unsigned short *pBeginOfPronunciationsInDict;
    struct _EXEVACTIVEDPSTATE *pDynProg; /* Dynamic-programming costs for configured states. */
} ExtraEventDP;

/* Command 2 calls this to restart scoring and seed the configured state chain. */
static void InitViterbi(ExtraEventDP *eventBlock)
{
    struct _EXEVACTIVEDPSTATE *currentState;
    u32 stateIndex = 0;

    eventBlock->BestScore = 0;
    eventBlock->AccumulatedScoreOffset = 0;
    eventBlock->u16FrameCounter = 65535;

    currentState = eventBlock->pDynProg;
    currentState->Score = eventBlock->Rejection_NBS_path_penalty;
    currentState->StateIndex = eventBlock->pStateInDict[0].StateIndex;
    currentState++;
    stateIndex++;

    while (stateIndex < eventBlock->NbrPhenomes) {
        /* Start unvisited states at the score ceiling, below the signed long limit. */
        currentState->Score = 2147483647 - 2048;
        currentState->StateIndex = eventBlock->pStateInDict[stateIndex].StateIndex;
        stateIndex++;
        currentState++;
    }
}

/* The process callback calls this for each input frame to update the state costs. */
static void DynProgExtraEventsProcess(ExtraEventDP *eventBlock)
{
    s32 chainScore;
    struct _EXEVACTIVEDPSTATE *stateTable;
    struct _EXEVACTIVEDPSTATE *terminalState;
    struct _EXEVACTIVEDPSTATE *transitionState;
    struct _EXEVACTIVEDPSTATE *transitionEnd;
    struct _EXEVACTIVEDPSTATE *scoredState;
    u16 stateIndex;
    s16 *stateScoreCache;
    u16 transitionIndex = 1;
    s32 candidateScore;
    s32 previousTerminalScore;

    candidateScore = 2147483647; /* Highest signed long score. */
    stateTable = eventBlock->pDynProg;
    terminalState = &stateTable[eventBlock->NbrPhenomes - 1];
    stateScoreCache = eventBlock->pCachedDistances_DISTBLK;
    previousTerminalScore = terminalState->Score;

    for (; transitionIndex <= eventBlock->NbrOfPronunciations_RejectionWord; transitionIndex++) {
        chainScore = stateTable[0].Score + eventBlock->Rejection_NBS_penalty;
        transitionState =
            &eventBlock->pDynProg[eventBlock->pBeginOfPronunciationsInDict[transitionIndex]];
        transitionEnd =
            &eventBlock->pDynProg[eventBlock->pBeginOfPronunciationsInDict[transitionIndex + 1]]
            - 1;

        while (transitionState < transitionEnd) {
            candidateScore = chainScore;
            chainScore = transitionState->Score;
            if (candidateScore < chainScore) {
                transitionState->Score = candidateScore;
            }
            transitionState++;
        }

        /* Each chain is compared with the terminal cost saved before this update. */
        if (transitionState->Score < previousTerminalScore) {
            terminalState->Score = transitionState->Score;
        }
        if (candidateScore < transitionState->Score) {
            transitionState->Score = candidateScore;
        }
    }

    if (previousTerminalScore < stateTable[0].Score) {
        stateTable[0].Score = terminalState->Score;
    }

    scoredState = eventBlock->pDynProg;
    for (stateIndex = 0; stateIndex < eventBlock->NbrPhenomes; stateIndex++) {
        s16 callbackScore;
        if ((callbackScore = stateScoreCache[scoredState->StateIndex]) == 32767) {
            callbackScore =
                eventBlock->CalculateDistance(eventBlock->pDistCalc, scoredState->StateIndex);
            stateScoreCache[scoredState->StateIndex] = callbackScore;
        }

        scoredState->Score += callbackScore;
        /* Leave headroom for the next frame's score update. */
        if (scoredState->Score > 2147483647 - 2048) {
            scoredState->Score = 2147483647 - 2048; /* Score ceiling. */
        }
        scoredState++;
    }

    if (stateTable[0].Score < terminalState->Score) {
        eventBlock->BestScore = stateTable[0].Score;
    } else {
        eventBlock->BestScore = terminalState->Score;
    }
}

/* TOS calls this process callback; only a non-null first input advances frame scoring. */
static u8 ProcessExtraEventDP(ExtraEventDP *eventBlock, void *pInputs, int NbrInputs)
{
    if (((void **)pInputs)[0] != NULL) {
        eventBlock->u16FrameCounter++;
        eventBlock->AccumulatedScoreOffset += eventBlock->GetDistanceOffset(eventBlock->pDistCalc);
        DynProgExtraEventsProcess(eventBlock);
    }

    return 0;
}

/* Binds the input port and applies the element size from profile key 15. */
static unsigned char InitExtraEventDP(ExtraEventDP *This)
{
    u32 inputElementSize;

    This->LabelInQueue = This->BaseBlock.InputQueues;
    inputElementSize = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x0F, 4);
    This->LabelInQueue->Size = inputElementSize;
    return 0;
}

/* Releases the state costs before changing or destroying a recognition context. */
static void FreeDynprog(ExtraEventDP *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    if (This->pDynProg != NULL) {
        heap_Free(pTos->hHeap, This->pDynProg);
        This->pDynProg = NULL;
    }
}

/* Installs the state chains and allocates their accumulated costs. */
static void InitDynprogContext(ExtraEventDP *This, struct _DYNPROG_EXEV_CTXINFO *pCtx)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    This->NbrOfPronunciations_RejectionWord = pCtx->NbrOfPronunciations_RejectionWord;
    This->Rejection_NBS_penalty = pCtx->Rejection_NBS_penalty;
    This->Rejection_NBS_path_penalty = pCtx->Rejection_NBS_path_penalty;
    This->NbrPhenomes = pCtx->uNrStatesInDict;
    This->pStateInDict = pCtx->pStateInDict;
    This->pBeginOfPronunciationsInDict = pCtx->pBeginOfPronunciationsInDict;
    This->pCachedDistances_DISTBLK = This->GetpCachedDistances_DISTBLK(This->pDistCalc);
    This->pDynProg = heap_Alloc(pTos->hHeap, This->NbrPhenomes * sizeof(struct _EXEVACTIVEDPSTATE));
    if (This->pDynProg == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x01);
    }
}

/* TOS calls this control callback for score queries, startup, configuration, and shutdown. */
static unsigned char ControlExtraEventDP(ExtraEventDP *This, unsigned char ControlId,
                                         unsigned long CParam)
{
    switch (ControlId) {
    case 0x01:
        if (CParam != 0) {
            *(s32 *) CParam = This->BestScore + This->AccumulatedScoreOffset;
        } else {
            return 1;
        }
        break;
    case 0x02:
        InitViterbi(This);
        /* Dispatch ignores the extra-event scorer's byte status. */
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int)) ProcessExtraEventDP;
        break;
    case 0x03:
        /* This control ID is accepted without changing the extra-event scorer. */
        break;
    case 0x07:
        /* This control ID is accepted without changing the extra-event scorer. */
        break;
    case 0x08: {
        struct _DYNPROG_EXEV_CTXINFO *pCtx = (struct _DYNPROG_EXEV_CTXINFO *)CParam;

        FreeDynprog(This);

        InitDynprogContext(This, pCtx);
        break;
    }
    case 0x06: {
        struct _DISTCALCFUNCS *pCtx = (struct _DISTCALCFUNCS *)CParam;

        This->CalculateDistance = pCtx->CalculateDistance;
        This->GetpCachedDistances_DISTBLK = pCtx->GetpCachedDistances;
        This->GetDistanceOffset = pCtx->GetDistanceOffset;
        This->pDistCalc = pCtx->pThis;
        break;
    }
    case 0xFF: {
        FreeDynprog(This);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    }
    default:
        return 0;
    }

    return 1;
}

/* The rec1600 block table uses this constructor for its extra-event scoring block. */
struct _BLOCK *ConstructExtraEventDp(struct _TOS *pTos, u8 BlockID)
{
    /* The base pointer addresses the ExtraEventDP prefix. Dispatch ignores the process status.
     * The control handler reads the command and its first payload word. */
    return tosBaseBlockConstruct(pTos, BlockID, 1, 0,
                                 (void (*)(struct _BLOCK *, void *, int)) ProcessExtraEventDP,
                                 (u8 (*)(struct _BLOCK *)) InitExtraEventDP,
                                 (u8 (*)(struct _BLOCK *, u8, u32, u32)) ControlExtraEventDP,
                                 sizeof(ExtraEventDP));
}
