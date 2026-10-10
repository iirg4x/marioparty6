/* Speech recognition callbacks forward signals and results to GSAPI clients. */
#include "types.h"

#include "gssdk/gsapi.h"

extern void *heap_Alloc(void *heap, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern s32 ContextGetParam(void *resultContext, u32 parameterId, u32 *resultValue);
extern s32 ContextGetAction(void *resultContext, void *resultObservation,
                            u32 *selectedAction);
extern u32 RestartEngine(void *engineState);

typedef struct GSResultWord {
    s32 leadingValue; /* Leading value returned with the recognized word. */
    u32 unreadWordValue; /* Recognizer word member not read by these callbacks. */
    u32 key; /* Word key used to select the action for a result. */
} GSResultWord;

typedef struct GSResultPathItem {
    u32 unreadPathValue; /* Recognizer path member not read by these callbacks. */
    u32 pathValueA; /* First opaque value copied to the optional detail record. */
    u32 pathValueB; /* Second opaque value copied to the optional detail record. */
    GSResultWord *word; /* Recognized word associated with this path item. */
} GSResultPathItem;

typedef struct GSResultEntry {
    u32 itemCount; /* Number of path items in this recognition result. */
    s32 score; /* Recognition score compared with the configured threshold. */
    u32 unreadEntryValue; /* Recognizer result member not read by these callbacks. */
    u32 detailMetadataA; /* Opaque result value copied when details are requested. */
    u32 detailMetadataB; /* Opaque result value copied when details are requested. */
    GSResultPathItem *items; /* Path items that make up this result. */
} GSResultEntry;

typedef struct GSResultExtraCopy GSResultExtraCopy;

struct GSResult {
    s32 status; /* Recognition status; nonzero results are rejected. */
    s32 count; /* Number of candidate result entries. */
    GSResultEntry *entries; /* Candidate results, ordered by the recognizer. */
};

typedef struct GSResultCopy {
    u32 status; /* Recognition status copied from the source result. */
    u16 itemCount; /* Number of word keys in this candidate. */
    u16 score; /* Candidate score narrowed to the callback record width. */
    u32 *keys; /* Word keys copied from the candidate path. */
    struct GSResultCopy *nextResult; /* Next candidate in the callback list. */
    GSResultExtraCopy *details; /* Optional per-candidate detail record. */
} GSResultCopy;

typedef struct GSResultDetailCopy {
    u16 leadingValue; /* Leading value from the recognized word. */
    u16 unassignedDetailValue; /* Detail-record member these callbacks leave untouched. */
    u32 pathValueA; /* First opaque value copied from the path item. */
    u32 pathValueB; /* Second opaque value copied from the path item. */
    u32 key; /* Word key associated with this detail. */
} GSResultDetailCopy;

struct GSResultExtraCopy {
    u32 metadataA; /* Opaque result value copied for the client callback. */
    u32 metadataB; /* Opaque result value copied for the client callback. */
    u16 itemCount; /* Number of detail records in this candidate. */
    u16 unassignedMetadataValue; /* Detail-record member these callbacks leave untouched. */
    GSResultDetailCopy *details; /* Detail records for the candidate path. */
};

typedef struct GSResultEngine {
    void *engineHandle; /* Recognition engine handle. */
    u32 mode; /* Engine mode; bit 1 requests a restart after result handling. */
    u8 parameters[40]; /* ASR parameter block carried with the engine state. */
    struct GSContext *resultContext; /* Context used to read thresholds and actions. */
    GSResultCopy *copiedResults; /* Temporary result list passed to the client. */
    u8 engineStateAt38[12]; /* Engine-state bytes not read by these callbacks. */
    u32 callbackValue; /* Client value forwarded with callback notifications. */
    u8 engineStateAt48[12]; /* Engine-state bytes not read by these callbacks. */
    u32 callbackActive; /* Nonzero while a recognizer callback is running. */
    u8 restartPending[4]; /* Engine flag set when a callback requests a deferred restart. */
    s32 stopRequested; /* Prevents a rejected result from restarting a stopped engine. */
} GSResultEngine;

/* The recognizer calls this for abnormal signals; GSAPI forwards notification event 4. */
s32 asrspi_cbAbnorm(u32 auxiliaryValue, u32 signalValue,
                    GSCallbackContext *callbackContext)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    callbackContext->callbackActive = TRUE;
    gGSAPI.notify(callbackContext, 4, signalValue, auxiliaryValue);
    callbackContext->callbackActive = FALSE;
    return 0;
}

/* The recognizer registers this AGC signal, but GSAPI does not forward it. */
s32 asrspi_cbAgc(void)
{
    return 0;
}

