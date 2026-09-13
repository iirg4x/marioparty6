#include "gssdk/gsapi.h"

#define GS_ERROR_FLAG (1U << 31)
#define GS_CONTEXT_ERROR_BASE (GS_ERROR_FLAG | (204U << 16))
#define GS_CONTEXT_ERROR_3 (GS_CONTEXT_ERROR_BASE | 3U)
#define GS_CONTEXT_ERROR_8 (GS_CONTEXT_ERROR_BASE | 8U)
#define GS_CONTEXT_ERROR_9 (GS_CONTEXT_ERROR_BASE | 9U)
#define GS_CONTEXT_OUT_OF_MEMORY (GS_CONTEXT_ERROR_BASE | 134U)
#define GS_SESSION_IMPORT_FAILURE (204U << 16)
#define GS_GCD_CHUNK_TYPE ('G' | ('C' << 8) | ('D' << 16) | (' ' << 24))

typedef struct GSContextParamState {
    u32 type;
    u32 size;
    u32 validParameters;
    u32 values[18];
} GSContextParamState;

typedef struct WrdWordList {
    u32 count;
    char **words;
} WrdWordList;

typedef struct WrdData {
    u32 reserved;
    u32 count;
    char words[];
} WrdData;

typedef struct GSDataChunk {
    s32 type;
    u32 size;
} GSDataChunk;

typedef struct GSActionTable {
    s32 type;
    u32 size;
    u32 count;
    u32 keysOffset;
    u32 actionsOffset;
} GSActionTable;

typedef struct GSActionThreshold {
    s32 threshold;
    u32 key;
} GSActionThreshold;

typedef struct GSActionWord {
    u8 reserved00[8];
    u32 key;
} GSActionWord;

typedef struct GSActionPath {
    u8 reserved00[28];
    GSActionWord *word;
} GSActionPath;

typedef struct GSActionResult {
    s32 score;
    u8 reserved04[16];
    GSActionPath *path;
} GSActionResult;

typedef struct GSActionObservation {
    u8 reserved00[8];
    GSActionResult *result;
} GSActionObservation;

typedef struct GSLoadedContext {
    void *data;
    GSDataChunk *gcdData;
    WrdData *wrdData;
    GSContextParamState *parameters;
    GSActionTable *actions;
    u8 reserved14[8];
    WrdWordList *wordList;
    u32 reserved20;
    void *words;
} GSLoadedContext;

typedef struct GSContext {
    void *engine;
    u8 reserved04[44];
    GSLoadedContext *data30;
    u8 reserved34[12];
    GSLoadedContext *data40;
    u8 reserved44[28];
    s32 sessionInitialized;
} GSContext;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern s32 asrspi_ActivateWords(void *engine, void *words, s32 index);

typedef struct GSSessionData {
    void *data;
    u32 size;
} GSSessionData;

typedef struct GSParameterValue {
    s32 parameter;
    u32 value;
} GSParameterValue;

extern void *heap_Alloc(void *heap, u32 size);
extern s32 asrspi_ExportSessionData(void *engine, s32 kind, void *data, u32 *size);
extern u32 asrspi_ActivateSessionData(void *engine, void *data);
extern u32 asrspi_ActivateCtx(void *engine, void *data, s32 activate);
extern s32 TranslateGsapiParamId2AsrSpiParamId(s32 parameter);
extern s32 asrspi_SetParamList(void *engine, GSParameterValue *parameters, s32 count);
s32 SessionDataFree(void *sessionData);
extern u32 WrdDestroyWordList(void *heap, WrdWordList *wordList);
extern u32 WrdCreateWordList(void *heap, WrdWordList *wordList, WrdData *data);
extern s32 ActivateDefaultParams(GSContext *context);
s32 ContextActivateParams(GSContext *runtime, GSLoadedContext *context);
s32 ContextSetActiveWords(GSContext *context, GSLoadedContext *activeWords);

static inline BOOL IsError(u32 result)
{
    return result >= GS_ERROR_FLAG ? TRUE : FALSE;
}

