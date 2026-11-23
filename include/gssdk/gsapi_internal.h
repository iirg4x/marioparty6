/* Internal engine and session contracts shared by the GSAPI implementation. */
#ifndef GSSDK_GSAPI_INTERNAL_H
#define GSSDK_GSAPI_INTERNAL_H

#include "gssdk/asrspi.h"
#include "gssdk/gsapi.h"
#include "gssdk/rsrc.h"

/* In-memory string index for a loaded word resource. */
struct tag_WRD_WORDLIST {
    unsigned long u32NbrWords;
    char **pWordStringList;
};

/* Indexes the strings in a WRD resource. */
long WrdCreateWordList(void *hHeap, struct tag_WRD_WORDLIST *pWordList, void *pWrd);

/* Releases the string index. */
long WrdDestroyWordList(void *hHeap, struct tag_WRD_WORDLIST *pWordList);

/* Parameter IDs used by the game speech API. */
enum _GSAPI_PARAMID {
    GSAPI_PARAM_TS_ENABLE = 0x0,
    GSAPI_PARAM_START_ENABLE = 0x1,
    GSAPI_PARAM_TIMEOUT = 0x2,
    GSAPI_PARAM_SENSITIVITY = 0x3,
    GSAPI_PARAM_MINSPEECH = 0x4,
    GSAPI_PARAM_MAXNBEST = 0x5,
    GSAPI_PARAM_FARTALK = 0x6,
    GSAPI_PARAM_AGCON = 0x7,
    GSAPI_PARAM_SELECTGENDER = 0x8,
    GSAPI_PARAM_CONF_THRESHOLD = 0x9,
    GSAPI_PARAM_TS = 0xa,
    GSAPI_PARAM_ACCURACY = 0xb,
    GSAPI_PARAM_REJECTION = 0xc,
    GSAPI_PARAM_GARBAGE = 0xd,
    GSAPI_PARAM_SECONDPASS_ENABLE = 0xe,
    GSAPI_PARAM_ABSOLUTE_THRESHOLD = 0xf,
    GSAPI_PARAM_USETREEDP = 0x10,
    GSAPI_PARAM_EXTRAEVENT_ENABLE = 0x11,
    GSAPI_PARAM_LAST = 0x12,
    GSAPI_PARAM_FORCE_DWORD = 0x7ffffff,
};

/* Parameter block used for context overrides and shared defaults. */
struct GSAPIParamBlock {
    unsigned long u32Type; /* GCD block type for serialized parameters. */
    unsigned long u32Size; /* Serialized block size. Zero marks a heap-owned context block. */
    unsigned long u32UsedParams; /* Bit mask of parameter IDs with values. */
    unsigned long Params[GSAPI_PARAM_LAST]; /* Values indexed by GSAPI parameter ID. */
};

struct _GSAPI_ACTIONSTRUCT;

struct _GSAPI_CONTEXT {
    void *pCtx; /* Recognition context data passed to the SPI. */
    void *pGcd; /* GCD resource containing parameters and action mappings. */
    void *pWrd; /* WRD resource containing the word strings. */
    struct GSAPIParamBlock *pParams; /* Context parameter overrides. */
    struct _GSAPI_ACTIONSTRUCT *pEvents; /* Word-to-action mapping from the GCD resource. */
    unsigned long u32MaxWords; /* The game's code never reads this field. */
    unsigned long u32MaxUserWords; /* The game's code never reads this field. */
    struct tag_WRD_WORDLIST *pWordList; /* In-memory index of the WRD strings. */
    unsigned long nActiveWords; /* The game's code never reads this field. */
    unsigned long *pWordIds; /* Word IDs supplied to the SPI for word activation. */
};

/* Audio-processing state maintained by GSAPI. */
enum _GSAPI_STATE {
    GSAPI_STATE_IDLE = 0x0, /* Audio processing is idle. */
    GSAPI_STATE_RECOGNIZING = 0x1, /* Audio is being processed for recognition. */
};

struct _GSAPI_ENGINE {
    void *hAsrSpiEngine; /* Recognition engine handle returned by the SPI. */
    unsigned long Mode; /* Flags controlling recognition and audio notifications. */
    struct _GSAPI_CONTEXT TempContext; /* Engine-owned context and parameter overrides. */
    struct _GSAPI_CONTEXT *pCurrentContext; /* Context used to resolve results and actions. */
    struct _GSAPI_RESULT *pResult; /* Temporary result list delivered to the client. */
    enum _GSAPI_STATE State; /* Current GSAPI audio-processing state. */
    enum _ASRSPI_STATE asrspiState; /* Latest state returned by the recognizer. */
    void *hActiveContext; /* Handle used to find engines sharing an active context. */
    unsigned long u32EngineUserData; /* Client value passed to engine callbacks. */
    unsigned long u32EnergyInterval; /* Interval for reporting signal energy. */
    unsigned int hSid; /* Handle for the selected audio input. */
    int bClosing; /* Set when engine shutdown begins. */
    int bInCallback; /* Set while a recognizer callback is running. */
    int bDelayedRestart; /* A restart is pending until audio processing can perform it. */
    int bManualStop; /* Client stop request used to suppress an automatic restart. */
    int DoSessionDataSave; /* Preserve the session across context activation. */
};

