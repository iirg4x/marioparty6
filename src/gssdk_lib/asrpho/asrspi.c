/* Speech recognition SPI engine lifecycle and callback handling. */
#include "types.h"

#include <string.h>

enum _CASRSTATE { /* size 0x4, DIE 0xe75e3 */
    CASR_BOOT = 0x0,
    CASR_DATAREADY = 0x1,
    CASR_IDLE = 0x2,
    CASR_SLEEP = 0x3,
    CASR_RUN = 0x4,
    CASR_RECOVER = 0x5,
    CASR_SECONDPASS = 0x6,
    CASR_SPELLSLEEP = 0x7,
    CASR_SPELLRUN = 0x8,
    CASR_SPELLRECOVER = 0x9,
    CASR_SPELLSECONDPASS = 0xa,
    CASR_ISOWORDSLEEP = 0xb,
    CASR_ISOWORDRUN = 0xc,
    CASR_ISOWORDRECOVER = 0xd,
    CASR_ISOWORDSECONDPASS = 0xe,
    CASR_PROMPT = 0xf,
    CASR_RECORDSLEEP = 0x10,
    CASR_RECORD = 0x11,
    CASR_ACCEPT = 0x12,
    CASR_CONFIRM = 0x13,
    CASR_UWTRAINED = 0x14,
};
enum _PROMPTTYPE { /* size 0x4, DIE 0xe1650 */
    PROMPTTYPE_START = 0x0,
    PROMPTTYPE_ACCEPT = 0x1,
    PROMPTTYPE_CONFIRM = 0x2,
    PROMPTTYPE_TRAINEND = 0x3,
    PROMPTTYPE_NOPROMPT = 0x4,
};
enum _ABNORMCOND { /* size 0x4, DIE 0xe0aa0 */
    BADSNR = 0x0,
    OVERLOAD = 0x1,
    TOOQUIET = 0x2,
    NOSIGNAL = 0x3,
    GARBLEDSOUND = 0x4,
    POORMIC = 0x5,
};
enum _ASRBASE_ABNORMCOND { /* size 0x4, DIE 0xe10c6 */
    ASRBASE_BADSNR = 0x0,
    ASRBASE_OVERLOAD = 0x1,
    ASRBASE_TOOQUIET = 0x2,
    ASRBASE_NOSIGNAL = 0x3,
    ASRBASE_GARBLEDSOUND = 0x4,
    ASRBASE_POORMIC = 0x5,
};
enum _CASRRESULTTYPE { /* size 0x4, DIE 0xe0978 */
    CASRRESULTTYPE_FINAL = 0x0,
    CASRRESULTTYPE_PREMATURE = 0x1,
    CASRRESULTTYPE_TIMEOUT = 0x2,
    CASRRESULTTYPE_DTMF = 0x3,
    CASRRESULTTYPE_REJECTED = 0x4,
};
struct _USERDATA { /* size 0x8, DIE 0xe104d */
    unsigned long dwSPIUserData; /* offset 0x0, DIE 0xe106b */
    unsigned long dwEngineUserData; /* offset 0x4, DIE 0xe1095 */
};
struct _CASRRESULT { /* size 0xc, DIE 0xe1d41 */
    int iNbr; /* offset 0x0, DIE 0xe1d61 */
    struct _SENTENCE *pSentences; /* offset 0x4, DIE 0xe1d82 */
    enum _CASRRESULTTYPE Type; /* offset 0x8, DIE 0xe1dae */
};
struct _CRECOGCALLBACKS { /* size 0x20, DIE 0xe2237 */
    long (*cbTSDetected)(unsigned long, struct _USERDATA *); /* offset 0x0, DIE 0xe225c */
    long (*cbStartDetected)(unsigned long, struct _USERDATA *); /* offset 0x4, DIE 0xe228a */
    long (*cbResult)(struct _CASRRESULT *, struct _USERDATA *); /* offset 0x8, DIE 0xe22bb */
    long (*cbState)(enum _CASRSTATE, unsigned long, struct _USERDATA *);
    /* offset 0xc, DIE 0xe22e5 */
    long (*cbTrain)(enum _PROMPTTYPE, struct _USERDATA *); /* offset 0x10, DIE 0xe230e */
    long (*cbAbnorm)(enum _ABNORMCOND, unsigned long, struct _USERDATA *);
    /* offset 0x14, DIE 0xe2337 */
    long (*cbAgc)(unsigned short, struct _USERDATA *); /* offset 0x18, DIE 0xe2361 */
    long (*cbAskCurrentGain)(struct _USERDATA *); /* offset 0x1c, DIE 0xe2388 */
};

typedef struct ASRSpiResource ASRSpiResource;
struct RSRC_S;

extern s32 casrGainSet(void *engine, u16 gain);
extern s32 casrAcknowledge(void *engine, s32 event);
extern s32 casrGetLastError(void *engine);
extern s32 casrGetCallbackErr(void *engine, s32 *error);
extern s32 casrGetState(void *engine, s32 *state);
extern s32 ConvParamToSpi(u32 parameter, s32 *spiParameter);
extern s32 casrAcquisition(void *engine, s32 mode, void *count, u32 options);
extern s32 casrGetSNR(void *engine, s16 *snr);
extern s32 casrGetSignalLevel(void *engine, s16 *level);
extern s32 casrExportData(void *engine, s32 type, void **data, u32 *size);
extern s32 casrStop(void *engine);
extern s32 casrStart(void *engine);
extern s32 critsec_Enter(u32 lock);
extern s32 critsec_Leave(u32 lock);
s32 ConvErr(void *session, unsigned long error);
extern long InitRsrcFunction(ASRSpiResource hRsrc, const struct RSRC_S **ppRsrc);
extern void *heap_Calloc(void *heap, s32 count, s32 size);
extern void *heap_Alloc(void *heap, s32 size);
extern void heap_Free(void *heap, void *memory);
extern void *casrOpen(void *engine, const struct _CRECOGCALLBACKS *callbacks, void *context);
extern s32 casrClose(void *handle);
extern s32 casrCloseContext(void *context);
extern s32 casrCloseData(void *data);
extern s32 casrActivateData(void *handle, void *data);
extern void *casrImportData(void *heap, u32 *data);
extern s32 casrActivateContext(void *handle, void *context, s32 mode);
extern void *casrImportContext(void *heap, void *context);
extern s32 casrGetActiveWords(void *handle, u32 **words, u32 *count);
extern s32 casrSetActiveWords(void *handle, u32 *words, s32 count);
extern void FillContextV2VirtualTable(void *context);
extern const struct _CRECOGCALLBACKS g_APIRecogCallBacks;
const long g_StateError[6] = {
    0x8004000B, 0x8004000C, 0x8004000D, 0x8004000E, 0x8004000F, 0x80040010
};

