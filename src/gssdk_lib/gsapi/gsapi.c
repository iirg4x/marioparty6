#include "types.h"

#include <string.h>

#include "gssdk/gsapi.h"
#include "gssdk/safeh.h"

#define GSAPI_ERROR_ALREADY_OPEN 0x80CC0000U
#define GSAPI_ERROR_NOT_OPEN 0x80CC0001U
#define GSAPI_ERROR_NO_LANGUAGE 0x80CC0002U
#define GSAPI_ERROR_ENGINE_LIMIT 0x80CC0005U
#define GSAPI_ERROR_AUDIO 0x80CC0006U
#define GSAPI_ERROR_MODE 0x80CC0007U
#define GSAPI_ERROR_NO_RESULT 0x80CC000AU
#define GSAPI_ERROR_PARAMETER_RANGE 0x80CC000BU
#define GSAPI_ERROR_PARAMETER_ID 0x80CC000CU
#define GSAPI_ERROR_NO_PARAMETERS 0x80CC000DU
#define GSAPI_ERROR_NO_PARAMETER_NAMES 0x80CC0080U
#define GSAPI_ERROR_INVALID_ARGUMENT 0x80CC0083U
#define GSAPI_ERROR_OUT_OF_MEMORY 0x80CC0086U
#define GSAPI_ERROR_INVALID_HANDLE 0x80CC0089U

#define GSAPI_PARAMETER_COUNT 18
#define GSAPI_LANGUAGE_HEADER 0x00091874U

typedef s32 (*GSResultCallback)(void);
typedef s32 (*AsrSpiCallback)(void);

typedef struct AsrSpiCallbacks {
    AsrSpiCallback result;
    AsrSpiCallback *signals;
} AsrSpiCallbacks;

typedef struct AsrSpiParameter {
    u32 id;
    s32 value;
} AsrSpiParameter;

typedef struct GSParameterState {
    u8 reserved00[8];
    u32 validParameters;
    s32 values[GSAPI_PARAMETER_COUNT];
} GSParameterState;

typedef struct GSResultValues {
    u32 lastIndex;
    u32 *values;
} GSResultValues;

typedef struct GSActiveResult {
    u8 reserved00[8];
    void *result;
    u8 reserved0C[0x10];
    GSResultValues *parameters;
} GSActiveResult;

typedef struct GSEngine {
    void *handle;
    u32 mode;
    u8 parameters[0x28];
    GSActiveResult *activeResult;
    void *resultData;
    s32 active;
    s32 asrState;
    void *context;
    u32 callbackValue;
    u32 energyInterval;
    void *audio;
    s32 closing;
    s32 callbackActive;
    s32 restartPending;
    s32 stopRequested;
    u32 reserved60;
} GSEngine;

typedef void *GSEngineHandle;

typedef struct GSAPIPrivate {
    void *heap;
    s32 heapSize;
    SafeHandle resource;
    s32 status;
    void *userData;
    GSResultCallback resultCallback;
    GSNotifyCallback notify;
    GSGetSamplesCallback getSamples;
    void *language;
    void *languageBuffer;
    GSParameterState defaultParameters;
    u32 copyResult;
    GSEngine *engines;
    u16 activeEngines;
    u16 maxEngines;
    u32 contextCount;
    u32 sessionDataCount;
} GSAPIPrivate;

typedef struct ExtAudioState ExtAudioState;
typedef struct SidState SidState;

#define GSAPI_PRIVATE ((GSAPIPrivate *)&gGSAPI)

static inline BOOL IsError(u32 result)
{
    return result >= 0x80000000 ? TRUE : FALSE;
}

static inline BOOL IsSuccess(u32 result)
{
    return result < 0x80000000 ? TRUE : FALSE;
}

