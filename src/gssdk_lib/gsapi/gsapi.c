/* Recognition engines, contexts, and audio processing for the game speech API. */
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/asrspi.h"
#include "gssdk/gsapi.h"
#include "gssdk/rsrc.h"
#include "gssdk/gsapi_internal.h"

#define GSAPI_ERROR_ALREADY_OPEN 0x80CC0000U
#define GSAPI_ERROR_NOT_OPEN 0x80CC0001U
#define GSAPI_ERROR_NO_LANGUAGE 0x80CC0002U
#define GSAPI_ERROR_ENGINE_LIMIT 0x80CC0005U
#define GSAPI_ERROR_AUDIO 0x80CC0006U
#define GSAPI_ERROR_MODE 0x80CC0007U
#define GSAPI_ERROR_NO_WORD_DATA 0x80CC000AU
#define GSAPI_ERROR_WORD_ID_RANGE 0x80CC000BU
#define GSAPI_ERROR_PARAMETER_ID 0x80CC000CU
#define GSAPI_ERROR_NO_PARAMETERS 0x80CC000DU
#define GSAPI_ERROR_NO_WORD_LIST 0x80CC0080U
#define GSAPI_ERROR_INVALID_ARGUMENT 0x80CC0083U
#define GSAPI_ERROR_OUT_OF_MEMORY 0x80CC0086U
#define GSAPI_ERROR_INVALID_HANDLE 0x80CC0089U

#define GSAPI_LANGUAGE_HEADER 0x00091874U

/* Tests whether the status has its error bit clear. */
static BOOL IsSuccess(unsigned long result)
{
    return result < 0x80000000 ? TRUE : FALSE;
}

/* Performs a pending restart and clears the restart flag. */
static void CheckDelayedRestart(struct _GSAPI_ENGINE *pEngine)
{
    if (pEngine->bDelayedRestart == 1) {
        RestartEngine(pEngine);
        pEngine->bDelayedRestart = 0;
    }
}

/* Checks that the handle addresses a recognizer slot with an SPI engine. */
static BOOL CheckEngineHandle(void *hEngine)
{
    unsigned long u32EngineAddress = (unsigned long)hEngine;
    unsigned long u32BaseAddress = (unsigned long)gGSAPI.Recognizers;

    if (hEngine == NULL) {
        return FALSE;
    }
    if (u32EngineAddress < u32BaseAddress) {
        return FALSE;
    }
    if ((u32EngineAddress - u32BaseAddress) % sizeof(struct _GSAPI_ENGINE) != 0) {
        return FALSE;
    }
    if (u32EngineAddress >
        u32BaseAddress + gGSAPI.u16MaxRecognizers * sizeof(struct _GSAPI_ENGINE)) {
        return FALSE;
    }
    if (((struct _GSAPI_ENGINE *)hEngine)->hAsrSpiEngine == NULL) {
        return FALSE;
    }
    return TRUE;
}

/* Checks that the client supplied a context handle. */
static BOOL IsValidContext(void *context)
{
    return context == NULL ? FALSE : TRUE;
}

const long TranslateParamTable[GSAPI_PARAM_LAST] = {
    0, 1, 3, 0x11, 6, 7, 8, 9, 0xA,
    0xFF, 0xD, 0xE, 0xF, 0x10, 4, 0xB, 0xC, 0x12,
};

struct _GSAPI_GLOBAL gGSAPI = { 0 };

char GSAPIVersionString[0x1C] = "  [ASRSPI %d.%02d.%04d]";

u16 MAXRECOGNIZERS = 4;
u16 DEFAULTENERGYINTERVAL = 500;

/* Maps a GSAPI parameter ID to the corresponding SPI ID. */
enum _ASRBASE_PARAMID TranslateGsapiParamId2AsrSpiParamId(enum _GSAPI_PARAMID parameter)
{
    return TranslateParamTable[parameter];
}

