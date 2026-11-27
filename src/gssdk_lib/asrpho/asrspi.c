/* Speech recognition SPI engine lifecycle and callback handling. */
#include "gssdk/asrspi.h"
#include "gssdk/heap.h"
#include "types.h"
#include "gssdk/context.h"
#include "gssdk/casr.h"
#include "gssdk/critsec.h"
#include "gssdk/rsrc.h"

#include <string.h>

/* SPI status codes returned when the engine is in an incompatible state. */
static const long g_StateError[6] = {
    0x8004000B, 0x8004000C, 0x8004000D, 0x8004000E, 0x8004000F, 0x80040010
};

/* SPI engine state, recognizer handles and client callbacks. */
struct ASRSPItag {
    int HandleValidityCheck; /* Marker checked before the SPI uses an engine handle. */
    struct HCASR__ *hSpi; /* Active CASR recognizer. */
    struct ASRCTXHEADERtag *pAsrSpiCtx; /* Original packed bytes retained for the active context. */
    struct HCTX__ *hSpiCtx; /* Imported context handle passed to CASR. */
    struct HCDATA__ *hSpiLng; /* Opened language data handle. */
    unsigned long dwSigCBUserData; /* Client value passed to signal and result callbacks. */
    unsigned long dwFeatCBUserData; /* Client value supplied at callback registration. */
    int Finished; /* Stops sample acquisition once the recognizer has finished. */
    int AllUttRecorded; /* Set by the recognizer when user-word recording ends. */
    int CBTrain_StartOrConfirmAcknowledged; /* Start or confirm prompt acknowledged. */
    void *pSpiUserWordBuf; /* Buffer passed to the client at the end of user-word recording. */
    unsigned long SpiUserWordBufSize; /* Byte length of the user-word buffer. */
    enum _ASRBASE_RECORDINGTYPE uswLastRecordType; /* Most recent user-word utterance outcome. */
    enum _ASRSPI_STATE State; /* Current state used to validate SPI calls. */
    int Busy; /* Set during operations that reject overlapping calls. */
    unsigned int CBSignalLevelIntervalInMSEC; /* Interval between signal reports in milliseconds. */
    unsigned int LoopingIntervalSize; /* Sample count toward the next signal report. */
    struct _ASRBASE_RESULT Result; /* SPI result passed to the client callback. */
    struct _ASRBASE_SIGNALCALLBACKS SigCallBacks; /* Signal callbacks supplied by the client. */
    /* Receives converted recognition results. */
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    /* Receives the recorded user-word buffer. */
    long (*cbUswResult)(void *, unsigned long, unsigned long);
    unsigned short InitialGain; /* Gain restored when the recognizer requests it. */
    unsigned int AbortingUsw; /* Suppresses the user-word result callback during abort. */
    unsigned long WasStartEnable; /* Start-detection setting restored after training. */
    enum _ASRBASE_SAMPLEFORMAT SampleFormat; /* Input format selected for the engine. */
    struct _ASRSPI_CONTEXT Context; /* SPI context handle and its virtual table. */
    void *hHeap; /* Heap inherited from the resource manager. */
    struct CRITSEC_TAG *hCritSec; /* Serializes engine state changes when supplied. */
};

/* Stored context header with the byte offset of its SPI payload. */
struct ASRCTXHEADERtag {
    unsigned long TotalSizeInBytes; /* Byte count used when importing the serialized context. */
    unsigned long OffsetToSpiBuffer; /* Byte offset from the context header to the SPI data. */
    unsigned long TS; /* Terminal-silence setting stored with the context. */
    unsigned long Accuracy; /* Decoder accuracy loaded when the context is activated. */
    unsigned long Rejection; /* Word rejection setting loaded with the context. */
    unsigned long Garbage; /* Garbage-model setting loaded with the context. */
};

