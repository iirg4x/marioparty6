#include "types.h"

#include "gssdk/gsapi.h"

s32 asrspi_cbAbnorm(u32 auxiliary, u32 value, GSCallbackContext *context)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    context->callbackActive = TRUE;
    gGSAPI.notify(context, 4, value, auxiliary);
    context->callbackActive = FALSE;
    return 0;
}

s32 asrspi_cbAgc(void)
{
    return 0;
}

s32 asrspi_cbEnergyLevel(u32 value, GSCallbackContext *context)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    context->callbackActive = TRUE;
    gGSAPI.notify(context, 5, value, context->value44);
    context->callbackActive = FALSE;
    return 0;
}

s32 asrspi_cbSNRLevel(u32 value, GSCallbackContext *context)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    context->callbackActive = TRUE;
    gGSAPI.notify(context, 6, value, context->value44);
    context->callbackActive = FALSE;
    return 0;
}

s32 asrspi_cbSpeechDetected(u32 value, GSCallbackContext *context)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    context->callbackActive = TRUE;
    gGSAPI.notify(context, 7, value, context->value44);
    context->callbackActive = FALSE;
    return 0;
}

s32 asrspi_cbTsDetected(u32 value, GSCallbackContext *context)
{
    if (gGSAPI.notify == NULL) {
        return 0;
    }

    context->callbackActive = TRUE;
    gGSAPI.notify(context, 8, value, context->value44);
    context->callbackActive = FALSE;
    return 0;
}

/* Reconstructed result conversion; names describe observed consumers. */
typedef struct GSRecognitionWord {
    s32 id;
    u32 value04;
    u32 value;
} GSRecognitionWord;

typedef struct GSRecognitionWordResult {
    u32 value00;
    s32 start;
    s32 end;
    GSRecognitionWord *word;
} GSRecognitionWordResult;

typedef struct GSRecognitionEntry {
    s32 wordCount;
    s32 score;
    u32 value08;
    s32 start;
    s32 end;
    GSRecognitionWordResult *words;
} GSRecognitionEntry;

typedef struct GSRecognitionResult {
    s32 status;
    s32 count;
    GSRecognitionEntry *entries;
} GSRecognitionResult;

typedef struct GSWordDetail {
    u16 id;
    s32 start;
    s32 end;
    u32 value;
} GSWordDetail;

typedef struct GSResultDetail {
    s32 start;
    s32 end;
    u16 wordCount;
    GSWordDetail *words;
} GSResultDetail;

typedef struct GSResult {
    s32 status;
    u16 wordCount;
    u16 score;
    u32 *words;
    struct GSResult *next;
    GSResultDetail *detail;
} GSResult;

extern s32 RestartEngine(GSCallbackContext *context);
extern s32 ContextGetParam(struct GSContext *context, u32 parameter, u32 *value);
extern s32 ContextGetAction(struct GSContext *context,
                           GSRecognitionResult *result, u32 *action);
extern void *heap_Alloc(void *heap, u32 bytes);
extern void heap_Free(void *heap, void *ptr);

s32 asrspi_cbResult(GSRecognitionResult *result, GSCallbackContext *context)
{
    u16 i, j;
    u32 bytes;
    u32 *wordValues;
    GSResultDetail *details;
    GSResultDetail *detail;
    GSWordDetail *wordDetail;
    GSWordDetail *nextWords;
    u32 action;
    u32 threshold;
    s32 paramStatus;

    details = NULL;
    action = 0;
    context->callbackActive = TRUE;
    if (result == NULL) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(context, 0, 0, 0);
        }
        if (context->flags & 2) {
            RestartEngine(context);
        }
        context->callbackActive = FALSE;
        return 0;
    }
    if (result->count == 0) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(context, 0, 0, 0);
        }
        if (context->flags & 2) {
            RestartEngine(context);
        }
        context->callbackActive = FALSE;
        return 0;
    }
    if (context->context == NULL) {
        context->callbackActive = FALSE;
        return 0;
    }
    paramStatus = ContextGetParam(context->context, 9, &threshold);
    bytes = (result->count * sizeof(GSResult) + 3) & ~3;
    for (i = 0; i < result->count; i++) {
        bytes += result->entries[i].wordCount * sizeof(u32);
    }
    context->results = heap_Alloc(gGSAPI.heap, bytes);
    if (context->results == NULL) {
        context->callbackActive = FALSE;
        return 0;
    }
    wordValues = (u32 *)((u8 *)context->results
        + ((result->count * sizeof(GSResult) + 3) & ~3));
    for (i = 0; i < result->count; i++) {
        context->results[i].status = result->status;
        context->results[i].wordCount = result->entries[i].wordCount;
        context->results[i].score = result->entries[i].score;
        context->results[i].words = wordValues;
        context->results[i].detail = NULL;
        context->results[i].next = i == result->count - 1
            ? NULL : context->results + i + 1;
        wordValues += context->results[i].wordCount;
        for (j = 0; j < context->results[i].wordCount; j++) {
            context->results[i].words[j] = result->entries[i].words[j].word->value;
        }
    }
    if (gGSAPI.resultDetails != 0) {
        bytes = (result->count * sizeof(GSResultDetail) + 3) & ~3;
        for (i = 0; i < result->count; i++) {
            bytes += (result->entries[i].wordCount * sizeof(GSWordDetail) + 3) & ~3;
        }
        details = heap_Alloc(gGSAPI.heap, bytes);
        if (details == NULL) {
            context->callbackActive = FALSE;
            return 0;
        }
        nextWords = (GSWordDetail *)((u8 *)details
            + ((result->count * sizeof(GSResultDetail) + 3) & ~3));
        for (i = 0, detail = details; i < result->count; i++, detail++) {
            context->results[i].detail = detail;
            detail->start = result->entries[i].start;
            detail->end = result->entries[i].end;
            detail->wordCount = result->entries[i].wordCount;
            detail->words = nextWords;
            wordDetail = nextWords;
            for (j = 0; j < detail->wordCount; j++) {
                wordDetail->start = result->entries[i].words[j].start;
                wordDetail->end = result->entries[i].words[j].end;
                wordDetail->id = result->entries[i].words[j].word->id;
                wordDetail->value = result->entries[i].words[j].word->value;
                wordDetail++;
            }
            nextWords += detail->wordCount;
        }
    }
    if (paramStatus != 0) {
        threshold = gGSAPI.resultThreshold;
    }
    if (result->status != 0 || result->entries[0].score < (s32)threshold) {
        if (gGSAPI.notify != NULL) {
            gGSAPI.notify(context, 0, (u32)context->results, threshold);
        }
        if (details != NULL) {
            heap_Free(gGSAPI.heap, details);
        }
        heap_Free(gGSAPI.heap, context->results);
        if (context->stopped == 0) {
            RestartEngine(context);
        }
        context->callbackActive = FALSE;
        return 0;
    }
    ContextGetAction(context->context, result, &action);
    gGSAPI.resultCallback(context, action, (u32)context->results, context->value44);
    if (details != NULL) {
        heap_Free(gGSAPI.heap, details);
    }
    heap_Free(gGSAPI.heap, context->results);
    if (context->flags & 2) {
        RestartEngine(context);
    }
    context->callbackActive = FALSE;
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