s32 SessionDataExport(GSContext *context, GSSessionData **output)
{
    u32 size;
    s32 result;
    GSSessionData *session;

    if (output != NULL) {
        *output = NULL;
    }
    result = asrspi_ExportSessionData(context->engine, 1, NULL, &size);
    if (result < 0) {
        return result;
    }
    session = heap_Alloc(gGSAPI.heap, size + sizeof(GSSessionData));
    session->size = size;
    session->data = session + 1;
    if (session == NULL) {
        return GS_CONTEXT_OUT_OF_MEMORY;
    }
    result = asrspi_ExportSessionData(context->engine, 1, session->data, &size);
    if (result < 0) {
        heap_Free(gGSAPI.heap, session);
        return result;
    }
    gGSAPI.sessionDataCount++;
    if (output != NULL) {
        *output = session;
    }
    return result;
}

u32 SessionDataImport(GSContext *context, GSSessionData *session, s32 release)
{
    u32 result = asrspi_ActivateSessionData(context->engine, session->data);

    if (IsError(result)) {
        result = GS_SESSION_IMPORT_FAILURE;
    }
    if (release) {
        SessionDataFree(session);
    }
    return result;
}

s32 SessionDataFree(void *sessionData)
{
    if (sessionData != 0) {
        heap_Free(gGSAPI.heap, sessionData);
        gGSAPI.sessionDataCount--;
    }

    return 0;
}

s32 ContextActivate(GSContext *runtime, GSLoadedContext *context)
{
    GSSessionData *session = NULL;
    s32 result;

    if (runtime->sessionInitialized) {
        result = SessionDataExport(runtime, &session);
        if (result < 0) {
            return result;
        }
    }
    result = asrspi_ActivateCtx(runtime->engine, context->data, 1);
    if (result >= 0) {
        runtime->data30 = context;
        runtime->data40 = context;
        result = ContextSetActiveWords(runtime, context);
        if (result >= 0) {
            result = ActivateDefaultParams(runtime);
        }
        if (result >= 0) {
            result = ContextActivateParams(runtime, context);
        }
        if (runtime->sessionInitialized && result >= 0) {
            u32 sessionStatus = SessionDataImport(runtime, session, 0);

            result = sessionStatus;
        }
        runtime->sessionInitialized = 1;
    }
    SessionDataFree(session);
    return result;
}

s32 ContextActivateParams(GSContext *runtime, GSLoadedContext *context)
{
    GSParameterValue parameters[17];
    s32 index;
    u32 mask;
    s32 count = 0;

    if (context->parameters == NULL) {
        return 0;
    }
    mask = context->parameters->validParameters;
    for (index = 0; index <= 17; index++, mask >>= 1) {
        if (mask & 1) {
            s32 id = TranslateGsapiParamId2AsrSpiParamId(index);

            if (id != 255) {
                parameters[count].parameter = id;
                parameters[count].value = context->parameters->values[index];
                count++;
            }
        }
    }
    return asrspi_SetParamList(runtime->engine, parameters, count);
}

s32 ContextAPIDeActivate(GSContext *context)
{
    context->data30 = 0;
    context->data40 = 0;
    return 0;
}

s32 ContextDeActivate(GSContext *context)
{
    u32 result = asrspi_ActivateCtx(context->engine, NULL, 0);

    if (IsError(result)) {
        return result;
    }
    context->data30 = 0;
    context->data40 = 0;
    return 0;
}

s32 ContextGetAction(GSLoadedContext *context, GSActionObservation *observation, u32 *action)
{
    u32 count;
    GSActionTable *table;
    u32 key;

    *action = 0;
    if (context == NULL) {
        return 0;
    }
    table = context->actions;
    if (table == NULL) {
        return GS_CONTEXT_ERROR_9;
    }
    key = observation->result->path->word->key;
    switch (table->type) {
    case 2:
        if (key < table->count) {
            *action = ((u32 *)((u8 *)table + table->keysOffset))[key];
        }
        break;
    case 3: {
        u32 index;
        u32 *actions;
        u32 *keys;
        u32 limit;

        limit = table->count;
        index = 0;
        actions = (u32 *)((u8 *)table + table->actionsOffset);
        keys = (u32 *)((u8 *)table + table->keysOffset);

        for (; index < limit; index++) {
            if (key == keys[index]) {
                *action = actions[index];
                break;
            }
        }
        break;
    }
    case 4: {
        GSActionThreshold *entry;
        u32 *actions;
        u32 index;

        count = table->count;
        entry = (GSActionThreshold *)((u8 *)table + table->keysOffset);
        actions = (u32 *)((u8 *)table + table->actionsOffset);

        for (index = 0; index < count; index++) {
            if (observation->result->score > entry[index].threshold && key == entry[index].key) {
                *action = actions[index];
            }
        }
        break;
    }
    }
    return 0;
}

