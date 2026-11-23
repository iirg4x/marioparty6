/* Public engines, contexts, and callbacks for the game speech API. */
#ifndef GSSDK_GSAPI_H
#define GSSDK_GSAPI_H

#include "types.h"

/* Events delivered through the notification callback. */
enum _GSAPI_NOTIFY_TYPE {
    GSAPI_NOTIFY_ASRRESULT = 0x0, /* A recognition result was rejected. */
    GSAPI_NOTIFY_WAVEDATA = 0x1, /* Audio samples are available. */
    GSAPI_NOTIFY_STARTLISTENING = 0x2, /* Audio acquisition is starting. */
    GSAPI_NOTIFY_STOPLISTENING = 0x3, /* Audio acquisition is stopping. */
    GSAPI_NOTIFY_ABNORMCOND = 0x4, /* Recognizer abnormal condition. */
    GSAPI_NOTIFY_ENERGYLEVEL = 0x5, /* Measured signal energy. */
    GSAPI_NOTIFY_SNRLEVEL = 0x6, /* Measured signal-to-noise ratio. */
    GSAPI_NOTIFY_SPEECHDETECTED = 0x7, /* The recognizer detected speech. */
    GSAPI_NOTIFY_TSDETECTED = 0x8, /* The recognizer detected terminal silence. */
    GSAPI_NOTIFY_FORCE_DWORD = 0x7ffffff, /* Keeps the enum at 32 bits. */
};

/* Receives an engine event and two event-specific values. */
typedef void (*GSNotifyCallback)(void *context, enum _GSAPI_NOTIFY_TYPE event,
                                 unsigned long value, unsigned long auxiliary);

/* Supplies the external sample buffer and updates the sample count. */
typedef long (*GSGetSamplesCallback)(void *source, short **samples, unsigned long *sampleCount);

/* Outcome reported with a recognition result. */
enum _GSAPI_RESULTTYPE {
    GSAPI_RESULTTYPE_FINAL = 0x0,
    GSAPI_RESULTTYPE_PREMATURE = 0x1,
    GSAPI_RESULTTYPE_TIMEOUT = 0x2,
    GSAPI_RESULTTYPE_REJECTED = 0x3,
    GSAPI_RESULTTYPE_FORCE_DWORD = 0x7ffffff,
};

/* Beginning and end times supplied by the recognizer. */
struct _GSAPI_TIMEINFO {
    unsigned long TimeBegin; /* Start of the word or candidate. */
    unsigned long TimeEnd; /* End of the word or candidate. */
};

struct _GSAPI_DETAILEDWORD {
    unsigned short u16Conf; /* Confidence of the recognized word. */
    struct _GSAPI_TIMEINFO TimeInfo; /* Timing for this word. */
    unsigned long WordId; /* Word ID in the active context. */
};

struct _GSAPI_DETAILEDRESULT {
    struct _GSAPI_TIMEINFO TimeInfo; /* Timing for the entire candidate. */
    unsigned short u16NbrWords; /* Number of records in the word detail array. */
    struct _GSAPI_DETAILEDWORD *pWords; /* Detail records in candidate word order. */
};

/* Candidate list and optional timing data passed to the GSAPI client. */
struct _GSAPI_RESULT {
    enum _GSAPI_RESULTTYPE Type; /* Recognition outcome. */
    unsigned short u16NbrWords; /* Word count, which includes the engine's boundary entries. */
    unsigned short u16Conf; /* Confidence of this candidate. */
    unsigned long *pWordIds; /* Word IDs in candidate order. */
    struct _GSAPI_RESULT *pNextBest; /* Next candidate in rank order. */
    struct _GSAPI_DETAILEDRESULT *pDetails; /* Optional candidate and word timing details. */
};

/* Receives the recognized action, ranked candidates, and engine client data. */
typedef void (*GSResultCallback)(void *context, unsigned long action,
                                 struct _GSAPI_RESULT *result, unsigned long userData);

struct _GSAPI_ENGINE;
struct _GSAPI_SESSIONDATA;

/* Closes the recognizers and releases the shared resource and heap. */
long gsapi_Close(void);

/* Releases the temporary context and activates the supplied context. */
long gsapi_ContextActivate(void *hEngine, void *context);

/* Allocates a context and sets its recognition data. */
long gsapi_ContextSetCtxData(void *data, void **contextOut);

/* Loads parameter overrides and word actions into a context. */
long gsapi_ContextSetGcdData(void *context, void *data);

/* Records a context parameter and applies it to engines using the context. */
long gsapi_ContextSetParam(void *context, long parameter, long value);

/* Loads the word resource and its string index into a context. */
long gsapi_ContextSetWrdData(void *context, void *data);

/* Deactivates and frees a loaded recognition context. */
long gsapi_ContextDeActivate(void *context);

/* Stops and closes a recognizer and releases its audio and context state. */
long gsapi_EngineClose(struct _GSAPI_ENGINE *pEngine);

/* Opens a recognizer for the selected audio device and activates the language. */
long gsapi_EngineOpen(long s32DevNum, struct _GSAPI_ENGINE **phEngine);

/* Sets the recognition mode after checking the client flags. */
long gsapi_EngineSetMode(void *hEngine, unsigned long Mode);

/* Sets a parameter on the engine's temporary context and applies it. */
long gsapi_EngineSetParam(void *hEngine, long ParamId, long ParamValue);

/* Installs the recognition callbacks and begins listening. */
long gsapi_EngineStart(void *hEngine);

/* Stops listening and returns the recognizer to the idle state. */
long gsapi_EngineStop(void *hEngine);

/* Processes pending restarts and available audio for every recognizer.
 * The game passes zero to this legacy entry point, which takes no arguments. */
void gsapi_EngineRestart();

/* Returns the string for a word ID in the engine's current context. */
long gsapi_EngineGetParam(void *hEngine, unsigned long WordId, char **pszResult);

/* Initializes the SDK and registers the recognition result callback. */
long gsapi_Init(GSResultCallback cbResult, unsigned long u32UserData);

/* Loads the language payload and sets the default parameters. */
long gsapi_LanguageLoadBuffer(const unsigned long *pu32LngData, const unsigned long *pu32GcdData);

/* Clears the currently loaded language pointers. */
long gsapi_LanguageUnLoad(void);

/* Copies the engine's session state into a new buffer. */
long gsapi_EngineSessionDataExport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA **ppSessionData);

/* Restores a saved session to an engine and optionally frees it. */
long gsapi_EngineSessionDataImport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA *pSessionData, int bFreeMemory);

/* Releases a saved session buffer. */
long gsapi_EngineSessionDataFree(struct _GSAPI_SESSIONDATA *pSessionData);

/* Sets the client callback for notifications. */
long gsapi_NotifySetCallback(GSNotifyCallback cbNotify);

/* Stores the client heap value before the SDK is initialized. */
long gsapi_SetUserData(long heapSize);

#endif
