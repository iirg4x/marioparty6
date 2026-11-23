/* Speech recognition callbacks forward signals and results to GSAPI clients. */
#include "types.h"

#include "gssdk/asrspi.h"
#include "gssdk/gsapi_internal.h"
#include "gssdk/heap.h"

/* The recognizer calls this for abnormal signals; GSAPI forwards notification event 4. */
long asrspi_cbAbnorm(enum _ASRBASE_ABNORMCOND AbnormCond, unsigned long u32Time,
                      unsigned long u32UserData)
{
    struct _GSAPI_ENGINE *callbackContext = (struct _GSAPI_ENGINE *)u32UserData;

    if (gGSAPI.cbNotify == NULL) {
        return 0;
    }

    callbackContext->bInCallback = TRUE;
    gGSAPI.cbNotify(callbackContext, GSAPI_NOTIFY_ABNORMCOND, u32Time, AbnormCond);
    callbackContext->bInCallback = FALSE;
    return 0;
}

/* The recognizer registers this AGC signal, but GSAPI does not forward it. */
long asrspi_cbAgc(unsigned short u16Gain, unsigned long u32UserData)
{
    return 0;
}

/* The recognizer calls this for enabled energy reports; GSAPI forwards event 5. */
long asrspi_cbEnergyLevel(int Level, unsigned long u32UserData)
{
    struct _GSAPI_ENGINE *callbackContext = (struct _GSAPI_ENGINE *)u32UserData;

    if (gGSAPI.cbNotify == NULL) {
        return 0;
    }

    callbackContext->bInCallback = TRUE;
    gGSAPI.cbNotify(callbackContext, GSAPI_NOTIFY_ENERGYLEVEL, Level,
                    callbackContext->u32EngineUserData);
    callbackContext->bInCallback = FALSE;
    return 0;
}

/* The recognizer calls this for enabled SNR reports; GSAPI forwards event 6. */
long asrspi_cbSNRLevel(int Level, unsigned long u32UserData)
{
    struct _GSAPI_ENGINE *callbackContext = (struct _GSAPI_ENGINE *)u32UserData;

    if (gGSAPI.cbNotify == NULL) {
        return 0;
    }

    callbackContext->bInCallback = TRUE;
    gGSAPI.cbNotify(callbackContext, GSAPI_NOTIFY_SNRLEVEL, Level,
                    callbackContext->u32EngineUserData);
    callbackContext->bInCallback = FALSE;
    return 0;
}

/* The recognizer calls this after speech detection; GSAPI forwards notification event 7. */
long asrspi_cbSpeechDetected(unsigned long u32Time, unsigned long u32UserData)
{
    struct _GSAPI_ENGINE *callbackContext = (struct _GSAPI_ENGINE *)u32UserData;

    if (gGSAPI.cbNotify == NULL) {
        return 0;
    }

    callbackContext->bInCallback = TRUE;
    gGSAPI.cbNotify(callbackContext, GSAPI_NOTIFY_SPEECHDETECTED, u32Time,
                    callbackContext->u32EngineUserData);
    callbackContext->bInCallback = FALSE;
    return 0;
}

/* The recognizer calls this after detecting terminal silence. */
long asrspi_cbTsDetected(unsigned long u32Time, unsigned long u32UserData)
{
    struct _GSAPI_ENGINE *callbackContext = (struct _GSAPI_ENGINE *)u32UserData;

    if (gGSAPI.cbNotify == NULL) {
        return 0;
    }

    callbackContext->bInCallback = TRUE;
    gGSAPI.cbNotify(callbackContext, GSAPI_NOTIFY_TSDETECTED, u32Time,
                    callbackContext->u32EngineUserData);
    callbackContext->bInCallback = FALSE;
    return 0;
}

/* Rounds the result storage size up to the required alignment. */
static inline u32 AlignResultStorage(u32 size)
{
    return (size + 3) & ~3U;
}

