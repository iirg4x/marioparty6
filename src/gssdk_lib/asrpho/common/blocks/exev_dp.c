/* Scores extra-event state chains used by the rec1600 speech recognizer. */
#include "types.h"

#include "gssdk/tos.h"

#define EXTRA_EVENT_SCORE_INFINITY 2147483647
#define EXTRA_EVENT_MAX_SCORE (EXTRA_EVENT_SCORE_INFINITY - 2048)

typedef s16 (*ExtraEventStateScoreFunction)(void *callbackContext, u32 stateId);
typedef s32 (*ExtraEventFrameScoreFunction)(void *callbackContext);
typedef s16 *(*ExtraEventScoreCacheFunction)(void *callbackContext);

typedef struct ExtraEventCallbacks {
void *callbackContext; /* Opaque data passed to each score callback. */
ExtraEventStateScoreFunction readStateScore; /* Returns the added cost for a state ID. */
void *reservedCallback; /* Unused callback slot in this bundle. */
ExtraEventScoreCacheFunction getScoreCache; /* Supplies cached state costs. */
ExtraEventFrameScoreFunction readFrameScore; /* Returns the added cost for an input frame. */
} ExtraEventCallbacks;

typedef struct ExtraEventContextInfo {
u32 stateCount; /* Number of configured state IDs. */
u16 *stateIds; /* State IDs indexed by the dynamic-programming state table. */
u16 *transitionBounds; /* Start and one-past-end state indices for each transition chain. */
u32 transitionCount; /* Number of transition chains. */
s32 transitionScore; /* Cost added when a chain advances from its first state. */
s32 initialScore; /* Starting cost assigned to the first state. */
} ExtraEventContextInfo;

typedef struct ExtraEventState {
s32 score; /* Accumulated signed cost for this state. */
u32 stateId; /* ID used by the score callback and cache. */
} ExtraEventState;

typedef struct ExtraEventDP {
TosBaseBlock base; /* TOS lifecycle and callback state for this block. */
TosQueuePort *input; /* Single input port whose frames drive scoring. */
void *callbackContext; /* Opaque data passed to the configured score callbacks. */
ExtraEventStateScoreFunction readStateScore; /* Computes a state ID's added cost. */
ExtraEventFrameScoreFunction readFrameScore; /* Computes an input frame's added cost. */
ExtraEventScoreCacheFunction getScoreCache; /* Supplies the cache indexed by state ID. */
s16 *scoreCache; /* Cached state costs; 32767 marks a cache miss. */
u32 stateCount; /* Number of entries in states and stateIds. */
u16 *stateIds; /* Configured IDs corresponding to each state entry. */
s32 bestScore; /* Lower of the first and terminal state costs. */
s32 accumulatedScore; /* Sum of frame costs processed by this block. */
u16 frameCount; /* Processed-frame counter; initialized to 65535 before the first frame. */
u16 reservedHalfword; /* Unused storage retained in the block layout. */
s32 transitionScore; /* Cost added when a transition chain advances. */
s32 initialScore; /* Initial cost assigned to the first state. */
u32 reservedWord; /* Unused storage retained in the block layout. */
u32 transitionCount; /* Number of configured transition chains. */
u16 *transitionBounds; /* State-table boundaries delimiting each transition chain. */
ExtraEventState *states; /* Dynamic-programming costs for configured states. */
} ExtraEventDP;

extern void *heap_Alloc(void *heap, u32 size);
extern void heap_Free(void *heap, void *ptr);

/* Command 2 calls this to restart scoring and seed the configured state chain. */
static void InitViterbi(ExtraEventDP *eventBlock)
{
ExtraEventState *currentState;
u32 stateIndex = 0;

eventBlock->bestScore = 0;
eventBlock->accumulatedScore = 0;
eventBlock->frameCount = 65535;

currentState = eventBlock->states;
currentState->score = eventBlock->initialScore;
currentState->stateId = eventBlock->stateIds[0];
currentState++;
stateIndex++;

while (stateIndex < eventBlock->stateCount) {
    currentState->score = EXTRA_EVENT_MAX_SCORE;
    currentState->stateId = eventBlock->stateIds[stateIndex];
    stateIndex++;
    currentState++;
}
}

/* The process callback calls this for each input frame to update the state costs. */
static void DynProgExtraEventsProcess(ExtraEventDP *eventBlock)
{
 s32 chainScore;
ExtraEventState *stateTable;
ExtraEventState *terminalState;
ExtraEventState *transitionState;
ExtraEventState *transitionEnd;
ExtraEventState *scoredState;
u16 stateIndex;
s16 *stateScoreCache;
u16 transitionIndex = 1;
s32 candidateScore;
s32 previousTerminalScore;

candidateScore = EXTRA_EVENT_SCORE_INFINITY;
stateTable = eventBlock->states;
terminalState = &stateTable[eventBlock->stateCount - 1];
stateScoreCache = eventBlock->scoreCache;
previousTerminalScore = terminalState->score;

for (; transitionIndex <= eventBlock->transitionCount; transitionIndex++) {
    chainScore = stateTable[0].score + eventBlock->transitionScore;
    transitionState = &eventBlock->states[eventBlock->transitionBounds[transitionIndex]];
    transitionEnd = &eventBlock->states[eventBlock->transitionBounds[transitionIndex + 1]] - 1;

    while (transitionState < transitionEnd) {
        candidateScore = chainScore;
        chainScore = transitionState->score;
        if (candidateScore < chainScore) {
            transitionState->score = candidateScore;
        }
        transitionState++;
    }

    /* Each chain is compared with the terminal cost saved before this update. */
    if (transitionState->score < previousTerminalScore) {
        terminalState->score = transitionState->score;
    }
    if (candidateScore < transitionState->score) {
        transitionState->score = candidateScore;
    }
}

if (previousTerminalScore < stateTable[0].score) {
    stateTable[0].score = terminalState->score;
}

scoredState = eventBlock->states;
for (stateIndex = 0; stateIndex < eventBlock->stateCount; stateIndex++) {
    s16 callbackScore;
    if ((callbackScore = stateScoreCache[scoredState->stateId]) == 32767) {
        callbackScore =
            eventBlock->readStateScore(eventBlock->callbackContext, scoredState->stateId);
        stateScoreCache[scoredState->stateId] = callbackScore;
    }

    scoredState->score += callbackScore;
    if (scoredState->score > EXTRA_EVENT_MAX_SCORE) {
        scoredState->score = EXTRA_EVENT_MAX_SCORE;
    }
    scoredState++;
}

if (stateTable[0].score < terminalState->score) {
    eventBlock->bestScore = stateTable[0].score;
} else {
    eventBlock->bestScore = terminalState->score;
}
}