/* The recognizer calls this for enabled energy reports; GSAPI forwards event 5. */
s32 asrspi_cbEnergyLevel(u32 signalValue, GSCallbackContext *callbackContext)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    callbackContext->callbackActive = TRUE;
    gGSAPI.notify(callbackContext, 5, signalValue, callbackContext->value44);
    callbackContext->callbackActive = FALSE;
    return 0;
}

/* The recognizer calls this for enabled SNR reports; GSAPI forwards event 6. */
s32 asrspi_cbSNRLevel(u32 signalValue, GSCallbackContext *callbackContext)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    callbackContext->callbackActive = TRUE;
    gGSAPI.notify(callbackContext, 6, signalValue, callbackContext->value44);
    callbackContext->callbackActive = FALSE;
    return 0;
}

/* The recognizer calls this after speech detection; GSAPI forwards notification event 7. */
s32 asrspi_cbSpeechDetected(u32 signalValue, GSCallbackContext *callbackContext)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    callbackContext->callbackActive = TRUE;
    gGSAPI.notify(callbackContext, 7, signalValue, callbackContext->value44);
    callbackContext->callbackActive = FALSE;
    return 0;
}

/* The recognizer calls this after timestamp detection; GSAPI forwards event 8. */
s32 asrspi_cbTsDetected(u32 signalValue, GSCallbackContext *callbackContext)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    callbackContext->callbackActive = TRUE;
    gGSAPI.notify(callbackContext, 8, signalValue, callbackContext->value44);
    callbackContext->callbackActive = FALSE;
    return 0;
}

static inline u32 AlignResultStorage(u32 size)
{
    return (size + 3) & ~3U;
}

