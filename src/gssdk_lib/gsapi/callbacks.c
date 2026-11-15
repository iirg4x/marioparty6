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
s32 asrspi_cbResult(const struct GSResult *pAsrspiResult, u32 u32UserData)
{
    s32 Error;
    GSResultEngine *pEngine = (GSResultEngine *)u32UserData;
    u32 Action = 0;
    u32 ConfidenceThreshold;
    GSResultExtraCopy *pDetailedResults = NULL;
    GSResultDetailCopy *pDetailedWords;
    u16 i;
    u16 j;
    int AllocSize;
    u32 *pWordIdList;

    pEngine->callbackActive = TRUE;
    if (pAsrspiResult == NULL) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(pEngine, 0, 0, 0);
        }
        if (pEngine->mode & (1 << 1)) {
            RestartEngine(pEngine);
        }
        pEngine->callbackActive = FALSE;
        return 0;
    }
    if (pAsrspiResult->count == 0) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(pEngine, 0, 0, 0);
        }
        if (pEngine->mode & (1 << 1)) {
            RestartEngine(pEngine);
        }
        pEngine->callbackActive = FALSE;
        return 0;
    }
    if (pEngine->resultContext == NULL) {
        pEngine->callbackActive = FALSE;
        return 0;
    }

    Error = ContextGetParam(pEngine->resultContext, 9, &ConfidenceThreshold);
    AllocSize = AlignResultStorage(sizeof(GSResultCopy) * pAsrspiResult->count);
    for (i = 0; i < pAsrspiResult->count;
         i++) {
        AllocSize +=
            pAsrspiResult->entries[i].itemCount * sizeof(u32);
    }
    pEngine->copiedResults = heap_Alloc(gGSAPI.heap, AllocSize);
    if (pEngine->copiedResults == NULL) {
        pEngine->callbackActive = FALSE;
        return 0;
    }

    i = 0;
    pWordIdList = (u32 *)((u8 *)pEngine->copiedResults +
                      AlignResultStorage(sizeof(GSResultCopy) * pAsrspiResult->count));
    for (; i < pAsrspiResult->count; i++) {
        pEngine->copiedResults[i].status = pAsrspiResult->status;
        pEngine->copiedResults[i].itemCount =
            (u16)pAsrspiResult->entries[i].itemCount;
        pEngine->copiedResults[i].score =
            (u16)pAsrspiResult->entries[i].score;
        pEngine->copiedResults[i].keys = pWordIdList;
        pEngine->copiedResults[i].details = NULL;
        pEngine->copiedResults[i].nextResult =
            (i == pAsrspiResult->count - 1) ? NULL :
            &pEngine->copiedResults[i] + 1;
        pWordIdList += pEngine->copiedResults[i].itemCount;
        for (j = 0;
             j < pEngine->copiedResults[i].itemCount;
             j++) {
            pEngine->copiedResults[i].keys[j] =
                pAsrspiResult->entries[i].items[j].word->key;
        }
    }

    if (gGSAPI.resultDetails != 0) {
        i = 0;
        AllocSize = AlignResultStorage(sizeof(GSResultExtraCopy) * pAsrspiResult->count);
        for (; i < pAsrspiResult->count;
             i++) {
            AllocSize +=
                AlignResultStorage(sizeof(GSResultDetailCopy) *
                pAsrspiResult->entries[i].itemCount);
        }
        if ((pDetailedResults = heap_Alloc(gGSAPI.heap, AllocSize)) == NULL) {
            /* The callback returns here with the candidate copy still allocated. */
            pEngine->callbackActive = FALSE;
            return 0;
        }

        i = 0;
        pDetailedWords = (GSResultDetailCopy *)((u8 *)pDetailedResults +
            AlignResultStorage(sizeof(GSResultExtraCopy) * pAsrspiResult->count));
        for (; i < pAsrspiResult->count; i++) {
            pEngine->copiedResults[i].details = &pDetailedResults[i];
            j = 0;
            pDetailedResults[i].metadataA =
                pAsrspiResult->entries[i].detailMetadataA;
            pDetailedResults[i].metadataB =
                pAsrspiResult->entries[i].detailMetadataB;
            pDetailedResults[i].itemCount =
                (u16) pAsrspiResult->entries[i].itemCount;
            pDetailedResults[i].details = pDetailedWords;
            for (; j < pDetailedResults[i].itemCount;
                 j++) {
                pDetailedWords[j].pathValueA =
                    pAsrspiResult->entries[i].items[j].pathValueA;
                pDetailedWords[j].pathValueB =
                    pAsrspiResult->entries[i].items[j].pathValueB;
                pDetailedWords[j].leadingValue =
                    pAsrspiResult->entries[i].items[j].word->leadingValue;
                pDetailedWords[j].key =
                    pAsrspiResult->entries[i].items[j].word->key;
            }
            pDetailedWords += pDetailedResults[i].itemCount;
        }
    }

    if (Error != 0) {
        ConfidenceThreshold = gGSAPI.resultThreshold;
    }
    if (pAsrspiResult->status != 0 || pAsrspiResult->entries[0].score < (s32)ConfidenceThreshold) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(pEngine, 0, (u32)pEngine->copiedResults, ConfidenceThreshold);
        }
        if (pDetailedResults != NULL) {
            heap_Free(gGSAPI.heap, pDetailedResults);
        }
        heap_Free(gGSAPI.heap, pEngine->copiedResults);
        if (pEngine->stopRequested == 0) {
            RestartEngine(pEngine);
        }
        pEngine->callbackActive = FALSE;
        return 0;
    }

    /* ContextGetAction initializes the output to zero; its status is ignored and zero is reported
     * when no action mapping supplies a value. */
    ContextGetAction(pEngine->resultContext, pAsrspiResult, &Action);
    gGSAPI.resultCallback(pEngine, Action, (u32)pEngine->copiedResults,
                          pEngine->callbackValue);
    if (pDetailedResults != NULL) {
        heap_Free(gGSAPI.heap, pDetailedResults);
    }
    heap_Free(gGSAPI.heap, pEngine->copiedResults);
    if (pEngine->mode & (1 << 1)) {
        RestartEngine(pEngine);
    }
    pEngine->callbackActive = FALSE;
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