static inline BOOL IsValidEngine(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    GSEngine *engines = api->engines;

    if (engine == NULL) {
        return FALSE;
    }
    if ((u32)engine < (u32)engines) {
        return FALSE;
    }
    if (((u32)engine - (u32)engines) % sizeof(GSEngine) != 0) {
        return FALSE;
    }
    if ((u32)engine >
        (u32)((u8 *)engines +
              (api->maxEngines * sizeof(GSEngine)))) {
        return FALSE;
    }
    if (engine->handle == NULL) {
        return FALSE;
    }
    return TRUE;
}

static inline BOOL IsValidContext(void *context)
{
    return context == NULL ? FALSE : TRUE;
}

extern AsrSpiCallback AsrSpiSignalCallBacks[];
extern AsrSpiCallbacks AsrSpiRecogCallBacks;
extern s32 asrspi_cbEnergyLevel(u32 value, GSCallbackContext *context);
extern s32 asrspi_cbSNRLevel(u32 value, GSCallbackContext *context);

extern s32 asrspi_Acquisition(
    void *engine, void *samples, u32 sampleBytes, s32 *complete);
extern s32 asrspi_ActivateLng(void *engine, void *language);
extern s32 asrspi_CloseEngine(void *engine);
extern s32 asrspi_GetState(void *engine, s32 *state);
extern s32 asrspi_OpenEngine(
    SafeHandle resource, u32 flags, GSEngine *engine);
extern s32 asrspi_SetParamList(
    void *engine, AsrSpiParameter *parameters, u32 count);
extern s32 asrspi_Start(
    void *engine, u32 energyInterval, AsrSpiCallbacks *callbacks,
    GSEngine *context);
extern s32 asrspi_Stop(void *engine);

extern s32 ExtAudioClose(void *heap, ExtAudioState *audio);
extern u32 ExtAudioGetSamples(void *source, void *samples, u32 sampleCount);
extern u32 ExtAudioGetSamplesLeft(ExtAudioState *audio);
extern s32 ExtAudioInit(void *heap, ExtAudioState **audio);
extern s32 ExtAudioStart(ExtAudioState *audio);
extern s32 ExtAudioStop(ExtAudioState *audio);

extern s32 SidAdvanceBuffer(SidState *sid, s32 samples);
extern s32 SidClose(void *heap, SidState *sid);
extern s32 SidGetSamples(SidState *sid, s16 **buffer, s32 samples);
extern s32 SidGetSamplesLeft(SidState *sid);
extern s32 SidInit(void *heap, void *unused, SidState **sid);
extern s32 SidIsRecording(SidState *sid);
extern s32 SidStart(SidState *sid);
extern s32 SidStop(SidState *sid);

extern s32 ContextAPIDeActivate(GSEngine *engine);
extern s32 ContextActivate(GSEngine *engine, void *context);
extern s32 ContextActivateParams(GSEngine *engine, void *context);
extern s32 ContextDeActivate(GSEngine *engine);
extern s32 ContextSetCtxData(void **contextData, void *data);
extern s32 ContextSetGcdData(void *context, void *data);
extern s32 ContextSetParam(void *context, s32 parameter, s32 value);
extern s32 ContextSetWrdData(void *context, void *data);
extern s32 ContextUnLoad(void *context);

extern s32 SessionDataExport(GSEngine *engine, void **sessionData);
extern s32 SessionDataFree(void *sessionData);
extern s32 SessionDataImport(
    GSEngine *engine, void *sessionData, s32 flags);

extern void *heap_Alloc(void *heap, u32 size);
extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Close(void *heap);
extern void heap_Free(void *heap, void *ptr);
extern s32 heap_Open(void **heap, s32 size);

extern s32 rsrc_Close(SafeHandle resource);
extern s32 rsrc_Open(
    void *heap, u32 flags, void *arg2, void *arg3, SafeHandle *resource);