/* TOS calls this process callback; only a non-null first input advances frame scoring. */
static u32 ProcessExtraEventDP(
TosBaseBlock *tosBlock, void **inputs, s32 inputCount)
{
ExtraEventDP *eventBlock = (ExtraEventDP *)tosBlock;

if (inputs[0] != NULL) {
    eventBlock->frameCount++;
    eventBlock->accumulatedScore += eventBlock->readFrameScore(eventBlock->callbackContext);
    DynProgExtraEventsProcess(eventBlock);
}

return 0;
}

/* TOS calls this initializer to bind the input port and apply profile key 15's element size. */
static u32 InitExtraEventDP(TosBaseBlock *tosBlock)
{
ExtraEventDP *eventBlock = (ExtraEventDP *)tosBlock;
u32 inputElementSize;

eventBlock->input = tosBlock->input;
inputElementSize = (u16)_tosGetProfileU32(tosBlock, 15, 4);
eventBlock->input->inputSize = inputElementSize;
return 0;
}

/* TOS calls this control callback for score queries, startup, configuration, and shutdown. */
static u32 ControlExtraEventDP(
ExtraEventDP *eventBlock, u32 command, void *commandData, u32 commandDataSize)
{
TosContext *ownerContext;
TosBaseBlock *tosBlock = (TosBaseBlock *)eventBlock;

switch ((u8)command) {
case 1:
    if (commandData != NULL) {
        *(s32 *)commandData = eventBlock->bestScore + eventBlock->accumulatedScore;
    } else {
        return 1;
    }
    break;
case 2:
    InitViterbi(eventBlock);
    eventBlock->base.process = (TosProcessFunction)ProcessExtraEventDP;
    break;
case 3:
    /* This command is accepted without changing the extra-event scorer. */
    break;
case 7:
    /* This command is accepted without changing the extra-event scorer. */
    break;
case 8:
    {
    TosContext *stateHeapContext = tosBlock->context;
    if (eventBlock->states != NULL) {
        heap_Free(stateHeapContext->heap, eventBlock->states);
        eventBlock->states = NULL;
    }

    ownerContext = tosBlock->context;
    eventBlock->transitionCount = ((ExtraEventContextInfo *)commandData)->transitionCount;
    eventBlock->transitionScore = ((ExtraEventContextInfo *)commandData)->transitionScore;
    eventBlock->initialScore = ((ExtraEventContextInfo *)commandData)->initialScore;
    eventBlock->stateCount = ((ExtraEventContextInfo *)commandData)->stateCount;
    eventBlock->stateIds = ((ExtraEventContextInfo *)commandData)->stateIds;
    eventBlock->transitionBounds = ((ExtraEventContextInfo *)commandData)->transitionBounds;
    eventBlock->scoreCache = eventBlock->getScoreCache(eventBlock->callbackContext);
    eventBlock->states = heap_Alloc(
        ownerContext->heap, eventBlock->stateCount * sizeof(ExtraEventState));
    if (eventBlock->states == NULL) {
        _tosErrorLog(eventBlock, 1);
    }
    break;
    }
case 6:
    eventBlock->readStateScore = ((ExtraEventCallbacks *)commandData)->readStateScore;
    eventBlock->getScoreCache = ((ExtraEventCallbacks *)commandData)->getScoreCache;
    eventBlock->readFrameScore = ((ExtraEventCallbacks *)commandData)->readFrameScore;
    eventBlock->callbackContext = ((ExtraEventCallbacks *)commandData)->callbackContext;
    break;
case 255:
    {
    ownerContext = tosBlock->context;
    if (eventBlock->states != NULL) {
        heap_Free(ownerContext->heap, eventBlock->states);
        eventBlock->states = NULL;
    }
    tosBaseBlockDestruct(eventBlock);
    break;
    }
default:
    return 0;
}

return 1;
}

/* The rec1600 block table uses this constructor for its extra-event scoring block. */
void *ConstructExtraEventDp(TosContext *context, u32 blockIndex)
{
return tosBaseBlockConstruct(
    context, blockIndex, 1, 0, (TosProcessFunction)ProcessExtraEventDP,
    (TosInitFunction)InitExtraEventDP,
    (TosControlFunction)ControlExtraEventDP, sizeof(ExtraEventDP));
}