/* Translates a CASR error into the SPI error returned to the caller. */
static long ConvErr(struct ASRSPItag *pAsrSpi, unsigned long SpiErr)
{
    int Err;

    switch (SpiErr) {
    case 1:
        Err = 0x8004001A; /* CASR uses this failure for allocation errors. */
        break;
    case 4:
        Err = 0x8004001D; /* The active context has no isolated-word tree. */
        break;
    case 5:
        Err = 0x8004001E; /* The requested control is absent from the current block table. */
        break;
    case 6:
        Err = 0x8004001F; /* The CASR engine handle is null. */
        break;
    case 7:
        Err = g_StateError[pAsrSpi->State];
        break;
    case 8:
        Err = 0x80040021;
        break;
    case 9:
        Err = 0x80040022;
        break;
    case 10:
        Err = 0x80040014; /* The requested parameter is unsupported. */
        break;
    case 11:
        Err = 0x80040001; /* The engine handle is invalid. */
        break;
    case 12:
        Err = 0x8004000A; /* The data type is invalid for the operation. */
        break;
    case 13:
        Err = 0x80040024;
        break;
    case 15:
        Err = 0x80040026; /* The context header has an invalid type. */
        break;
    case 17:
        Err = 0x80040028; /* The user-word prompt decision is unsupported. */
        break;
    case 19:
        Err = 0x80040015; /* The language and imported data versions differ. */
        break;
    case 20:
        Err = 0x80040029; /* CASR rejected the requested gender mask. */
        break;
    case 21:
        Err = 0x8004002A;
        break;
    case 18:
        Err = 0x8004002B; /* A required CASR argument is invalid. */
        break;
    case 22:
        Err = 0x8004002C;
        break;
    case 25:
        Err = 0x80040018; /* The word ID is outside the active context. */
        break;
    case 27:
        casrGetCallbackErr(pAsrSpi->hSpi, &Err);
        break;
    case 28:
        Err = 0x8004002D; /* The active language does not support enabling the second pass. */
        break;
    case 29:
        Err = 0x80040008; /* The engine could not use the supplied language. */
        break;
    case 30:
        Err = 0x80040009; /* The engine could not use the supplied context. */
        break;
    case 31:
        Err = 0x8004000A; /* The data type is invalid for the operation. */
        break;
    case 32:
        Err = 0x80040004; /* The sample format is unsupported. */
        break;
    case 33:
        Err = 0x80040013; /* The operation requires an active language. */
        break;
    case 34:
        Err = 0x80040034; /* The imported data does not match the active engine configuration. */
        break;
    case 39:
        Err = 0x8004003A;
        break;
    case 35:
        Err = 0x00040035; /* CASR code 35 is returned with the failure bit clear. */
        break;
    case 36:
        Err = 0x00040036; /* CASR code 36 is returned with the failure bit clear. */
        break;
    case 37:
        Err = 0x80040038;
        break;
    case 38:
        Err = 0x80040039;
        break;
    default:
        Err = 0x8004002E; /* Fallback when there is no SPI translation for the CASR error. */
        break;
    }
    return Err;
}

/* Looks up the CASR parameter ID for an SPI parameter. */
static long ConvParamToSpi(enum _ASRBASE_PARAMID Param, unsigned long *pSpiParam)
{
    switch (Param) {
    case ASRBASE_PARAM_TS_ENABLE:
        *pSpiParam = 0x1;
        break;
    case ASRBASE_PARAM_START_ENABLE:
        *pSpiParam = 0x2;
        break;
    case ASRBASE_PARAM_ENABLEPREMRES:
        *pSpiParam = 0x3;
        break;
    case ASRBASE_PARAM_TIMEOUT:
        *pSpiParam = 0x5;
        break;
    case ASRBASE_PARAM_SENSITIVITY:
        *pSpiParam = 0x9;
        break;
    case ASRBASE_PARAM_MINSPEECH:
        *pSpiParam = 0xa;
        break;
    case ASRBASE_PARAM_ABSOLUTE_THRESHOLD:
        *pSpiParam = 0x11;
        break;
    case ASRBASE_PARAM_MAXNBEST:
        *pSpiParam = 0xd;
        break;
    case ASRBASE_PARAM_FARTALK:
        *pSpiParam = 0xc;
        break;
    case ASRBASE_PARAM_AGCON:
        *pSpiParam = 0xb;
        break;
    case ASRBASE_PARAM_SECONDPASS_ENABLE:
        *pSpiParam = 0xf;
        break;
    case ASRBASE_PARAM_EXTRAEVENT_ENABLE:
        *pSpiParam = 0x12;
        break;
    case ASRBASE_PARAM_SELECTGENDER:
        *pSpiParam = 0xe;
        break;
    case ASRBASE_PARAM_USETREEDP:
        *pSpiParam = 0x10;
        break;
    case ASRBASE_CTXPARAM_REJECTION:
        *pSpiParam = 0x7;
        break;
    case ASRBASE_CTXPARAM_GARBAGE:
        *pSpiParam = 0x8;
        break;
    case ASRBASE_CTXPARAM_ACCURACY:
        *pSpiParam = 0x6;
        break;
    case ASRBASE_CTXPARAM_TS:
        *pSpiParam = 0x4;
        break;
    default:
        /* The requested context parameter has no CASR equivalent. */
        return 0x80040014;
    }
    return 0;
}

/* Converts a CASR result type to its SPI result type. */
static long ConvResType(enum _CASRRESULTTYPE SpiType, enum _ASRBASE_RESULTTYPE *pType)
{
    switch (SpiType) {
    case CASRRESULTTYPE_FINAL:
        *pType = ASRBASE_RESULTTYPE_FINAL;
        break;
    case CASRRESULTTYPE_PREMATURE:
        *pType = ASRBASE_RESULTTYPE_PREMATURE;
        break;
    case CASRRESULTTYPE_TIMEOUT:
        *pType = ASRBASE_RESULTTYPE_TIMEOUT;
        break;
    case CASRRESULTTYPE_REJECTED:
        *pType = ASRBASE_RESULTTYPE_REJECTED;
        break;
    default:
        /* CASR returned a result type the SPI cannot represent. */
        return 0x8004002E;
    }
    return 0;
}