u32 TranslateGsapiParamId2AsrSpiParamId(s32 parameter);
void ActivateDefaultParams(void **engine);
static void InitDefaultParams(void);
static s32 ProcessAudio(GSEngine *engine);
u32 RestartEngine(GSEngine *engine);
s32 StartListening(GSEngine *engine);
s32 StopListening(GSEngine *engine);
s32 gsapi_Close(void);
s32 gsapi_ContextActivate(GSEngineHandle engineHandle, void *context);
s32 gsapi_ContextSetCtxData(void *data, void **context);
s32 gsapi_ContextSetGcdData(void *context, void *data);
s32 gsapi_ContextSetParam(void *context, s32 parameter, s32 value);
s32 gsapi_ContextSetWrdData(void *context, void *data);
s32 gsapi_ContextDeActivate(void *context);
s32 gsapi_EngineClose(GSEngine *engine);
s32 gsapi_EngineOpen(s32 device, GSEngine **engineOut);
s32 gsapi_EngineSetMode(GSEngineHandle engineHandle, u32 mode);
s32 gsapi_EngineSetParam(
    GSEngineHandle engineHandle, s32 parameter, s32 value);
s32 gsapi_EngineStart(GSEngine *engine);
s32 gsapi_EngineStop(GSEngineHandle engineHandle);
void gsapi_EngineRestart(void);
s32 gsapi_EngineGetParam(GSEngine *engine, u32 parameter, u32 *value);
s32 gsapi_Init(GSResultCallback resultCallback, void *userData);
s32 gsapi_LanguageLoadBuffer(void *buffer, u32 loadParameters);
s32 gsapi_LanguageUnLoad(void);
s32 gsapi_EngineSessionDataExport(GSEngine *engine, void **sessionData);
s32 gsapi_EngineSessionDataImport(
    GSEngine *engine, void *sessionData, s32 flags);
s32 gsapi_EngineSessionDataFree(void *sessionData);
s32 gsapi_NotifySetCallback(GSNotifyCallback callback);
s32 gsapi_SetUserData(s32 heapSize);

const s32 TranslateParamTable[GSAPI_PARAMETER_COUNT] = {
    0, 1, 3, 0x11, 6, 7, 8, 9, 0xA,
    0xFF, 0xD, 0xE, 0xF, 0x10, 4, 0xB, 0xC, 0x12,
};

GSAPIState gGSAPI = { 0 };

char GSAPIVersionString[0x1C] = "  [ASRSPI %d.%02d.%04d]";

u16 MAXRECOGNIZERS = 4;
u16 DEFAULTENERGYINTERVAL = 500;

u32 TranslateGsapiParamId2AsrSpiParamId(s32 parameter)
{
    return TranslateParamTable[parameter];
}

void ActivateDefaultParams(void **engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    AsrSpiParameter parameters[GSAPI_PARAMETER_COUNT - 1];
    u32 count = 0;
    u32 i;

    for (i = 0; i < GSAPI_PARAMETER_COUNT; i++) {
        if (TranslateParamTable[i] != 0xFF) {
            parameters[count].id = TranslateParamTable[i];
            parameters[count].value = api->defaultParameters.values[i];
            count++;
        }
    }

    asrspi_SetParamList(*engine, parameters, count);
}

static void InitDefaultParams(void)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    api->defaultParameters.validParameters = 0x1FFFF;
    api->defaultParameters.values[0] = 1;
    api->defaultParameters.values[1] = 1;
    api->defaultParameters.values[2] = 0;
    api->defaultParameters.values[3] = 0x32;
    api->defaultParameters.values[4] = 0x3C;
    api->defaultParameters.values[5] = 1;
    api->defaultParameters.values[6] = 1;
    api->defaultParameters.values[7] = 0;
    api->defaultParameters.values[8] = 0x7FFFFFFF;
    api->defaultParameters.values[9] = 0x1388;
    api->defaultParameters.values[10] = 0xC8;
    api->defaultParameters.values[11] = 0x12C;
    api->defaultParameters.values[12] = 0x28;
    api->defaultParameters.values[13] = 0x28;
    api->defaultParameters.values[14] = 1;
    api->defaultParameters.values[15] = -0x1C20;
    api->defaultParameters.values[16] = 0;
    api->defaultParameters.values[17] = 0;
}