/* Maps global default parameter IDs to SPI IDs and applies their values. */
long ActivateDefaultParams(struct _GSAPI_ENGINE *pEngine)
{
    struct _ASRBASE_PARAM ParamList[GSAPI_PARAM_LAST - 1];
    unsigned long count = 0;
    unsigned long i;

    for (i = 0; i < GSAPI_PARAM_LAST; i++) {
        if (TranslateParamTable[i] != 0xFF) {
            ParamList[count].ID = TranslateParamTable[i];
            ParamList[count].Value = gGSAPI.DefaultParams.Params[i];
            count++;
        }
    }

    return asrspi_SetParamList(pEngine->hAsrSpiEngine, ParamList, count);
}

/* Initializes the shared recognition parameter values. */
static void InitDefaultParams(void)
{
    gGSAPI.DefaultParams.u32UsedParams = 0x1FFFF;
    gGSAPI.DefaultParams.Params[0] = 1;
    gGSAPI.DefaultParams.Params[1] = 1;
    gGSAPI.DefaultParams.Params[2] = 0;
    gGSAPI.DefaultParams.Params[3] = 0x32;
    gGSAPI.DefaultParams.Params[4] = 0x3C;
    gGSAPI.DefaultParams.Params[5] = 1;
    gGSAPI.DefaultParams.Params[6] = 1;
    gGSAPI.DefaultParams.Params[7] = 0;
    gGSAPI.DefaultParams.Params[8] = 0x7FFFFFFF;
    gGSAPI.DefaultParams.Params[9] = 0x1388;
    gGSAPI.DefaultParams.Params[10] = 0xC8;
    gGSAPI.DefaultParams.Params[11] = 0x12C;
    gGSAPI.DefaultParams.Params[12] = 0x28;
    gGSAPI.DefaultParams.Params[13] = 0x28;
    gGSAPI.DefaultParams.Params[14] = 1;
    gGSAPI.DefaultParams.Params[15] = -0x1C20;
    gGSAPI.DefaultParams.Params[16] = 0;
    gGSAPI.DefaultParams.Params[17] = 0;
}

/* Applies the parameter overrides selected by the GCD parameter mask. */
static long LoadDefaultParams(const unsigned long *pGcdData)
{
    struct GSAPIParamBlock *ParamBuf;
    int i;
    unsigned long u32Used;
    unsigned long *pValue;

    if (pGcdData != NULL) {
        /* The SDK reads the parameter mask and values through a NULL block. */
        ParamBuf = NULL;
        u32Used = ParamBuf->u32UsedParams;
        pValue = ParamBuf->Params;
        for (i = 0; i < GSAPI_PARAM_LAST; i++) {
            if (u32Used & 1) {
                gGSAPI.DefaultParams.Params[i] = *pValue;
            }
            pValue++;
            u32Used >>= 1;
        }
    }
    return 0;
}

/* Feeds available audio to the recognizer and handles acquisition completion. */
static long ProcessAudio(struct _GSAPI_ENGINE *pEngine)
{
    signed short *pWaveData = NULL;
    long nSamplesRead;
    long nSamplesLeft;
    int bFinished = 0;

    gGSAPI.lhError = asrspi_GetState(pEngine->hAsrSpiEngine, &pEngine->asrspiState);
    if (pEngine->State == GSAPI_STATE_IDLE) {
        return 0;
    }
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }

    switch (pEngine->State) {
    case GSAPI_STATE_IDLE:
        break;
    case GSAPI_STATE_RECOGNIZING:
        do {
            if (gGSAPI.cbGetWaveData != NULL) {
                nSamplesRead = ExtAudioGetSamples(pEngine, &pWaveData, 0x6E);
            } else {
                nSamplesRead = SidGetSamples(pEngine->hSid, &pWaveData, 0x6E);
            }
            if (nSamplesRead > 0) {
                if (pEngine->Mode & 4) {
                    if (gGSAPI.cbNotify != NULL) {
                        gGSAPI.cbNotify(
                            pEngine, GSAPI_NOTIFY_WAVEDATA, (unsigned long)pWaveData,
                            nSamplesRead * 2);
                    }
                }
                gGSAPI.lhError = asrspi_Acquisition(
                    pEngine->hAsrSpiEngine, pWaveData, nSamplesRead * 2, &bFinished);
                if (IsError(gGSAPI.lhError)) {
                    return gGSAPI.lhError;
                }
                if (gGSAPI.cbGetWaveData == NULL) {
                    SidAdvanceBuffer(pEngine->hSid, nSamplesRead);
                }
                if (bFinished) {
                    StopListening(pEngine);
                }
            } else if (nSamplesRead < 0) {
                return GSAPI_ERROR_AUDIO;
            }
            if (gGSAPI.cbGetWaveData != NULL) {
                nSamplesLeft = ExtAudioGetSamplesLeft(pEngine);
            } else {
                nSamplesLeft = SidGetSamplesLeft(pEngine->hSid);
            }
        } while (nSamplesLeft >= 0x6E);
        CheckDelayedRestart(pEngine);
        break;
    }
    return 0;
}