/* Converts a CASR abnormal condition to its SPI condition. */
static long ConvCond(enum _ABNORMCOND SpiCond, enum _ASRBASE_ABNORMCOND *pCond)
{
    switch (SpiCond) {
    case BADSNR:
        *pCond = ASRBASE_BADSNR;
        break;
    case OVERLOAD:
        *pCond = ASRBASE_OVERLOAD;
        break;
    case TOOQUIET:
        *pCond = ASRBASE_TOOQUIET;
        break;
    case NOSIGNAL:
        *pCond = ASRBASE_NOSIGNAL;
        break;
    case GARBLEDSOUND:
        *pCond = ASRBASE_GARBLEDSOUND;
        break;
    case POORMIC:
        *pCond = ASRBASE_POORMIC;
        break;
    }
    return 0;
}

/* Converts and forwards an abnormal input condition to the SPI client. */
long ASRSPICBABNORM(enum _ABNORMCOND AbnormalCondition, unsigned long Time,
    struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;
    enum _ASRBASE_ABNORMCOND AsrSpiCond;
    long RetVal;

    ConvCond(AbnormalCondition, &AsrSpiCond);
    if (pAsrSpi->SigCallBacks.cbAbnorm != NULL) {
        RetVal = pAsrSpi->SigCallBacks.cbAbnorm(AsrSpiCond, Time, pAsrSpi->dwSigCBUserData);
        return RetVal;
    }
    return 0;
}

/* Forwards the automatic gain adjustment to the SPI client. */
long ASRSPICBAGC(unsigned short NewGain, struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;

    if (pAsrSpi->SigCallBacks.cbAgc != NULL) {
        return pAsrSpi->SigCallBacks.cbAgc(NewGain, pAsrSpi->dwSigCBUserData);
    }
    return 0;
}

/* Restores the recognizer's saved initial gain. */
long ASRSPICBASKCURRENTGAIN(struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;

    return casrGainSet(pAsrSpi->hSpi, pAsrSpi->InitialGain);
}

/* Handles training prompts and sends completed utterance data to the client. */
long ASRSPICBTRAIN(enum _PROMPTTYPE Type, struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;
    long RetVal = 0;

    switch (Type) {
    case PROMPTTYPE_START:
        RetVal = casrAcknowledge(pAsrSpi->hSpi, 1);
        pAsrSpi->CBTrain_StartOrConfirmAcknowledged = 1;
        break;
    case PROMPTTYPE_ACCEPT:
        if (pAsrSpi->AbortingUsw != 0) {
            RetVal = casrAcknowledge(pAsrSpi->hSpi, 3);
        } else {
            pAsrSpi->Finished = 1;
        }
        break;
    case PROMPTTYPE_CONFIRM:
        RetVal = casrAcknowledge(pAsrSpi->hSpi, 1);
        if (RetVal != 0) {
            RetVal = casrGetLastError(pAsrSpi->hSpi);
        }
        if (RetVal == 27U) {
            casrGetCallbackErr(pAsrSpi->hSpi, (int *)&RetVal);
        } else if (RetVal != 0) {
            RetVal = ConvErr(pAsrSpi, RetVal);
        }
        pAsrSpi->CBTrain_StartOrConfirmAcknowledged = 1;
        break;
    case PROMPTTYPE_TRAINEND:
        pAsrSpi->AllUttRecorded = 1;
        if (pAsrSpi->cbUswResult != NULL && pAsrSpi->AbortingUsw == 0) {
            RetVal = pAsrSpi->cbUswResult(
                pAsrSpi->pSpiUserWordBuf, pAsrSpi->SpiUserWordBufSize, pAsrSpi->dwSigCBUserData);
        }
        break;
    case PROMPTTYPE_NOPROMPT:
        break;
    }
    return RetVal;
}

/* Notifies the SPI client when the recognizer reports terminal silence. */
long ASRSPICBTS(unsigned long Time, struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;
    long Err = 0;

    if (pAsrSpi->SigCallBacks.cbTsDetected != NULL) {
        Err = pAsrSpi->SigCallBacks.cbTsDetected(Time, pAsrSpi->dwSigCBUserData);
    }
    return Err;
}

/* Notifies the SPI client when the recognizer detects the start of speech. */
long ASRSPICBSTARTDET(unsigned long Time, struct _USERDATA *pUserData)
{
    long Err = 0;
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;

    if (pAsrSpi->SigCallBacks.cbSpeechDetected != NULL) {
        Err = pAsrSpi->SigCallBacks.cbSpeechDetected(Time, pAsrSpi->dwSigCBUserData);
    }
    return Err;
}