static s32 ProcessAudio(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *status;
    GSGetSamplesCallback *getSamples;
    s16 *samples = NULL;
    s32 complete = 0;
    s32 sampleCount;
    s32 samplesLeft;
    s32 currentStatus;

    sampleCount = asrspi_GetState(engine->handle, &engine->asrState);
    *(status = &api->status) = sampleCount;
    if (engine->active == 0) {
        return 0;
    }
    currentStatus = *status;
    if (currentStatus != 0) {
        return currentStatus;
    }

    switch (engine->active) {
    case 1:
        for (;;) {
            if (*(getSamples = &api->getSamples) != NULL) {
                sampleCount = ExtAudioGetSamples(engine, &samples, 0x6E);
            } else {
                sampleCount = SidGetSamples(
                    (SidState *)engine->audio, &samples, 0x6E);
            }

            if (sampleCount > 0) {
                if ((engine->mode & 4) != 0 && api->notify != NULL) {
                    api->notify(engine, 1, (u32)samples, sampleCount * 2);
                }
                *status = asrspi_Acquisition(
                    engine->handle, samples, sampleCount * 2, &complete);
                if (IsError(*status)) {
                    return *status;
                }
                if (*getSamples == NULL) {
                    SidAdvanceBuffer(
                        (SidState *)engine->audio, sampleCount);
                }
                if (complete != 0) {
                    StopListening(engine);
                }
            } else if (sampleCount < 0) {
                return GSAPI_ERROR_AUDIO;
            }

            if (*getSamples != NULL) {
                samplesLeft =
                    ExtAudioGetSamplesLeft((ExtAudioState *)engine);
            } else {
                samplesLeft = SidGetSamplesLeft((SidState *)engine->audio);
            }
            if (samplesLeft < 0x6E) {
                if (engine->restartPending == 1) {
                    RestartEngine(engine);
                    engine->restartPending = 0;
                }
                break;
            }
        }
    case 0:
    default:
        return 0;
    }
}

u32 RestartEngine(GSEngine *engine)
{
    s32 *status;

    if (engine->callbackActive != 0) {
        if (engine->stopRequested == 0) {
            engine->restartPending = 1;
        }
        return 0;
    }

    if (engine->energyInterval != 0) {
        AsrSpiSignalCallBacks[4] =
            (AsrSpiCallback)asrspi_cbEnergyLevel;
        AsrSpiSignalCallBacks[5] =
            (AsrSpiCallback)asrspi_cbSNRLevel;
    } else {
        AsrSpiSignalCallBacks[4] = NULL;
        AsrSpiSignalCallBacks[5] = NULL;
    }

    GSAPI_PRIVATE->status = asrspi_Start(
        engine->handle, engine->energyInterval, &AsrSpiRecogCallBacks,
        engine);
    status = &GSAPI_PRIVATE->status;
    if (IsError(*status)) {
        return *status;
    }
    *status = StartListening(engine);
    return *status;
}

s32 StartListening(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (engine->audio != NULL) {
        if (api->getSamples != NULL) {
            if (api->notify != NULL) {
                api->notify(engine, 2, 0, engine->callbackValue);
            }
            return ExtAudioStart((ExtAudioState *)engine->audio);
        }
        if (SidIsRecording((SidState *)engine->audio) == 0) {
            if (api->notify != NULL) {
                api->notify(engine, 2, 0, engine->callbackValue);
            }
            return SidStart((SidState *)engine->audio);
        }
    }
    return 0;
}

s32 StopListening(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (engine->audio != NULL) {
        if (api->getSamples != NULL) {
            if (api->notify != NULL) {
                api->notify(engine, 3, 0, engine->callbackValue);
            }
            return ExtAudioStop((ExtAudioState *)engine->audio);
        }
        if (SidIsRecording((SidState *)engine->audio) != 0) {
            if (api->notify != NULL) {
                api->notify(engine, 3, 0, engine->callbackValue);
            }
            return SidStop((SidState *)engine->audio);
        }
    }
    return 0;
}