/* Restarts recognition and reinstalls the signal callbacks. */
long RestartEngine(struct _GSAPI_ENGINE *pEngine)
{
    if (pEngine->bInCallback != 0) {
        if (pEngine->bManualStop == 0) {
            pEngine->bDelayedRestart = 1;
        }
        return 0;
    }

    if (pEngine->u32EnergyInterval != 0) {
        AsrSpiSignalCallBacks.cbEnergyLevel = asrspi_cbEnergyLevel;
        AsrSpiSignalCallBacks.cbSNRLevel = asrspi_cbSNRLevel;
    } else {
        AsrSpiSignalCallBacks.cbEnergyLevel = NULL;
        AsrSpiSignalCallBacks.cbSNRLevel = NULL;
    }

    gGSAPI.lhError = asrspi_Start(
        pEngine->hAsrSpiEngine, pEngine->u32EnergyInterval, &AsrSpiRecogCallBacks,
        (unsigned long)pEngine);
    if (IsError(gGSAPI.lhError)) {
        return gGSAPI.lhError;
    }
    gGSAPI.lhError = StartListening(pEngine);
    return gGSAPI.lhError;
}

/* Starts audio input and notifies the client when listening begins. */
long StartListening(struct _GSAPI_ENGINE *pEngine)
{
    if (pEngine->hSid != 0) {
        if (gGSAPI.cbGetWaveData != NULL) {
            if (gGSAPI.cbNotify != NULL) {
                gGSAPI.cbNotify(
                    pEngine, GSAPI_NOTIFY_STARTLISTENING, 0, pEngine->u32EngineUserData);
            }
            return ExtAudioStart(pEngine->hSid);
        }
        if (SidIsRecording(pEngine->hSid) == 0) {
            if (gGSAPI.cbNotify != NULL) {
                gGSAPI.cbNotify(
                    pEngine, GSAPI_NOTIFY_STARTLISTENING, 0, pEngine->u32EngineUserData);
            }
            return SidStart(pEngine->hSid);
        }
    }
    return 0;
}

/* Stops audio input and notifies the client when listening ends. */
long StopListening(struct _GSAPI_ENGINE *pEngine)
{
    if (pEngine->hSid != 0) {
        if (gGSAPI.cbGetWaveData != NULL) {
            if (gGSAPI.cbNotify != NULL) {
                gGSAPI.cbNotify(pEngine, GSAPI_NOTIFY_STOPLISTENING, 0, pEngine->u32EngineUserData);
            }
            return ExtAudioStop(pEngine->hSid);
        }
        if (SidIsRecording(pEngine->hSid) != 0) {
            if (gGSAPI.cbNotify != NULL) {
                gGSAPI.cbNotify(pEngine, GSAPI_NOTIFY_STOPLISTENING, 0, pEngine->u32EngineUserData);
            }
            return SidStop(pEngine->hSid);
        }
    }
    return 0;
}