/* Translates and forwards a recognition result to the SPI client. */
long ASRSPICBRESULT(struct _CASRRESULT *pSpiResult, struct _USERDATA *pUserData)
{
    struct ASRSPItag *pAsrSpi = (struct ASRSPItag *)pUserData->dwEngineUserData;
    enum _ASRBASE_RESULTTYPE Type;
    long RetVal;

    pAsrSpi->Finished = 1;
    RetVal = ConvResType(pSpiResult->Type, &Type);
    if (RetVal != 0) {
        return RetVal;
    }
    pAsrSpi->Result.Type = Type;
    pAsrSpi->Result.iNbr = pSpiResult->iNbr;
    /* CASR and SPI expose separate sentence types with the same record layout. */
    pAsrSpi->Result.pSentences = (struct _ASRBASE_SENTENCE *)pSpiResult->pSentences;
    if (pAsrSpi->cbResult != NULL) {
        return pAsrSpi->cbResult(&pAsrSpi->Result, pAsrSpi->dwSigCBUserData);
    }
    return 0;
}

/* Recognizer callbacks that forward events to the SPI client. */
static const struct _CRECOGCALLBACKS g_APIRecogCallBacks = {
    ASRSPICBTS, ASRSPICBSTARTDET, ASRSPICBRESULT, NULL,
    ASRSPICBTRAIN, ASRSPICBABNORM, ASRSPICBAGC, ASRSPICBASKCURRENTGAIN
};

/* Allocates and opens a recognition engine for the requested sample format. */
long asrspi_OpenEngine(SafeHandle hRsrc, enum _ASRBASE_SAMPLEFORMAT SampleFormat, void *phApiAsr)
{
    struct ASRSPItag *pAsrSpi;
    struct HCASR__ *hSpi;
    const struct RSRC_S *pRsrc;
    long lhError;
    long Error;

    if (phApiAsr == NULL) {
        return 0x80040083;
    }
    lhError = InitRsrcFunction(hRsrc, &pRsrc);
    if (IsError(lhError)) {
        return lhError;
    }
    pAsrSpi = heap_Calloc(pRsrc->hHeap, 1, sizeof(*pAsrSpi));
    if (pAsrSpi == NULL) {
        return 0x8004002F;
    }
    memset(pAsrSpi, 0, sizeof(*pAsrSpi));
    pAsrSpi->hHeap = pRsrc->hHeap;
    if (SampleFormat != ASRBASE_PCM_16_11KHZ &&
        SampleFormat != ASRBASE_UTTFORMAT_CEPSTRA_11KHZ &&
        SampleFormat != ASRBASE_UTTFORMAT_LABELS_11KHZ) {
        heap_Free(pAsrSpi->hHeap, pAsrSpi);
        return 0x80040004;
    }
    hSpi = casrOpen(pRsrc->hHeap, &g_APIRecogCallBacks, (unsigned long)pAsrSpi);
    if (hSpi == NULL) {
        heap_Free(pAsrSpi->hHeap, pAsrSpi);
        return 0x80040086;
    }
    pAsrSpi->hSpi = hSpi;
    pAsrSpi->State = ASRBASE_BOOT;
    pAsrSpi->HandleValidityCheck = 0x2B67;
    pAsrSpi->SampleFormat = SampleFormat;
    pAsrSpi->uswLastRecordType = ASRBASE_RECORDINGTYPE_ACCEPTED;
    FillContextV2VirtualTable(&pAsrSpi->Context);
    pAsrSpi->InitialGain = 0xBFFF;
    *(struct ASRSPItag **)phApiAsr = pAsrSpi;
    return 0;
}

/* Returns the current state's error when it differs from the allowed state. */
static long CheckState(struct ASRSPItag *pAsrSpi, enum _ASRSPI_STATE State1)
{
    if (pAsrSpi->State != State1) {
        return g_StateError[pAsrSpi->State];
    }
    return 0;
}

/* Returns the current state's error when none of three states is allowed. */
static long Check3States(struct ASRSPItag *pAsrSpi, enum _ASRSPI_STATE State1,
    enum _ASRSPI_STATE State2, enum _ASRSPI_STATE State3)
{
    if (pAsrSpi->State != State1 && pAsrSpi->State != State2 && pAsrSpi->State != State3) {
        return g_StateError[pAsrSpi->State];
    }
    return 0;
}

/* Returns the current state's error when none of four states is allowed. */
static long Check4States(struct ASRSPItag *pAsrSpi, enum _ASRSPI_STATE State1,
    enum _ASRSPI_STATE State2, enum _ASRSPI_STATE State3, enum _ASRSPI_STATE State4)
{
    if (pAsrSpi->State != State1 && pAsrSpi->State != State2 &&
        pAsrSpi->State != State3 && pAsrSpi->State != State4) {
        return g_StateError[pAsrSpi->State];
    }
    return 0;
}