s32 gsapi_Close(void)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    SafeHandle *resource = &api->resource;
    u32 engineOffset;
    GSEngine *engine;
    s32 i;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }

    for (i = 0, engineOffset = 0; i < api->maxEngines;
         engineOffset += sizeof(GSEngine), i++) {
        engine = (GSEngine *)((u8 *)api->engines + engineOffset);
        if (engine->handle != NULL) {
            api->status = gsapi_EngineClose(engine);
        }
    }
    gsapi_LanguageUnLoad();
    heap_Free(api->heap, api->engines);
    api->status = rsrc_Close(*resource);
    if (api->status != 0) {
        return api->status;
    }
    *resource = g_lhNullHandle;
    heap_Close(api->heap);
    api->heap = NULL;
    return 0;
}

s32 gsapi_ContextActivate(
    GSEngineHandle engineHandle, void *context)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *status;
    GSEngine *engine;
    s32 deactivateStatus;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine((GSEngine *)engineHandle)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (!IsValidContext(context)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }

    engine = (GSEngine *)engineHandle;
    deactivateStatus = ContextAPIDeActivate(engine);
    status = &api->status;
    *status = deactivateStatus;
    if (IsError(*status)) {
        return *status;
    }
    if (*(u32 *)engine->parameters != 0) {
        *status = ContextUnLoad(engine->parameters);
        if (IsError(*status)) {
            return *status;
        }
        memset(
            engine->parameters, 0, sizeof(engine->parameters));
    }
    *status = ContextActivate(engine, context);
    return *status;
}

s32 gsapi_ContextSetCtxData(void *data, void **contextOut)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *status;
    s32 contextStatus;
    void *context;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (data == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    if (contextOut == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    context = heap_Alloc(api->heap, 0x28);
    if (context == NULL) {
        return GSAPI_ERROR_OUT_OF_MEMORY;
    }
    memset(context, 0, 0x28);
    contextStatus = ContextSetCtxData((void **)context, data);
    status = &api->status;
    *status = contextStatus;
    if (IsError(*status)) {
        heap_Free(api->heap, context);
        return *status;
    }
    api->contextCount++;
    *contextOut = context;
    return 0;
}

s32 gsapi_ContextSetGcdData(void *context, void *data)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    return ContextSetGcdData(context, data);
}

s32 gsapi_ContextSetParam(void *context, s32 parameter, s32 value)
{
    void *activeContext = context;
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 i;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(activeContext)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (parameter < 0 || parameter > GSAPI_PARAMETER_COUNT - 1) {
        return GSAPI_ERROR_PARAMETER_ID;
    }

    api->status = ContextSetParam(activeContext, parameter, value);
    if (api->status != 0) {
        return api->status;
    }
    for (i = 0; i < api->maxEngines; i++) {
        if (api->engines[i].context == activeContext) {
            ContextActivateParams(&api->engines[i], activeContext);
        }
    }
    return 0;
}

s32 gsapi_ContextSetWrdData(void *context, void *data)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    return ContextSetWrdData(context, data);
}

s32 gsapi_ContextDeActivate(void *context)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    void *activeContext = context;
    s32 *status;
    s32 i;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(activeContext)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }

    status = &api->status;
    for (i = 0; i < api->maxEngines; i++) {
        if (api->engines[i].context == activeContext) {
            *status = ContextDeActivate(&api->engines[i]);
            if (IsError(*status)) {
                return *status;
            }
        }
    }
    api->status = ContextUnLoad(activeContext);
    status = &api->status;
    heap_Free(api->heap, activeContext);
    api->contextCount--;
    return *status;
}

s32 gsapi_EngineClose(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *status;
    s32 closeStatus;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine(engine)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }

    engine->closing = 1;
    gsapi_EngineStop(engine);
    if (api->getSamples != NULL) {
        ExtAudioClose(api->heap, (ExtAudioState *)engine->audio);
    } else {
        if (SidIsRecording((SidState *)engine->audio) != 0) {
            SidStop((SidState *)engine->audio);
        }
        SidClose(api->heap, (SidState *)engine->audio);
    }
    ContextDeActivate(engine);
    if (*(u32 *)engine->parameters != 0) {
        ContextUnLoad(engine->parameters);
        memset(engine->parameters, 0, sizeof(engine->parameters));
    }
    closeStatus = asrspi_CloseEngine(engine->handle);
    status = &api->status;
    *status = closeStatus;
    if (*status == 0) {
        engine->handle = NULL;
        api->activeEngines--;
    }
    return *status;
}

