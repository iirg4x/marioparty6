/* Recognizer API, scheduler callbacks, context activation and session data. */
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "gssdk/secondpass.h"
#include "gssdk/heap.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/mathusage.h"
#include <string.h>

/* Selects the blocks that consume each label format. */
static const u8 ActiveBlocks_UTTFORMAT_LABELS[3] = {0x1F, 0x20, 0};
static const u8 ActiveBlocks_UTTFORMAT_LABELS_ISO[2] = {0x23, 0};

/* Validates the input and output queue counts requested for an architecture. */
static u8 cbInitArchitectureQueues(struct _TOS *pTos, u8 NrInputs, u8 NrOutputs)
{
    if ((NrInputs != 1) || (NrOutputs != 4)) {
        return 1;
    }
    return 0;
}

/* Stores the reader queue and index supplied by the TOS architecture callback. */
static u8 cbRegisterInputQueue(struct _TOS *pTos, struct _QUEUE *QuePtr, u16 ReadPos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    pEngine->LabelInQueue.pQ = QuePtr;
    pEngine->LabelInQueue.ReadPos = ReadPos;
    return 0;
}

/* Registers the queue in the first available output port. */
static u8 cbRegisterOutputQueue(struct _TOS *pTos, struct _QUEUE *QuePtr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    struct _QUEUE **slot;

    if (pEngine->SampleOutQueue.pQ == 0) {
        slot = &pEngine->SampleOutQueue.pQ;
    } else if (pEngine->SilOutQueue.pQ == 0) {
        slot = &pEngine->SilOutQueue.pQ;
    } else if (pEngine->CepstraOutQueue.pQ == 0) {
        slot = &pEngine->CepstraOutQueue.pQ;
    } else if (pEngine->LabelOutQueue.pQ == 0) {
        slot = &pEngine->LabelOutQueue.pQ;
    } else {
        return 1;
    }
    *slot = QuePtr;
    return 0;
}