s32 ContextGetParam(GSLoadedContext *context, u32 parameter, u32 *value)
{
    GSContextParamState *parameters = context->parameters;

    if (parameters == 0) {
        return GS_CONTEXT_ERROR_8;
    }
    if ((parameters->validParameters & (1U << parameter)) == 0) {
        return GS_CONTEXT_ERROR_8;
    }

    *value = parameters->values[parameter];
    return 0;
}

s32 ContextSetActiveWords(GSContext *context, GSLoadedContext *activeWords)
{
    if (activeWords->words == 0) {
        return 0;
    }

    return asrspi_ActivateWords(context->engine, activeWords->words, -1);
}

s32 ContextSetCtxData(void **contextData, void *data)
{
    *contextData = data;
    return 0;
}

s32 ContextSetGcdData(GSLoadedContext *context, GSDataChunk *data)
{
    u32 size;
    u8 *cursor;

    if (context->parameters != NULL) {
        if (context->parameters->size == 0) {
            heap_Free(gGSAPI.heap, context->parameters);
        }
        context->parameters = NULL;
    }
    if (data == NULL) {
        context->gcdData = NULL;
        context->actions = NULL;
        return 0;
    }
    if (data->type != GS_GCD_CHUNK_TYPE) {
        return GS_CONTEXT_ERROR_3;
    }
    context->gcdData = data;
    context->actions = NULL;
    size = context->gcdData->size;
    if (size <= sizeof(GSDataChunk)) {
        return GS_CONTEXT_ERROR_3;
    }
    cursor = (u8 *)context->gcdData + sizeof(GSDataChunk);
    while (cursor != (u8 *)context->gcdData + size) {
        GSDataChunk chunk;

        chunk.size = ((GSDataChunk *)cursor)->size;
        chunk.type = ((GSDataChunk *)cursor)->type;

        if (chunk.size == 0) {
            return GS_CONTEXT_ERROR_3;
        }
        switch (chunk.type) {
        case 1:
            context->parameters = (GSContextParamState *)cursor;
            if (context->parameters->size != sizeof(GSContextParamState)) {
                return GS_CONTEXT_ERROR_3;
            }
            break;
        case 2:
            context->actions = (GSActionTable *)cursor;
            break;
        case 3:
            context->actions = (GSActionTable *)cursor;
            break;
        case 4:
            context->actions = (GSActionTable *)cursor;
            break;
        default:
            return GS_CONTEXT_ERROR_3;
        }
        cursor += chunk.size;
    }
    return 0;
}

s32 ContextSetParam(GSLoadedContext *context, u32 parameter, u32 value)
{
    if (context->parameters == 0) {
        context->parameters = heap_Calloc(gGSAPI.heap, 1, sizeof(GSContextParamState));
        if (context->parameters == 0) {
            return GS_CONTEXT_OUT_OF_MEMORY;
        }
    }
    context->parameters->validParameters |= 1U << parameter;
    context->parameters->values[parameter] = value;
    return 0;
}

s32 ContextSetWrdData(GSLoadedContext *context, WrdData *data)
{
    context->wrdData = data;
    if (context->wordList != NULL) {
        WrdDestroyWordList(gGSAPI.heap, context->wordList);
    }
    if (context->wrdData != NULL) {
        if (context->wordList == NULL) {
            context->wordList = heap_Alloc(gGSAPI.heap, sizeof(WrdWordList));
            if (context->wordList == NULL) {
                return GS_CONTEXT_OUT_OF_MEMORY;
            }
        }
        WrdCreateWordList(gGSAPI.heap, context->wordList, context->wrdData);
    } else if (context->wordList != NULL) {
        heap_Free(gGSAPI.heap, context->wordList);
        context->wordList = NULL;
    }
    return 0;
}

s32 ContextUnLoad(GSLoadedContext *context)
{
    context->wrdData = NULL;
    context->data = NULL;
    if (context->parameters != NULL && context->parameters->size == 0) {
        heap_Free(gGSAPI.heap, context->parameters);
    }
    context->gcdData = NULL;
    context->parameters = NULL;
    context->actions = NULL;
    if (context->wordList != NULL) {
        WrdDestroyWordList(gGSAPI.heap, context->wordList);
        heap_Free(gGSAPI.heap, context->wordList);
        context->wordList = NULL;
    }
    if (context->words != NULL) {
        heap_Free(gGSAPI.heap, context->words);
        context->words = NULL;
    }
    return 0;
}