s32 gsapi_EngineOpen(s32 device, GSEngine **engineOut)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    SafeHandle *resource = &api->resource;
    u16 *activeEngines;
    void **language;
    s32 *status;
    GSEngine *cursor;
    GSEngine *engine = NULL;
    s32 i;

    if (safeh_HandlesEqual(*resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (engineOut == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    if (*(language = &api->language) == NULL) {
        return GSAPI_ERROR_NO_LANGUAGE;
    }
    if (*(activeEngines = &api->activeEngines) == api->maxEngines) {
        return GSAPI_ERROR_ENGINE_LIMIT;
    }

    cursor = api->engines;
    for (i = 0; i < api->maxEngines; i++, cursor++) {
        engine = cursor;
        if (cursor->handle == NULL) {
            break;
        }
    }

    engine->closing = 0;
    engine->callbackActive = 0;
    engine->restartPending = 0;
    engine->reserved60 = 0;
    engine->activeResult = NULL;
    engine->mode = 1;
    engine->energyInterval = DEFAULTENERGYINTERVAL;
    engine->active = 0;
    engine->context = NULL;
    engine->callbackValue = 0;

    if (api->getSamples != NULL) {
        api->status = ExtAudioInit(
            api->heap, (ExtAudioState **)&engine->audio);
    } else {
        api->status = SidInit(
            api->heap, (void *)device, (SidState **)&engine->audio);
    }
    status = &api->status;
    if (IsError(*status)) {
        return api->status;
    }

    memset(engine->parameters, 0, sizeof(engine->parameters));
    *status = asrspi_OpenEngine(*resource, 0, engine);
    if (IsSuccess(*status)) {
        (*activeEngines)++;
        *status = asrspi_ActivateLng(engine->handle, *language);
        *engineOut = engine;
    }
    return *status;
}

s32 gsapi_EngineSetMode(GSEngineHandle engineHandle, u32 mode)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    GSEngine *engine;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine((GSEngine *)engineHandle)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (mode == 0) {
        return GSAPI_ERROR_MODE;
    }
    if ((mode & 1) != 0 && (mode & 2) != 0) {
        return GSAPI_ERROR_MODE;
    }
    engine = (GSEngine *)engineHandle;
    engine->mode = mode;
    return 0;
}

s32 gsapi_EngineSetParam(
    GSEngineHandle engineHandle, s32 parameter, s32 value)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *status;
    GSEngine *engine;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine((GSEngine *)engineHandle)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (parameter < 0 || parameter > GSAPI_PARAMETER_COUNT - 1) {
        return GSAPI_ERROR_PARAMETER_ID;
    }
    if (*(u32 *)((GSEngine *)engineHandle)->parameters == 0) {
        return GSAPI_ERROR_NO_PARAMETERS;
    }

    engine = (GSEngine *)engineHandle;
    status = &api->status;
    *status = ContextSetParam(
        engine->parameters, parameter, value);
    if (*status != 0) {
        return *status;
    }
    *status = ContextActivateParams(engine, engine->parameters);
    return *status;
}