/* Closes the recognition engine and releases its resources. */
long asrspi_CloseEngine(void *hAsrSpi)
{
    struct ASRSPItag *pAsrSpi;
    long Err;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = Check4States(pAsrSpi, ASRBASE_BOOT, ASRBASE_LANGREADY,
                            ASRBASE_USWTRAINED, ASRBASE_IDLE)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->State == ASRBASE_USWTRAINED) {
        if (pAsrSpi->pSpiUserWordBuf != NULL) {
            heap_Free(pAsrSpi->hHeap, pAsrSpi->pSpiUserWordBuf);
        }
    }
    pAsrSpi->pSpiUserWordBuf = NULL;
    pAsrSpi->SpiUserWordBufSize = 0;
    if (casrClose(pAsrSpi->hSpi) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (pAsrSpi->hSpiCtx != NULL && casrCloseContext(pAsrSpi->hSpiCtx) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040086;
    }
    if (pAsrSpi->hSpiLng != NULL && casrCloseData(pAsrSpi->hSpiLng) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040086;
    }
    pAsrSpi->HandleValidityCheck = 0;
    heap_Free(pAsrSpi->hHeap, pAsrSpi);
    return 0;
}

/* Reads the current recognition state from the engine. */
long asrspi_GetState(void *hAsrSpi, enum _ASRSPI_STATE *pState)
{
    struct ASRSPItag *pAsrSpi = hAsrSpi;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pState == NULL) {
        return 0x80040083;
    }
    *pState = pAsrSpi->State;
    return 0;
}