/* Dispatches the recognizer state and records its first callback error. */
static u8 cbStateHandler(struct _TOS *pTos, u16 NewState)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    s32 error;

    if (pEngine->CRecogCallBacks.cbState != 0) {
        error = pEngine->CRecogCallBacks.cbState(
            NewState, pEngine->FrameCount * 0x6E, &pEngine->UserData);
        if (pEngine->CallbackErrLog == 0 && error != 0) {
            pEngine->CallbackErrLog = error;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return 0;
}

/* Dispatches a gain request and records its first callback error. */
unsigned char cbGainRequest(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    s32 error;

    if (pEngine->CRecogCallBacks.cbAgc != 0) {
        error = pEngine->CRecogCallBacks.cbAgc((u16)CParam1, &pEngine->UserData);
        if (pEngine->CallbackErrLog == 0 && error != 0) {
            pEngine->CallbackErrLog = error;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return 0;
}

/* Calls an optional gain callback and returns its status. */
static inline s32 CallGainCallback(struct _ENGINE *context)
{
    s32 result;

    result = context->CRecogCallBacks.cbAskCurrentGain(&context->UserData);
    return result;
}

/* Requests the current gain and reports the first callback error through TOS. */
unsigned char cbAskCurrentGain(struct _TOS *pTos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    if (pEngine->CRecogCallBacks.cbAskCurrentGain != 0) {
        s32 error = CallGainCallback(pEngine);

        if (pEngine->CallbackErrLog == 0 && error != 0) {
            pEngine->CallbackErrLog = error;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return 0;
}

/* Reports an abnormal condition through the recognizer callback. */
unsigned char cbTOSAbnormalCond(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    long CallbackErr;

    if (pEngine->CRecogCallBacks.cbAbnorm != 0) {
        if (CParam1 == 0x46) {
            CParam1 = GARBLEDSOUND;
        }
        CallbackErr = pEngine->CallbackErrLog = pEngine->CRecogCallBacks.cbAbnorm(
            (enum _ABNORMCOND)CParam1, CParam2 * 0x6E, &pEngine->UserData);
        if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
            pEngine->CallbackErrLog = CallbackErr;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return 0;
}

/* Maps the duration-normalized sentence and garbage score difference to confidence. */
long CalcSentenceConfidence(unsigned long SentenceDuration, unsigned long SentenceScore,
                            unsigned long SentenceGarbage)
{
    float Confidence = ((float)SentenceScore - (float)SentenceGarbage) / (float)SentenceDuration;
    if (Confidence < -26.0f)
        Confidence = 10000.0f / (1.0f + expf(0.027330978f + 0.08544444f * Confidence));
    else if (Confidence <= 78.0f)
        Confidence = 76.9f * (91.0f - Confidence);
    else
        Confidence = 10000.0f / (1.0f + expf(-4.4671087f + 0.08544444f * Confidence));
    return Confidence;
}

/* Computes rounded word confidence from the score and time-aligned garbage scores. */
static long CalcWordConfidence(struct _DYNPROG_RESULT *pRes, long Score,
                               unsigned long EntryTime, unsigned long ExitTime)
{
    long WordConfidence;
    float FltWordConfidence;
    float Temp;
    if (ExitTime == EntryTime) return 0;
    Temp = (float)Score - (float)pRes->pSingleWordGarbage[ExitTime]
           + (float)pRes->pSingleWordGarbage[EntryTime];
    FltWordConfidence = Temp / (float)(ExitTime - EntryTime);
    if (FltWordConfidence < -26.0f)
        FltWordConfidence =
            10000.0f / (1.0f + expf(0.027330978f + 0.08544444f * FltWordConfidence));
    else if (FltWordConfidence <= 78.0f)
        FltWordConfidence = 76.9f * (91.0f - FltWordConfidence);
    else
        FltWordConfidence = 10000.0f / (1.0f + expf(-4.4671087f + 0.08544444f * FltWordConfidence));
    WordConfidence = FltWordConfidence + 0.5;
    return WordConfidence;
}

/* Builds recognized sentences and their word confidence scores from the backtrace. */
static void casrCreateResult(struct _ENGINE *pEngine, struct _CASRRESULT **ppCasrResult,
    struct _DYNPROG_RESULT *pResult, unsigned char ConvertTimes)
{
    struct _DYNPROG_RESULT_NODE *pRes = pResult->pResult;
    struct _CASRRESULT *pCasrResult;
    struct _SENTENCE *pSentences;
    unsigned int BeginTimeOffset = *pEngine->pBeginTime;
    unsigned int NbrSentences = 0;
    struct _NEWID2OLDID *pConvertWordID = pEngine->CtxInfo.pNewWordID2OldWordID;
    struct _DICTSTATE *pTheStateDict;
    int NbrWordsFirst = 0;
    unsigned long MinTimeLS = 0xFFFFFFFFU;
    unsigned long MinScoreLS = 0xFFFFFFFFU;
    unsigned long MinTimeTS = 0xFFFFFFFFU;
    unsigned long MinScoreTS = 0xFFFFFFFFU;
    unsigned long *pConfInfo;
    unsigned long *pConfidenceInfo;
    unsigned long *pBeginOfWords;
    int WasSecondPass = 1;
    struct _DYNPROG_CTXINFO *pCtxInfo = pEngine->CtxInfo.pSecondPassDynProgCtxInfo;
    struct _ASRSPI_CONTEXT Ctx;
    int LangIsChinese;

    LangIsChinese = pEngine->pLang->langCheckBitField(pEngine->pLang, 8);
    FillContextV2VirtualTable(&Ctx);
    Ctx.pCtx = pEngine->ActivatedContext->pCtx;
    if (pCtxInfo == 0) {
        pCtxInfo = pEngine->CtxInfo.pDynProgCtxInfo;
        WasSecondPass = 0;
    }
    pTheStateDict = pCtxInfo->pStateInDict;
    pBeginOfWords = pCtxInfo->pBeginOfWords;
    *ppCasrResult = heap_Alloc(pEngine->hHeap, sizeof(struct _CASRRESULT));
    pCasrResult = *ppCasrResult;
    if (pCasrResult == 0) return;
    if (pEngine->ParamList.ArchiParam.TimeOut != 0 &&
        pEngine->FrameCount > pEngine->ParamList.ArchiParam.TimeOut)
        pCasrResult->Type = CASRRESULTTYPE_TIMEOUT;
    else if (pEngine->DTMFDetected != 0)
        pCasrResult->Type = CASRRESULTTYPE_DTMF;
    else {
        unsigned long ExtraEventScore = 0xFFFFFFFFU;
        unsigned long BestNormalScore = pRes != 0 ? pRes->Score : 0xFFFFFFFFU;
        if (_tosControl(&pEngine->Object, 0x63, (u32)&ExtraEventScore, 0) != 0) return;
        if (ExtraEventScore < BestNormalScore) pCasrResult->Type = CASRRESULTTYPE_REJECTED;
        else pCasrResult->Type = CASRRESULTTYPE_FINAL;
    }
    if (pRes == 0) {
        pCasrResult->iNbr = 0;
        pCasrResult->pSentences = 0;
        return;
    }
    while (pRes) {
        NbrSentences++;
        pRes = pRes->pNext;
    }
    pSentences = heap_Alloc(pEngine->hHeap, NbrSentences * sizeof(struct _SENTENCE));
    pConfInfo = pConfidenceInfo =
        heap_Alloc(pEngine->hHeap, NbrSentences * (3 * sizeof(unsigned long)));
    if (pSentences == 0 || pConfInfo == 0) {
        if (pConfInfo) heap_Free(pEngine->hHeap, pConfidenceInfo);
        else if (pSentences) heap_Free(pEngine->hHeap, pSentences);
        pCasrResult->iNbr = 0;
        pCasrResult->pSentences = 0;
        return;
    }
    pCasrResult->iNbr = NbrSentences;
    pCasrResult->pSentences = pSentences;
    for (pRes = pResult->pResult; pRes; pRes = pRes->pNext) {
        unsigned char First = 1;
        unsigned long ExitScore;
        struct _DYNPROG_RESULT_NODE *pTrans = pRes->pInfo;
        struct _DYNPROG_RESULT_NODE *pSent;
        struct _RECWORD *pWords;
        unsigned int NbrWords = 0;
        int NbrTerminalWords = 0;
        unsigned long NextBeginTime = 0;
        unsigned long NextBeginScore = 0;
        unsigned long AnySpeechTime = 0;
        unsigned long AnySpeechSentScore = 0;
        unsigned long AnySpeechGarbScore = 0;
        while (pTrans) {
            struct _DICTSTATE *pDict = pTheStateDict + pTrans->StateNbr;
            if (pDict->StateIndex & 1) {
                long Word = pDict[1].StateIndex + (pDict[2].StateIndex << 0x10);
                if (pConvertWordID == 0 ||
                    (pConvertWordID[Word].OldWordID != 0xFFFFFFFFU &&
                     pConvertWordID[Word].OldWordID != 0xFFFFFFFE && /* Skipped transition. */
                     pConvertWordID[Word].OldWordID != 0xFFFFFFFC)) { /* Skipped transition. */
                    NbrWords++;
                    if (pCtxInfo->pWordProps[Word] & 0x80) NbrTerminalWords++;
                }
            }
            pTrans = pTrans->pNext;
        }
        if (NbrWordsFirst == 0) NbrWordsFirst = NbrTerminalWords;
        pWords = heap_Alloc(pEngine->hHeap, NbrWords * sizeof(struct _RECWORD));
        if (pWords == 0) {
            if (pConfidenceInfo) heap_Free(pEngine->hHeap, pConfidenceInfo);
            return;
        }
        pSentences->iNbrWords = NbrWords;
        pSentences->lScore = pRes->Score - NbrWordsFirst * pEngine->WordPenalty;
        pSentences->TimeInfo.TimeBegin = 0xFFFFFFFFU;
        pSentences->pWords = pWords;
        pTrans = pRes->pInfo;
        pSent = pTrans;
        while (pSent) {
            struct _DICTSTATE *pDict = pTheStateDict + pTrans->StateNbr;
            struct _DICTSTATE *pTransDict;
            struct _DICTSTATE *pEndTransDict;
            struct _DYNPROG_RESULT_NODE *pEndTrans;
            struct _ALTERNATIVE *pAlternatives;
            long Word = pDict[1].StateIndex + (pDict[2].StateIndex << 0x10);
            unsigned long ExitTime;
            unsigned long BeginTime = NextBeginTime;
            unsigned long BeginScore = NextBeginScore;
            pSent = pTrans->pNext;
            if (pSent && WasSecondPass && !LangIsChinese) {
                pTransDict = pTheStateDict + pSent->StateNbr;
                if (pTransDict->StateIndex & 1) {
                    unsigned int TransState;
                    unsigned int PrevState;
                    unsigned int NextState;
                    struct _SECONDPASSINFO Info;
                    long NextWord;
                    unsigned char LC;
                    unsigned char RC;
                    pEndTrans = pSent->pNext;
                    pEndTransDict = pTheStateDict + pEndTrans->StateNbr;
                    NextWord = pEndTransDict[1].StateIndex + (pEndTransDict[2].StateIndex << 0x10);
                    if (GetSecondPassInfo(pEngine, &Ctx, &Info,
                        (pEngine->CtxInfo.pNewWordID2OldWordID)[NextWord].OldPronID) != 0) {
                        if (pConfidenceInfo) heap_Free(pEngine->hHeap, pConfidenceInfo);
                        return;
                    }
                    LC = Info.LeftSpeechUnitClass;
                    if (GetSecondPassInfo(pEngine, &Ctx, &Info,
                        (pEngine->CtxInfo.pNewWordID2OldWordID)[Word].OldPronID) != 0) {
                        if (pConfidenceInfo) heap_Free(pEngine->hHeap, pConfidenceInfo);
                        return;
                    }
                    RC = Info.RightSpeechUnitClass;
                    if (LC) {
                        ExitScore = pTrans->Score;
                        ExitTime = pTrans->ExitTime;
                        NextBeginScore = ExitScore;
                        NextBeginTime = ExitTime;
                    } else if (RC) {
                        ExitScore = pSent->Score;
                        ExitTime = pSent->ExitTime;
                        NextBeginScore = ExitScore;
                        NextBeginTime = ExitTime;
                    } else {
                        TransState = pTransDict->StateIndex >> 2;
                        PrevState = pDict->StateIndex >> 2;
                        if (TransState == PrevState) {
                            ExitScore = pSent->Score;
                            ExitTime = pSent->ExitTime;
                            NextBeginScore = ExitScore;
                            NextBeginTime = ExitTime;
                        } else if (TransState == (NextState =
                                       pTheStateDict[pBeginOfWords[pBeginOfWords[NextWord]]]
                                           .StateIndex >> 2)) {
                            ExitScore = pTrans->Score;
                            ExitTime = pTrans->ExitTime;
                            NextBeginScore = ExitScore;
                            NextBeginTime = ExitTime;
                        } else {
                            ExitScore = pTrans->Score;
                            ExitTime = pTrans->ExitTime;
                            NextBeginScore = pSent->Score;
                            NextBeginTime = pSent->ExitTime;
                        }
                    }
                    pTrans = pSent->pNext;
                } else {
                    ExitScore = pSent->Score;
                    ExitTime = pSent->ExitTime;
                    pEndTrans = pSent->pNext;
                    pEndTransDict = pTheStateDict + pEndTrans->StateNbr;
                    if (pEndTransDict->StateIndex & 1) {
                        NextBeginScore = pSent->Score;
                        NextBeginTime = pSent->ExitTime;
                        pTrans = pEndTrans->pNext;
                    } else {
                        NextBeginScore = pEndTrans->Score;
                        NextBeginTime = pEndTrans->ExitTime;
                        pTrans = pEndTrans->pNext->pNext;
                    }
                }
            } else {
                if (LangIsChinese && pConvertWordID) {
                    /* This transition leaves the next word's start unchanged. */
                    if (pConvertWordID[Word].OldWordID == 0xFFFFFFFC) {
                        pTrans = pSent;
                        continue;
                    }
                    /* This transition advances the next word's start. */
                    if (pConvertWordID[Word].OldWordID == 0xFFFFFFFE) {
                        NextBeginTime = pTrans->ExitTime;
                        NextBeginScore = pTrans->Score;
                        pTrans = pSent;
                        continue;
                    }
                }
                ExitTime = pTrans->ExitTime;
                ExitScore = pTrans->Score;
                NextBeginTime = ExitTime;
                NextBeginScore = ExitScore;
                if (pTrans->pNext == 0) {
                    unsigned long WordLength = ExitTime - BeginTime;
                    if (WordLength < MinTimeTS) {
                        MinTimeTS = WordLength;
                        MinScoreTS = ExitScore - BeginScore;
                    }
                }
                pTrans = pSent;
            }
            if (First) {
                First = 0;
                if (ExitTime < MinTimeLS) {
                    MinTimeLS = ExitTime;
                    MinScoreLS = ExitScore;
                }
            }
            if ((pDict->StateIndex >> 2) == pEngine->AnySpeechPhenom) {
                AnySpeechTime += ExitTime - BeginTime;
                AnySpeechSentScore += ExitScore - BeginScore;
                AnySpeechGarbScore += pResult->pSentenceGarbage[ExitTime]
                                      - pResult->pSentenceGarbage[BeginTime];
            }
            pAlternatives = heap_Alloc(pEngine->hHeap, sizeof(struct _ALTERNATIVE));
            if (pAlternatives == 0) {
                if (pConfidenceInfo) heap_Free(pEngine->hHeap, pConfidenceInfo);
                return;
            }
            if (pSentences->TimeInfo.TimeBegin == 0xFFFFFFFFU) {
                pSentences->TimeInfo.TimeBegin = BeginTime;
            }
            pWords->iNbrAlternatives = 1;
            pWords->TimeInfo.TimeBegin = BeginTime;
            pWords->TimeInfo.TimeEnd = ExitTime;
            pWords->pAlternatives = pAlternatives;
            pAlternatives->iConf =
                CalcWordConfidence(pResult, ExitScore - BeginScore, BeginTime, ExitTime);
            if (pCtxInfo->pWordProps[Word] & 0x80)
                pAlternatives->lScore = ExitScore - BeginScore - pEngine->WordPenalty;
            else pAlternatives->lScore = ExitScore - BeginScore;
            if (pConvertWordID) pAlternatives->idWord = pConvertWordID[Word].OldWordID;
            else pAlternatives->idWord = Word;
            pWords++;
        }
        *pConfInfo++ = NextBeginScore - AnySpeechSentScore - NbrWordsFirst * pEngine->WordPenalty;
        *pConfInfo++ = NextBeginTime - AnySpeechTime;
        *pConfInfo++ = pResult->pSentenceGarbage[NextBeginTime] - AnySpeechGarbScore;
        pSentences->TimeInfo.TimeEnd = NextBeginTime;
        pSentences++;
    }
    {
        int i;
        int NrSentences = pCasrResult->iNbr;
        struct _SENTENCE *pSent = pCasrResult->pSentences;
        pConfInfo = pConfidenceInfo;
        for (i = 0; i < NrSentences; i++) {
            unsigned long SentenceScore = *pConfInfo++;
            unsigned long SentenceDuration = *pConfInfo++;
            unsigned long SentenceGarbage = *pConfInfo++;
            SentenceScore -= MinScoreLS + MinScoreTS;
            SentenceGarbage -= pResult->pSentenceGarbage[MinTimeLS] +
                (pResult->pSentenceGarbage[pSent->TimeInfo.TimeEnd] -
                 pResult->pSentenceGarbage[pSent->TimeInfo.TimeEnd - MinTimeTS]);
            SentenceDuration -= MinTimeTS + MinTimeLS;
            pSent->iConf = CalcSentenceConfidence(SentenceDuration, SentenceScore, SentenceGarbage);
            pSent++;
        }
    }
    heap_Free(pEngine->hHeap, pConfidenceInfo);
    if (ConvertTimes) {
        int i;
        int NrSentences = pCasrResult->iNbr;
        struct _SENTENCE *pSent = pCasrResult->pSentences;
        for (i = 0; i < NrSentences; i++) {
            int j;
            int NbrWords = pSent->iNbrWords;
            struct _RECWORD *pWords = pSent->pWords;
            for (j = NbrWords; j > 0; j--) {
                pWords->TimeInfo.TimeBegin = (BeginTimeOffset + pWords->TimeInfo.TimeBegin) * 0x6E;
                pWords->TimeInfo.TimeEnd = (BeginTimeOffset + pWords->TimeInfo.TimeEnd) * 0x6E;
                pWords++;
            }
            pSent->TimeInfo.TimeBegin = (BeginTimeOffset + pSent->TimeInfo.TimeBegin) * 0x6E;
            pSent->TimeInfo.TimeEnd = (BeginTimeOffset + pSent->TimeInfo.TimeEnd) * 0x6E;
            pSent++;
        }
    }
}

/* Releases the word buffers, records, and result allocated for recognition. */
static void casrFreeResult(struct _ENGINE *pEngine, struct _CASRRESULT *pCasrResult)
{
    struct _SENTENCE *pSentences = pCasrResult->pSentences;
    struct _RECWORD *pWords;
    s32 i = 0;

    while (i < pCasrResult->iNbr) {
        s32 j = 0;

        pWords = pSentences->pWords;

        while (j < pSentences->iNbrWords) {
            heap_Free(pEngine->hHeap, pWords->pAlternatives);
            j++;
            pWords++;
        }
        heap_Free(pEngine->hHeap, pSentences->pWords);
        i++;
        pSentences++;
    }
    if (i != 0) {
        heap_Free(pEngine->hHeap, pCasrResult->pSentences);
    }
    heap_Free(pEngine->hHeap, pCasrResult);
}

/* Reports the detected time in recognizer time units and clears the active block selection. */
unsigned char cbTSDetected(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    long CallbackErr;

    pEngine->TSDetected = 1;
    CallbackErr = pEngine->CRecogCallBacks.cbTSDetected(CParam1 * 0x6E, &pEngine->UserData);
    if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
        pEngine->CallbackErrLog = CallbackErr;
        _tosErrorLog(&pEngine->Object, 0x1B);
    }
    pEngine->pActiveBlocks = 0;
    return 0;
}

/* Reports the detected time relative to the saved start frame. */
unsigned char cbTSAfterStartDetected(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;

    return cbTSDetected(pTos, BlockID, ControlID, CParam1 + *pEngine->pBeginTime, CParam2);
}

/* Marks the detected start and reports callback failures through TOS. */
unsigned char cbStartDetected(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    s32 error;

    pEngine->StartDetected = 1;
    error = pEngine->CRecogCallBacks.cbStartDetected(
        (CParam2 - CParam1) * 0x6E, &pEngine->UserData);
    if (pEngine->CallbackErrLog == 0 && error != 0) {
        pEngine->CallbackErrLog = error;
        _tosErrorLog(&pEngine->Object, 0x1B);
    }
    return 0;
}

/* Creates a recognition result, reports callback errors, and releases the result. */
unsigned char cbTOSRecognised(
    struct _TOS *pTos, unsigned char BlockID, long ControlID, unsigned long CParam1)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    struct _CASRRESULT *pCasrResult;
    s32 CallbackErr;

    casrCreateResult(pEngine, &pCasrResult,
                     (struct _DYNPROG_RESULT *)CParam1, 1);
    CallbackErr = pEngine->CRecogCallBacks.cbResult(pCasrResult, &pEngine->UserData);
    if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
        pEngine->CallbackErrLog = CallbackErr;
        _tosErrorLog(&pEngine->Object, 0x1B);
    }
    casrFreeResult(pEngine, pCasrResult);
    return 0;
}

/* Builds the isolated-word result and reports it through the recognition callback. */
unsigned char cbIsoWordRecognised(
    struct _TOS *pTos, long BlockID, long ControlID, unsigned long CParam1, unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    struct _ASRSPI_CONTEXT Ctx;
    int i;
    int NBest = CParam2;
    unsigned int BeginTimeOffset = *pEngine->pBeginTime;
    struct _DYNPROG_RESULT *pRecogResult = (struct _DYNPROG_RESULT *)CParam1;
    struct _WORDRESULT *pResult = (struct _WORDRESULT *)pRecogResult->pResult;
    struct _CASRRESULT *pCasrResult;
    unsigned long GarbScore;
    unsigned long BeginTime;
    unsigned long EndTime;
    long CallbackErr;

    pCasrResult = heap_Alloc(pEngine->hHeap, sizeof(*pCasrResult));
    if (pCasrResult == 0) return 1;
    pCasrResult->iNbr = NBest;
    if (NBest != 0)
        pCasrResult->pSentences =
            heap_Alloc(pEngine->hHeap, NBest * sizeof(*pCasrResult->pSentences));
    else pCasrResult->pSentences = 0;
    FillContextV2VirtualTable(&Ctx);
    Ctx.pCtx = pEngine->ActivatedContext->pCtx;
    BeginTime = BeginTimeOffset;
    EndTime = pEngine->FrameCount;
    GarbScore = pRecogResult->pSentenceGarbage[EndTime - BeginTime];
    i = 0;
    while (i < NBest) {
        long WordID = pResult->WordID;
        long Score = pResult->Score;
        pResult++;
        pCasrResult->pSentences[i].iConf = CalcSentenceConfidence(
            EndTime - BeginTime, Score - pEngine->WordPenalty, GarbScore);
        pCasrResult->pSentences[i].lScore = Score;
        pCasrResult->pSentences[i].TimeInfo.TimeBegin = BeginTime * 0x6E;
        pCasrResult->pSentences[i].TimeInfo.TimeEnd = EndTime * 0x6E;

        if (WordID < 0) {
            unsigned long Index;
            unsigned long NbrWords;
            unsigned long j;
            struct _ISOWORD_BACKTRACE *pArr = pEngine->pBacktraceArray;
            WordID = -1 - WordID;
            Index = WordID;
            NbrWords = 0;
            while (Index != 0xFFFFFFFFU) {
                Index = pArr[Index].u32PrevWordIdx;
                NbrWords++;
            }
            pCasrResult->pSentences[i].pWords = heap_Alloc(
                pEngine->hHeap,
                NbrWords * sizeof(*pCasrResult->pSentences[i].pWords));
            pCasrResult->pSentences[i].iNbrWords = NbrWords;
            j = NbrWords - 1;
            Index = WordID;
            while (Index != 0xFFFFFFFFU) {
                pCasrResult->pSentences[i].pWords[j].pAlternatives =
                    heap_Alloc(pEngine->hHeap,
                        sizeof(*pCasrResult->pSentences[i].pWords[j].pAlternatives));
                pCasrResult->pSentences[i].pWords[j].iNbrAlternatives = 1;
                pCasrResult->pSentences[i].pWords[j].TimeInfo.TimeBegin = -1;
                pCasrResult->pSentences[i].pWords[j].TimeInfo.TimeEnd = -1;
                pCasrResult->pSentences[i].pWords[j].pAlternatives->idWord = pArr[Index].s32WordID;
                pCasrResult->pSentences[i].pWords[j].pAlternatives->iConf = 0;
                pCasrResult->pSentences[i].pWords[j].pAlternatives->lScore = -1;
                Index = pArr[Index].u32PrevWordIdx;
                j--;
            }
        } else {
            pCasrResult->pSentences[i].pWords = heap_Alloc(
                pEngine->hHeap, sizeof(*pCasrResult->pSentences[i].pWords));
            pCasrResult->pSentences[i].iNbrWords = 1;
            pCasrResult->pSentences[i].pWords[0].pAlternatives = heap_Alloc(
                pEngine->hHeap,
                sizeof(*pCasrResult->pSentences[i].pWords[0].pAlternatives));
            pCasrResult->pSentences[i].pWords[0].iNbrAlternatives = 1;
            pCasrResult->pSentences[i].pWords[0].TimeInfo.TimeBegin = BeginTime * 0x6E;
            pCasrResult->pSentences[i].pWords[0].TimeInfo.TimeEnd =
                (BeginTimeOffset + pEngine->FrameCount) * 0x6E;
            pCasrResult->pSentences[i].pWords[0].pAlternatives->idWord = WordID;
            pCasrResult->pSentences[i].pWords[0].pAlternatives->iConf = 0;
            pCasrResult->pSentences[i].pWords[0].pAlternatives->lScore = Score;
        }
        i++;
    }

    if (pEngine->ParamList.ArchiParam.TimeOut != 0 &&
        pEngine->FrameCount > pEngine->ParamList.ArchiParam.TimeOut) {
        pCasrResult->Type = CASRRESULTTYPE_TIMEOUT;
    } else {
        unsigned long ExtraEventScore;
        unsigned long BestNormalScore;

        ExtraEventScore = BestNormalScore = 0xFFFFFFFFU;
        if (pRecogResult->pResult != 0) {
            BestNormalScore = ((struct _WORDRESULT *)pRecogResult->pResult)->Score;
        }
        if (_tosControl(&pEngine->Object, 0x63,
                (u32)&ExtraEventScore, 0) != 0) {
            return 1;
        }
        if (ExtraEventScore < BestNormalScore) {
            pCasrResult->Type = CASRRESULTTYPE_REJECTED;
        } else {
            pCasrResult->Type = CASRRESULTTYPE_FINAL;
        }
    }

    {
        CallbackErr = pEngine->CRecogCallBacks.cbResult(
            pCasrResult, &pEngine->UserData);

        casrFreeResult(pEngine, pCasrResult);
        if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
            pEngine->CallbackErrLog = CallbackErr;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return 0;
}

/* Invokes the TOS recognition callback with its result-argument record. */
static inline s32 CallTOSRecognisedCallback(
    struct _ENGINE *context, struct _CASRRESULT *pCasrResult)
{
    s32 result;

    result = context->CRecogCallBacks.cbResult(
        pCasrResult, &context->UserData);
    return result;
}

/* Stores the first callback failure and reports it to the TOS error log. */
static inline void RecordCallbackError(struct _ENGINE *context, s32 error)
{
    s32 callbackError = context->CallbackErrLog;

    if (callbackError == 0 && error != 0) {
        context->CallbackErrLog = error;
        _tosErrorLog(&context->Object, 0x1B);
    }
}

/* Saves the spelling pass result for the engine. */
unsigned char cbSpellRecognised(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    pEngine->pRes = (struct _DYNPROG_RESULT *)CParam1;
    return 0;
}

/* Reports an empty final or timeout result through the recognition callback. */
unsigned char cbNothingRecognised(struct _TOS *pTos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    struct _CASRRESULT CasrResult;
    s32 error;

    CasrResult.iNbr = 0;
    CasrResult.pSentences = 0;
    if (pEngine->ParamList.ArchiParam.TimeOut != 0 &&
        pEngine->FrameCount > pEngine->ParamList.ArchiParam.TimeOut) {
        CasrResult.Type = CASRRESULTTYPE_TIMEOUT;
    } else {
        CasrResult.Type = CASRRESULTTYPE_FINAL;
    }
    error = CallTOSRecognisedCallback(pEngine, &CasrResult);
    RecordCallbackError(pEngine, error);
    return 0;
}

/* Reports an empty final or timeout result from the spelling pass. */
unsigned char cbSpellNothingRecognised(struct _TOS *pTos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    struct _CASRRESULT CasrResult;
    s32 error;

    CasrResult.iNbr = 0;
    CasrResult.pSentences = 0;
    if (pEngine->ParamList.ArchiParam.TimeOut != 0 &&
        pEngine->FrameCount > pEngine->ParamList.ArchiParam.TimeOut) {
        CasrResult.Type = CASRRESULTTYPE_TIMEOUT;
    } else {
        CasrResult.Type = CASRRESULTTYPE_FINAL;
    }
    error = CallTOSRecognisedCallback(pEngine, &CasrResult);
    RecordCallbackError(pEngine, error);
    return 0;
}

/* Schedules a second pass when enabled, or reports the first-pass result. */
unsigned char cbDoSecondPassIfEnabled(
    struct _TOS *pTos, long BlockID, long ControlID, unsigned long CParam1, unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    u32 SecondPassEnable;
    u8 RetVal;

    if (casrGetParam((struct HCASR__ *)pEngine, 15, &SecondPassEnable) != 0) {
        return 1;
    }
    if (SecondPassEnable != 0) {
        if (_tosControl(&pEngine->Object, 2, 0, 0) != 0) {
            return 1;
        }
        pEngine->DoSecondPass = 1;
        pEngine->NBest = CParam2;
        pEngine->pRes = (struct _DYNPROG_RESULT *)CParam1;
        RetVal = 0;
    } else {
        if (_tosControl(&pEngine->Object, 7, 0, 0) != 0) {
            return 1;
        }
        if (pEngine->IsActiveContextIsoWord != 0) {
            RetVal =
                cbIsoWordRecognised(pTos, BlockID, ControlID, CParam1, CParam2);
        } else {
            RetVal = cbTOSRecognised(pTos, BlockID, ControlID, CParam1);
        }
    }
    if (RetVal == 1) {
        return 1;
    }
    return 0;
}

/* Delivers the training prompt and records the first callback error. */
static unsigned char cbTrain(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    long error = pEngine->CRecogCallBacks.cbTrain(CParam1, &pEngine->UserData);

    if (pEngine->CallbackErrLog == 0 && error != 0) {
        pEngine->CallbackErrLog = error;
        _tosErrorLog(&pEngine->Object, 0x1B);
    }
    return 0;
}

/* Resets the recognition state fields used when a new user word is reported. */
unsigned char cbNewUserWordUtterance(struct _TOS *pTos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    pEngine->FrameCount = 0;
    pEngine->SampleBufSize = 0;
    pEngine->CSampleFormat = UTTFORMAT_UNKNOWN;
    pEngine->TSDetected = 0;
    pEngine->DTMFDetected = 0;
    return 0;
}

/* Clears the indexed gender entry and enables the requested gender mode. */
unsigned char cbGenderDisable(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    pEngine->GenderActive[CParam1] = 0;
    switch (CParam2) {
    case 0:
        pEngine->ParamList.GenderParam.EnabledGendersMask = 1;
        break;
    case 1:
        pEngine->ParamList.GenderParam.EnabledGendersMask = 2;
        break;
    case 2:
        pEngine->ParamList.GenderParam.EnabledGendersMask = 4;
        break;
    default:
        return 1;
    }
    return 0;
}

/* Initializes the engine's parameter-list defaults. */
static u8 InitParamList(struct _ENGINE *pEngine);

/* Returns the public error code when a recognition context is invalid. */
static unsigned long casrCheckHandle(struct _ENGINE *pEngine)
{
    if (pEngine == 0) {
        return 6;
    }
    if (pEngine->ValidEngine != 0x218DF4F5) { /* Marker set by casrOpen. */
        return 0xB;
    }
    return 0;
}

/* Applies gender controls supported by the recognizer's language. */
static u8 casrDisableGenderInput(struct _ENGINE *pEngine,
    struct _WARPINFO *pWarpFactors, u32 Gender, u8 Active)
{
    static const u8 ControlID_Disable[3] = { 0x6C, 0x6D, 0x6E };
    u32 *pWarpFactor = (u32 *)&pWarpFactors[Gender].WarpingFactor;
    u32 WarpFactor = *pWarpFactor;
    if (_tosControl(&pEngine->Object, ControlID_Disable[Gender],
            Gender, WarpFactor) != 0)
        return 1;
    pEngine->GenderActive[Gender] = Active;
    return 0;
}

/* Enables or disables the indexed gender using the language's warping factor. */
static u8 casrEnableGender(struct _ENGINE *pEngine,
    struct _WARPINFO *pWarpFactors, u32 Gender, u8 Active)
{
    static const u8 ControlID_Enable[3] = { 0x40, 0x41, 0x42 };
    static const u8 ControlID_Disable[3] = { 0x3D, 0x3E, 0x3F };
    const u8 *pControl;
    u32 *pWarpFactor;
    u32 WarpFactor;
    if (Active != 0)
        pControl = ControlID_Enable;
    else
        pControl = ControlID_Disable;
    pWarpFactor = (u32 *)&pWarpFactors[Gender].WarpingFactor;
    WarpFactor = *pWarpFactor;
    if (_tosControl(&pEngine->Object, pControl[Gender], Gender, WarpFactor) != 0)
        return 1;
    pEngine->GenderActive[Gender] = Active;
    return 0;
}

/* Activates the requested gender models supported by the loaded language. */
static u32 casrEnableGenders(struct _ENGINE *pEngine, u32 Active)
{
    struct _WARPINFO *pWarpFactors;
    u32 NbrGenders;
    u32 GenderMask = 0;
    u32 i;
    u8 StateChange[3] = {0, 0, 0};
    if (pEngine->pLang == 0) {
        return 0;
    }
    NbrGenders = pEngine->pLang->langGetNbrWarpFactors(pEngine->pLang);
    pWarpFactors = pEngine->pLang->langGetpWarpFactors(pEngine->pLang);
    for (i = 0; i < NbrGenders; i++) {
        GenderMask |= pWarpFactors[i].GenderMask;
    }
    if ((Active & GenderMask) == 0) {
        return 1;
    }
    /* Accept all-gender masks with or without the high bit set. */
    if ((Active != 0x7FFFFFFFU) && (Active != 0xFFFFFFFFU) &&
        (Active & ~(GenderMask | 0x80000000U)) != 0) {
        return 1;
    }
    for (i = 0; i < 3; i++) {
        u8 ThisActive = 0;
        if ((Active & pWarpFactors[i].GenderMask) != 0) {
            ThisActive = 1;
        }

        if (ThisActive != pEngine->GenderActive[i]) {
            StateChange[i] = 1;
            if (casrEnableGender(pEngine, pWarpFactors, i, ThisActive) != 0)
                return 1;
        }
    }
    for (i = 0; i < 3; i++) {
        u8 ThisActive = 0;
        if ((Active & pWarpFactors[i].GenderMask) != 0) {
            ThisActive = 1;
        }

        if ((ThisActive == 0) && StateChange[i] != 0) {
            if (casrDisableGenderInput(pEngine, pWarpFactors, i, ThisActive) != 0)
                return 1;
        }
    }
    return 0;
}

/* Returns and clears the TOS error after validating the recognizer context. */
unsigned long casrGetLastError(struct HCASR__ *hcAsr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u32 ErrorID;

    if (pEngine != 0) {
        ErrorID = casrCheckHandle(pEngine);
        if (ErrorID == 0) {
            u32 *lastError = &pEngine->Tos.ErrorLog;
            ErrorID = *lastError;
            *lastError = 0;
            return ErrorID;
        }
    }
    return 0xB;
}

/* Opens and initializes a recognizer instance with its caller callbacks. */
struct HCASR__ *casrOpen(
    void *hHeap, const struct _CRECOGCALLBACKS *pRecogCallBacks, unsigned long dwUserData)
{
    struct _TOSDATA TosData = {
        cbStateHandler, cbInitArchitectureQueues,
        cbRegisterInputQueue, cbRegisterOutputQueue, 0, 0
    };
    struct _ENGINE *pEngine;
    unsigned long i;

    if (pRecogCallBacks == 0) {
        return 0;
    }
    pEngine = heap_Calloc(hHeap, 1, sizeof(struct _ENGINE));
    if (pEngine == 0) {
        return 0;
    }
    pEngine->hHeap = hHeap;
    pEngine->Object.pTos = &pEngine->Tos;
    pEngine->Object.BlockID = 0;
    pEngine->CRecogCallBacks = *pRecogCallBacks;
    pEngine->UserData.dwSPIUserData = 0;
    if (tosInit(&pEngine->Tos, hHeap, &TosData, IniFile,
            InitialProcs, QueueWriters, QueueReaders, ControlTable, 0, 0) != 0) {
        goto failed;
    }
    pEngine->cbTrain = cbTrain;
    pEngine->DoPrompt = PROMPTTYPE_NOPROMPT;
    pEngine->NbrUttForTrain = 3;
    pEngine->CurrUttIdx = 0;
    pEngine->ppUserwordSegmentation = heap_Alloc(pEngine->hHeap,
        10 * sizeof(*pEngine->ppUserwordSegmentation));
    if (pEngine->ppUserwordSegmentation == 0) {
        pEngine->Tos.ErrorLog = 2;
        goto failed;
    }
    pEngine->ppUserwordSegmentation[0] = 0;
    pEngine->Epsilon = 0.0001f;
    if (_tosControl(&pEngine->Object, 4,
            (u32)&pEngine->pSignalLevel, 0) != 0) {
        goto failed;
    }
    if (_tosControl(&pEngine->Object, 0x2A,
            (u32)&pEngine->pBeginTime, 0) != 0) {
        goto failed;
    }
    if (InitParamList(pEngine) != 0) {
        goto failed;
    }
    pEngine->UserData.dwEngineUserData = dwUserData;
    pEngine->ValidEngine = 0x218DF4F5; /* Marks an initialized CASR engine. */
    for (i = 0; i < 3; i++) {
        pEngine->GenderActive[i] = 0;
    }
    return (struct HCASR__ *)pEngine;

 failed:
    heap_Free(pEngine->hHeap, pEngine);
    return 0;
}

/* Header of a CASR session stream. */
typedef struct {
    unsigned long DataType; /* Selects the session data layout. */
    unsigned long Size; /* Byte extent of the session data. */
} CASRSessionHeader;

/* Input configuration stored after the CASR session header. */
typedef struct {
    unsigned long GenderActive[0x3]; /* Stored state of the three gender inputs. */
    unsigned long CSampleFormat; /* Sample format used when saving the session. */
} CASRSessionFormat;

/* Returns the selected session data for the recognition engine. */
u8 casrGetSessionData(struct _ENGINE *pEngine, void *ppBuf, unsigned long *pSize)
{
    unsigned int Size = 0;
    CASRSessionHeader *pSessionInfo;
    CASRSessionFormat *pExtraSessionInfo;
    unsigned char *pData;
    unsigned long i;

    if (_tosControl(&pEngine->Object, 0x27, 0, (u32)&Size) != 0)
        return 1;
    Size += sizeof(*pSessionInfo) + sizeof(*pExtraSessionInfo) +
        sizeof(pEngine->ActivatedLanguage->pData->VersionInfo);
    pSessionInfo = heap_Alloc(pEngine->hHeap, Size);
    if (pSessionInfo == 0)
        return 1;
    pSessionInfo->Size = Size;
    pSessionInfo->DataType = 4;
    pExtraSessionInfo = (void *)(pSessionInfo + 1);
    pData = (void *)(pExtraSessionInfo + 1);
    for (i = 0; i < 0x3; i++)
        pExtraSessionInfo->GenderActive[i] = pEngine->GenderActive[i];
    pExtraSessionInfo->CSampleFormat = pEngine->CSampleFormat;
    memcpy(pData, pEngine->ActivatedLanguage->pData->VersionInfo,
           sizeof(pEngine->ActivatedLanguage->pData->VersionInfo));
    pData += sizeof(pEngine->ActivatedLanguage->pData->VersionInfo);
    Size = 0;
    if (_tosControl(&pEngine->Object, 0x27, (u32)pData, (u32)&Size) != 0) {
        if (pSessionInfo != 0)
            heap_Free(pEngine->hHeap, pSessionInfo);
        return 1;
    }
    *pSize = Size + sizeof(*pSessionInfo) + sizeof(*pExtraSessionInfo) +
        sizeof(pEngine->ActivatedLanguage->pData->VersionInfo);
    *(void **)ppBuf = pSessionInfo;
    return 0;
}

/* Releases the active recognizer context and its heap-owned resources. */
int casrClose(struct HCASR__ *hcAsr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    struct _TOS *pTos = &pEngine->Tos;
    unsigned long Err = 0;
    u16 ActiveState;
    s32 i;

    pTos->ErrorLog = Err;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) {
        return Err;
    }
    ActiveState = pTos->ActiveState;
    if ((ActiveState != CASR_BOOT) && (ActiveState != CASR_DATAREADY) &&
        (ActiveState != CASR_UWTRAINED) && (ActiveState != CASR_IDLE)) {
        return (u8)_tosErrorLog(&pEngine->Object, 7);
    }
    if (pEngine->ActivatedLanguage != 0) {
        heap_Free(pEngine->hHeap, pEngine->pLang);
        pEngine->pLang = 0;
    }
    if (pEngine->ActivatedContext != 0) {
        if (_tosControl(&pEngine->Object, 0x29, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
        FreeCtxInfo(pEngine);
    }
    for (i = 0; i < 3; i++) {
        if (pEngine->UtteranceLabels[i] != 0) {
            heap_Free(pEngine->hHeap, pEngine->UtteranceLabels[i]);
        }
    }
    if (pEngine->ppUserwordSegmentation[0] != 0) {
        heap_Free(pEngine->hHeap, pEngine->ppUserwordSegmentation[0]);
    }
    heap_Free(pEngine->hHeap, pEngine->ppUserwordSegmentation);
    if (tosDestruct(pTos) != 0) {
        return 1;
    }
    pEngine->ValidEngine = 0;
    heap_Free(pEngine->hHeap, pEngine);
    return 0;
}

/* Starts recognition after selecting its input and gender configuration. */
int casrStart(struct HCASR__ *hcAsr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    struct _TOS *pTos = &pEngine->Tos;
    unsigned long Err;

    pTos->ErrorLog = 0;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) {
        return Err;
    }
    if (pTos->ActiveState != CASR_IDLE) {
        return (u8)_tosErrorLog(&pEngine->Object, 7);
    }
    pEngine->pWriteLab[0] = pEngine->UtteranceLabels[0];
    pEngine->FrameCount = 0;
    pEngine->TSDetected = 0;
    pEngine->DTMFDetected = 0;
    pEngine->DoSecondPass = 0;
    pEngine->SampleBufSize = 0;
    pEngine->CurrUttIdx = 0;
    if (casrEnableGenders(pEngine, pEngine->ParamList.GenderParam.EnabledGendersMask) != 0) {
        return (u8)_tosErrorLog(&pEngine->Object, 0x14);
    }
    pEngine->CSampleFormat = UTTFORMAT_UNKNOWN;
    if (pEngine->IsActiveContextIsoWord != 0) {
        if (_tosControl(&pEngine->Object, 0x5A, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
        if (_tosControl(&pEngine->Object, 8, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
    } else {
        if (_tosControl(&pEngine->Object, 0x57, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
        if (pEngine->IsActiveContextSpelling != 0) {
            if (_tosControl(&pEngine->Object, 6, 0, 0) != 0) {
                return (u8)pTos->ErrorLog;
            }
        } else if (_tosControl(&pEngine->Object, 1, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
    }
    pEngine->StartDetected = 0;
    if (pEngine->ParamList.TriggerParam.DoStartDetection == 0) {
        if (_tosControl(&pEngine->Object, 2, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
        pEngine->StartDetected = 1;
    }
    return (u8)pTos->ErrorLog;
}

/* Enables label delivery by changing the registered queue's control mode. */
unsigned char casrEnableLabelInQueue(struct _ENGINE *pEngine, unsigned char Enable)
{
    if (Enable != 0) {
        u8 result = qQueueControl(pEngine->LabelInQueue.pQ, 5, pEngine->LabelInQueue.ReadPos, 0);
        return !result;
    }
    {
        u8 result = qQueueControl(pEngine->LabelInQueue.pQ, 8, pEngine->LabelInQueue.ReadPos, 0);
        return !result;
    }
}

/* Copies each queued label into its recognition buffer, growing the buffer as needed. */
static u8 casrReadLabelInQueue(struct _ENGINE *pEngine, int UttIdx)
{
    struct _QUEUE *pQ = pEngine->LabelInQueue.pQ;
    unsigned long LabelInQueReadPos = pEngine->LabelInQueue.ReadPos;
    unsigned char *pSrc;
    unsigned char *pWriteLab;
    unsigned char *pNextWriteLab;
    unsigned char *OldLabBuf;
    unsigned long bufSize;
    unsigned char *NewLabBuf;

    for (;;) {
        pSrc = qDeQueueOne(pQ, LabelInQueReadPos);
        if (pSrc == 0)
            break;
        pWriteLab = pEngine->pWriteLab[UttIdx];
        pNextWriteLab = pWriteLab + pQ->Size;
        if (pNextWriteLab > pEngine->pEndLab[UttIdx]) {
            OldLabBuf = pEngine->UtteranceLabels[UttIdx];
            bufSize = pEngine->pEndLab[UttIdx] - OldLabBuf;
            NewLabBuf = heap_Alloc(pEngine->hHeap, bufSize + 0x7D0);
            if (NewLabBuf == 0)
                return _tosErrorLog(&pEngine->Object, 2);
            if (OldLabBuf != 0) {
                memcpy(NewLabBuf, OldLabBuf, bufSize);
                heap_Free(pEngine->hHeap, OldLabBuf);
            }
            pWriteLab = NewLabBuf + (pWriteLab - OldLabBuf);
            pNextWriteLab = pWriteLab + pQ->Size;
            pEngine->UtteranceLabels[UttIdx] = NewLabBuf;
            pEngine->pEndLab[UttIdx] = NewLabBuf + bufSize + 0x7D0;
        }
        memcpy(pWriteLab, pSrc, pQ->Size);
        pEngine->pWriteLab[UttIdx] = pNextWriteLab;
    }
    qFreeUnusedElemements(pQ);
    return 0;
}

/* Stops recognition after draining its output and reporting the stop callback. */
int casrStop(struct HCASR__ *hcAsr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u16 state;
    u32 Err;
    s32 callbackResult;

    state = pEngine->Tos.ActiveState;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) {
        return Err;
    }
    pEngine->Tos.ErrorLog = 0;
    pEngine->CallbackErrLog = 0;
    switch (state) {
    case CASR_SLEEP:
    case CASR_RUN:
    case CASR_SPELLSLEEP:
    case CASR_SPELLRUN:
    case CASR_ISOWORDSLEEP:
    case CASR_ISOWORDRUN:
    case CASR_RECORDSLEEP:
    case CASR_RECORD:
        break;
    default:
        return (u8)_tosErrorLog(&pEngine->Object, 7);
    }
    if (_tosControl(&pEngine->Object, 3, 0, 0) != 0) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    if (casrReadLabelInQueue(pEngine, pEngine->CurrUttIdx) != 0) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    if ((pEngine->DoSecondPass != 0) && (casrDoSecondPass(pEngine) != 0)) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    if (pEngine->DoPrompt != PROMPTTYPE_NOPROMPT) {
        s32 previousValue = pEngine->DoPrompt;

        pEngine->DoPrompt = PROMPTTYPE_NOPROMPT;
        callbackResult = pEngine->CRecogCallBacks.cbTrain(previousValue, &pEngine->UserData);
        if ((pEngine->CallbackErrLog == 0) && (callbackResult != 0)) {
            pEngine->CallbackErrLog = callbackResult;
            _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return (u8)pEngine->Tos.ErrorLog;
}

/* Sets the active dictionary word indices for the current recognizer. */
int casrSetActiveWords(struct HCASR__ *hcAsr, unsigned long *pidWords, unsigned long NbrWords)
{
    unsigned int i;
    unsigned int NbrWordsInCurrentContext;
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    unsigned long Err;
    struct _ASRSPI_CONTEXT Ctx;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) return Err;
    if (pidWords == 0 && NbrWords != 0) return _tosErrorLog(&pEngine->Object, 0x12);
    if (pEngine->IsActiveContextIsoWord) {
        return SetIsolatedWordDpActiveWords(pEngine, pidWords, NbrWords);
    }
    FillContextV2VirtualTable(&Ctx);
    Ctx.pCtx = pEngine->ActivatedContext->pCtx;
    NbrWordsInCurrentContext = Ctx.contextGetNbrWord(&Ctx);
    for (i = 0; i < NbrWords; i++) {
        if (pidWords[i] >= NbrWordsInCurrentContext) return _tosErrorLog(&pEngine->Object, 0x19);
    }
    if (NbrWords == 0) pidWords = 0;
    return _tosControl(&pEngine->Object, 0x19, (u32)pidWords, NbrWords);
}

/* Returns the active dictionary word indices for the current recognizer. */
int casrGetActiveWords(struct HCASR__ *hcAsr, unsigned long **ppidWords, unsigned long *pNbrWords)
{
    unsigned char *pMask;
    unsigned char Mask;
    unsigned char BitTest;
    unsigned long Size; /* Length of the active-word mask in bytes. */
    unsigned long NbrWords = 0;
    unsigned long WordID = 0;
    unsigned long i;
    unsigned long *pidWords;
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    struct _TOS *pTos;
    unsigned long Err;
    Err = casrCheckHandle(pEngine);
    if (Err) return Err;
    if (ppidWords == 0 || pNbrWords == 0) return _tosErrorLog(&pEngine->Object, 0x12);
    if (pEngine->IsActiveContextIsoWord) {
        return GetIsolatedWordDpActiveWords(pEngine, ppidWords, pNbrWords);
    }
    pTos = &pEngine->Tos;
    pTos->ErrorLog = 0;
    if (_tosControl(&pEngine->Object, 0x1A, (u32)&pMask, (u32)&Size)) return 1;
    for (i = 0; i < Size; i++) {
        Mask = pMask[i];
        for (BitTest = 0x80; BitTest != 0; BitTest >>= 1) {
            if (Mask & BitTest) NbrWords++;
        }
    }
    if (NbrWords) {
        pidWords = heap_Alloc(pEngine->hHeap, NbrWords * sizeof(unsigned long));
        if (pidWords == 0) return _tosErrorLog(&pEngine->Object, 1);
    } else pidWords = 0;
    *pNbrWords = NbrWords;
    *ppidWords = pidWords;
    for (i = 0; i < Size; i++) {
        Mask = pMask[i];
        for (BitTest = 0x80; BitTest != 0; BitTest >>= 1) {
            if (Mask & BitTest) *pidWords++ = WordID;
            WordID++;
        }
    }
    return 0;
}

/* Initializes the recognizer parameters and retrieves their default settings. */
static u8 InitParamList(struct _ENGINE *pEngine)
{
    pEngine->ParamList.ArchiParam.TimeOut = 0;
    pEngine->ParamList.ArchiParam.SecondPassEnable = 1;
    pEngine->ParamList.ArchiParam.UWMinSpeechDur = 0x1E;
    pEngine->ParamList.ArchiParam.UWConfThres1 = -10;
    pEngine->ParamList.ArchiParam.UWConfThres2 = -20;
    pEngine->ParamList.ArchiParam.ExtraEventEnable = 1;
    if (_tosControl(
            &pEngine->Object, 0x46,
            (u32)&pEngine->ParamList.VadParam, 0) != 0) {
        return 1;
    }
    if (_tosControl(
            &pEngine->Object, 0x47,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0) {
        return 1;
    }
    if (_tosControl(
            &pEngine->Object, 0x48,
            (u32)&pEngine->ParamList.GenderParam.EnabledGendersMask, 0) != 0) {
        return 1;
    }
    if (_tosControl(
            &pEngine->Object, 0x49,
            (u32)&pEngine->ParamList.DistParam.AnySpeechPenalty, 0) != 0) {
        return 1;
    }
    return _tosControl(
        &pEngine->Object, 0x4A,
        (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0);
}

/* Returns a recognizer parameter selected by its public parameter number. */
int casrGetParam(struct HCASR__ *hcAsr, unsigned long ParamID, unsigned long *pParamValue)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u32 result;

    result = casrCheckHandle(pEngine);
    if (result != 0) {
        return result;
    }
    if (pParamValue == 0) {
        return (u8)_tosErrorLog(&pEngine->Object, 0x12);
    }
    switch (ParamID) {
    case 1: *pParamValue = pEngine->ParamList.TriggerParam.DoStopDetection; break;
    case 2: *pParamValue = pEngine->ParamList.TriggerParam.DoStartDetection; break;
    case 3: *pParamValue = pEngine->ParamList.DynProgParam.PremRes; break;
    case 4: *pParamValue = pEngine->ParamList.TriggerParam.TSCount; break;
    case 5: *pParamValue = pEngine->ParamList.ArchiParam.TimeOut / 100; break;
    case 0xF: *pParamValue = pEngine->ParamList.ArchiParam.SecondPassEnable; break;
    case 0x12: *pParamValue = pEngine->ParamList.ArchiParam.ExtraEventEnable; break;
    case 0x10: *pParamValue = pEngine->MaxNbrWordsInISOWordSentence; break;
    case 6: *pParamValue = pEngine->ParamList.DynProgParam.Accuracy; break;
    case 7: *pParamValue = 0x32 - pEngine->ParamList.DistParam.RejectionPenalty; break;
    case 8: *pParamValue = 0x32 - pEngine->ParamList.DistParam.AnySpeechPenalty; break;
    case 9: *pParamValue = pEngine->ParamList.TriggerParam.Sensitivity; break;
    case 0xA: *pParamValue = pEngine->ParamList.TriggerParam.MinSpeechDur; break;
    case 0x11: *pParamValue = pEngine->ParamList.TriggerParam.AbsTriggThreshold; break;
    case 0xB: *pParamValue = pEngine->ParamList.VadParam.On; break;
    case 0xD: *pParamValue = pEngine->ParamList.DynProgParam.MaxNBest; break;
    case 0xC: *pParamValue = pEngine->ParamList.VadParam.FarTalk; break;
    case 0xE: *pParamValue = pEngine->ParamList.GenderParam.EnabledGendersMask; break;
    default:
        return (u8)_tosErrorLog(&pEngine->Object, 0xA);
    }
    return 0;
}

/* Sets or clears the end-backtrace flag on the selected pronunciations. */
static void Set_EndBackTraceMask_In_StateDict(
    unsigned char Enable_EndBackTraceMask, struct _DICTSTATE *pStateInDict,
    unsigned long *pPron, unsigned long *pEndOfProns)
{
    struct _DICTSTATE *pStateInDict_Minus4 = pStateInDict - 4;
    struct _DICTSTATE *pDictState;
    unsigned short InvBackTraceMask;
    if (Enable_EndBackTraceMask) {
        while (pPron <= pEndOfProns) {
            pDictState = pStateInDict_Minus4 + *pPron;
            pDictState->StateIndex |= 2;
            pPron++;
        }
    } else {
        InvBackTraceMask = 0xFFFD;
        while (pPron <= pEndOfProns) {
            pDictState = pStateInDict_Minus4 + *pPron;
            pDictState->StateIndex &= InvBackTraceMask;
            pPron++;
        }
    }
}

/* Updates the context used by extra-event recognition. */
static inline unsigned char UpdateExevStatus(struct _ENGINE *pEngine)
{
    if (pEngine->ParamList.ArchiParam.ExtraEventEnable != 0) {
        struct _LANGUAGE *pLang = pEngine->pLang;

        if (pLang->langCheckBitField(pLang, 0x40) != 0) {
            if (_tosControl(&pEngine->Object, 0x64, 0, 0) != 0) {
                return 1;
            }
            if (_tosControl(&pEngine->Object, 0x56, (u32)&pEngine->DistCalcFuncs, 0) != 0) {
                return 1;
            }
            if (CreateExEvCtxInfo(pEngine) != 0) {
                return 1;
            }
            if (_tosControl(&pEngine->Object, 0x66, (u32)pEngine->CtxInfo.pExEvCtxInfo, 0) != 0) {
                FreeCtxInfo(pEngine);
                return 1;
            }
            goto done;
        }
    }
    if (_tosControl(&pEngine->Object, 0x65, 0, 0) != 0) {
        return 1;
    }

done:
    return 0;
}

/* Updates a recognition parameter and propagates it to the affected blocks. */
int casrSetParam(struct HCASR__ *hcAsr, unsigned long ParamID, unsigned long ParamValue)
{
    void *pHandle = hcAsr;
    struct _ENGINE *pEngine = pHandle;
    struct _TOS *pTos = &pEngine->Tos;
    unsigned short ActiveState;
    unsigned long Err;
    struct _LANGUAGE *pLang;
    struct _DICTSTATE *pStateInDict;
    unsigned long *pBeginOfWords;
    unsigned int NbrOfWords;
    unsigned long *pBeginOfProns;
    unsigned long *pPron;
    struct _DICTSTATE *pDictState;
    unsigned char ENDBACKTRACEMASK_Is_Enabled_In_Active_Ctx;
    unsigned long *pEndOfProns;
    unsigned long u32OldMaxNbr;
    struct _ASRSPI_CONTEXT Context;
    long DummyError;

    Err = casrCheckHandle(pEngine);
    if (Err != 0) return Err;
    ActiveState = pTos->ActiveState;
    switch (ActiveState) {
    case CASR_BOOT: case CASR_DATAREADY: case CASR_IDLE: case CASR_RECOVER: case CASR_SECONDPASS:
    case CASR_SPELLRECOVER: case CASR_SPELLSECONDPASS:
    case CASR_ISOWORDRECOVER: case CASR_ISOWORDSECONDPASS: case CASR_UWTRAINED:
        break;
    default: return (u8)_tosErrorLog(&pEngine->Object, 7);
    }
    pTos->ErrorLog = 0;
    switch (ParamID) {
    case 1:
        pEngine->ParamList.TriggerParam.DoStopDetection = ParamValue;
        pEngine->ParamList.DynProgParam.TSEnable = ParamValue;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        if (_tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 2:
        pEngine->ParamList.TriggerParam.DoStartDetection = ParamValue;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 3:
        pEngine->ParamList.DynProgParam.PremRes = ParamValue;
        if (_tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 4:
        pEngine->ParamList.TriggerParam.TSCount = ParamValue;
        pEngine->ParamList.DynProgParam.TSCount = ParamValue * 2;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        if (_tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 5: pEngine->ParamList.ArchiParam.TimeOut = ParamValue * 0x64; break;
    case 0xF:
        pLang = pEngine->pLang;
        if (pLang != 0) {
            if (ParamValue != 0 && pLang->langCheckBitField(pLang, 0x10) != 0)
                return (u8)_tosErrorLog(&pEngine->Object, 0x1C);
            if (pLang->langCheckBitField(pLang, 8) != 0 && pEngine->CtxInfo.pDynProgCtxInfo != 0) {
                pStateInDict = pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict;
                pBeginOfWords = pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords;
                NbrOfWords = pEngine->CtxInfo.pDynProgCtxInfo->uNbrOfWords;
                pBeginOfProns = pBeginOfWords + NbrOfWords + 1;
                pPron = pBeginOfProns + 1;
                pDictState = pStateInDict + *pPron - 4;
                ENDBACKTRACEMASK_Is_Enabled_In_Active_Ctx = pDictState->StateIndex & 2;
                if ((ParamValue != 0 && !ENDBACKTRACEMASK_Is_Enabled_In_Active_Ctx) ||
                    (ParamValue == 0 && ENDBACKTRACEMASK_Is_Enabled_In_Active_Ctx)) {
                    pEndOfProns = pBeginOfWords + pBeginOfWords[NbrOfWords];
                    Set_EndBackTraceMask_In_StateDict(ParamValue, pStateInDict, pPron, pEndOfProns);
                }
            }
        }
        pEngine->ParamList.ArchiParam.SecondPassEnable = ParamValue;
        break;
    case 0x12:
        pEngine->ParamList.ArchiParam.ExtraEventEnable = ParamValue;
        if (pEngine->ActivatedContext != 0) {
            (void)UpdateExevStatus(pEngine);
        }
        break;
    case 0x10:
        u32OldMaxNbr = pEngine->MaxNbrWordsInISOWordSentence;
        pEngine->MaxNbrWordsInISOWordSentence = ParamValue;
        if (pEngine->ActivatedContext != 0) {
            FillContextV2VirtualTable(&Context);
            Context.pCtx = pEngine->ActivatedContext->pCtx;
            if (Context.contextIsIsoWordContext(&Context) == 0 &&
                Context.contextIsSpellingContext(&Context) == 0 &&
                pEngine->MaxNbrWordsInISOWordSentence != pEngine->IsActiveContextIsoWord) {
                Err = casrActivateContext(hcAsr,
                    (struct HCTX__ *)pEngine->ActivatedContext, pEngine->bSetOptimalParameters);
                if (Err != 0) {
                    pEngine->MaxNbrWordsInISOWordSentence = u32OldMaxNbr;
                    DummyError = casrActivateContext(hcAsr,
                        (struct HCTX__ *)pEngine->ActivatedContext, pEngine->bSetOptimalParameters);
                    if (DummyError != 0) return DummyError;
                    return Err;
                }
            }
        }
        break;
    case 6:
        pEngine->ParamList.DynProgParam.Accuracy = ParamValue;
        if (_tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 7:
        pEngine->ParamList.DistParam.RejectionPenalty = 0x32 - ParamValue;
        if (_tosControl(&pEngine->Object, 0x53,
            (u32)&pEngine->ParamList.DistParam.AnySpeechPenalty, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 8:
        pEngine->ParamList.DistParam.AnySpeechPenalty = 0x32 - ParamValue;
        if (_tosControl(&pEngine->Object, 0x53,
            (u32)&pEngine->ParamList.DistParam.AnySpeechPenalty, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 9:
        pEngine->ParamList.TriggerParam.Sensitivity = ParamValue;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0xA:
        pEngine->ParamList.TriggerParam.MinSpeechDur = ParamValue;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0x11:
        pEngine->ParamList.TriggerParam.AbsTriggThreshold = ParamValue;
        if (_tosControl(&pEngine->Object, 0x51,
            (u32)&pEngine->ParamList.TriggerParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0xB:
        pEngine->ParamList.VadParam.On = ParamValue;
        if (_tosControl(&pEngine->Object, 0x50, (u32)&pEngine->ParamList.VadParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0xD:
        pEngine->ParamList.DynProgParam.MaxNBest = ParamValue;
        if (_tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0xC:
        pEngine->ParamList.VadParam.FarTalk = ParamValue;
        if (_tosControl(&pEngine->Object, 0x50, (u32)&pEngine->ParamList.VadParam, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    case 0xE:
        if (ActiveState == CASR_BOOT) return (u8)_tosErrorLog(&pEngine->Object, 0x21);
        if (casrEnableGenders(pEngine, ParamValue) != 0)
            return (u8)_tosErrorLog(&pEngine->Object, 0x14);
        pEngine->ParamList.GenderParam.EnabledGendersMask = ParamValue;
        if (_tosControl(&pEngine->Object, 0x52,
            (u32)&pEngine->ParamList.GenderParam.EnabledGendersMask, 0) != 0)
            return (u8)pTos->ErrorLog;
        if (_tosControl(&pEngine->Object, 0x26, 0, 0) != 0)
            return (u8)pTos->ErrorLog;
        break;
    default: return (u8)_tosErrorLog(&pEngine->Object, 0xA);
    }
    return (u8)pTos->ErrorLog;
}

/* Sends a gain update to the recognition scheduler. */
int casrGainSet(struct HCASR__ *hcAsr, unsigned short Gain)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u32 result;

    result = casrCheckHandle(pEngine);
    if (result != 0) {
        return result;
    }
    pEngine->Tos.ErrorLog = 0;
    if (_tosControl(&pEngine->Object, 0x35, Gain, 0) != 0) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    return (u8)pEngine->Tos.ErrorLog;
}

/* Returns the current TOS state after validating the recognizer context. */
int casrGetState(struct HCASR__ *hcAsr, enum _CASRSTATE *pState)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    if (pEngine == 0) {
        return 6;
    }
    if (pEngine->ValidEngine != 0x218DF4F5) { /* Marker set by casrOpen. */
        return 0xB;
    }
    if (pState == 0) {
        return _tosErrorLog(&pEngine->Object, 0x12);
    }
    *pState = pEngine->Tos.ActiveState;
    return 0;
}

/* Returns the current signal level after validating the recognizer context. */
int casrGetSignalLevel(struct HCASR__ *hcAsr, signed short *pCasrLevel)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    if (pEngine == 0) {
        return 6;
    }
    if (pEngine->ValidEngine != 0x218DF4F5) { /* Marker set by casrOpen. */
        return 0xB;
    }
    if (pCasrLevel == 0) {
        return _tosErrorLog(&pEngine->Object, 0x12);
    }
    *pCasrLevel = *pEngine->pSignalLevel;
    return 0;
}

/* Reads the requested SNR value through the recognizer's control interface. */
int casrGetSNR(struct HCASR__ *hcAsr, signed short *pSNR)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u32 result;

    result = casrCheckHandle(pEngine);
    if (result != 0) {
        return result;
    }
    if (pSNR == 0) {
        return _tosErrorLog(&pEngine->Object, 0x12);
    }
    pEngine->Tos.ErrorLog = 0;
    if (_tosControl(&pEngine->Object, 5, (u32)pSNR, 0) != 0) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    return (u8)pEngine->Tos.ErrorLog;
}

/* Allocates a handle that keeps imported data together with its owning heap. */
struct HCDATA__ *casrImportData(void *hHeap, struct _DATA *pData)
{
    struct _DATAIMPORT *pNewDataImport;

    if (pData == 0) {
        return 0;
    }
    pNewDataImport = heap_Alloc(hHeap, sizeof(struct _DATAIMPORT));
    if (pNewDataImport == 0) {
        return 0;
    }
    pNewDataImport->pData = pData;
    pNewDataImport->hHeap = hHeap;
    return (struct HCDATA__ *)pNewDataImport;
}

/* Exports the current recognizer session data in the requested format. */
int casrExportData(struct HCASR__ *hcAsr, unsigned long DataType, void *ppBuf, unsigned long *pSize)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u16 state;
    u32 result;

    pEngine->Tos.ErrorLog = result = 0;
    if (pEngine == 0) {
        result = 6;
    } else if (pEngine->ValidEngine != 0x218DF4F5) { /* Marker set by casrOpen. */
        result = 0xB;
    }
    if (result != 0) {
        return result;
    }
    if ((ppBuf == 0) || (pSize == 0)) {
        return (u8)_tosErrorLog(&pEngine->Object, 0x12);
    }
    if (DataType != 4) {
        *(void **)ppBuf = 0;
        *pSize = 0;
        return (u8)_tosErrorLog(&pEngine->Object, 0xC);
    }
    state = pEngine->Tos.ActiveState;
    if ((state != CASR_IDLE) && (state != CASR_DATAREADY) && (state != CASR_UWTRAINED) &&
        (state != CASR_ACCEPT)) {
        *(void **)ppBuf = 0;
        *pSize = 0;
        return (u8)_tosErrorLog(&pEngine->Object, 7);
    }
    if (casrGetSessionData(pEngine, ppBuf, pSize) != 0) {
        return 1;
    }
    return 0;
}

/* Loads a data record and activates it according to its data type. */
int casrActivateData(struct HCASR__ *hcAsr, struct HCDATA__ *hcData)
{
    void *pHandle = hcAsr;
    struct _ENGINE *pEngine = pHandle;
    struct _TOS *pTos = &pEngine->Tos;
    struct _DATAIMPORT *pDataImport = (struct _DATAIMPORT *)hcData;
    unsigned long Err;
    struct _DISTCALCFUNCS DistCalcFuncs;

    unsigned long i;
    unsigned long SessionGenderMask;

    Err = 0;
    pTos->ErrorLog = Err;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) {
        return Err;
    }

    if ((pTos->ActiveState != CASR_BOOT) && (pTos->ActiveState != CASR_DATAREADY) &&
        (pTos->ActiveState != CASR_UWTRAINED) && (pTos->ActiveState != CASR_IDLE)) {
        return _tosErrorLog(&pEngine->Object, 7);
    }

    if (pDataImport == 0) {
        if (pEngine->ActivatedLanguage != 0) {
            pEngine->ActivatedLanguage = 0;
            heap_Free(pEngine->hHeap, pEngine->pLang);
            pEngine->pLang = 0;
            if ((pEngine->ActivatedContext != 0) &&
                (casrActivateContext(hcAsr, 0, 0) != 0)) {
                return 1;
            }
            if (_tosControl(&pEngine->Object, 0x5F, 0, 0) != 0) {
                return 1;
            }
        }
        return 0;
    }

    switch (pDataImport->pData->DataType) {
    case 0x1003: {
        long Index = 0;
        unsigned long SecondPassEnable;
        unsigned short NbrOfCodeBooks;
        struct _LANGUAGE *pLang;
        if (pEngine->ActivatedLanguage == 0) {
            struct _LANGUAGE *pLang;

            pEngine->pLang = heap_Alloc(pEngine->hHeap, sizeof(struct _LANGUAGE));

            if ((pLang = pEngine->pLang) == 0) {
                return 1;
            }
            FillLanguageVirtualTable(pLang);
        }

        pEngine->pLang->pLang = (struct _LANGUAGEINFO *)pDataImport->pData;
        if ((pEngine->ActivatedContext != 0) &&
            (casrActivateContext(hcAsr, 0, 0) != 0)) {
            return 1;
        }
        if (_tosControl(&pEngine->Object, 0x25,
                        (u32)pEngine->pLang, 0) != 0) {
            return 1;
        }

        pLang = pEngine->pLang;
        {
            unsigned short NbrOfCodeBooks = pLang->langGetNbrCodeBook(pLang);
            if (NbrOfCodeBooks < 5) {
                if (_tosControl(&pEngine->Object, 0x5D, 0, 0) != 0) {
                    return 1;
                }
                if ((pEngine->CSampleFormat == UTTFORMAT_LABELS) && (pEngine->FrameSize == 5)) {
                    pEngine->FrameSize--;
                }
            } else {
                if (_tosControl(&pEngine->Object, 0x5E, 0, 0) != 0) {
                    return 1;
                }
                if (pEngine->CSampleFormat == UTTFORMAT_LABELS) {
                    pEngine->FrameSize = 5;
                }
            }
        }
        if (casrGetParam(hcAsr, 0xF, &SecondPassEnable) != 0) {
            return 1;
        }
        if (SecondPassEnable != 0) {
            pLang = pEngine->pLang;
            if (pLang->langCheckBitField(pLang, 0x10) != 0) {
                return _tosErrorLog(&pEngine->Object, 0x1C);
            }
        }
        if (_tosControl(&pEngine->Object, 0x55,
                        (u32)&DistCalcFuncs, 0) != 0) {
            return 1;
        }
        if (_tosControl(&pEngine->Object, 0x56,
                        (u32)&DistCalcFuncs, 0) != 0) {
            return 1;
        }

        /* Enable all genders, leaving the high bit clear. */
        pEngine->ParamList.GenderParam.EnabledGendersMask = 0x7FFFFFFF;
        pLang = pEngine->pLang;
        NbrOfCodeBooks = (u16)pLang->langGetNbrCodeBook(pLang);
        for (i = 0; i < NbrOfCodeBooks; i++) {
            void *pCodeBook;
            pLang = pEngine->pLang;
            pCodeBook = pLang->langGetpCodeBook(pLang, i);
            pLang = pEngine->pLang;
            pEngine->ppSmoother[i] = (f32 *)pLang->cdbGetpSmoothMatrix(pLang, pCodeBook);
        }
        pLang = pEngine->pLang;
        pEngine->AnySpeechPhenom = pLang->langGetAnySpeechGarbagePhenome(pLang);
        pEngine->ActivatedLanguage = pDataImport;
        if (_tosControl(&pEngine->Object, 0x26, 0,
                        (u32)&Index) != 0) {
            return 1;
        }
        return 0;
    }
    case 4: {
        CASRSessionHeader *pSessionInfo = (void *)pDataImport->pData;
        u8 *pData;
        long Index = 0;
        if (pSessionInfo->Size == sizeof(*pSessionInfo)) {
            pData = 0;
        } else {
            CASRSessionFormat *pExtraInfo;
            struct _LANGUAGE *pLang;
            struct _WARPINFO *pWarpFactors;
            unsigned long j;

            pExtraInfo = (void *)(pSessionInfo + 1);
            SessionGenderMask = 0;
            pLang = pEngine->pLang;
            pWarpFactors = pLang->langGetpWarpFactors(pLang);
            for (j = 0; j < 3; j++) {
                if (pExtraInfo->GenderActive[j] != 0) {
                    SessionGenderMask |= pWarpFactors[j].GenderMask;
                }
            }
            pEngine->ParamList.GenderParam.EnabledGendersMask = SessionGenderMask;
            if (casrEnableGenders(pEngine, SessionGenderMask) != 0) {
                return _tosErrorLog(&pEngine->Object, 0x14);
            }

            for (j = 0; j < 3; j++) {
                if (pEngine->GenderActive[j] != pExtraInfo->GenderActive[j]) {
                    return _tosErrorLog(&pEngine->Object, 0x22);
                }
            }

            switch (pEngine->CSampleFormat) {
            case PCM_16_11KHZ:
                if ((pExtraInfo->CSampleFormat == UTTFORMAT_LABELS) ||
                    (pExtraInfo->CSampleFormat == UTTFORMAT_LABELSRAW) ||
                    (pExtraInfo->CSampleFormat == UTTFORMAT_CEPSTRA)) {
                    return _tosErrorLog(&pEngine->Object, 0x22);
                }
                break;
            case UTTFORMAT_CEPSTRA:
                if ((pExtraInfo->CSampleFormat == UTTFORMAT_LABELS) ||
                    (pExtraInfo->CSampleFormat == UTTFORMAT_LABELSRAW)) {
                    return _tosErrorLog(&pEngine->Object, 0x22);
                }
                break;
            case UTTFORMAT_LABELS:
            case UTTFORMAT_LABELSRAW:
                /* Label input needs no additional session format check. */
                break;
            }

            pData = (u8 *)pSessionInfo + sizeof(*pSessionInfo) + sizeof(*pExtraInfo);
            pDataImport = pEngine->ActivatedLanguage;
            if ((pDataImport == 0) ||
                (memcmp(pData, pDataImport->pData->VersionInfo,
                        sizeof(pDataImport->pData->VersionInfo)) != 0)) {
                return _tosErrorLog(&pEngine->Object, 0x13);
            }
            pData += sizeof(pDataImport->pData->VersionInfo);
        }
        if (_tosControl(&pEngine->Object, 0x26,
                        (u32)pData, (u32)&Index) != 0) {
            return 1;
        }
        return 0;
    }
    default:
        return _tosErrorLog(&pEngine->Object, 0xC);
    }
}

/* Releases a data import handle through the heap that owns it. */
int casrCloseData(struct HCDATA__ *hcData)
{
    struct _DATAIMPORT *pDataImport = (struct _DATAIMPORT *)hcData;

    heap_Free(pDataImport->hHeap, pDataImport);
    return 0;
}

/* Imports a validated context descriptor with its owning heap. */
struct HCTX__ *casrImportContext(void *hHeap, struct _DATA *pCtx)
{
    struct _CTXIMPORT *pNewCtxImport;

    if (pCtx == 0) {
        return 0;
    }
    if (pCtx->DataType != 6) {
        return 0;
    }
    pNewCtxImport = heap_Alloc(hHeap, sizeof(struct _CTXIMPORT));
    if (pNewCtxImport == 0) {
        return 0;
    }
    pNewCtxImport->hHeap = hHeap;
    pNewCtxImport->pCtx = pCtx;
    return (struct HCTX__ *)pNewCtxImport;
}

/* Releases the context import handle through its owning heap. */
int casrCloseContext(struct HCTX__ *hCtx)
{
    struct _CTXIMPORT *pCtx = (struct _CTXIMPORT *)hCtx;

    heap_Free(pCtx->hHeap, pCtx);
    return 0;
}

/* Updates the scheduler state and reports the first callback failure when it changes. */
static unsigned char casrEngineSetState(struct _ENGINE *pEngine, u16 NewState)
{
    s32 CallbackErr;
    u16 *statePointer;

    statePointer = &((struct _TOS *)pEngine)->ActiveState;
    if (*statePointer != NewState) {
        *statePointer = NewState;
        if (pEngine->CRecogCallBacks.cbState != 0) {
            CallbackErr = pEngine->CRecogCallBacks.cbState(NewState, 0,
                                                   &pEngine->UserData);
            if ((pEngine->CallbackErrLog == 0) && (CallbackErr != 0)) {
                pEngine->CallbackErrLog = CallbackErr;
                _tosErrorLog(&pEngine->Object, 0x1B);
            }
        }
    }
    return 0;
}

/* Selects a context and configures the recognizer to use it. */
int casrActivateContext(struct HCASR__ *hcAsr, struct HCTX__ *hCtx,
    int bSetOptimalParameters)
{
    struct _ENGINE *pEngine;
    struct _TOS *pTos = &((struct _ENGINE *)hcAsr)->Tos;
    struct _CTXIMPORT *pCtx = (struct _CTXIMPORT *)hCtx;
    struct _ASRSPI_CONTEXT Ctx;
    unsigned long Err;
    unsigned long Acc;
    struct _DISTCALCFUNCS DistCalcFuncs;
    struct ISOWORD_USERWORD_LNG UserWordLng;

    Err = 0;
    pEngine = (struct _ENGINE *)hcAsr;
    Err = casrCheckHandle(pEngine);
    Acc = 0;
    if (Err != 0) {
        return (s32)Err;
    }
    if ((pTos->ActiveState != CASR_IDLE) && (pTos->ActiveState != CASR_DATAREADY) &&
        (pTos->ActiveState != CASR_UWTRAINED)) {
        return (u8)_tosErrorLog(&pEngine->Object, 7);
    }

    pTos->ErrorLog = 0;
    if (pCtx != 0) {
        if (memcmp(pCtx->pCtx->VersionInfo,
                   pEngine->ActivatedLanguage->pData->VersionInfo,
                   sizeof(pCtx->pCtx->VersionInfo)) != 0) {
            return (u8)_tosErrorLog(&pEngine->Object, 0x13);
        }
    }

    if (pEngine->ActivatedContext != 0) {
        if (_tosControl(&pEngine->Object, 0x29, 0, 0) != 0) {
            return (u8)pTos->ErrorLog;
        }
        FreeCtxInfo(pEngine);
    }

    if (pCtx == 0) {
        if (pEngine->ActivatedContext != 0) {
            pEngine->ActivatedContext = 0;
        }
        return casrEngineSetState(pEngine, CASR_DATAREADY);
    }

    FillContextV2VirtualTable(&Ctx);
    Ctx.pCtx = pCtx->pCtx;
    if (bSetOptimalParameters == 0) {
        Acc = pEngine->ParamList.DynProgParam.Accuracy;
    }
    pEngine->bSetOptimalParameters = bSetOptimalParameters;
    pEngine->IsActiveContextSpelling = Ctx.contextIsSpellingContext(&Ctx);
    pEngine->IsActiveContextIsoWord = Ctx.contextIsIsoWordContext(&Ctx);
    if ((pEngine->IsActiveContextIsoWord == 0) &&
        (pEngine->IsActiveContextSpelling == 0)) {
        pEngine->IsActiveContextIsoWord = (u8)pEngine->MaxNbrWordsInISOWordSentence;
    }
    if (pEngine->IsActiveContextSpelling != 0) {
        struct _LANGUAGE *pLang = pEngine->pLang;
        pEngine->WordPenalty = pLang->langGetSpellingWtp(pLang);
    } else {
        struct _LANGUAGE *pLang = pEngine->pLang;
        pEngine->WordPenalty = pLang->langGetRecogWtp(pLang);
    }
    if (_tosControl(&pEngine->Object, 0x55,
                    (u32)&pEngine->DistCalcFuncs, 0) != 0) {
        return 1;
    }

    if (pEngine->IsActiveContextIsoWord != 0) {
        if (CreateCtxTree(pEngine, &Ctx) != 0) {
            return 1;
        }
        UserWordLng.pFirstPhenomOfUserWordSpeechUnit = pEngine->pFirstPhenomOfUserWordSpeechUnit;
        UserWordLng.pUserWordLngSpeechUnitLex = pEngine->pUserWordLngSpeechUnitLex;
        if (bSetOptimalParameters != 0) {
            Acc = CalcAccuracyCtxTree(pEngine);
        }
        if ((pEngine->CSampleFormat == UTTFORMAT_LABELS) ||
            (pEngine->CSampleFormat == UTTFORMAT_LABELSRAW)) {
            pEngine->pActiveBlocks = ActiveBlocks_UTTFORMAT_LABELS_ISO;
        }
        if ((_tosControl(&pEngine->Object, 0x55,
                         (u32)&DistCalcFuncs, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x62,
                         (u32)pEngine->pLang, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x56,
                         (u32)&DistCalcFuncs, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x5A, 0, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x24,
                         (u32)pEngine->CtxInfo.pCtxTree, (u32)&UserWordLng) != 0) ||
            (_tosControl(&pEngine->Object, 0x54,
                         (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)) {
            FreeCtxInfo(pEngine);
            return 1;
        }
    } else {
        if (CreateCtxInfo(pEngine, &Ctx) != 0) {
            return 1;
        }
        if (bSetOptimalParameters != 0) {
            Acc = CalcAccuracyCtxInfo(pEngine);
        }
        if ((pEngine->CSampleFormat == UTTFORMAT_LABELS) ||
            (pEngine->CSampleFormat == UTTFORMAT_LABELSRAW)) {
            pEngine->pActiveBlocks = ActiveBlocks_UTTFORMAT_LABELS;
        }
        if ((_tosControl(&pEngine->Object, 0x61, 0, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x57, 0, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x24,
                         (u32)pEngine->CtxInfo.pDynProgCtxInfo, 0) != 0) ||
            (_tosControl(&pEngine->Object, 0x54,
                         (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0)) {
            FreeCtxInfo(pEngine);
            return 1;
        }
    }

    if ((bSetOptimalParameters != 0) &&
        ((casrSetParam(hcAsr, 6, (s32)Acc) != 0) ||
         (casrSetParam(hcAsr, 7,
                       (s32)((struct _ASRSPI_CONTEXTINFO *)Ctx.pCtx)->Rejection) != 0) ||
         (casrSetParam(hcAsr, 4,
                       (s32)((struct _ASRSPI_CONTEXTINFO *)Ctx.pCtx)->TS) != 0) ||
         (casrSetParam(hcAsr, 8,
                       (s32)((struct _ASRSPI_CONTEXTINFO *)Ctx.pCtx)->Garbage) != 0))) {
        return 1;
    }

    if (UpdateExevStatus(pEngine) != 0) {
        return 1;
    }
    pEngine->ActivatedContext = pCtx;
    return (u8)pTos->ErrorLog;
}

/* Selects the enabled second-pass routine and reports its failure status. */
u8 casrDoSecondPass(struct _ENGINE *pEngine)
{
    if (pEngine->IsActiveContextSpelling != 0) {
        if (casrDoSpelling(pEngine) != 0) {
            return 1;
        }
    } else if (pEngine->IsActiveContextIsoWord != 0) {
        if (casrDoIsoWordSecondPass(pEngine) != 0) {
            return 1;
        }
    } else if (casrDoNormalSecondPass(pEngine) != 0) {
        return 1;
    }
    return 0;
}

/* Runs a parameter operation without changing the recognizer's TOS error. */
static s32 casrGetParamNoClearErrorLog(
    struct _ENGINE *pEngine, u32 ParamID, unsigned long *pParamValue)
{
    u32 Error = pEngine->Tos.ErrorLog;
    s32 RetVal = casrGetParam((struct HCASR__ *)pEngine, ParamID, pParamValue);

    pEngine->Tos.ErrorLog = Error;
    return RetVal;
}

/* Sets a recognizer parameter and restores the previous TOS error. */
static s32 casrSetParamNoClearErrorLog(
    struct _ENGINE *pEngine, unsigned long ParamID, unsigned long ParamValue)
{
    u32 Error = pEngine->Tos.ErrorLog;
    s32 RetVal = casrSetParam((struct HCASR__ *)pEngine, ParamID, ParamValue);

    pEngine->Tos.ErrorLog = Error;
    return RetVal;
}

/* Runs the normal second-pass label search and restores the recognizer settings. */
u8 casrDoNormalSecondPass(struct _ENGINE *pEngine)
{
    unsigned long TSEnable;

    if (pEngine->pLang->langCheckBitField(pEngine->pLang, 8) != 0) {
        if (CreateSecondPassContext_ForChinese(pEngine) != 0) {
            return 1;
        }
    } else if (CreateSecondPassContext(pEngine) != 0) {
        return 1;
    }
    if (_tosControl(&pEngine->Object, 0x24,
            (u32)pEngine->CtxInfo.pSecondPassDynProgCtxInfo, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x11, 0, 0) != 0 ||
        casrGetParamNoClearErrorLog(pEngine, 1, &TSEnable) != 0 ||
        casrSetParamNoClearErrorLog(pEngine, 1, 0) != 0 ||
        DoLabelStart(pEngine) != 0 ||
        SendLabelsToEngine(pEngine, ActiveBlocks_UTTFORMAT_LABELS, 0) != 0 ||
        DoLabelStop(pEngine, ActiveBlocks_UTTFORMAT_LABELS) != 0 ||
        _tosControl(&pEngine->Object, 0x15, 0, 0) != 0 ||
        casrSetParamNoClearErrorLog(pEngine, 1, TSEnable) != 0 ||
        _tosControl(&pEngine->Object, 0x24,
            (u32)pEngine->CtxInfo.pDynProgCtxInfo, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0) {
        FreeSecondPassContext(pEngine);
        return 1;
    }
    FreeSecondPassContext(pEngine);
    pEngine->DoSecondPass = 0;
    return 0;
}

/* Runs the isolated-word second-pass label search and restores recognizer settings. */
u8 casrDoIsoWordSecondPass(struct _ENGINE *pEngine)
{
    struct _LANGUAGE *pLang = pEngine->pLang;
    unsigned long TSEnable;

    if (pLang->langCheckBitField(pLang, 8) != 0) {
        if (CreateSecondPassTreeContext_ForChinese(pEngine) != 0) {
            return 1;
        }
    } else if (CreateSecondPassTreeContext(pEngine) != 0) {
        return 1;
    }
    if (_tosControl(&pEngine->Object, 0x57, 0, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x24,
            (u32)pEngine->CtxInfo.pSecondPassDynProgCtxInfo, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x11, 0, 0) != 0 ||
        casrGetParamNoClearErrorLog(pEngine, 1, &TSEnable) != 0 ||
        casrSetParamNoClearErrorLog(pEngine, 1, 0) != 0 ||
        DoLabelStart(pEngine) != 0 ||
        SendLabelsToEngine(pEngine, ActiveBlocks_UTTFORMAT_LABELS, 0) != 0 ||
        DoLabelStop(pEngine, ActiveBlocks_UTTFORMAT_LABELS) != 0 ||
        _tosControl(&pEngine->Object, 0x15, 0, 0) != 0 ||
        casrSetParamNoClearErrorLog(pEngine, 1, TSEnable) != 0 ||
        _tosControl(&pEngine->Object, 0x29, 0, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x5A, 0, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x24,
            (u32)pEngine->CtxInfo.pCtxTree, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0) {
        FreeSecondPassContext(pEngine);
        return 1;
    }
    FreeSecondPassContext(pEngine);
    pEngine->DoSecondPass = 0;
    return 0;
}

/* Runs the spelling pass using the saved first-pass labels. */
u8 casrDoSpelling(struct _ENGINE *pEngine)
{
    unsigned char RetVal;
    unsigned long TSEnable;
    unsigned long MaxNBest;
    long CallbackErr;
    struct _DYNPROG_RESULT_NODE *pResTemp;
    struct _CASRRESULT *pCasrResult;
    struct _RECWORD *pRecWords;
    struct _ALTERNATIVE *pAlternatives;
    unsigned long NbrNormalisedScore;
    unsigned long BestScore;
    float TotalScore;
    unsigned int i;
    int WordNr;
    float *pNormalisedScore;
    struct _DICTSTATE *pTheStateDict;
    unsigned int cActiveWords;
    struct _DYNPROG_RESULT_NODE *pSpellRes;
    float p;
    unsigned long WordID;

    NbrNormalisedScore = 0x1E;
    RetVal = 0;
    pTheStateDict = pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict;
    pNormalisedScore = heap_Alloc(pEngine->hHeap, NbrNormalisedScore * sizeof(float));
    if (pNormalisedScore == 0) return _tosErrorLog(&pEngine->Object, 1);
    pResTemp = pEngine->pRes->pResult->pNext;
    pEngine->pRes->pResult->pNext = 0;
    casrCreateResult(pEngine, &pCasrResult, pEngine->pRes, 0);
    pEngine->pRes->pResult->pNext = pResTemp;
    pRecWords = pCasrResult->pSentences->pWords;
    if (_tosControl(&pEngine->Object, 0x24,
            (u32)pEngine->CtxInfo.pSpellingDynProgCtxInfo, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
            (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    if (_tosControl(&pEngine->Object, 0x11, 0, 0) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    if (casrGetParamNoClearErrorLog(pEngine, 1, &TSEnable) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    if (casrSetParamNoClearErrorLog(pEngine, 1, 0) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    if (casrGetParamNoClearErrorLog(pEngine, 0xD, &MaxNBest) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    if (casrSetParamNoClearErrorLog(pEngine, 0xD, 0x2710) != 0) {
        heap_Free(pEngine->hHeap, pNormalisedScore);
        return 1;
    }
    for (WordNr = 1; WordNr < pCasrResult->pSentences->iNbrWords - 1; WordNr++) {
        cActiveWords = 0;
        if (DoLabelStart(pEngine) != 0) {
            heap_Free(pEngine->hHeap, pNormalisedScore);
            return 1;
        }
        if (SendPartOfLabelsToEngine(pEngine, ActiveBlocks_UTTFORMAT_LABELS, 0,
                                     pRecWords[WordNr].TimeInfo.TimeBegin,
                                     pRecWords[WordNr].TimeInfo.TimeEnd) != 0) {
            heap_Free(pEngine->hHeap, pNormalisedScore);
            return 1;
        }
        if (DoLabelStop(pEngine, ActiveBlocks_UTTFORMAT_LABELS) != 0) {
            heap_Free(pEngine->hHeap, pNormalisedScore);
            return 1;
        }
        heap_Free(pEngine->hHeap, pRecWords[WordNr].pAlternatives);
        pSpellRes = pEngine->pRes->pResult;
        if (pSpellRes == 0) {
            pRecWords[WordNr].iNbrAlternatives = 0;
            pRecWords[WordNr].pAlternatives = 0;
        } else {
            BestScore = 0xFFFFFFFFU;
            while (pSpellRes) {
                if (pSpellRes->Score < BestScore) BestScore = pSpellRes->Score;
                pSpellRes = pSpellRes->pNext;
                cActiveWords++;
            }
            pRecWords[WordNr].iNbrAlternatives = cActiveWords;
            pAlternatives = pRecWords[WordNr].pAlternatives =
                heap_Alloc(pEngine->hHeap, cActiveWords * sizeof(struct _ALTERNATIVE));
            if (pAlternatives == 0) {
                heap_Free(pEngine->hHeap, pNormalisedScore);
                return 1;
            }
            if (cActiveWords > NbrNormalisedScore) {
                NbrNormalisedScore = cActiveWords;
                heap_Free(pEngine->hHeap, pNormalisedScore);
                pNormalisedScore = heap_Alloc(pEngine->hHeap, NbrNormalisedScore * sizeof(float));
                if (pNormalisedScore == 0) return 1;
            }
            TotalScore = 0.0f;
            pSpellRes = pEngine->pRes->pResult;
            for (i = 0; i < cActiveWords; i++) {
                TotalScore += (pNormalisedScore[i] =
                    expf((float)-(long)(pSpellRes->pInfo->Score - BestScore) / 1000.0));
                pSpellRes = pSpellRes->pNext;
            }
            pSpellRes = pEngine->pRes->pResult;
            for (i = 0; i < cActiveWords; i++) {
                p = pNormalisedScore[i] / TotalScore;
                if (p > 0.001f)
                    pAlternatives[i].iConf = -868.589f * logf_check(p + 0.5);
                else pAlternatives[i].iConf = 0x1770;
                pAlternatives[i].lScore = pSpellRes->pInfo->Score;
                WordID = pSpellRes->pInfo->StateNbr;
                pAlternatives[i].idWord = pTheStateDict[WordID + 1].StateIndex +
                    (pTheStateDict[WordID + 2].StateIndex << 0x10);
                pSpellRes = pSpellRes->pNext;
            }
        }
    }
    CallbackErr = pEngine->CRecogCallBacks.cbResult(pCasrResult, &pEngine->UserData);
    if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
        pEngine->CallbackErrLog = CallbackErr;
        RetVal = 0x1B;
    }
    casrFreeResult(pEngine, pCasrResult);
    heap_Free(pEngine->hHeap, pNormalisedScore);
    if (_tosControl(&pEngine->Object, 7, 0, 0) != 0) return 1;
    if (_tosControl(&pEngine->Object, 0x15, 0, 0) != 0) return 1;
    if (casrSetParamNoClearErrorLog(pEngine, 1, TSEnable) != 0) return 1;
    if (casrSetParamNoClearErrorLog(pEngine, 0xD, MaxNBest) != 0) return 1;
    if (_tosControl(&pEngine->Object, 0x24, (u32)pEngine->CtxInfo.pDynProgCtxInfo, 0) != 0 ||
        _tosControl(&pEngine->Object, 0x54,
                    (u32)&pEngine->ParamList.DynProgParam.Accuracy, 0) != 0) {
        return 1;
    }
    pEngine->DoSecondPass = 0;
    if (RetVal != 0) _tosErrorLog(&pEngine->Object, RetVal);
    return RetVal;
}

/* Configures acquisition for the selected input sample format. */
u8 casrInitSampleFormat(struct _ENGINE *pEngine, enum _CSAMPLEFORMAT SampleFormat)
{
    pEngine->CSampleFormat = SampleFormat;
    switch (SampleFormat) {
    case PCM_16_11KHZ:
        pEngine->OutQueue = pEngine->SampleOutQueue;
        pEngine->pActiveBlocks = 0;
        break;
    case UTTFORMAT_CEPSTRA:
    case UTTFORMAT_LABELS:
    case UTTFORMAT_LABELSRAW:
        pEngine->OutQueue = pEngine->LabelOutQueue;
        if (pEngine->IsActiveContextIsoWord != 0) {
            pEngine->pActiveBlocks = ActiveBlocks_UTTFORMAT_LABELS_ISO;
        } else {
            pEngine->pActiveBlocks = ActiveBlocks_UTTFORMAT_LABELS;
        }
        break;
    case UTTFORMAT_UNKNOWN:
    default:
        return 0x20;
    }
    if (SampleFormat != PCM_16_11KHZ) {
        if (_tosControl(&pEngine->Object, 0x6A, 0, 0) != 0) {
            return 1;
        }
    } else if (_tosControl(&pEngine->Object, 0x6B, 0, 0) != 0) {
        return 1;
    }
    if ((pEngine->ParamList.TriggerParam.DoStartDetection != 0) && (SampleFormat != PCM_16_11KHZ) &&
        (_tosControl(&pEngine->Object, 2, 0, 0) != 0)) {
        return 1;
    }
    if ((pEngine->ParamList.TriggerParam.DoStartDetection != 0) && (SampleFormat == PCM_16_11KHZ)) {
        pEngine->StartDetected = 0;
        if (_tosControl(&pEngine->Object, 0x33, 0, 0) != 0) {
            return 1;
        }
    } else {
        pEngine->StartDetected = 1;
        if (_tosControl(&pEngine->Object, 0x34, 0, 0) != 0) {
            return 1;
        }
    }
    pEngine->FrameSize = pEngine->OutQueue.pQ->Size;
    if (SampleFormat == UTTFORMAT_LABELS) {
        if (pEngine->pLang != 0) {
            if ((u16)pEngine->pLang->langGetNbrCodeBook(pEngine->pLang) < 5) {
                pEngine->FrameSize--;
            }
        } else {
            pEngine->FrameSize--;
        }
    }
    return 0;
}

/* Feeds input samples through TOS and collects recognized words. */
int casrAcquisition(struct HCASR__ *hcAsr, enum _CSAMPLEFORMAT SampleFormat,
                    void *pSampleBuf, unsigned long dwBufSize)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    struct _TOS *pTos = &pEngine->Tos;
    unsigned short ActiveState;
    unsigned char *pSamples;
    unsigned long FrameSize;
    unsigned long FrameSizeExtra = SampleFormat == PCM_16_11KHZ ? 0 : 1;
    unsigned long i;
    unsigned long Err;
    unsigned char RetVal;
    Err = casrCheckHandle(pEngine);
    if (Err != 0) return Err;
    if (pSampleBuf == NULL) return (u8)_tosErrorLog(&pEngine->Object, 0x12);
    ActiveState = pEngine->Tos.ActiveState;
    switch (ActiveState) {
    default:
        return (u8)_tosErrorLog(&pEngine->Object, 7);

    case CASR_SLEEP: case CASR_RUN: case CASR_SPELLSLEEP: case CASR_SPELLRUN:
    case CASR_ISOWORDSLEEP: case CASR_ISOWORDRUN: case CASR_RECORDSLEEP: case CASR_RECORD:
        if (pEngine->CSampleFormat == UTTFORMAT_UNKNOWN) {
            RetVal = casrInitSampleFormat(pEngine, SampleFormat);
            if (RetVal != 0) return RetVal;
        } else if (SampleFormat != pEngine->CSampleFormat) {
            return (u8)_tosErrorLog(&pEngine->Object, 0x20);
        }
        break;
    }
    pEngine->Tos.ErrorLog = 0;
    pEngine->CallbackErrLog = 0;
    pSamples = pSampleBuf;
    FrameSize = pEngine->FrameSize;
    dwBufSize += pEngine->SampleBufSize;
    while (dwBufSize >= FrameSize) {
        unsigned char *pSamplesIn = qEnQueueOne(pEngine->OutQueue.pQ);
        if (pSamplesIn == NULL) return 1;
        if (pEngine->SampleBufSize != 0) {
            memcpy(pSamplesIn, pEngine->SampleBuf, pEngine->SampleBufSize);
            memcpy(pSamplesIn + pEngine->SampleBufSize, pSamples,
                FrameSize - pEngine->SampleBufSize);
            pSamples += FrameSize - pEngine->SampleBufSize;
            pEngine->SampleBufSize = 0;
        } else {
            memcpy(pSamplesIn, pSamples, FrameSize);
            pSamples += FrameSize;
        }
        if (FrameSizeExtra != 0) {
            unsigned char SilOut = 0;
            if (pEngine->CSampleFormat == UTTFORMAT_LABELS) {
                SilOut = pSamplesIn[0] >> 7;
                if (SilOut != 0) pSamplesIn[0] &= 0x7F;
            }
            RetVal = qEnQueue(pEngine->SilOutQueue.pQ, &SilOut, 1);
            if (RetVal == 0) return 1;
        }
        pEngine->FrameCount++;
        if (tosRun(pTos, pEngine->pActiveBlocks) != 0) return (u8)pEngine->Tos.ErrorLog;
        if (casrReadLabelInQueue(pEngine, pEngine->CurrUttIdx) != 0) {
            return (u8)pEngine->Tos.ErrorLog;
        }
        if (pEngine->TSDetected != 0) { dwBufSize = 0; break; }
        if ((pEngine->ParamList.ArchiParam.TimeOut != 0 &&
            pEngine->FrameCount > pEngine->ParamList.ArchiParam.TimeOut) ||
            pEngine->DTMFDetected != 0) {
            if (_tosControl(&pEngine->Object, 3, 0, 0) != 0) return (u8)pEngine->Tos.ErrorLog;
            if (casrReadLabelInQueue(pEngine, pEngine->CurrUttIdx) != 0) {
                return (u8)pEngine->Tos.ErrorLog;
            }
            dwBufSize = 0;
            break;
        }
        dwBufSize -= FrameSize;
    }
    if (pEngine->DoSecondPass != 0 && casrDoSecondPass(pEngine) != 0) {
        return (u8)pEngine->Tos.ErrorLog;
    }
    if (pEngine->DoPrompt != PROMPTTYPE_NOPROMPT) {
        s32 PromptType = pEngine->DoPrompt;
        long CallbackErr;
        pEngine->DoPrompt = PROMPTTYPE_NOPROMPT;
        CallbackErr = pEngine->CRecogCallBacks.cbTrain(PromptType, &pEngine->UserData);
        if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
            pEngine->CallbackErrLog = CallbackErr;
            return (u8)_tosErrorLog(&pEngine->Object, 0x1B);
        }
    } else {
        for (i = pEngine->SampleBufSize; i < dwBufSize; i++) {
            pEngine->SampleBuf[i] = *pSamples++;
        }
        pEngine->SampleBufSize = dwBufSize;
    }
    return (u8)pEngine->Tos.ErrorLog;
}

/* Acknowledges a recognized user word when the TOS state accepts it. */
int casrAcknowledge(struct HCASR__ *hcAsr, unsigned long Decision)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    unsigned char RetVal;
    unsigned short ActiveState;
    unsigned long Err;
    int DoPrompt;
    long CallbackErr;
    Err = casrCheckHandle(pEngine);
    if (Err) return Err;
    ActiveState = pEngine->Tos.ActiveState;
    if (ActiveState != CASR_PROMPT && ActiveState != CASR_ACCEPT && ActiveState != CASR_CONFIRM) {
        return _tosErrorLog(&pEngine->Object, 7);
    }
    RetVal = userwordAcknowledge(pEngine, Decision);
    if (pEngine->DoPrompt != PROMPTTYPE_NOPROMPT) {
        DoPrompt = pEngine->DoPrompt;
        pEngine->DoPrompt = PROMPTTYPE_NOPROMPT;
        CallbackErr = pEngine->CRecogCallBacks.cbTrain(DoPrompt, &pEngine->UserData);
        if (pEngine->CallbackErrLog == 0 && CallbackErr != 0) {
            pEngine->CallbackErrLog = CallbackErr;
            RetVal = _tosErrorLog(&pEngine->Object, 0x1B);
        }
    }
    return RetVal;
}

/* Initializes the recognizer for utterance segmentation. */
void casrInitSegmentation(struct _ENGINE *pEngine)
{
    unsigned long TotalSegSize = 0;
    unsigned long i;
    unsigned short *pSeg;
    unsigned short **ppSeg = pEngine->ppUserwordSegmentation;
    unsigned long Incr;
    unsigned long j;
    if (*ppSeg != 0) heap_Free(pEngine->hHeap, *ppSeg);
    for (i = 0; i < 3; i++) TotalSegSize += pEngine->UserWordUtt[i].NbrPhenoms + 2;
    TotalSegSize *= 3;
    *ppSeg = heap_Alloc(pEngine->hHeap, TotalSegSize * sizeof(unsigned short));
    pSeg = *ppSeg;
    for (i = 0; i < 3; i++) {
        Incr = pEngine->UserWordUtt[i].NbrPhenoms + 2;
        for (j = 0; j < 3; j++) {
            pSeg += Incr;
            *++ppSeg = pSeg;
        }
    }
}

/* Returns the segmentation start array for a training transcription and utterance. */
unsigned short *casrGetSegmStart(struct _ENGINE *pEngine, unsigned long TransCnt,
                                 unsigned long UttCnt)
{
    return (pEngine->ppUserwordSegmentation + TransCnt * 3)[UttCnt];
}

/* Stores backtrace exit times for the selected transcription and utterance. */
void casrStoreSegmentation(
    struct _ENGINE *pEngine, unsigned long TransCnt, unsigned long UttCnt,
    struct _SCOREUWBACKTRACE *pBackTrace)
{
    u16 *start = (pEngine->ppUserwordSegmentation + TransCnt * 3)[UttCnt];

    if (pBackTrace != 0) {
        while (pBackTrace != 0) {
            *start = pBackTrace->ExitTime;
            start++;
            pBackTrace = pBackTrace->Previous;
        }
        return;
    }
    *start = 0xFFFF;
}

/* Returns and clears the callback error reported by the recognizer. */
int casrGetCallbackErr(struct HCASR__ *hcAsr, int *pCallbackErr)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)hcAsr;
    u32 result;

    result = casrCheckHandle(pEngine);
    if (result != 0) {
        return result;
    }
    *pCallbackErr = pEngine->CallbackErrLog;
    pEngine->CallbackErrLog = 0;
    return 0;
}