struct _GSAPI_GLOBAL {
    void *hHeap; /* Heap used for SDK allocations. */
    unsigned long u32HeapUserData; /* Client-supplied value passed to heap_Open. */
    SafeHandle hRsrc; /* Shared resource handle used to check initialization. */
    long lhError; /* Most recent saved API status. */
    unsigned long u32UserData; /* Client value registered with the result callback. */
    GSResultCallback cbResult; /* Receives successful recognition results. */
    GSNotifyCallback cbNotify; /* Receives status and audio notifications. */
    GSGetSamplesCallback cbGetWaveData; /* Supplies external audio when registered. */
    void *pLang; /* Language payload passed to the recognizer. */
    void *pHDECLang; /* Base address of a language buffer with an HDEC header. */
    struct GSAPIParamBlock DefaultParams; /* Default parameter values applied to engines. */
    unsigned long bDetailedResults; /* Enables the result detail records. */
    struct _GSAPI_ENGINE *Recognizers; /* Allocated recognizer slots. */
    unsigned short u16NumRecognizers; /* Number of open recognizers. */
    unsigned short u16MaxRecognizers; /* Capacity of the recognizer array. */
    long NumContextsLoaded; /* Number of loaded contexts. */
    long NumSessionDataBuffersLoaded; /* Number of allocated session buffers. */
};

extern struct _GSAPI_GLOBAL gGSAPI;

/* ASRSPI recognition and signal callback registrations. */
extern struct _ASRBASE_SIGNALCALLBACKS AsrSpiSignalCallBacks;
extern struct _ASRSPI_RECOGCALLBACKS AsrSpiRecogCallBacks;

/* Delivers the measured signal energy to the client. */
long asrspi_cbEnergyLevel(int Level, unsigned long u32UserData);

/* Delivers the measured signal-to-noise ratio to the client. */
long asrspi_cbSNRLevel(int Level, unsigned long u32UserData);

/* Starts audio input and notifies the client when listening begins. */
long StartListening(struct _GSAPI_ENGINE *pEngine);

/* Stops audio input and notifies the client when listening ends. */
long StopListening(struct _GSAPI_ENGINE *pEngine);

/* Activates a recognition context and applies its settings. */
long ContextActivate(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext);

/* Clears the cached GSAPI context pointers. */
long ContextAPIDeActivate(struct _GSAPI_ENGINE *pEngine);

/* Deactivates the SPI context and clears the cached GSAPI context pointers. */
long ContextDeActivate(struct _GSAPI_ENGINE *pEngine);

/* Stores the recognition context data. */
long ContextSetCtxData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32CtxData);

/* Records an explicit parameter value for a context. */
long ContextSetParam(struct _GSAPI_CONTEXT *pContext, enum _GSAPI_PARAMID ParamId,
                     unsigned long Value);

/* Sends the context's explicit parameters to the recognizer. */
long ContextActivateParams(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext);

/* Loads the parameter and action blocks from a GCD resource. */
long ContextSetGcdData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32GcdData);

/* Sets the WRD resource and rebuilds its word string index. */
long ContextSetWrdData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32WrdData);

/* Activates the words in the context's word ID list. */
long ContextSetActiveWords(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext);

/* Releases context-owned data and clears the stored resource pointers. */
long ContextUnLoad(struct _GSAPI_CONTEXT *pContext);

/* Reads an explicitly set context parameter. */
long ContextGetParam(struct _GSAPI_CONTEXT *pContext, enum _GSAPI_PARAMID ParamId,
                     unsigned long *pValue);

/* Resolves a recognition result to the action stored in the context. */
long ContextGetAction(struct _GSAPI_CONTEXT *pContext, struct _ASRBASE_RESULT *pResult,
                      unsigned long *pAction);

/* Restarts recognition or marks it for a deferred restart. */
long RestartEngine(struct _GSAPI_ENGINE *pEngine);

/* Copies the recognizer's session state into a new buffer. */
long SessionDataExport(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA **ppSessionData);

/* Restores a saved session and optionally frees its buffer. */
long SessionDataImport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA *pSessionData, int bFreeMemory);

/* Releases a saved session buffer. */
long SessionDataFree(struct _GSAPI_SESSIONDATA *pSessionData);

/* Applies the shared defaults to an engine. */
long ActivateDefaultParams(struct _GSAPI_ENGINE *pEngine);

/* Consumes samples from the hardware audio buffer. */
long SidAdvanceBuffer(unsigned int hSid, long nSamples);

/* Closes hardware audio and releases its state. */
long SidClose(void *hHeap, unsigned int hSid);

/* Gets available hardware samples while recording. */
long SidGetSamples(unsigned int hSid, short **pWaveBuffer, long nSamples);

/* Returns the remaining hardware sample count while recording. */
long SidGetSamplesLeft(unsigned int hSid);

/* Opens hardware audio and returns its input handle. */
long SidInit(void *hHeap, long s32DevNum, unsigned int *phSid);

/* Returns the hardware recording flag. */
int SidIsRecording(unsigned int hSid);

/* Starts hardware recording. */
long SidStart(unsigned int hSid);

/* Stops hardware recording. */
long SidStop(unsigned int hSid);

/* Allocates state for external audio input. */
long ExtAudioInit(void *hHeap, unsigned int *phSid);

/* Releases the external audio input state. */
long ExtAudioClose(void *hHeap, unsigned int hSid);

/* Sets the external audio recording flag. */
long ExtAudioStart(unsigned int hSid);

/* Clears the external audio recording flag. */
long ExtAudioStop(unsigned int hSid);

/* Requests audio samples from the client and returns the reported count. */
long ExtAudioGetSamples(void *hEngine, short **Buffer, unsigned long nSamples);

/* Returns the external sample count remaining, which is always zero in MP6. */
long ExtAudioGetSamplesLeft(void *hEngine);

enum _ASRBASE_PARAMID TranslateGsapiParamId2AsrSpiParamId(enum _GSAPI_PARAMID parameter);

#endif