/* The recognizer calls this with candidate results; GSAPI copies them and rejects nonzero status
* or a first-entry score below the minimum, then reports the selected action or a no-result
* notification. Rejected results restart when stopRequested is zero; empty results restart only
* with mode bit 1, as do successful results. */
s32 asrspi_cbResult(const struct GSResult *recognitionResult, u32 engineStateAddress)
{
    GSResultEngine *engine = (GSResultEngine *)engineStateAddress;
    u32 selectedAction = 0;
    GSResultExtraCopy *detailCopy = NULL;
    s32 parameterStatus;
    s32 minimumScore;
    int allocationSize;
    u16 entryIndex;
    u16 pathIndex;
    u8 *storageCursor;

    engine->callbackActive = TRUE;
    if (recognitionResult == NULL) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(engine, 0, 0, 0);
        }
        if (engine->mode & (1 << 1)) {
            RestartEngine(engine);
        }
        engine->callbackActive = FALSE;
        return 0;
    }
    if (recognitionResult->count == 0) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(engine, 0, 0, 0);
        }
        if (engine->mode & (1 << 1)) {
            RestartEngine(engine);
        }
        engine->callbackActive = FALSE;
        return 0;
    }
    if (engine->resultContext == NULL) {
        engine->callbackActive = FALSE;
        return 0;
    }

    parameterStatus = ContextGetParam(engine->resultContext, 9, (u32 *)&minimumScore);
    allocationSize = AlignResultStorage(sizeof(GSResultCopy) * recognitionResult->count);
    for (entryIndex = 0; entryIndex < recognitionResult->count;
         entryIndex++) {
        allocationSize +=
            recognitionResult->entries[entryIndex].itemCount * sizeof(u32);
    }
    engine->copiedResults = heap_Alloc(gGSAPI.heap, allocationSize);
    if (engine->copiedResults == NULL) {
        engine->callbackActive = FALSE;
        return 0;
    }

    entryIndex = 0;
    storageCursor = (u8 *)((u8 *)engine->copiedResults +
                      AlignResultStorage(sizeof(GSResultCopy) * recognitionResult->count));
    for (; entryIndex < recognitionResult->count; entryIndex++) {
        engine->copiedResults[entryIndex].status = recognitionResult->status;
        engine->copiedResults[entryIndex].itemCount =
            (u16)recognitionResult->entries[entryIndex].itemCount;
        engine->copiedResults[entryIndex].score =
            (u16)recognitionResult->entries[entryIndex].score;
        engine->copiedResults[entryIndex].keys = (u32 *)storageCursor;
        engine->copiedResults[entryIndex].details = NULL;
        engine->copiedResults[entryIndex].nextResult =
            (entryIndex == recognitionResult->count - 1) ? NULL :
            &engine->copiedResults[entryIndex] + 1;
        storageCursor += engine->copiedResults[entryIndex].itemCount * sizeof(u32);
        for (pathIndex = 0;
             pathIndex < engine->copiedResults[entryIndex].itemCount;
             pathIndex++) {
            engine->copiedResults[entryIndex].keys[pathIndex] =
                recognitionResult->entries[entryIndex].items[pathIndex].word->key;
        }
    }

    if (gGSAPI.resultDetails != 0) {
        entryIndex = 0;
        allocationSize = AlignResultStorage(sizeof(GSResultExtraCopy) * recognitionResult->count);
        for (; entryIndex < recognitionResult->count;
             entryIndex++) {
            allocationSize +=
                AlignResultStorage(sizeof(GSResultDetailCopy) *
                recognitionResult->entries[entryIndex].itemCount);
        }
        if ((detailCopy = heap_Alloc(gGSAPI.heap, allocationSize)) == NULL) {
            /* The callback returns here with the candidate copy still allocated. */
            engine->callbackActive = FALSE;
            return 0;
        }

        entryIndex = 0;
        storageCursor = (u8 *)((u8 *)detailCopy +
            AlignResultStorage(sizeof(GSResultExtraCopy) * recognitionResult->count));
        for (; entryIndex < recognitionResult->count; entryIndex++) {
            engine->copiedResults[entryIndex].details = &detailCopy[entryIndex];
            pathIndex = 0;
            detailCopy[entryIndex].metadataA =
                recognitionResult->entries[entryIndex].detailMetadataA;
            detailCopy[entryIndex].metadataB =
                recognitionResult->entries[entryIndex].detailMetadataB;
            detailCopy[entryIndex].itemCount =
                (u16) recognitionResult->entries[entryIndex].itemCount;
            detailCopy[entryIndex].details = (GSResultDetailCopy *)storageCursor;
            for (; pathIndex < detailCopy[entryIndex].itemCount;
                 pathIndex++) {
                ((GSResultDetailCopy *)storageCursor)[pathIndex].pathValueA =
                    recognitionResult->entries[entryIndex].items[pathIndex].pathValueA;
                ((GSResultDetailCopy *)storageCursor)[pathIndex].pathValueB =
                    recognitionResult->entries[entryIndex].items[pathIndex].pathValueB;
                ((GSResultDetailCopy *)storageCursor)[pathIndex].leadingValue =
                    recognitionResult->entries[entryIndex].items[pathIndex].word->leadingValue;
                ((GSResultDetailCopy *)storageCursor)[pathIndex].key =
                    recognitionResult->entries[entryIndex].items[pathIndex].word->key;
            }
            storageCursor += detailCopy[entryIndex].itemCount * sizeof(GSResultDetailCopy);
        }
    }

    if (parameterStatus != 0) {
        minimumScore = gGSAPI.resultThreshold;
    }
    if (recognitionResult->status != 0 || recognitionResult->entries[0].score < minimumScore) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(engine, 0, (u32)engine->copiedResults, minimumScore);
        }
        if (detailCopy != NULL) {
            heap_Free(gGSAPI.heap, detailCopy);
        }
        heap_Free(gGSAPI.heap, engine->copiedResults);
        if (engine->stopRequested == 0) {
            RestartEngine(engine);
        }
        engine->callbackActive = FALSE;
        return 0;
    }

    /* ContextGetAction initializes the output to zero; its status is ignored and zero is reported
     * when no action mapping supplies a value. */
    ContextGetAction(engine->resultContext, recognitionResult, &selectedAction);
    gGSAPI.resultCallback(engine, selectedAction, (u32)engine->copiedResults,
                          engine->callbackValue);
    if (detailCopy != NULL) {
        heap_Free(gGSAPI.heap, detailCopy);
    }
    heap_Free(gGSAPI.heap, engine->copiedResults);
    if (engine->mode & (1 << 1)) {
        RestartEngine(engine);
    }
    engine->callbackActive = FALSE;
    return 0;
}

typedef s32 (*AsrSpiCallback)(void);

typedef struct AsrSpiCallbacks {
    AsrSpiCallback result;
    AsrSpiCallback *signals;
} AsrSpiCallbacks;

AsrSpiCallback AsrSpiSignalCallBacks[] = {
    (AsrSpiCallback)asrspi_cbAbnorm,
    (AsrSpiCallback)asrspi_cbSpeechDetected,
    (AsrSpiCallback)asrspi_cbTsDetected,
    (AsrSpiCallback)asrspi_cbAgc,
    (AsrSpiCallback)asrspi_cbEnergyLevel,
    (AsrSpiCallback)asrspi_cbSNRLevel,
};

AsrSpiCallbacks AsrSpiRecogCallBacks = {
    (AsrSpiCallback)asrspi_cbResult,
    AsrSpiSignalCallBacks,
};