/* The recognizer calls this with candidate results; GSAPI copies them and rejects nonzero status
* or a first-entry score below the minimum, then reports the selected action or a no-result
* notification. Rejected results restart when bManualStop is zero; empty results restart only
* with mode bit 1, as do successful results. */
long asrspi_cbResult(struct _ASRBASE_RESULT *pAsrspiResult, unsigned long u32UserData)
{
    s32 Error;
    struct _GSAPI_ENGINE *pEngine = (struct _GSAPI_ENGINE *)u32UserData;
    unsigned long Action = 0;
    unsigned long ConfidenceThreshold;
    struct _GSAPI_DETAILEDRESULT *pDetailedResults = NULL;
    struct _GSAPI_DETAILEDWORD *pDetailedWords;
    u16 i;
    u16 j;
    int AllocSize;
    unsigned long *pWordIdList;

    pEngine->bInCallback = TRUE;
    if (pAsrspiResult == NULL) {
        if (gGSAPI.cbNotify != NULL) {
            gGSAPI.cbNotify(pEngine, GSAPI_NOTIFY_ASRRESULT, 0, 0);
        }
        if (pEngine->Mode & (1 << 1)) {
            RestartEngine(pEngine);
        }
        pEngine->bInCallback = FALSE;
        return 0;
    }
    if (pAsrspiResult->iNbr == 0) {
        if (gGSAPI.cbNotify != NULL) {
            gGSAPI.cbNotify(pEngine, GSAPI_NOTIFY_ASRRESULT, 0, 0);
        }
        if (pEngine->Mode & (1 << 1)) {
            RestartEngine(pEngine);
        }
        pEngine->bInCallback = FALSE;
        return 0;
    }
    if (pEngine->pCurrentContext == NULL) {
        pEngine->bInCallback = FALSE;
        return 0;
    }

    Error = ContextGetParam(pEngine->pCurrentContext, GSAPI_PARAM_CONF_THRESHOLD,
                            &ConfidenceThreshold);
    AllocSize = AlignResultStorage(sizeof(struct _GSAPI_RESULT) * pAsrspiResult->iNbr);
    for (i = 0; i < pAsrspiResult->iNbr;
         i++) {
        AllocSize +=
            pAsrspiResult->pSentences[i].iNbrWords * sizeof(u32);
    }
    pEngine->pResult = heap_Alloc(gGSAPI.hHeap, AllocSize);
    if (pEngine->pResult == NULL) {
        pEngine->bInCallback = FALSE;
        return 0;
    }

    i = 0;
    pWordIdList = (unsigned long *)((u8 *)pEngine->pResult +
                      AlignResultStorage(sizeof(struct _GSAPI_RESULT) * pAsrspiResult->iNbr));
    for (; i < pAsrspiResult->iNbr; i++) {
        pEngine->pResult[i].Type = (enum _GSAPI_RESULTTYPE)pAsrspiResult->Type;
        pEngine->pResult[i].u16NbrWords =
            (u16)pAsrspiResult->pSentences[i].iNbrWords;
        pEngine->pResult[i].u16Conf =
            (u16)pAsrspiResult->pSentences[i].iConf;
        pEngine->pResult[i].pWordIds = pWordIdList;
        pEngine->pResult[i].pDetails = NULL;
        pEngine->pResult[i].pNextBest =
            (i == pAsrspiResult->iNbr - 1) ? NULL :
            &pEngine->pResult[i] + 1;
        pWordIdList += pEngine->pResult[i].u16NbrWords;
        for (j = 0;
             j < pEngine->pResult[i].u16NbrWords;
             j++) {
            pEngine->pResult[i].pWordIds[j] =
                pAsrspiResult->pSentences[i].pWords[j].pAlternatives->idWord;
        }
    }

    if (gGSAPI.bDetailedResults != 0) {
        i = 0;
        AllocSize = AlignResultStorage(sizeof(struct _GSAPI_DETAILEDRESULT) * pAsrspiResult->iNbr);
        for (; i < pAsrspiResult->iNbr;
             i++) {
            AllocSize +=
                AlignResultStorage(sizeof(struct _GSAPI_DETAILEDWORD) *
                pAsrspiResult->pSentences[i].iNbrWords);
        }
        if ((pDetailedResults = heap_Alloc(gGSAPI.hHeap, AllocSize)) == NULL) {
            /* The callback returns here with the candidate copy still allocated. */
            pEngine->bInCallback = FALSE;
            return 0;
        }

        i = 0;
        pDetailedWords = (struct _GSAPI_DETAILEDWORD *)((u8 *)pDetailedResults +
            AlignResultStorage(sizeof(struct _GSAPI_DETAILEDRESULT) * pAsrspiResult->iNbr));
        for (; i < pAsrspiResult->iNbr; i++) {
            pEngine->pResult[i].pDetails = &pDetailedResults[i];
            j = 0;
            pDetailedResults[i].TimeInfo.TimeBegin =
                pAsrspiResult->pSentences[i].TimeInfo.TimeBegin;
            pDetailedResults[i].TimeInfo.TimeEnd =
                pAsrspiResult->pSentences[i].TimeInfo.TimeEnd;
            pDetailedResults[i].u16NbrWords =
                (u16) pAsrspiResult->pSentences[i].iNbrWords;
            pDetailedResults[i].pWords = pDetailedWords;
            for (; j < pDetailedResults[i].u16NbrWords;
                 j++) {
                pDetailedWords[j].TimeInfo.TimeBegin =
                    pAsrspiResult->pSentences[i].pWords[j].TimeInfo.TimeBegin;
                pDetailedWords[j].TimeInfo.TimeEnd =
                    pAsrspiResult->pSentences[i].pWords[j].TimeInfo.TimeEnd;
                pDetailedWords[j].u16Conf =
                    pAsrspiResult->pSentences[i].pWords[j].pAlternatives->iConf;
                pDetailedWords[j].WordId =
                    pAsrspiResult->pSentences[i].pWords[j].pAlternatives->idWord;
            }
            pDetailedWords += pDetailedResults[i].u16NbrWords;
        }
    }

    if (Error != 0) {
        ConfidenceThreshold = gGSAPI.DefaultParams.Params[GSAPI_PARAM_CONF_THRESHOLD];
    }
    if (pAsrspiResult->Type != 0 || pAsrspiResult->pSentences[0].iConf < (s32)ConfidenceThreshold) {
        if (gGSAPI.cbNotify != NULL) {
            gGSAPI.cbNotify(pEngine, GSAPI_NOTIFY_ASRRESULT, (u32)pEngine->pResult,
                            ConfidenceThreshold);
        }
        if (pDetailedResults != NULL) {
            heap_Free(gGSAPI.hHeap, pDetailedResults);
        }
        heap_Free(gGSAPI.hHeap, pEngine->pResult);
        if (pEngine->bManualStop == 0) {
            RestartEngine(pEngine);
        }
        pEngine->bInCallback = FALSE;
        return 0;
    }

    /* ContextGetAction initializes the output to zero; its status is ignored and zero is reported
     * when no action mapping supplies a value. */
    ContextGetAction(pEngine->pCurrentContext, pAsrspiResult, &Action);
    gGSAPI.cbResult(pEngine, Action, pEngine->pResult,
                    pEngine->u32EngineUserData);
    if (pDetailedResults != NULL) {
        heap_Free(gGSAPI.hHeap, pDetailedResults);
    }
    heap_Free(gGSAPI.hHeap, pEngine->pResult);
    if (pEngine->Mode & (1 << 1)) {
        RestartEngine(pEngine);
    }
    pEngine->bInCallback = FALSE;
    return 0;
}

/* Signal callbacks registered alongside the GSAPI result callback. */
struct _ASRBASE_SIGNALCALLBACKS AsrSpiSignalCallBacks = {
    asrspi_cbAbnorm, asrspi_cbSpeechDetected, asrspi_cbTsDetected,
    asrspi_cbAgc, asrspi_cbEnergyLevel, asrspi_cbSNRLevel,
};

/* Routes recognizer results and signals to GSAPI. */
struct _ASRSPI_RECOGCALLBACKS AsrSpiRecogCallBacks = {
    asrspi_cbResult, &AsrSpiSignalCallBacks,
};