s32 gsapi_EngineStart(GSEngine *engine)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 status;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine(engine)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }

    {
        s32 *stopRequested = &engine->stopRequested;

        *stopRequested = 0;
        if (engine->callbackActive != 0) {
            if (*stopRequested == 0) {
                engine->restartPending = 1;
            }
            status = 0;
        } else {
            u32 *energyInterval = &engine->energyInterval;

            if (*energyInterval != 0) {
                AsrSpiSignalCallBacks[4] =
                    (AsrSpiCallback)asrspi_cbEnergyLevel;
                AsrSpiSignalCallBacks[5] =
                    (AsrSpiCallback)asrspi_cbSNRLevel;
            } else {
                AsrSpiSignalCallBacks[4] = NULL;
                AsrSpiSignalCallBacks[5] = NULL;
            }
            api->status = asrspi_Start(
                engine->handle, *energyInterval,
                &AsrSpiRecogCallBacks, engine);
            status = api->status;
            if (IsError(status) == FALSE) {
                api->status = StartListening(engine);
                status = api->status;
            }
        }
    }
    if (status >= 0) {
        engine->active = 1;
    }
    return status;
}

s32 gsapi_EngineStop(GSEngineHandle engineHandle)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    s32 *apiStatus;
    GSEngine *engine;
    s32 status;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine((GSEngine *)engineHandle)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }

    engine = (GSEngine *)engineHandle;
    engine->stopRequested = 1;
    if (engine->audio == NULL) {
        goto engine_stop_zero;
    }
    if (api->getSamples != NULL) {
        if (api->notify != NULL) {
            api->notify(
                engine, 3, 0, engine->callbackValue);
        }
        status = ExtAudioStop((ExtAudioState *)engine->audio);
        goto engine_stop_done;
    }
    goto engine_stop_sid;

engine_stop_sid:
    if (SidIsRecording((SidState *)engine->audio) == 0) {
        goto engine_stop_zero;
    }
    if (api->notify != NULL) {
        api->notify(
            engine, 3, 0, engine->callbackValue);
    }
    status = SidStop((SidState *)engine->audio);
    goto engine_stop_done;

engine_stop_zero:
    status = 0;
engine_stop_done:

    apiStatus = &api->status;
    *apiStatus = status;
    if (*apiStatus != 0) {
        return *apiStatus;
    }
    *apiStatus = asrspi_GetState(
        engine->handle, &engine->asrState);
    if (*apiStatus != 0) {
        return *apiStatus;
    }
    if (engine->asrState == 3) {
        *apiStatus = asrspi_Stop(engine->handle);
    }
    if (IsSuccess(*apiStatus)) {
        engine->active = 0;
    }
    return *apiStatus;
}

void gsapi_EngineRestart(void)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    u32 engineOffset;
    SafeHandle nullHandle;
    s32 *status = &api->status;
    s32 i;
    GSEngine *engine;

    *status = 0;
    nullHandle = g_lhNullHandle;
    if (safeh_HandlesEqual(api->resource, nullHandle)) {
        *status = GSAPI_ERROR_NOT_OPEN;
        return;
    }

    for (engineOffset = 0, i = 0; i < api->maxEngines;
         engineOffset += sizeof(GSEngine), i++) {
        engine = (GSEngine *)((u8 *)api->engines + engineOffset);
        if (engine->restartPending == 1) {
            RestartEngine(engine);
            engine->restartPending = 0;
        }
        *status = ProcessAudio(engine);
    }
}

s32 gsapi_EngineGetParam(GSEngine *engine, u32 parameter, u32 *value)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    GSActiveResult *result;
    GSResultValues *parameters;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine(engine)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (value == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    result = engine->activeResult;
    if (result->result == NULL) {
        return GSAPI_ERROR_NO_RESULT;
    }
    parameters = result->parameters;
    if (parameters == NULL) {
        return GSAPI_ERROR_NO_PARAMETER_NAMES;
    }
    if (parameter > parameters->lastIndex) {
        return GSAPI_ERROR_PARAMETER_RANGE;
    }
    *value = parameters->values[parameter];
    return 0;
}