/* Closes the recognizers and releases the shared resource and heap. */
long gsapi_Close(void)
{
    unsigned long engineOffset;
    struct _GSAPI_ENGINE *pEngine;
    long i;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }

    for (i = 0, engineOffset = 0; i < gGSAPI.u16MaxRecognizers;
         engineOffset += sizeof(struct _GSAPI_ENGINE), i++) {
        pEngine = (struct _GSAPI_ENGINE *)((u8 *)gGSAPI.Recognizers + engineOffset);
        if (pEngine->hAsrSpiEngine != NULL) {
            gGSAPI.lhError = gsapi_EngineClose(pEngine);
        }
    }
    gsapi_LanguageUnLoad();
    heap_Free(gGSAPI.hHeap, gGSAPI.Recognizers);
    gGSAPI.lhError = rsrc_Close(gGSAPI.hRsrc);
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }
    gGSAPI.hRsrc = g_lhNullHandle;
    heap_Close(gGSAPI.hHeap);
    gGSAPI.hHeap = NULL;
    return 0;
}

/* Releases the temporary context and activates the supplied context. */
long gsapi_ContextActivate(void *hEngine, void *context)
{
    struct _GSAPI_ENGINE *pEngine;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(hEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (!IsValidContext(context)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }

    pEngine = (struct _GSAPI_ENGINE *)hEngine;
    gGSAPI.lhError = ContextAPIDeActivate(pEngine);
    if (IsError(gGSAPI.lhError)) {
        return gGSAPI.lhError;
    }
    if (pEngine->TempContext.pCtx != NULL) {
        gGSAPI.lhError = ContextUnLoad(&pEngine->TempContext);
        if (IsError(gGSAPI.lhError)) {
            return gGSAPI.lhError;
        }
        memset(
            &pEngine->TempContext, 0, sizeof(pEngine->TempContext));
    }
    gGSAPI.lhError = ContextActivate(pEngine, context);
    return gGSAPI.lhError;
}

/* Allocates a context and sets its recognition data. */
long gsapi_ContextSetCtxData(void *data, void **contextOut)
{
    struct _GSAPI_CONTEXT *context;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (data == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    if (contextOut == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    context = heap_Alloc(gGSAPI.hHeap, sizeof *context);
    if (context == NULL) {
        return GSAPI_ERROR_OUT_OF_MEMORY;
    }
    memset(context, 0, sizeof *context);
    gGSAPI.lhError = ContextSetCtxData(context, data);
    if (IsError(gGSAPI.lhError)) {
        heap_Free(gGSAPI.hHeap, context);
        return gGSAPI.lhError;
    }
    gGSAPI.NumContextsLoaded++;
    *contextOut = context;
    return 0;
}

/* Loads parameter overrides and word actions into a context. */
long gsapi_ContextSetGcdData(void *context, void *data)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    return ContextSetGcdData(context, data);
}

/* Records a context parameter and applies it to engines using the context. */
long gsapi_ContextSetParam(void *context, long parameter, long value)
{
    struct _GSAPI_ENGINE *pEngine;
    int i;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (parameter < 0 || parameter > GSAPI_PARAM_LAST - 1) {
        return GSAPI_ERROR_PARAMETER_ID;
    }

    gGSAPI.lhError = ContextSetParam(context, parameter, value);
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }
    for (i = 0; i < gGSAPI.u16MaxRecognizers; i++) {
        pEngine = &gGSAPI.Recognizers[i];
        if (pEngine->hActiveContext == context) {
            ContextActivateParams(pEngine, context);
        }
    }
    return 0;
}

/* Loads the word resource and its string index into a context. */
long gsapi_ContextSetWrdData(void *context, void *data)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    return ContextSetWrdData(context, data);
}

/* Deactivates and frees a loaded recognition context. */
long gsapi_ContextDeActivate(void *context)
{
    struct _GSAPI_ENGINE *pEngine;
    int i;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!IsValidContext(context)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }

    for (i = 0; i < gGSAPI.u16MaxRecognizers; i++) {
        pEngine = &gGSAPI.Recognizers[i];
        if (pEngine->hActiveContext == context) {
            gGSAPI.lhError = ContextDeActivate(pEngine);
            if (IsError(gGSAPI.lhError)) {
                return gGSAPI.lhError;
            }
        }
    }
    gGSAPI.lhError = ContextUnLoad(context);
    heap_Free(gGSAPI.hHeap, context);
    gGSAPI.NumContextsLoaded--;
    return gGSAPI.lhError;
}