typedef s32 (*ASRSpiOneArgCallback)(void *context);
typedef long (*ASRSpiContextCallback)(unsigned long, unsigned long);
typedef s32 (*ASRSpiTwoArgCallback)(s32 value, void *context);
typedef long (*ASRSpiThreeArgCallback)(enum _ASRBASE_ABNORMCOND, unsigned long, unsigned long);
typedef s32 (*ASRSpiTrainCallback)(s32 first, s32 second, void *context);

enum _ASRBASE_RESULTTYPE {
    ASRBASE_RESULTTYPE_FINAL = 0x0,
    ASRBASE_RESULTTYPE_PREMATURE = 0x1,
    ASRBASE_RESULTTYPE_TIMEOUT = 0x2,
    ASRBASE_RESULTTYPE_REJECTED = 0x3,
};
enum _ASRBASE_RECORDINGTYPE {
    ASRBASE_RECORDINGTYPE_ACCEPTED = 0x0,
    ASRBASE_RECORDINGTYPE_REJECTED_BY_USER = 0x1,
    ASRBASE_RECORDINGTYPE_TOO_SHORT = 0x2,
    ASRBASE_RECORDINGTYPE_INCONSISTENT = 0x3,
};
enum _ASRSPI_STATE {
    ASRBASE_BOOT=0,
    ASRBASE_LANGREADY=1,
    ASRBASE_IDLE=2,
    ASRBASE_RECOGNIZING=3,
    ASRBASE_RECORDING=4,
    ASRBASE_ACCEPT=5,
    ASRBASE_USWTRAINED=6,
};
enum _ASRBASE_SAMPLEFORMAT { /* size 0x4, DIE 0xe0e6a */
    ASRBASE_PCM_16_11KHZ = 0x0,
    ASRBASE_PCM_16_8KHZ = 0x1,
    ASRBASE_UTTFORMAT_CEPSTRA_11KHZ = 0x2,
    ASRBASE_UTTFORMAT_CEPSTRA_8KHZ = 0x3,
    ASRBASE_UTTFORMAT_LABELS_11KHZ = 0x4,
    ASRBASE_UTTFORMAT_LABELS_8KHZ = 0x5,
    ASRBASE_UTTFORMAT_UNKNOWN = 0x6,
};
struct _ASRBASE_RESULT {
    enum _ASRBASE_RESULTTYPE Type;
    int iNbr;
    struct _ASRBASE_SENTENCE *pSentences;
};
struct _ASRSPI_CONTEXT {
    void *pCtx;
    int (*contextCheckDataType)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetSize)(struct _ASRSPI_CONTEXT *);
    char *(*contextGetLanguageVersionInfo)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetTS)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetAccuracy)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetRejection)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetGarbage)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrItem)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrPron)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrFollower)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrStartWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrStopWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetGarbageWordID)(struct _ASRSPI_CONTEXT *);
    unsigned char (*contextIsSpellingContext)(struct _ASRSPI_CONTEXT *);
    unsigned short *(*contextGetpLex)(struct _ASRSPI_CONTEXT *);
    unsigned long *(*contextGetpBeginOfWords)(struct _ASRSPI_CONTEXT *);
    unsigned short *(*contextGetpWordProp)(struct _ASRSPI_CONTEXT *);
    unsigned long *(*contextGetpSyntax)(struct _ASRSPI_CONTEXT *);
    unsigned char (*contextIsIsoWordContext)(struct _ASRSPI_CONTEXT *);
};
struct _ASRBASE_SIGNALCALLBACKS {
    long (*cbAbnorm)(enum _ASRBASE_ABNORMCOND, unsigned long, unsigned long);
    long (*cbSpeechDetected)(unsigned long, unsigned long);
    long (*cbTsDetected)(unsigned long, unsigned long);
    long (*cbAgc)(unsigned short, unsigned long);
    long (*cbEnergyLevel)(int, unsigned long);
    long (*cbSNRLevel)(int, unsigned long);
};