s32 gsapi_Init(GSResultCallback resultCallback, void *userData)
{
    SafeHandle *resource;
    s32 status;

    if (resultCallback == NULL) {
        return GSAPI_PRIVATE->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    resource = &GSAPI_PRIVATE->resource;
    if (!safeh_HandlesEqual(
            *(SafeHandle *)&gGSAPI.reserved04[4], g_lhNullHandle)) {
        return GSAPI_PRIVATE->status = GSAPI_ERROR_ALREADY_OPEN;
    }

    GSAPI_PRIVATE->maxEngines = MAXRECOGNIZERS;
    GSAPI_PRIVATE->activeEngines = 0;
    GSAPI_PRIVATE->resultCallback = resultCallback;
    GSAPI_PRIVATE->notify = NULL;
    GSAPI_PRIVATE->getSamples = NULL;
    GSAPI_PRIVATE->contextCount = 0;
    GSAPI_PRIVATE->userData = userData;

    status = heap_Open(&GSAPI_PRIVATE->heap, GSAPI_PRIVATE->heapSize);
    if (IsError(status)) {
        return status;
    }
    GSAPI_PRIVATE->engines = heap_Calloc(
        GSAPI_PRIVATE->heap, MAXRECOGNIZERS, sizeof(GSEngine));
    if (GSAPI_PRIVATE->engines == NULL) {
        return GSAPI_ERROR_OUT_OF_MEMORY;
    }
    status = rsrc_Open(GSAPI_PRIVATE->heap, 0, NULL, NULL, resource);
    if (IsError(status)) {
        return status;
    }
    return 0;
}

s32 gsapi_LanguageLoadBuffer(void *buffer, u32 loadParameters)
{
    GSParameterState *parameters = NULL;
    s32 *parameterValue;
    u32 validParameters;
    s32 i;

    if (safeh_HandlesEqual(GSAPI_PRIVATE->resource, g_lhNullHandle)) {
        return GSAPI_PRIVATE->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (buffer == NULL) {
        return GSAPI_PRIVATE->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    gsapi_LanguageUnLoad();
    if (*(u32 *)buffer == GSAPI_LANGUAGE_HEADER) {
        GSAPI_PRIVATE->languageBuffer = buffer;
        GSAPI_PRIVATE->language = (u8 *)buffer + 0x10;
    } else {
        GSAPI_PRIVATE->language = buffer;
        GSAPI_PRIVATE->languageBuffer = NULL;
    }
    InitDefaultParams();

    if (loadParameters != 0) {
        validParameters = parameters->validParameters;
        parameterValue = parameters->values;
        for (i = 0; i < GSAPI_PARAMETER_COUNT; i++) {
            if ((validParameters & 1) != 0) {
                GSAPI_PRIVATE->defaultParameters.values[i] = *parameterValue;
            }
            validParameters >>= 1;
            parameterValue++;
        }
    }
    return 0;
}

s32 gsapi_LanguageUnLoad(void)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;
    void **language;
    void *loadedLanguage;
    s32 i;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    loadedLanguage = *(language = &api->language);
    if (loadedLanguage == NULL) {
        return GSAPI_ERROR_NO_LANGUAGE;
    }

    for (i = 0; i < api->maxEngines; i++) {
    }
    *language = NULL;
    api->languageBuffer = NULL;
    return 0;
}

s32 gsapi_EngineSessionDataExport(
    GSEngine *engine, void **sessionData)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine(engine)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (sessionData == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataExport(engine, sessionData);
}

s32 gsapi_EngineSessionDataImport(
    GSEngine *engine, void *sessionData, s32 flags)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidEngine(engine)) {
        return api->status = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (sessionData == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataImport(engine, sessionData, flags);
}

s32 gsapi_EngineSessionDataFree(void *sessionData)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    if (sessionData == NULL) {
        return api->status = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataFree(sessionData);
}

s32 gsapi_NotifySetCallback(GSNotifyCallback callback)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_NOT_OPEN;
    }
    api->notify = callback;
    return 0;
}

s32 gsapi_SetUserData(s32 heapSize)
{
    GSAPIPrivate *api = GSAPI_PRIVATE;

    if (!safeh_HandlesEqual(api->resource, g_lhNullHandle)) {
        return api->status = GSAPI_ERROR_ALREADY_OPEN;
    }
    api->heapSize = heapSize;
    return 0;
}