/* Imports and activates the requested recognition language. */
long asrspi_ActivateLng(void *hAsrSpi, void *pAsrSpiLng)
{
    struct ASRSPItag *pAsrSpi;
    struct HCDATA__ *hSpiLng;
    long Err;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = Check4States(pAsrSpi, ASRBASE_BOOT, ASRBASE_LANGREADY,
                            ASRBASE_USWTRAINED, ASRBASE_IDLE)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->State == ASRBASE_USWTRAINED && pAsrSpi->pSpiUserWordBuf != NULL) {
        heap_Free(pAsrSpi->hHeap, pAsrSpi->pSpiUserWordBuf);
    }
    pAsrSpi->pSpiUserWordBuf = NULL;
    pAsrSpi->SpiUserWordBufSize = 0;
    if (pAsrSpiLng == NULL) {
        hSpiLng = NULL;
    } else {
        if (((struct _DATA *)pAsrSpiLng)->DataType != 0x1004 &&
            ((struct _DATA *)pAsrSpiLng)->DataType != 0x1003) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x80040008;
        }
        hSpiLng = casrImportData(pAsrSpi->hHeap, pAsrSpiLng);
        if (hSpiLng == NULL) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x80040086;
        }
    }
    if (casrActivateData(pAsrSpi->hSpi, hSpiLng) != 0) {
        Err = ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
        /* Report a data type failure as a language activation failure here. */
        if (Err == 0x8004000A) {
            Err = 0x80040008; /* Failed to activate the supplied language. */
        }
        casrCloseData(hSpiLng);
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->hSpiCtx != NULL && casrCloseContext(pAsrSpi->hSpiCtx) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040086;
    }
    pAsrSpi->hSpiCtx = NULL;
    if (pAsrSpi->hSpiLng != NULL && casrCloseData(pAsrSpi->hSpiLng) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040086;
    }
    pAsrSpi->hSpiLng = hSpiLng;
    if (pAsrSpiLng != NULL) {
        pAsrSpi->State = ASRBASE_LANGREADY;
    } else {
        pAsrSpi->State = ASRBASE_BOOT;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Imports and activates a recognition context and optionally sets its preferred parameters. */
long asrspi_ActivateCtx(void *hAsrSpi, void *pAsrSpiCtx, int bSetOptimalParameters)
{
    /* CASR validates and imports the packed bytes supplied by the caller. */
    struct _DATA *pSpiCtx = (struct _DATA *)pAsrSpiCtx;
    struct HCTX__ *hSpiCtx; /* Imported context handle passed to CASR. */
    struct ASRCTXHEADERtag *pHeader = pAsrSpiCtx;
    struct ASRSPItag *pAsrSpi;
    long Error;
    long Err;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    {
        long Error;

        if ((Error = Check3States(pAsrSpi, ASRBASE_LANGREADY, ASRBASE_USWTRAINED,
                                 ASRBASE_IDLE)) != 0) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return Error;
        }
    }
    if (pAsrSpi->State == ASRBASE_USWTRAINED && pAsrSpi->pSpiUserWordBuf != NULL) {
        heap_Free(pAsrSpi->hHeap, pAsrSpi->pSpiUserWordBuf);
    }
    pAsrSpi->pSpiUserWordBuf = NULL;
    pAsrSpi->SpiUserWordBufSize = 0;
    if (pSpiCtx != NULL) {
        if (pSpiCtx->Size == sizeof(struct ASRCTXHEADERtag) && pSpiCtx->DataType != 0x2) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x80040026;
        }
        hSpiCtx = casrImportContext(pAsrSpi->hHeap, pSpiCtx);
        if (hSpiCtx == NULL) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x80040009;
        }
    } else {
        hSpiCtx = NULL;
    }
    if (casrActivateContext(pAsrSpi->hSpi, hSpiCtx, bSetOptimalParameters) != 0) {
        Err = ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
        casrCloseContext(hSpiCtx);
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->hSpiCtx != NULL) {
        casrCloseContext(pAsrSpi->hSpiCtx);
    }
    pAsrSpi->pAsrSpiCtx = pHeader;
    pAsrSpi->hSpiCtx = hSpiCtx;
    pAsrSpi->Context.pCtx = pHeader;
    if (pSpiCtx != NULL) {
        pAsrSpi->State = ASRBASE_IDLE;
    } else {
        pAsrSpi->State = ASRBASE_LANGREADY;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Applies a list of recognition parameters to the engine. */
long asrspi_SetParamList(void *hAsrSpi, const struct _ASRBASE_PARAM *pParamList, unsigned int Nbr)
{
    struct ASRSPItag *pAsrSpi;
    unsigned long SpiParamID;
    long Err;
    u32 i;

    if (pParamList == NULL) {
        return 0x80040083;
    }
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = Check4States(pAsrSpi, ASRBASE_BOOT, ASRBASE_LANGREADY,
                            ASRBASE_USWTRAINED, ASRBASE_IDLE)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    for (i = 0; i < Nbr; i++) {
        Err = ConvParamToSpi(pParamList[i].ID, &SpiParamID);
        if (Err != 0) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return Err;
        }
        /* This early rejection leaves Busy set. */
        if (SpiParamID == 14 && pAsrSpi->State == ASRBASE_BOOT) {
            return 0x80040013;
        }
    }
    for (i = 0; i < Nbr; i++) {
        ConvParamToSpi(pParamList[i].ID, &SpiParamID);
        if (casrSetParam(pAsrSpi->hSpi, SpiParamID, pParamList[i].Value) != 0) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
        }
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Finds a word ID in the active list or returns the missing-word sentinel. */
static unsigned long InList(
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

/* Adds the requested words to the active set or activates every word. */
long asrspi_ActivateWords(void *hAsrSpi, unsigned long *pWordIDs, unsigned int cNewWords)
{
    struct ASRSPItag *pAsrSpi;
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
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    Err = CheckState(pAsrSpi, ASRBASE_IDLE);
    if (Err != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
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
            pNewActiveWords =
                heap_Alloc(pAsrSpi->hHeap, (cExtraWords + cWords) * sizeof(*pNewActiveWords));
            if (pNewActiveWords == NULL) {
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter(pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave(pAsrSpi->hCritSec);
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
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
        }
        cExtraWords = 0;
        for (i = 0; i < cNewWords; i++) {
            if (pWordIDs[i] >= NrWordsInCtx) {
                heap_Free(pAsrSpi->hHeap, pActiveWords);
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter(pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave(pAsrSpi->hCritSec);
                }
                return 0x80040018;
            }
            if (InList(pWordIDs[i], pActiveWords, cWords) == 0xFFFFFFFF) {
                cExtraWords++;
            }
        }
        if (cExtraWords + cWords != 0) {
            pNewActiveWords =
                heap_Alloc(pAsrSpi->hHeap, (cExtraWords + cWords) * sizeof(*pNewActiveWords));
            if (pNewActiveWords == NULL) {
                heap_Free(pAsrSpi->hHeap, pActiveWords);
                if (pAsrSpi->hCritSec != NULL) {
                    critsec_Enter(pAsrSpi->hCritSec);
                    pAsrSpi->Busy = 0;
                    critsec_Leave(pAsrSpi->hCritSec);
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
    if (casrSetActiveWords(pAsrSpi->hSpi, pNewActiveWords, cWords + cExtraWords) != 0) {
        heap_Free(pAsrSpi->hHeap, pNewActiveWords);
        if (pActiveWords != NULL) {
            heap_Free(pAsrSpi->hHeap, pActiveWords);
        }
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
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
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Starts recognition from idle and registers the client signal and result callbacks. */
long asrspi_Start(void *hAsrSpi, unsigned int Interval,
    struct _ASRSPI_RECOGCALLBACKS *pCallBacks, unsigned long dwUserData)
{
    struct ASRSPItag *pAsrSpi;
    long Err;

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
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = CheckState(pAsrSpi, ASRBASE_IDLE)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (Interval != 0 && pCallBacks->pSignalCallbacks != NULL &&
        pAsrSpi->SampleFormat != ASRBASE_PCM_16_11KHZ &&
        pAsrSpi->SampleFormat != ASRBASE_PCM_16_8KHZ &&
        (pCallBacks->pSignalCallbacks->cbEnergyLevel != NULL ||
         pCallBacks->pSignalCallbacks->cbSNRLevel != NULL)) {
        return 0x80040017;
    }
    pAsrSpi->Finished = 0;
    pAsrSpi->CBSignalLevelIntervalInMSEC = Interval;
    pAsrSpi->LoopingIntervalSize = 0;
    pAsrSpi->dwSigCBUserData = dwUserData;
    pAsrSpi->dwFeatCBUserData = dwUserData;
    if (pCallBacks->pSignalCallbacks != NULL) {
        memcpy(&pAsrSpi->SigCallBacks, pCallBacks->pSignalCallbacks, sizeof(pAsrSpi->SigCallBacks));
    } else {
        memset(&pAsrSpi->SigCallBacks, 0, sizeof(pAsrSpi->SigCallBacks));
    }
    pAsrSpi->cbResult = pCallBacks->cbResult;
    if (casrStart(pAsrSpi->hSpi) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    pAsrSpi->State = ASRBASE_RECOGNIZING;
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Stops recognition and checks whether the recognizer reached idle after an error. */
long asrspi_Stop(void *hAsrSpi)
{
    enum _CASRSTATE State;
    struct ASRSPItag *pAsrSpi;
    long Err;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = CheckState(pAsrSpi, ASRBASE_RECOGNIZING)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (casrStop(pAsrSpi->hSpi) != 0) {
        casrGetState(pAsrSpi->hSpi, &State);
        if (State == CASR_IDLE) {
            pAsrSpi->State = ASRBASE_IDLE;
        }
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    pAsrSpi->State = ASRBASE_IDLE;
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Translates the requested sample format for the recognition engine. */
static inline long ConvFormatToSpi(enum _ASRBASE_SAMPLEFORMAT Format,
                                   enum _CSAMPLEFORMAT *pSpiFormat)
{
    switch (Format) {
    case ASRBASE_PCM_16_11KHZ:
        *pSpiFormat = PCM_16_11KHZ;
        break;
    case ASRBASE_UTTFORMAT_CEPSTRA_11KHZ:
        *pSpiFormat = UTTFORMAT_CEPSTRA;
        break;
    case ASRBASE_UTTFORMAT_LABELS_11KHZ:
        *pSpiFormat = UTTFORMAT_LABELS;
        break;
    case ASRBASE_UTTFORMAT_UNKNOWN:
        *pSpiFormat = UTTFORMAT_UNKNOWN;
        break;
    default:
        /* The SPI sample format has no CASR equivalent. */
        return 0x80040004;
    }
    return 0;
}

/* Feeds samples to the engine and dispatches signal reports at the configured intervals. */
static long intAcquisition(struct ASRSPItag *pAsrSpi,
    enum _ASRBASE_SAMPLEFORMAT SampleFormat, void *pSampleBuf, unsigned long dwBufSize)
{
    enum _CSAMPLEFORMAT SpiFormat;
    long Err;
    unsigned int Size;
    unsigned int RestSize;
    unsigned int IntervalSize;
    char *pSampleBufHelp;
    signed short EnergyLevel;
    signed short SNRLevel;

    Err = ConvFormatToSpi(SampleFormat, &SpiFormat);
    if (Err != 0) {
        return Err;
    }
    if ((pAsrSpi->SigCallBacks.cbEnergyLevel != NULL || pAsrSpi->SigCallBacks.cbSNRLevel != NULL) &&
        SampleFormat <= (unsigned int) ASRBASE_PCM_16_8KHZ) {
        RestSize = dwBufSize;
        pSampleBufHelp = pSampleBuf;
        if (SampleFormat == ASRBASE_PCM_16_11KHZ) {
            IntervalSize = pAsrSpi->CBSignalLevelIntervalInMSEC * 22;
        } else {
            IntervalSize = pAsrSpi->CBSignalLevelIntervalInMSEC * 16;
        }
        while (RestSize != 0 && pAsrSpi->Finished == 0) {
            long Err;

            if (RestSize >= IntervalSize - pAsrSpi->LoopingIntervalSize) {
                Size = IntervalSize - pAsrSpi->LoopingIntervalSize;
                pAsrSpi->LoopingIntervalSize = 0;
            } else {
                pAsrSpi->LoopingIntervalSize += RestSize;
                Size = RestSize;
            }
            if (casrAcquisition(pAsrSpi->hSpi, SpiFormat, pSampleBufHelp, Size) != 0) {
                return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
            }
            if (pAsrSpi->LoopingIntervalSize == 0) {
                if (casrGetSignalLevel(pAsrSpi->hSpi, &EnergyLevel) != 0) {
                    return ConvErr(pAsrSpi,
                        casrGetLastError(pAsrSpi->hSpi));
                }
                if (casrGetSNR(pAsrSpi->hSpi, &SNRLevel) != 0) {
                    return ConvErr(pAsrSpi,
                        casrGetLastError(pAsrSpi->hSpi));
                }
                if (pAsrSpi->SigCallBacks.cbSNRLevel != NULL) {
                    Err = pAsrSpi->SigCallBacks.cbSNRLevel(SNRLevel, pAsrSpi->dwSigCBUserData);
                    if (Err != 0) {
                        return Err;
                    }
                }
                if (pAsrSpi->SigCallBacks.cbEnergyLevel != NULL) {
                    Err = pAsrSpi->SigCallBacks.cbEnergyLevel(EnergyLevel,
                                                         pAsrSpi->dwSigCBUserData);
                    if (Err != 0) {
                        return Err;
                    }
                }
            }
            RestSize -= Size;
            pSampleBufHelp += Size;
        }
    } else if (casrAcquisition(pAsrSpi->hSpi, SpiFormat, pSampleBuf, dwBufSize) != 0) {
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    return 0;
}

/* Feeds a sample buffer to the recognizer and reports when acquisition finishes. */
long asrspi_Acquisition(void *hAsrSpi, void *pSampleBuf, unsigned long dwBufSize, int *pFinished)
{
    struct ASRSPItag *pAsrSpi;
    long Err;
    long stateError;

    if (pFinished == NULL || pSampleBuf == NULL) {
        return 0x80040083;
    }
    *pFinished = 0;
    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    stateError = CheckState(pAsrSpi, ASRBASE_RECOGNIZING);
    if (stateError != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return stateError;
    }
    Err = intAcquisition(pAsrSpi, pAsrSpi->SampleFormat, pSampleBuf, dwBufSize);
    *pFinished = pAsrSpi->Finished;
    if (pAsrSpi->Finished != 0) {
        pAsrSpi->State = ASRBASE_IDLE;
    }
    if (Err != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}

/* Selects the engine's session data type. */
static long ConvTypeToSpi(enum _ASRBASE_DATATYPE DataType, unsigned long *pSpiType)
{
    *pSpiType = 4;
    return 0;
}

/* Exports engine session data through the SPI handle into the caller's buffer. */
long asrspi_ExportSessionData(void *hAsrSpi, enum _ASRBASE_DATATYPE DataType, void *pBuf,
    unsigned long *pSize)
{
    struct ASRSPItag *pAsrSpi;
    long Err;
    unsigned long SpiType;
    unsigned long Size;
    void *pBufTemp;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if (pSize == NULL) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040083;
    }
    if (pBuf == NULL) {
        *pSize = 0;
    }
    if ((Err = ConvTypeToSpi(DataType, &SpiType)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return 0x80040019;
    }
    if (casrExportData(pAsrSpi->hSpi, SpiType, &pBufTemp, &Size) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (*pSize) {
        if (*pSize < Size) {
            heap_Free(pAsrSpi->hHeap, pBufTemp);
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x80040085;
        }
        memcpy(pBuf, pBufTemp, Size);
    }
    *pSize = Size;
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    heap_Free(pAsrSpi->hHeap, pBufTemp);
    return 0;
}

/* Imports and activates saved recognition session data. */
long asrspi_ActivateSessionData(void *hAsrSpi, void *pBuf)
{
    struct ASRSPItag *pAsrSpi;
    struct HCDATA__ *hData;
    struct _DATA Buffer;
    struct _DATA *pBuffer;
    long Err;

    if (hAsrSpi == NULL) {
        return 0x80040083;
    }
    pAsrSpi = (struct ASRSPItag *)hAsrSpi;
    if (pAsrSpi->HandleValidityCheck != 0x2B67) {
        return 0x80040001;
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        if (pAsrSpi->Busy != 0) {
            critsec_Leave(pAsrSpi->hCritSec);
            return 0x80040003;
        }
        pAsrSpi->Busy = 1;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    if ((Err = Check3States(pAsrSpi, ASRBASE_LANGREADY, ASRBASE_USWTRAINED, ASRBASE_IDLE)) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return Err;
    }
    if (pBuf == NULL) {
        Buffer.Size = 8;
        Buffer.DataType = 4;
        pBuffer = &Buffer;
    } else {
        pBuffer = pBuf;
        if (pBuffer->DataType != 4) {
            if (pAsrSpi->hCritSec != NULL) {
                critsec_Enter(pAsrSpi->hCritSec);
                pAsrSpi->Busy = 0;
                critsec_Leave(pAsrSpi->hCritSec);
            }
            return 0x8004000A;
        }
    }
    hData = casrImportData(pAsrSpi->hHeap, pBuffer);
    if (hData == NULL) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (casrActivateData(pAsrSpi->hSpi, hData) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (casrCloseData(hData) != 0) {
        if (pAsrSpi->hCritSec != NULL) {
            critsec_Enter(pAsrSpi->hCritSec);
            pAsrSpi->Busy = 0;
            critsec_Leave(pAsrSpi->hCritSec);
        }
        return ConvErr(pAsrSpi, casrGetLastError(pAsrSpi->hSpi));
    }
    if (pAsrSpi->hCritSec != NULL) {
        critsec_Enter(pAsrSpi->hCritSec);
        pAsrSpi->Busy = 0;
        critsec_Leave(pAsrSpi->hCritSec);
    }
    return 0;
}