/* Stops and closes a recognizer and releases its audio and context state. */
long gsapi_EngineClose(struct _GSAPI_ENGINE *pEngine)
{

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(pEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }

    pEngine->bClosing = 1;
    gsapi_EngineStop(pEngine);
    if (gGSAPI.cbGetWaveData != NULL) {
        ExtAudioClose(gGSAPI.hHeap, pEngine->hSid);
    } else {
        if (SidIsRecording(pEngine->hSid) != 0) {
            SidStop(pEngine->hSid);
        }
        SidClose(gGSAPI.hHeap, pEngine->hSid);
    }
    ContextDeActivate(pEngine);
    if (pEngine->TempContext.pCtx != NULL) {
        ContextUnLoad(&pEngine->TempContext);
        memset(&pEngine->TempContext, 0, sizeof(pEngine->TempContext));
    }
    gGSAPI.lhError = asrspi_CloseEngine(pEngine->hAsrSpiEngine);
    if (gGSAPI.lhError == 0) {
        pEngine->hAsrSpiEngine = NULL;
        gGSAPI.u16NumRecognizers--;
    }
    return gGSAPI.lhError;
}

/* Opens a recognizer for the selected audio device and activates the current language. */
long gsapi_EngineOpen(long s32DevNum, struct _GSAPI_ENGINE **phEngine)
{
    struct _GSAPI_ENGINE *pEngine = NULL;
    int i;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (phEngine == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    if (gGSAPI.pLang == NULL) {
        return GSAPI_ERROR_NO_LANGUAGE;
    }
    if (gGSAPI.u16NumRecognizers == gGSAPI.u16MaxRecognizers) {
        return GSAPI_ERROR_ENGINE_LIMIT;
    }

    for (i = 0; i < gGSAPI.u16MaxRecognizers; i++) {
        pEngine = &gGSAPI.Recognizers[i];
        if (pEngine->hAsrSpiEngine == NULL) {
            break;
        }
    }
    pEngine->bClosing = 0;
    pEngine->bInCallback = 0;
    pEngine->bDelayedRestart = 0;
    pEngine->DoSessionDataSave = 0;
    pEngine->pCurrentContext = NULL;
    pEngine->Mode = 1;
    pEngine->u32EnergyInterval = DEFAULTENERGYINTERVAL;
    pEngine->State = GSAPI_STATE_IDLE;
    pEngine->hActiveContext = NULL;
    pEngine->u32EngineUserData = 0;

    if (gGSAPI.cbGetWaveData != NULL) {
        gGSAPI.lhError = ExtAudioInit(gGSAPI.hHeap, &pEngine->hSid);
    } else {
        gGSAPI.lhError = SidInit(gGSAPI.hHeap, s32DevNum, &pEngine->hSid);
    }
    if (IsError(gGSAPI.lhError)) {
        return gGSAPI.lhError;
    }

    memset(&pEngine->TempContext, 0, sizeof(pEngine->TempContext));
    gGSAPI.lhError = asrspi_OpenEngine(gGSAPI.hRsrc, ASRBASE_PCM_16_11KHZ, &pEngine->hAsrSpiEngine);
    if (IsSuccess(gGSAPI.lhError)) {
        gGSAPI.u16NumRecognizers++;
        gGSAPI.lhError = asrspi_ActivateLng(pEngine->hAsrSpiEngine, gGSAPI.pLang);
        *phEngine = pEngine;
    }
    return gGSAPI.lhError;
}

/* Sets the recognition mode after checking the client flags. */
long gsapi_EngineSetMode(void *hEngine, unsigned long Mode)
{
    struct _GSAPI_ENGINE *pEngine;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(hEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (Mode == 0) {
        return GSAPI_ERROR_MODE;
    }
    if ((Mode & 1) != 0 && (Mode & 2) != 0) {
        return GSAPI_ERROR_MODE;
    }
    pEngine = (struct _GSAPI_ENGINE *)hEngine;
    pEngine->Mode = Mode;
    return 0;
}

/* Sets a parameter on the engine's temporary context and applies it. */
long gsapi_EngineSetParam(void *hEngine, long ParamId, long ParamValue)
{
    struct _GSAPI_ENGINE *pEngine;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(hEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (ParamId < 0 || ParamId > GSAPI_PARAM_LAST - 1) {
        return GSAPI_ERROR_PARAMETER_ID;
    }
    pEngine = (struct _GSAPI_ENGINE *)hEngine;
    if (pEngine->TempContext.pCtx == NULL) {
        return GSAPI_ERROR_NO_PARAMETERS;
    }
    gGSAPI.lhError = ContextSetParam(&pEngine->TempContext, ParamId, ParamValue);
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }
    gGSAPI.lhError = ContextActivateParams(pEngine, &pEngine->TempContext);
    return gGSAPI.lhError;
}

/* Installs the recognition callbacks and begins listening. */
long gsapi_EngineStart(void *hEngine)
{
    long Error;
    struct _GSAPI_ENGINE *pEngine;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(hEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    pEngine = (struct _GSAPI_ENGINE *)hEngine;
    pEngine->bManualStop = 0;
    Error = RestartEngine((struct _GSAPI_ENGINE *)hEngine);
    if (Error >= 0) {
        pEngine->State = GSAPI_STATE_RECOGNIZING;
    }
    return Error;
}

/* Stops listening and stops the recognizer if recognition is active. */
long gsapi_EngineStop(void *hEngine)
{
    struct _GSAPI_ENGINE *pEngine;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(hEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }

    pEngine = (struct _GSAPI_ENGINE *)hEngine;
    pEngine->bManualStop = 1;
    gGSAPI.lhError = StopListening(pEngine);
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }
    gGSAPI.lhError = asrspi_GetState(
        pEngine->hAsrSpiEngine, &pEngine->asrspiState);
    if (gGSAPI.lhError != 0) {
        return gGSAPI.lhError;
    }
    if (pEngine->asrspiState == ASRBASE_RECOGNIZING) {
        gGSAPI.lhError = asrspi_Stop(pEngine->hAsrSpiEngine);
    }
    if (IsSuccess(gGSAPI.lhError)) {
        pEngine->State = GSAPI_STATE_IDLE;
    }
    return gGSAPI.lhError;
}

/* Processes pending restarts and available audio for every recognizer. */
void gsapi_EngineRestart()
{
    int i;
    struct _GSAPI_ENGINE *pEngine;

    gGSAPI.lhError = 0;
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
        return;
    }

    for (i = 0; i < gGSAPI.u16MaxRecognizers; i++) {
        pEngine = &gGSAPI.Recognizers[i];
        CheckDelayedRestart(pEngine);
        gGSAPI.lhError = ProcessAudio(pEngine);
        if (IsError(gGSAPI.lhError)) {
            /* no code in the release build */
        }
    }
}

/* Returns the string for a word ID in the engine's current context. */
long gsapi_EngineGetParam(void *hEngine, unsigned long WordId, char **pszResult)
{
    struct _GSAPI_ENGINE *pEngine = hEngine;
    struct _GSAPI_CONTEXT *pContext;
    struct tag_WRD_WORDLIST *wordList;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(pEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (pszResult == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    pContext = pEngine->pCurrentContext;
    if (pContext->pWrd == NULL) {
        return GSAPI_ERROR_NO_WORD_DATA;
    }
    wordList = pContext->pWordList;
    if (wordList == NULL) {
        return GSAPI_ERROR_NO_WORD_LIST;
    }
    if (WordId > wordList->u32NbrWords) {
        return GSAPI_ERROR_WORD_ID_RANGE;
    }
    *pszResult = wordList->pWordStringList[WordId];
    return 0;
}

/* Creates the SDK heap and shared resource and allocates the recognizer slots. */
long gsapi_Init(GSResultCallback cbResult, unsigned long u32UserData)
{
    long Error;

    if (cbResult == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    if (!safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_ALREADY_OPEN;
    }

    gGSAPI.u16MaxRecognizers = MAXRECOGNIZERS;
    gGSAPI.u16NumRecognizers = 0;
    gGSAPI.cbResult = cbResult;
    gGSAPI.cbNotify = NULL;
    gGSAPI.cbGetWaveData = NULL;
    gGSAPI.NumContextsLoaded = 0;
    gGSAPI.u32UserData = u32UserData;

    Error = heap_Open(&gGSAPI.hHeap, gGSAPI.u32HeapUserData);
    if (IsError(Error)) {
        return Error;
    }
    gGSAPI.Recognizers = heap_Calloc(gGSAPI.hHeap, MAXRECOGNIZERS, sizeof(struct _GSAPI_ENGINE));
    if (gGSAPI.Recognizers == NULL) {
        return GSAPI_ERROR_OUT_OF_MEMORY;
    }
    Error = rsrc_Open(gGSAPI.hHeap, NULL, NULL, NULL, &gGSAPI.hRsrc);
    if (IsError(Error)) {
        return Error;
    }
    return 0;
}

/* Loads the language payload and sets the default parameters. */
long gsapi_LanguageLoadBuffer(const unsigned long *pu32LngData, const unsigned long *pu32GcdData)
{
    unsigned char *p = (unsigned char *)pu32LngData;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (pu32LngData == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }

    gsapi_LanguageUnLoad();
    if (*pu32LngData == GSAPI_LANGUAGE_HEADER) {
        gGSAPI.pHDECLang = p;
        gGSAPI.pLang = p + 0x10;
    } else {
        gGSAPI.pLang = p;
        gGSAPI.pHDECLang = NULL;
    }
    InitDefaultParams();
    LoadDefaultParams(pu32GcdData);
    return 0;
}

/* Clears the currently loaded language pointers. */
long gsapi_LanguageUnLoad(void)
{
    long i;

    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (gGSAPI.pLang == NULL) {
        return GSAPI_ERROR_NO_LANGUAGE;
    }

    /* The SDK visits each recognizer slot without changing its state. */
    for (i = 0; i < gGSAPI.u16MaxRecognizers; i++) {
    }
    gGSAPI.pLang = NULL;
    gGSAPI.pHDECLang = NULL;
    return 0;
}

/* Copies the engine's session state into a new buffer. */
long gsapi_EngineSessionDataExport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA **ppSessionData)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(pEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (ppSessionData == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataExport(pEngine, ppSessionData);
}

/* Restores a saved session to an engine and optionally frees it. */
long gsapi_EngineSessionDataImport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA *pSessionData, int bFreeMemory)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (!CheckEngineHandle(pEngine)) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_HANDLE;
    }
    if (pSessionData == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataImport(pEngine, pSessionData, bFreeMemory);
}

/* Releases a saved session buffer. */
long gsapi_EngineSessionDataFree(struct _GSAPI_SESSIONDATA *pSessionData)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    if (pSessionData == NULL) {
        return gGSAPI.lhError = GSAPI_ERROR_INVALID_ARGUMENT;
    }
    return SessionDataFree(pSessionData);
}

/* Sets the client callback for notifications. */
long gsapi_NotifySetCallback(GSNotifyCallback cbNotify)
{
    if (safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_NOT_OPEN;
    }
    gGSAPI.cbNotify = cbNotify;
    return 0;
}

/* Stores the client heap value before the SDK is initialized. */
long gsapi_SetUserData(long heapSize)
{
    if (!safeh_HandlesEqual(gGSAPI.hRsrc, g_lhNullHandle)) {
        return gGSAPI.lhError = GSAPI_ERROR_ALREADY_OPEN;
    }
    gGSAPI.u32HeapUserData = heapSize;
    return 0;
}