typedef struct ASRSpiSession {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiSession;

typedef struct ASRSpiCallback {
    unsigned long dwSPIUserData;
    unsigned long dwEngineUserData;
} ASRSpiCallback;

struct ASRSpiResource {
    void *pHandleData;
    unsigned long u32Check;
};

typedef struct RSRC_S {
    unsigned short u16HandleValidityCheck;
    void *hHeap;
    void *hCSClass;
    void *hBrk;
    void *hDbg;
} ASRSpiResourceInfo;

typedef struct ASRCTXHEADERtag {
    unsigned long TotalSizeInBytes;
    unsigned long OffsetToSpiBuffer;
    unsigned long TS;
    unsigned long Accuracy;
    unsigned long Rejection;
    unsigned long Garbage;
} ASRSpiContextHeader;

typedef struct ASRSpiEngine {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngine;

typedef struct ASRSpiEngineCloseData {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineCloseData;

typedef struct ASRSpiEngineCloseView {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineCloseView;

typedef struct ASRSpiEngineAcquisitionData {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineAcquisitionData;

typedef struct ASRSpiStartPayload {
    long (*cbAbnorm)(enum _ASRBASE_ABNORMCOND, unsigned long, unsigned long);
    long (*cbSpeechDetected)(unsigned long, unsigned long);
    long (*cbTsDetected)(unsigned long, unsigned long);
    long (*cbAgc)(unsigned short, unsigned long);
    long (*cbEnergyLevel)(int, unsigned long);
    long (*cbSNRLevel)(int, unsigned long);
} ASRSpiStartPayload;

typedef struct ASRSpiStartParameters {
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    ASRSpiStartPayload *pSignalCallbacks;
} ASRSpiStartParameters;

typedef struct ASRSpiEngineStartData {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    ASRSpiStartPayload SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineStartData;

typedef struct ASRSpiEngineActivationData {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineActivationData;

typedef struct ASRSpiEngineWordsData {
    int HandleValidityCheck;
    struct HCASR__ *hSpi;
    struct ASRCTXHEADERtag *pAsrSpiCtx;
    struct HCTX__ *hSpiCtx;
    struct HCDATA__ *hSpiLng;
    unsigned long dwSigCBUserData;
    unsigned long dwFeatCBUserData;
    int Finished;
    int AllUttRecorded;
    int CBTrain_StartOrConfirmAcknowledged;
    void *pSpiUserWordBuf;
    unsigned long SpiUserWordBufSize;
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType;
    enum _ASRSPI_STATE State;
    int Busy;
    unsigned int CBSignalLevelIntervalInMSEC;
    unsigned int LoopingIntervalSize;
    struct _ASRBASE_RESULT Result;
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks;
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain;
    unsigned int AbortingUsw;
    unsigned long WasStartEnable;
    enum _ASRBASE_SAMPLEFORMAT SampleFormat;
    struct _ASRSPI_CONTEXT Context;
    void *hHeap;
    struct CRITSEC_TAG *hCritSec;
} ASRSpiEngineWordsData;

s32 ConvErr(void *pAsrSpi, unsigned long SpiErr)
{
    s32 result;

    switch (SpiErr) {
    case 1: result = 0x8004001A; break;
    case 4: result = 0x8004001D; break;
    case 5: result = 0x8004001E; break;
    case 6: result = 0x8004001F; break;
    case 7: result = g_StateError[((ASRSpiSession *)pAsrSpi)->State]; break;
    case 8: result = 0x80040021; break;
    case 9: result = 0x80040022; break;
    case 10: result = 0x80040014; break;
    case 11: result = 0x80040001; break;
    case 12: result = 0x8004000A; break;
    case 13: result = 0x80040024; break;
    case 15: result = 0x80040026; break;
    case 17: result = 0x80040028; break;
    case 19: result = 0x80040015; break;
    case 20: result = 0x80040029; break;
    case 21: result = 0x8004002A; break;
    case 18: result = 0x8004002B; break;
    case 22: result = 0x8004002C; break;
    case 25: result = 0x80040018; break;
    case 27:
        casrGetCallbackErr(((ASRSpiSession *)pAsrSpi)->hSpi, &result);
        break;
    case 28: result = 0x8004002D; break;
    case 29: result = 0x80040008; break;
    case 30: result = 0x80040009; break;
    case 31: result = 0x8004000A; break;
    case 32: result = 0x80040004; break;
    case 33: result = 0x80040013; break;
    case 34: result = 0x80040034; break;
    case 39: result = 0x8004003A; break;
    case 35: result = 0x00040035; break;
    case 36: result = 0x00040036; break;
    case 37: result = 0x80040038; break;
    case 38: result = 0x80040039; break;
    default: result = 0x8004002E; break;
    }
    return result;
}

s32 ConvParamToSpi(u32 Param, s32 *pSpiParam)
{
    switch (Param) {
    case 0: *pSpiParam = 1; break;
    case 1: *pSpiParam = 2; break;
    case 2: *pSpiParam = 3; break;
    case 3: *pSpiParam = 5; break;
    case 17: *pSpiParam = 9; break;
    case 6: *pSpiParam = 10; break;
    case 11: *pSpiParam = 17; break;
    case 7: *pSpiParam = 13; break;
    case 8: *pSpiParam = 12; break;
    case 9: *pSpiParam = 11; break;
    case 4: *pSpiParam = 15; break;
    case 18: *pSpiParam = 18; break;
    case 10: *pSpiParam = 14; break;
    case 12: *pSpiParam = 16; break;
    case 15: *pSpiParam = 7; break;
    case 16: *pSpiParam = 8; break;
    case 14: *pSpiParam = 6; break;
    case 13: *pSpiParam = 4; break;
    default: return 0x80040014;
    }
    return 0;
}

long ASRSPICBABNORM(
    enum _ABNORMCOND AbnormalCondition, unsigned long Time, struct _USERDATA *pUserData)
{
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    switch (AbnormalCondition) {
    case 0:
        AbnormalCondition = 0;
        break;
    case 1:
        AbnormalCondition = 1;
        break;
    case 2:
        AbnormalCondition = 2;
        break;
    case 3:
        AbnormalCondition = 3;
        break;
    case 4:
        AbnormalCondition = 4;
        break;
    case 5:
        AbnormalCondition = 5;
        break;
    }
    if (session->SigCallBacks.cbAbnorm != NULL) {
        return session->SigCallBacks.cbAbnorm(AbnormalCondition, Time, session->dwSigCBUserData);
    }
    return 0;
}

long ASRSPICBAGC(unsigned short NewGain, struct _USERDATA *pUserData)
{
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    if (session->SigCallBacks.cbAgc != NULL) {
        return session->SigCallBacks.cbAgc(NewGain, session->dwSigCBUserData);
    }
    return 0;
}

long ASRSPICBASKCURRENTGAIN(struct _USERDATA *pUserData)
{
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    return casrGainSet(session->hSpi, session->InitialGain);
}

long ASRSPICBTRAIN(enum _PROMPTTYPE Type, struct _USERDATA *pUserData)
{
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    s32 result = 0;

    switch (Type) {
    case 0:
        result = casrAcknowledge(session->hSpi, 1);
        session->CBTrain_StartOrConfirmAcknowledged = 1;
        break;
    case 1:
        if (session->AbortingUsw != 0) {
            result = casrAcknowledge(session->hSpi, 3);
        } else {
            session->Finished = 1;
        }
        break;
    case 2:
        result = casrAcknowledge(session->hSpi, 1);
        if (result != 0) {
            result = casrGetLastError(session->hSpi);
        }
        if ((u32)result == 0x1B) {
            casrGetCallbackErr(session->hSpi, &result);
        } else if (result != 0) {
            result = ConvErr(session, result);
        }
        session->CBTrain_StartOrConfirmAcknowledged = 1;
        break;
    case 3:
        session->AllUttRecorded = 1;
        if (session->cbUswResult != NULL && session->AbortingUsw == 0) {
            result = session->cbUswResult(session->pSpiUserWordBuf, session->SpiUserWordBufSize,
        session->dwSigCBUserData);
        }
        break;
    case 4:
        break;
    }
    return result;
}

long ASRSPICBTS(unsigned long Time, struct _USERDATA *pUserData)
{
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    s32 result = 0;
    if (session->SigCallBacks.cbTsDetected != NULL) {
        result = session->SigCallBacks.cbTsDetected(Time, session->dwSigCBUserData);
    }
    return result;
}

long ASRSPICBSTARTDET(unsigned long Time, struct _USERDATA *pUserData)
{
    s32 result = 0;
    ASRSpiSession *session = (ASRSpiSession *)pUserData->dwEngineUserData;
    if (session->SigCallBacks.cbSpeechDetected != NULL) {
        result = session->SigCallBacks.cbSpeechDetected(Time, session->dwSigCBUserData);
    }
    return result;
}

long ASRSPICBRESULT(struct _CASRRESULT *result, struct _USERDATA *callback)
{
    ASRSpiSession *session = (ASRSpiSession *)callback->dwEngineUserData;
    s32 mappedType;
    s32 error;
    session->Finished = 1;
    switch (result->Type) {
    case 0:
        mappedType = 0;
        goto validResult;
    case 1:
        mappedType = 1;
        goto validResult;
    case 2:
        mappedType = 2;
        goto validResult;
    case 4:
        mappedType = 3;
        goto validResult;
    default:
        error = 0x8004002E;
        break;
    validResult:
        error = 0;
        break;
    }
    if (error != 0) {
        return error;
    }
    session->Result.Type = mappedType;
    session->Result.iNbr = result->iNbr;
    session->Result.pSentences = (struct _ASRBASE_SENTENCE *)result->pSentences;
    if (session->cbResult != NULL) {
        return session->cbResult(&session->Result, session->dwSigCBUserData);
    }
    return 0;
}

static inline s32 ASRSpiIsError(u32 result)
{
    s32 isError;

    if (result >= 0x80000000U) {
        isError = 1;
    } else {
        isError = 0;
    }
    return isError;
}

s32 asrspi_OpenEngine(ASRSpiResource *hRsrc, s32 SampleFormat, ASRSpiEngine **phApiAsr)
{
    ASRSpiEngine *engine;
    void *handle;
    const ASRSpiResourceInfo *resourceInfo;
    long lhError;
    long Error;

    if (phApiAsr == NULL) {
        return 0x80040083;
    }
    lhError = InitRsrcFunction(*hRsrc, &resourceInfo);
    if ((lhError >= 0x80000000U ? 1 : 0)) {
        return lhError;
    }
    engine = heap_Calloc(resourceInfo->hHeap, 1, 0xDC);
    if (engine == NULL) {
        return 0x8004002F;
    }
    memset(engine, 0, 0xDC);
    engine->hHeap = resourceInfo->hHeap;
    if (SampleFormat != 0 && SampleFormat != 2 && SampleFormat != 4) {
        heap_Free(engine->hHeap, engine);
        return 0x80040004;
    }
    handle = casrOpen(resourceInfo->hHeap, &g_APIRecogCallBacks, engine);
    if (handle == NULL) {
        heap_Free(engine->hHeap, engine);
        return 0x80040086;
    }
    engine->hSpi = handle;
    engine->State = 0;
    engine->HandleValidityCheck = 0x2B67;
    engine->SampleFormat = SampleFormat;
    engine->uswLastRecordType = 0;
    FillContextV2VirtualTable(&engine->Context);
    engine->InitialGain = 0xBFFF;
    *phApiAsr = engine;
    return 0;
}

// The error for an engine state other than idle (0), closed data (1), stopped (6) or ready (2):
// the calls
// that reconfigure the engine (closing it, activating a language, setting parameters) check it
// first.
static inline s32 ASRSpiConfigStateError(s32 state)
{
    if (state != 0 && state != 1 && state != 6 && state != 2) {
        return g_StateError[state];
    }
    return 0;
}

// The error for an engine state other than closed data (1), stopped (6) or ready (2): the calls
// that need a
// language loaded (activating a context or session data) check it first.
static inline s32 ASRSpiLanguageStateError(s32 state)
{
    if (state != 1 && state != 6 && state != 2) {
        return g_StateError[state];
    }
    return 0;
}

s32 asrspi_CloseEngine(void *hAsrSpi)
{
    ASRSpiEngineCloseView *engine;
    s32 state;
    s32 stateError;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngineCloseView *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    state = engine->State;
    if ((stateError = ASRSpiConfigStateError(state)) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return stateError;
    }
    if (state == 6) {
        if (engine->pSpiUserWordBuf != NULL) {
            heap_Free(engine->hHeap, engine->pSpiUserWordBuf);
        }
    }
    engine->pSpiUserWordBuf = NULL;
    engine->SpiUserWordBufSize = 0;
    if (casrClose(engine->hSpi) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return ConvErr(engine, casrGetLastError(engine->hSpi));
    }
    if (engine->hSpiCtx != NULL && casrCloseContext(engine->hSpiCtx) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return 0x80040086;
    }
    if (engine->hSpiLng != NULL && casrCloseData(engine->hSpiLng) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return 0x80040086;
    }
    engine->HandleValidityCheck = 0;
    heap_Free(engine->hHeap, engine);
    return 0;
}

s32 asrspi_GetState(ASRSpiSession *hAsrSpi, s32 *pState)
{
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    if ((s32)hAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pState == NULL) {
        return 0x80040083;
    }
    *pState = hAsrSpi->State;
    return 0;
}

s32 asrspi_ActivateLng(void *hAsrSpi, u32 *pAsrSpiLng)
{
    ASRSpiEngineActivationData *engine;
    s32 state;
    void *importedData;
    s32 result;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngineActivationData *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    state = engine->State;
    if ((result = ASRSpiConfigStateError(state)) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (state == 6 && engine->pSpiUserWordBuf != NULL) {
        heap_Free(engine->hHeap, engine->pSpiUserWordBuf);
    }
    engine->pSpiUserWordBuf = NULL;
    engine->SpiUserWordBufSize = 0;
    if (pAsrSpiLng == NULL) {
        importedData = NULL;
    } else {
        if (pAsrSpiLng[0] != 0x1004 && pAsrSpiLng[0] != 0x1003) {
            if (engine->hCritSec != NULL) {
                critsec_Enter((u32)engine->hCritSec);
                engine->Busy = 0;
                critsec_Leave((u32)engine->hCritSec);
            }
            return 0x80040008;
        }
        importedData = casrImportData(engine->hHeap, pAsrSpiLng);
        if (importedData == NULL) {
            if (engine->hCritSec != NULL) {
                critsec_Enter((u32)engine->hCritSec);
                engine->Busy = 0;
                critsec_Leave((u32)engine->hCritSec);
            }
            return 0x80040086;
        }
    }
    if (casrActivateData(engine->hSpi, importedData) != 0) {
        result = ConvErr(engine, casrGetLastError(engine->hSpi));
        if ((u32)(result + 0x7FFC0000) == 0x0A) {
            result = 0x80040008;
        }
        casrCloseData(importedData);
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (engine->hSpiCtx != NULL && casrCloseContext(engine->hSpiCtx) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return 0x80040086;
    }
    engine->hSpiCtx = NULL;
    if (engine->hSpiLng != NULL && casrCloseData(engine->hSpiLng) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return 0x80040086;
    }
    engine->hSpiLng = importedData;
    if (pAsrSpiLng != NULL) {
        engine->State = 1;
    } else {
        engine->State = 0;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        engine->Busy = 0;
        critsec_Leave((u32)engine->hCritSec);
    }
    return 0;
}

typedef struct _DATA {
    unsigned long DataType;
    unsigned long Size;
    char VersionInfo[0x80];
} _DATA;

static inline long Check3States(
    ASRSpiEngineActivationData *pAsrSpi, enum _ASRSPI_STATE State1, enum _ASRSPI_STATE State2,
    enum _ASRSPI_STATE State3)
{
    if (pAsrSpi->State != State1 && pAsrSpi->State != State2 && pAsrSpi->State != State3) return
        g_StateError[pAsrSpi->State];
    return 0;
}

s32 asrspi_ActivateCtx(void *hAsrSpi, ASRSpiContextHeader *pAsrSpiCtx, int bSetOptimalParameters)
{
    _DATA *pSpiCtx = (_DATA *)pAsrSpiCtx;
    void *importedContext;
    ASRSpiContextHeader *pHeader = pAsrSpiCtx;
    ASRSpiEngineActivationData *engine;
    long Error;
    s32 stateError;
    s32 result;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngineActivationData *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    if ((stateError = Check3States(engine, ASRBASE_LANGREADY, ASRBASE_USWTRAINED,
    ASRBASE_IDLE)) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return stateError;
    }
    if (engine->State == 6 && engine->pSpiUserWordBuf != NULL) {
        heap_Free(engine->hHeap, engine->pSpiUserWordBuf);
    }
    engine->pSpiUserWordBuf = NULL;
    engine->SpiUserWordBufSize = 0;
    if (pSpiCtx != NULL) {
        if (pSpiCtx->Size == 0x18 && pSpiCtx->DataType != 2) {
            if (engine->hCritSec != NULL) {
                critsec_Enter((u32)engine->hCritSec);
                engine->Busy = 0;
                critsec_Leave((u32)engine->hCritSec);
            }
            return 0x80040026;
        }
        importedContext = casrImportContext(engine->hHeap, pSpiCtx);
        if (importedContext == NULL) {
            if (engine->hCritSec != NULL) {
                critsec_Enter((u32)engine->hCritSec);
                engine->Busy = 0;
                critsec_Leave((u32)engine->hCritSec);
            }
            return 0x80040009;
        }
    } else {
        importedContext = NULL;
    }
    if (casrActivateContext(engine->hSpi, importedContext, bSetOptimalParameters) != 0) {
        result = ConvErr(engine, casrGetLastError(engine->hSpi));
        casrCloseContext(importedContext);
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (engine->hSpiCtx != NULL) {
        casrCloseContext(engine->hSpiCtx);
    }
    engine->pAsrSpiCtx = pHeader;
    engine->hSpiCtx = importedContext;
    engine->Context.pCtx = pHeader;
    if (pSpiCtx != NULL) {
        engine->State = 2;
    } else {
        engine->State = 1;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        engine->Busy = 0;
        critsec_Leave((u32)engine->hCritSec);
    }
    return 0;
}

s32 asrspi_SetParamList(void *hAsrSpi, const s32 *pParamList, unsigned int Nbr)
{
    ASRSpiEngine *session;
    s32 state;
    u32 spiParameter;
    s32 result;
    u32 index;

    if (pParamList == NULL) {
        return 0x80040083;
    }
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    session = (ASRSpiEngine *)hAsrSpi;
    if ((s32)session->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (session->hCritSec != NULL) {
        critsec_Enter((u32)session->hCritSec);
        if (session->Busy != 0) {
            critsec_Leave((u32)session->hCritSec);
            return 0x80040003;
        }
        session->Busy = 1;
        critsec_Leave((u32)session->hCritSec);
    }
    state = session->State;
    if ((result = ASRSpiConfigStateError(state)) != 0) {
        if (session->hCritSec != NULL) {
            critsec_Enter((u32)session->hCritSec);
            session->Busy = 0;
            critsec_Leave((u32)session->hCritSec);
        }
        return result;
    }
    for (index = 0; index < Nbr; index++) {
        result = ConvParamToSpi((u32)pParamList[index * 2], (s32 *)&spiParameter);
        if (result != 0) {
            if (session->hCritSec != NULL) {
                critsec_Enter((u32)session->hCritSec);
                session->Busy = 0;
                critsec_Leave((u32)session->hCritSec);
            }
            return result;
        }
        if (spiParameter == 14 && session->State == 0) {
            return 0x80040013;
        }
    }
    for (index = 0; index < Nbr; index++) {
        ConvParamToSpi((u32)pParamList[index * 2], (s32 *)&spiParameter);
        if (casrSetParam(session->hSpi, spiParameter, pParamList[index * 2 + 1]) != 0) {
            if (session->hCritSec != NULL) {
                critsec_Enter((u32)session->hCritSec);
                session->Busy = 0;
                critsec_Leave((u32)session->hCritSec);
            }
            return ConvErr(session, casrGetLastError(session->hSpi));
        }
    }
    if (session->hCritSec != NULL) {
        critsec_Enter((u32)session->hCritSec);
        session->Busy = 0;
        critsec_Leave((u32)session->hCritSec);
    }
    return 0;
}

static inline unsigned long InList(
    unsigned long WordID, unsigned long *pActiveWords, unsigned long cWords)
{
    unsigned long i;
    for (i = 0; i < cWords; i++) {
        if (WordID == pActiveWords[i]) {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

long asrspi_ActivateWords(void *hAsrSpi, unsigned long *pWordIDs, unsigned int cNewWords)
{
    ASRSpiEngineWordsData *pAsrSpi;
    long Err;
    unsigned long cWords = 0;
    unsigned long cExtraWords = 0;
    unsigned long NrWordsInCtx;
    unsigned long i;
    unsigned long *pActiveWords;
    unsigned long *pNewActiveWords;

    pActiveWords = NULL;
    pNewActiveWords = NULL;

    if (pWordIDs == NULL && cNewWords != 0xFFFFFFFFU) {
        return 0x80040083;
    }
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (ASRSpiEngineWordsData *)hAsrSpi;
    if ((s32)pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter((u32)pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave((u32)pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave((u32)pAsrSpi->hCritSec);
    }
    if (pAsrSpi->State != 2) {
        Err = g_StateError[pAsrSpi->State];
    } else {
        Err = 0;
    }
    if (Err != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter((u32)pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave((u32)pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->Context.pCtx != NULL) {
        NrWordsInCtx = pAsrSpi->Context.contextGetNbrWord(&pAsrSpi->Context);
    } else {
        NrWordsInCtx = 0;
    }
    if (cNewWords == 0xFFFFFFFFU) {
        if (pAsrSpi->Context.pCtx != NULL) {
            cExtraWords = NrWordsInCtx;
            pNewActiveWords = heap_Alloc(pAsrSpi->hHeap, (s32)((cExtraWords + cWords) * 4));
            if (pNewActiveWords == NULL) {
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter((u32)pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave((u32)pAsrSpi->hCritSec);
                }
                return 0x8004002F;
            }
            for (i = 0; i < NrWordsInCtx; i++) {
                pNewActiveWords[i] = i;
            }
        }
    } else {
        if (casrGetActiveWords(pAsrSpi->hSpi, &pActiveWords, &cWords) != 0) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter((u32)pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave((u32)pAsrSpi->hCritSec);
            }
            return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
        }
        cExtraWords = 0;
        for (i = 0; i < cNewWords; i++) {
            if (pWordIDs[i] >= NrWordsInCtx) {
                heap_Free(pAsrSpi->hHeap, pActiveWords);
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter((u32)pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave((u32)pAsrSpi->hCritSec);
                }
                return 0x80040018;
            }
            if (InList(pWordIDs[i], pActiveWords, cWords) == 0xFFFFFFFF) {
                cExtraWords++;
            }
        }
        if (cExtraWords + cWords != 0) {
            pNewActiveWords = heap_Alloc(pAsrSpi->hHeap, (s32)((cExtraWords + cWords) * 4));
            if (pNewActiveWords == NULL) {
                heap_Free(pAsrSpi->hHeap, pActiveWords);
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter((u32)pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave((u32)pAsrSpi->hCritSec);
                }
                return 0x8004002F;
            }
        } else {
            pNewActiveWords = NULL;
        }
        for (i = 0; i < cWords; i++) {
            pNewActiveWords[i] = pActiveWords[i];
        }
        cExtraWords = 0;
        for (i = 0; i < cNewWords; i++) {
            if (InList(pWordIDs[i], pActiveWords, cWords) == 0xFFFFFFFF) {
                pNewActiveWords[cWords + cExtraWords] = pWordIDs[i];
                cExtraWords++;
            }
        }
    }
    if (casrSetActiveWords(pAsrSpi->hSpi, pNewActiveWords, (s32)(cWords + cExtraWords)) != 0) {
        heap_Free(pAsrSpi->hHeap, pNewActiveWords);
        if (pActiveWords != NULL) {
            heap_Free(pAsrSpi->hHeap, pActiveWords);
        }
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter((u32)pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave((u32)pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (pNewActiveWords != NULL) {
        heap_Free(pAsrSpi->hHeap, pNewActiveWords);
    }
    if (pActiveWords != NULL) {
        heap_Free(pAsrSpi->hHeap, pActiveWords);
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter((u32)pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave((u32)pAsrSpi->hCritSec);
    }
    return 0;
}

s32 asrspi_Start(
    void *hAsrSpi, unsigned int Interval, ASRSpiStartParameters *pCallBacks,
    unsigned long dwUserData)
{
    ASRSpiEngineStartData *session;
    s32 result;

    if (pCallBacks == NULL) {
        return 0x80040083;
    }
    if (Interval == 0 && pCallBacks->pSignalCallbacks != NULL &&
        (pCallBacks->pSignalCallbacks->cbEnergyLevel != NULL ||
        pCallBacks->pSignalCallbacks->cbSNRLevel != NULL)) {
        return 0x80040016;
    }
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    session = (ASRSpiEngineStartData *)hAsrSpi;
    if ((s32)session->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (session->hCritSec != NULL) {
        critsec_Enter((u32)session->hCritSec);
        if (session->Busy != 0) {
            critsec_Leave((u32)session->hCritSec);
            return 0x80040003;
        }
        session->Busy = 1;
        critsec_Leave((u32)session->hCritSec);
    }
    if ((result = session->State != 2 ? g_StateError[session->State] : 0) != 0) {
        if (session->hCritSec != NULL) {
            critsec_Enter((u32)session->hCritSec);
            session->Busy = 0;
            critsec_Leave((u32)session->hCritSec);
        }
        return result;
    }
    if (Interval != 0 && pCallBacks->pSignalCallbacks != NULL && session->SampleFormat != 0 &&
        session->SampleFormat != 1 &&
        (pCallBacks->pSignalCallbacks->cbEnergyLevel != NULL ||
        pCallBacks->pSignalCallbacks->cbSNRLevel != NULL)) {
        return 0x80040017;
    }
    session->Finished = 0;
    session->CBSignalLevelIntervalInMSEC = Interval;
    session->LoopingIntervalSize = 0;
    session->dwSigCBUserData = dwUserData;
    session->dwFeatCBUserData = dwUserData;
    if (pCallBacks->pSignalCallbacks != NULL) {
        memcpy(&session->SigCallBacks, pCallBacks->pSignalCallbacks, 0x18);
    } else {
        memset(&session->SigCallBacks, 0, 0x18);
    }
    session->cbResult = pCallBacks->cbResult;
    if (casrStart(session->hSpi) != 0) {
        if (session->hCritSec != NULL) {
            critsec_Enter((u32)session->hCritSec);
            session->Busy = 0;
            critsec_Leave((u32)session->hCritSec);
        }
        return ConvErr(session, casrGetLastError(session->hSpi));
    }
    session->State = 3;
    if (session->hCritSec != NULL) {
        critsec_Enter((u32)session->hCritSec);
        session->Busy = 0;
        critsec_Leave((u32)session->hCritSec);
    }
    return 0;
}

s32 asrspi_Stop(void *hAsrSpi)
{
    ASRSpiEngine *engine;
    s32 state;
    s32 result;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngine *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    if ((result = engine->State != 3 ? g_StateError[engine->State] : 0) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (casrStop(engine->hSpi) != 0) {
        casrGetState(engine->hSpi, &state);
        if (state == 2) {
            engine->State = 2;
        }
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return ConvErr(engine, casrGetLastError(engine->hSpi));
    }
    engine->State = 2;
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        engine->Busy = 0;
        critsec_Leave((u32)engine->hCritSec);
    }
    return 0;
}

enum _CSAMPLEFORMAT { /* size 0x4, DIE 0xe7a64 */
    PCM_16_11KHZ = 0x0,
    UTTFORMAT_CEPSTRA = 0x1,
    UTTFORMAT_LABELS = 0x2,
    UTTFORMAT_LABELSRAW = 0x3,
    UTTFORMAT_UNKNOWN = 0x4,
};

static inline long ConvFormatToSpi(
    enum _ASRBASE_SAMPLEFORMAT Format, enum _CSAMPLEFORMAT *pSpiFormat)
{
    switch (Format) {
    case ASRBASE_PCM_16_11KHZ: *pSpiFormat = PCM_16_11KHZ; break;
    case ASRBASE_UTTFORMAT_CEPSTRA_11KHZ: *pSpiFormat = UTTFORMAT_CEPSTRA; break;
    case ASRBASE_UTTFORMAT_LABELS_11KHZ: *pSpiFormat = UTTFORMAT_LABELS; break;
    case ASRBASE_UTTFORMAT_UNKNOWN: *pSpiFormat = UTTFORMAT_UNKNOWN; break;
    default: return 0x80040004;
    }
    return 0;
}

static long intAcquisition(
    ASRSpiEngine *pAsrSpi, enum _ASRBASE_SAMPLEFORMAT SampleFormat, void *pSampleBuf,
    unsigned long dwBufSize)
{
    ASRSpiEngineAcquisitionData *acquisition;
    enum _CSAMPLEFORMAT SpiFormat;
    long Err;
    unsigned int Size;
    unsigned int RestSize;
    unsigned int IntervalSize;
    char *pSampleBufHelp;
    signed short EnergyLevel;
    signed short SNRLevel;

    acquisition = (ASRSpiEngineAcquisitionData *)pAsrSpi;
    Err = ConvFormatToSpi(SampleFormat, &SpiFormat);
    if (Err != 0) {
        return Err;
    }
    if ((acquisition->SigCallBacks.cbEnergyLevel != NULL || acquisition->SigCallBacks.cbSNRLevel
        != NULL) && SampleFormat <= 1U) {
        RestSize = dwBufSize;
        pSampleBufHelp = pSampleBuf;
        if ((s32)SampleFormat == 0) {
            IntervalSize = acquisition->CBSignalLevelIntervalInMSEC * 0x16;
        } else {
            IntervalSize = acquisition->CBSignalLevelIntervalInMSEC * 0x10;
        }
        while (RestSize != 0 && acquisition->Finished == 0) {
            long Err;

            if (RestSize >= IntervalSize - acquisition->LoopingIntervalSize) {
                Size = IntervalSize - acquisition->LoopingIntervalSize;
                acquisition->LoopingIntervalSize = 0;
            } else {
                acquisition->LoopingIntervalSize += RestSize;
                Size = RestSize;
            }
            if (casrAcquisition(acquisition->hSpi, SpiFormat, pSampleBufHelp, Size) != 0) {
                return ConvErr((ASRSpiEngine *)acquisition, casrGetLastError(acquisition->hSpi));
            }
            if (acquisition->LoopingIntervalSize == 0) {
                if (casrGetSignalLevel(acquisition->hSpi, &EnergyLevel) != 0) {
                    return ConvErr((ASRSpiEngine *)acquisition,
        casrGetLastError(acquisition->hSpi));
                }
                if (casrGetSNR(acquisition->hSpi, &SNRLevel) != 0) {
                    return ConvErr((ASRSpiEngine *)acquisition,
        casrGetLastError(acquisition->hSpi));
                }
                if (acquisition->SigCallBacks.cbSNRLevel != NULL) {
                    Err = acquisition->SigCallBacks.cbSNRLevel(SNRLevel,
        acquisition->dwSigCBUserData);
                    if (Err != 0) {
                        return Err;
                    }
                }
                if (acquisition->SigCallBacks.cbEnergyLevel != NULL) {
                    Err = acquisition->SigCallBacks.cbEnergyLevel(EnergyLevel,
        acquisition->dwSigCBUserData);
                    if (Err != 0) {
                        return Err;
                    }
                }
            }
            RestSize -= Size;
            pSampleBufHelp += Size;
        }
    } else if (casrAcquisition(acquisition->hSpi, SpiFormat, pSampleBuf, dwBufSize) != 0) {
        return ConvErr((ASRSpiEngine *)acquisition, casrGetLastError(acquisition->hSpi));
    }
    return 0;
}

s32 asrspi_Acquisition(void *hAsrSpi, void *pSampleBuf, unsigned long dwBufSize, s32 *pFinished)
{
    ASRSpiEngine *engine;
    s32 result;
    s32 stateError;

    if (pFinished == NULL || pSampleBuf == 0) {
        return 0x80040083;
    }
    *pFinished = 0;
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngine *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    stateError = engine->State != 3 ? g_StateError[engine->State] : 0;
    if (stateError != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return stateError;
    }
    result = intAcquisition(engine, engine->SampleFormat, pSampleBuf, dwBufSize);
    *pFinished = engine->Finished;
    if (engine->Finished != 0) {
        engine->State = 2;
    }
    if (result != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        engine->Busy = 0;
        critsec_Leave((u32)engine->hCritSec);
    }
    return 0;
}

static inline long ConvTypeToSpi(unsigned long *pSpiType)
{
    *pSpiType = 4;
    return 0;
}

s32 asrspi_ExportSessionData(void *hAsrSpi, void *DataType, void *pBuf, u32 *pSize)
{
    ASRSpiEngine *pAsrSpi;
    long Err;
    unsigned long SpiType;
    u32 Size;
    void *pBufTemp;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (ASRSpiEngine *)hAsrSpi;
    if ((s32)pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter((u32)pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave((u32)pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave((u32)pAsrSpi->hCritSec);
    }
    if (pSize == NULL) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter((u32)pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave((u32)pAsrSpi->hCritSec);
        }
        return 0x80040083;
    }
    if (pBuf == NULL) {
        *pSize = 0;
    }
    if ((Err = ConvTypeToSpi(&SpiType)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter((u32)pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave((u32)pAsrSpi->hCritSec);
        }
        return 0x80040019;
    }
    if (casrExportData(pAsrSpi->hSpi, SpiType, &pBufTemp, &Size) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter((u32)pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave((u32)pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (*pSize) {
        if (*pSize < Size) {
            heap_Free(pAsrSpi->hHeap, pBufTemp);
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter((u32)pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave((u32)pAsrSpi->hCritSec);
            }
            return 0x80040085;
        }
        memcpy(pBuf, pBufTemp, Size);
    }
    *pSize = Size;
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter((u32)pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave((u32)pAsrSpi->hCritSec);
    }
    heap_Free(pAsrSpi->hHeap, pBufTemp);
    return 0;
}

s32 asrspi_ActivateSessionData(void *hAsrSpi, u32 *pBuf)
{
    ASRSpiEngine *engine;
    u32 *importData;
    u32 defaultData[34];
    // a default session-data block; only its two header words are set here
    s32 state;
    s32 result;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    engine = (ASRSpiEngine *)hAsrSpi;
    if ((s32)engine->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        if (engine->Busy != 0) {
            critsec_Leave((u32)engine->hCritSec);
            return 0x80040003;
        }
        engine->Busy = 1;
        critsec_Leave((u32)engine->hCritSec);
    }
    state = engine->State;
    if ((result = ASRSpiLanguageStateError(state)) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return result;
    }
    if (pBuf == NULL) {
        defaultData[1] = 8;
        defaultData[0] = 4;
        importData = defaultData;
    } else {
        importData = pBuf;
        if (pBuf[0] != 4) {
            if (engine->hCritSec != NULL) {
                critsec_Enter((u32)engine->hCritSec);
                engine->Busy = 0;
                critsec_Leave((u32)engine->hCritSec);
            }
            return 0x8004000A;
        }
    }
    result = (s32)casrImportData(engine->hHeap, importData);
    if (result == 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return ConvErr(engine, casrGetLastError(engine->hSpi));
    }
    if (casrActivateData(engine->hSpi, (void *)result) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return ConvErr(engine, casrGetLastError(engine->hSpi));
    }
    if (casrCloseData((void *)result) != 0) {
        if (engine->hCritSec != NULL) {
            critsec_Enter((u32)engine->hCritSec);
            engine->Busy = 0;
            critsec_Leave((u32)engine->hCritSec);
        }
        return ConvErr(engine, casrGetLastError(engine->hSpi));
    }
    if (engine->hCritSec != NULL) {
        critsec_Enter((u32)engine->hCritSec);
        engine->Busy = 0;
        critsec_Leave((u32)engine->hCritSec);
    }
    return 0;
}

const struct _CRECOGCALLBACKS g_APIRecogCallBacks = {
    ASRSPICBTS, ASRSPICBSTARTDET, ASRSPICBRESULT, NULL,
    ASRSPICBTRAIN, ASRSPICBABNORM, ASRSPICBAGC, ASRSPICBASKCURRENTGAIN
};
