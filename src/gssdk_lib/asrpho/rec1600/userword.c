/* Trains user-word models from recorded utterances and scores them. */
#include "types.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include <string.h>

#include "gssdk/casr.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/mqueue.h"
#include "gssdk/train.h"

/* Releases stored utterances and stops the user-word controls. */
u8 ExitUserWordTraining(struct _ENGINE *pEngine);

/* Builds a user-word model or flags utterances for another recording. */
u8 DoUserWordTraining(struct _ENGINE *pEngine);

/* Zero-terminated IDs of the blocks that receive queued labels. */
const u8 LabelBlockIDs[] = { 0x1F, 0x20, 0x21, 0x22, 0 };

/* The acknowledgement advances or rejects the current training prompt. */
u8 userwordAcknowledge(struct _ENGINE *pEngine, unsigned long Decision) {
    u8 RetVal;

    switch (Decision) {
    case 0x1:
        if (pEngine->CurrUttIdx >= pEngine->NbrUttForTrain - 1) {
            RetVal = _tosControl(&pEngine->Object, 0x16, 0, 0);
            if ((pEngine->Tos.ActiveState == CASR_CONFIRM || pEngine->Tos.ActiveState
                == CASR_UWTRAINED) &&
                pEngine->UWUttNotAcceptedByEngine != 0) {
                if (pEngine->Tos.ActiveState == CASR_UWTRAINED) {
                    if (_tosControl(&pEngine->Object, 0x23, 0, 0) != 0) {
                        return 1;
                    }
                    if (_tosControl(&pEngine->Object, 0x15, 0, 0) != 0) {
                        return 1;
                    }
                }
                if (_tosControl(&pEngine->Object, 0x69, 0, 0) != 0) {
                    return 1;
                }
            }
        } else {
            RetVal = _tosControl(&pEngine->Object, 0xA, 0, 0);
        }
        if (pEngine->Tos.ActiveState == CASR_RECORDSLEEP &&
            pEngine->ParamList.TriggerParam.DoStartDetection == 0) {
            RetVal |= _tosControl(&pEngine->Object, 0x2, 0, 0);
        }
        break;
    case 0x3:
        RetVal = _tosControl(&pEngine->Object, 0xB, 0, 0);
        break;
    case 0x2:
        RetVal = _tosControl(&pEngine->Object, 0xC, 0, 0);
        break;
    default:
        RetVal = _tosErrorLog(&pEngine->Object, 0x11);
        break;
    }
    return RetVal;
}

/* Cancellation clears the output addresses and exits active training. */
unsigned char cbCancelUserWordTraining(struct _TOS *pTos)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;

    pEngine->AddrUserWordBuff = NULL;
    pEngine->AddrUserWordBuffSize = NULL;
    if (pEngine->bUserWordTraining != 0 && ExitUserWordTraining(pEngine) != 0) {
        return 1;
    }
    return 0;
}

/* Resets the current utterance and asks for another recording. */
unsigned char cbUtteranceNotAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *) pTos;

    pEngine->pWriteLab[pEngine->CurrUttIdx] = pEngine->UtteranceLabels[pEngine->CurrUttIdx];
    heap_Free(pEngine->hHeap, pEngine->UserWordUtt[pEngine->CurrUttIdx].pPhenoms);
    pEngine->UserWordUtt[pEngine->CurrUttIdx].pPhenoms = 0;
    pEngine->DoPrompt = PROMPTTYPE_START;
    return 0;
}

/* The callback advances to the next utterance after acceptance. */
unsigned char cbUtteranceAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *) pTos;

    if (pEngine->UserWordUtt[pEngine->CurrUttIdx].SpeechFrames <
        pEngine->ParamList.ArchiParam.UWMinSpeechDur) {
        _tosWarningLog(&pEngine->Object, 0x23);
        pEngine->UWUttNotAcceptedByEngine = 1;
        return cbUtteranceNotAccepted(pTos, BlockId, ControlId, CParam1, CParam2);
    }
    pEngine->CurrUttIdx++;
    pEngine->UWUttNotAcceptedByEngine = 0;
    pEngine->DoPrompt = PROMPTTYPE_START;
    return 0;
}

/* The callback advances from the final utterance to confirmation. */
unsigned char cbLastUtteranceAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *) pTos;

    if (pEngine->UserWordUtt[pEngine->CurrUttIdx].SpeechFrames <
        pEngine->ParamList.ArchiParam.UWMinSpeechDur) {
        _tosWarningLog(&pEngine->Object, 0x23);
        pEngine->UWUttNotAcceptedByEngine = 1;
        return cbUtteranceNotAccepted(pTos, BlockId, ControlId, CParam1, CParam2);
    }
    pEngine->CurrUttIdx++;
    pEngine->UWUttNotAcceptedByEngine = 0;
    pEngine->DoPrompt = PROMPTTYPE_CONFIRM;
    return 0;
}

/* The callback stores the current utterance result and copies its phonemes. */
unsigned long cbUserWordResultAvailable(
    struct _TOS *pTos, unsigned char BlockId, unsigned long ControlId, void *CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    const struct _USERWORDRES *pResult = CParam1;
    int UttIdx = pEngine->CurrUttIdx;
    unsigned long NbrPhenoms = pResult->NbrPhenoms;
    struct _USERWORDUTT *pUtt = &pEngine->UserWordUtt[UttIdx];
    unsigned short **ppPhenoms = &pEngine->UserWordUtt[UttIdx].pPhenoms;

    pUtt->SpeechFrames = pResult->SpeechFrames;
    pUtt->NbrPhenoms = NbrPhenoms;
    if (*ppPhenoms != NULL) {
        heap_Free(pEngine->hHeap, *ppPhenoms);
    }
    if (NbrPhenoms != 0) {
        NbrPhenoms *= sizeof(unsigned short);
        *ppPhenoms = heap_Alloc(pEngine->hHeap, NbrPhenoms);
        memcpy(*ppPhenoms, pResult->pPhenoms, NbrPhenoms);
    } else {
        *ppPhenoms = NULL;
    }
    pEngine->DoPrompt = PROMPTTYPE_ACCEPT;
    return 0;
}

/* The callback saves the most recent user-word scores. */
unsigned long cbScoreUserWordResultAvailable(
    struct _TOS *pTos, unsigned char BlockId, unsigned long ControlId, void *CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;
    const struct _SCOREUSERWORDRES *pResult = CParam1;

    pEngine->LastUserWordRes = *pResult;
    return 0;
}

/* Releases the utterance phonemes and exits training. */
u8 ExitUserWordTraining(struct _ENGINE *pEngine) {
    u32 i = 0;

    while (i < pEngine->NbrUttForTrain) {
        if (pEngine->UserWordUtt[i].pPhenoms != NULL) {
            heap_Free(pEngine->hHeap, pEngine->UserWordUtt[i].pPhenoms);
        }
        i++;
    }
    pEngine->DoPrompt = PROMPTTYPE_TRAINEND;
    if (_tosControl(&pEngine->Object, 0x23, 0, 0) != 0) {
        return 1;
    }
    if (_tosControl(&pEngine->Object, 0x15, 0, 0) != 0) {
        return 1;
    }
    if (_tosControl(&pEngine->Object, 0x10, 0, 0) != 0) {
        return 1;
    }
    pEngine->bUserWordTraining = 0;
    return 0;
}

/* Sets the label queue control to zero and sends the label-start event. */
u8 DoLabelStart(struct _ENGINE *pEngine) {
    if (casrEnableLabelInQueue(pEngine, 0) != 0) {
        return 1;
    }
    return _tosControl(&pEngine->Object, 0x1F, 0, 0);
}

/* The routine sends a block of stored labels to the engine. */
u8 SendPartOfLabelsToEngine(struct _ENGINE *pEngine, const u8 *pBlockIDs, unsigned int UttIdx,
                            unsigned int StartIdx, unsigned int EndIdx) {
    struct _TOS *pTos = &pEngine->Tos;
    u8 *pRead = pEngine->UtteranceLabels[UttIdx] + StartIdx * pEngine->LabelOutQueue.pQ->Size;
    u8 *pEnd = pEngine->UtteranceLabels[UttIdx] + EndIdx * pEngine->LabelOutQueue.pQ->Size;

    if (pRead > pEngine->pWriteLab[UttIdx]) {
        pRead = pEngine->pWriteLab[UttIdx];
    }
    if (pEnd > pEngine->pWriteLab[UttIdx]) {
        pEnd = pEngine->pWriteLab[UttIdx];
    }
    while (pRead < pEnd) {
        if ((u8) qEnQueue(pEngine->LabelOutQueue.pQ, pRead, 1) == 0) {
            return _tosErrorLog(&pEngine->Object, 1);
        }
        pRead += pEngine->LabelOutQueue.pQ->Size;
        if (tosRun(pTos, pBlockIDs) != 0) {
            return 1;
        }
    }
    return 0;
}

/* The selected utterance's label segments are sent to the engine. */
u8 SendLabelsToEngine(struct _ENGINE *pEngine, const u8 *pBlockIDs, unsigned int UttIdx) {
    u8 *pEnd;
    u8 *pRead;

    pEnd = pEngine->pWriteLab[UttIdx];
    pRead = pEngine->UtteranceLabels[UttIdx];
    return SendPartOfLabelsToEngine(pEngine, pBlockIDs, UttIdx, 0,
                                    (unsigned long) (pEnd - pRead) /
                                        pEngine->LabelOutQueue.pQ->Size);
}

/* Sends the label-stop event and sets the label queue control to one. */
u8 DoLabelStop(struct _ENGINE *pEngine) {
    if (_tosControl(&pEngine->Object, 0x20, 0, 0) != 0) {
        return 1;
    }
    return casrEnableLabelInQueue(pEngine, 1);
}

/* The callback runs training and releases stored utterances after completion. */
unsigned char cbDoUserWordTraining(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, void *CParam1,
    unsigned long CParam2)
{
    struct _ENGINE *pEngine = (struct _ENGINE *)pTos;

    if (DoUserWordTraining(pEngine) != 0) {
        return 1;
    }
    if (pEngine->UWUttNotAcceptedByEngine == 0 && ExitUserWordTraining(pEngine) != 0) {
        return 1;
    }
    return 0;
}

/* The first feature compares the paired length with the other lengths for an utterance. */
static void CalcF1(f32 *F1, f32 *pLength, u8 NbrUtt) {
    unsigned char u, t;
    float S, F;

    for (t = 0; t < NbrUtt; t++) {
        S = 0.0f;
        for (u = 0; u < NbrUtt; u++) {
            if (u != t) {
                S += pLength[u * NbrUtt + t];
            }
        }
        F = S ? (1.305f < pLength[t * NbrUtt + t] / S - 0.5f ? 1.305f
                                                             : pLength[t * NbrUtt + t] / S - 0.5f)
              : 1.305f;
        F1[t] = expf(-powf(0.0f > F ? 0.0f : F, 2.0f) / 0.02f);
    }
}

/* The second feature measures how far an utterance length lies from the others. */
static void CalcF2(f32 *F2, f32 *pLength, u8 NbrUtt) {
    unsigned char t, t1, t2, u1, u2;
    float X[9];
    float F, q1, q2, quot;

    for (t1 = 0; t1 < NbrUtt; t1++) {
        for (t2 = 0; t2 < NbrUtt; t2++) {
            X[t1 * NbrUtt + t2] = 0.0f;
            for (u1 = 0; u1 < NbrUtt; u1++) {
                if (u1 != t2) {
                    X[t1 * NbrUtt + t2] += pLength[t1 * NbrUtt + u1];
                }
            }
        }
    }
    for (t = 0; t < NbrUtt; t++) {
        F = 0.0f;
        for (u2 = 0; u2 < NbrUtt; u2++) {
            if (t != u2) {
                q1 = X[u2 * NbrUtt + t] ? X[t * NbrUtt + t] / X[u2 * NbrUtt + t] : 0.0f;
                q2 = X[t * NbrUtt + t] ? X[u2 * NbrUtt + t] / X[t * NbrUtt + t] : 0.0f;
                quot = q1 < q2 ? q1 : q2;
                F += quot;
            }
        }
        F /= (float) (NbrUtt - 1);
        F = F < 1.923 ? F : 1.923;
        F2[t] = expf(-powf(F - 1.0f, 2.0f) / 0.04f);
    }
}

/* The third feature compares the paired score with the other scores for an utterance. */
static void CalcF3(f32 *F3, f32 *pScore, u8 NbrUtt) {
    unsigned char t, u;
    float S, F;

    for (t = 0; t < NbrUtt; t++) {
        S = 0.0f;
        for (u = 0; u < NbrUtt; u++) {
            if (u != t) {
                S += pScore[u * NbrUtt + t];
            }
        }
        F = S ? 2.0f * pScore[t * NbrUtt + t] / S : 0.0f;
        F = -0.923f > F - 0.9f ? -0.923f : F - 0.9f;
        F3[t] = expf(-powf(F, 2.0f) / 0.02f);
    }
}

/* Divides the mean off-diagonal score by the largest diagonal score. */
static f32 CalcMeanScore(f32 *pScore, u8 NbrUtt) {
    u8 t;
    u8 u;
    u16 Nbr;
    f32 Mean = 0.0f;
    f32 Max = 0.0f;

    Nbr = 0;
    for (t = 0; t < NbrUtt; t++) {
        for (u = 0; u < NbrUtt; u++) {
            if (t != u) {
                Nbr++;
                Mean += pScore[t * NbrUtt + u];
            } else {
                f32 diagonalScore = pScore[t * NbrUtt + u];
                Max = Max > diagonalScore ? Max : diagonalScore;
            }
        }
    }
    Mean /= (f32) Nbr;
    return Mean / Max;
}

/* The score comparisons select the best training transcription. */
static u8 FindBestTranscription(struct _ENGINE *pEngine, f32 (*pScore)[3], f32 (*pLength)[3],
                                u32 *BestTransIdx) {
    u32 BestTransIdx2 = 0;
    f32 BestTransF = 0.0f;
    f32 BestTransF1 = 0.0f;
    f32 F1[3];
    f32 F2[3];
    f32 F3[3];
    f32 maxF = 0.0f;
    f32 meanF = 0.0f;
    f32 minF = 3.0f;
    f32 NormMeanScore;
    u8 t;
    f32 F1_2;
    f32 F;

    CalcF1(F1, *pLength, 3);
    CalcF2(F2, *pLength, 3);
    CalcF3(F3, *pScore, 3);
    NormMeanScore = CalcMeanScore(*pScore, 3);

    for (t = 0; t < 3; t++) {
        F1_2 = F1[t] < F2[t] ? 2.0f * F1[t] : 2.0f * F2[t];
        F = F1_2 + F3[t];
        meanF += F;
        if (F > maxF) {
            BestTransIdx2 = t;
            maxF = F;
        }
        if (F < minF) {
            minF = F;
        }
        if (t == *BestTransIdx) {
            BestTransF = F;
            BestTransF1 = F1[t];
        }
    }
    meanF /= 3.0f;

    if (BestTransF <= 1.4 || BestTransF1 <= 0.6) {
        *BestTransIdx = BestTransIdx2;
    }
    return 0;
}

/* Duration modeling uses the lengths of the selected transcription. */
static u8 DoDurationModeling(struct _ENGINE *pEngine, unsigned int BestTransIdx) {
    unsigned long oldNrPhenomes = pEngine->UserWordUtt[BestTransIdx].NbrPhenoms;
    unsigned long newNrPhenomes;
    unsigned short *pPhenoms = pEngine->UserWordUtt[BestTransIdx].pPhenoms;
    unsigned short *pNewPhenoms;
    unsigned long oldSegmentSize = oldNrPhenomes + 2;
    unsigned short *pOldSegmentBase = heap_Calloc(
        pEngine->hHeap, oldSegmentSize * pEngine->NbrUttForTrain, sizeof(unsigned short));
    unsigned short *pOldSegment = pOldSegmentBase;
    unsigned long *nrRepetitions;
    long phenomeIdx;
    long newPhenomIndx;
    unsigned short UttCnt;
    for (UttCnt = 0; UttCnt < (unsigned long) pEngine->NbrUttForTrain; UttCnt++) {
        memcpy(pOldSegment, casrGetSegmStart(pEngine, BestTransIdx, UttCnt),
               oldSegmentSize * sizeof(unsigned short));
        pOldSegment += oldSegmentSize;
    }
    pOldSegment = pOldSegmentBase;
    newNrPhenomes = 0;
    nrRepetitions = heap_Calloc(pEngine->hHeap, oldNrPhenomes, sizeof(unsigned long));
    for (phenomeIdx = oldNrPhenomes - 1; phenomeIdx >= 0; phenomeIdx--) {
        unsigned short *fromPtr = pOldSegment++;
        if (pPhenoms[phenomeIdx] == pEngine->UserWordSilPhenom) {
            nrRepetitions[phenomeIdx] = 1;
            newNrPhenomes++;
        } else {
            unsigned long minDuration = 0x2710;
            for (UttCnt = 0; UttCnt < (unsigned long) pEngine->NbrUttForTrain; UttCnt++) {
                unsigned long thisDuration = fromPtr[0] - fromPtr[1];
                if (thisDuration < minDuration) {
                    minDuration = thisDuration;
                }
                fromPtr += oldSegmentSize;
            }
            nrRepetitions[phenomeIdx] = (minDuration + 1) >> 1;
            newNrPhenomes += nrRepetitions[phenomeIdx];
        }
    }
    pNewPhenoms = heap_Calloc(pEngine->hHeap, newNrPhenomes, sizeof(unsigned short));
    newPhenomIndx = newNrPhenomes - 1;
    for (phenomeIdx = oldNrPhenomes - 1; phenomeIdx >= 0; phenomeIdx--) {
        unsigned long repCount;
        for (repCount = 0; repCount < nrRepetitions[phenomeIdx]; repCount++) {
            pNewPhenoms[newPhenomIndx--] = pPhenoms[phenomeIdx];
        }
    }
    heap_Free(pEngine->hHeap, pEngine->UserWordUtt[BestTransIdx].pPhenoms);
    pEngine->UserWordUtt[BestTransIdx].NbrPhenoms = newNrPhenomes;
    pEngine->UserWordUtt[BestTransIdx].pPhenoms = pNewPhenoms;
    casrInitSegmentation(pEngine);
    pOldSegment = pOldSegmentBase;
    for (UttCnt = 0; UttCnt < (unsigned long) pEngine->NbrUttForTrain; UttCnt++) {
        unsigned short *pNewSegmentation = casrGetSegmStart(pEngine, BestTransIdx, UttCnt);
        for (phenomeIdx = oldNrPhenomes - 1; phenomeIdx >= 0; phenomeIdx--, pOldSegment++) {
            unsigned long repCount;
            unsigned short thisExitTime = pOldSegment[0];
            unsigned short thisDuration = thisExitTime - pOldSegment[1];
            unsigned short newDuration = thisDuration / nrRepetitions[phenomeIdx];
            unsigned long sumDuration = 0;
            for (repCount = 0; repCount < nrRepetitions[phenomeIdx]; repCount++) {
                *pNewSegmentation++ = thisExitTime - sumDuration;
                sumDuration += newDuration;
            }
        }
        for (; phenomeIdx >= -2; phenomeIdx--) {
            *pNewSegmentation++ = *pOldSegment++;
        }
    }
    heap_Free(pEngine->hHeap, nrRepetitions);
    heap_Free(pEngine->hHeap, pOldSegmentBase);
    return 0;
}

/* The training utterances are scored against the candidate transcriptions. */
static u8 ScoreTranscriptions(struct _ENGINE *pEngine, f32 (*pNormScore)[3], f32 (*pLength)[3],
                              u32 *BestTransIdx, u8 *pRejectedUttrs, u8 *anyRejected) {
    struct _LANGUAGE *pLang;
    unsigned int TransCnt;
    long UttCnt;
    unsigned long BestScore;
    unsigned long pScore[3];
    int phenomeCount[3];
    u16 *pReducedPhenomeList;
    unsigned int maxNbrPhenoms;
    u8 retValue;
    u8 bSecondTime;
    unsigned long reducedUttIdx;

    pLang = pEngine->pLang;
    BestScore = 0xFFFFFFFF;
    pReducedPhenomeList = NULL;
    maxNbrPhenoms = 0;
    retValue = 0;
    bSecondTime = 0;
    reducedUttIdx = 0;
    *anyRejected = 0;

    if (_tosControl(&pEngine->Object, 0x59, (u32) pLang, 0) != 0) {
        retValue = 1;
        goto end;
    }
    if (_tosControl(&pEngine->Object, 0x11, 0, 0) != 0) {
        retValue = 1;
        goto end;
    }
    if (_tosControl(&pEngine->Object, 0x22, 0, 0) != 0) {
        retValue = 1;
        goto end;
    }
    for (TransCnt = 0; TransCnt < pEngine->NbrUttForTrain; TransCnt++) {
        if (pEngine->UserWordUtt[TransCnt].NbrPhenoms > maxNbrPhenoms) {
            maxNbrPhenoms = pEngine->UserWordUtt[TransCnt].NbrPhenoms;
        }
    }
    pReducedPhenomeList = heap_Calloc(pEngine->hHeap, maxNbrPhenoms, 2);
    casrInitSegmentation(pEngine);
    pEngine->UserWordSilPhenom = pLang->langGetSilencePhenome(pLang);

    for (TransCnt = 0; TransCnt < pEngine->NbrUttForTrain; TransCnt++) {
        unsigned long TransScore = 0;
        unsigned long Size;
        struct _INITSCOREWUSERWORDDATA InitData;
        u16 *pPhenoms = pEngine->UserWordUtt[TransCnt].pPhenoms;
        u16 *pPhenomsEnd = pPhenoms + pEngine->UserWordUtt[TransCnt].NbrPhenoms;
        u16 UWSilPhenome = pLang->langGetUWSilencePhenome(pLang);
        u8 reducedListRecorded = 0;
        u8 multipleReduced = 0;
        int reducedListPhenomeCount = 0;

        for (; pPhenoms < pPhenomsEnd; pPhenoms++) {
            if (*pPhenoms == UWSilPhenome) {
                *pPhenoms = pLang->langGetSilencePhenome(pLang);
            }
        }
        for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
            pScore[UttCnt] = 0;
            phenomeCount[UttCnt] = 0;
        }
        InitData.NbrPhenoms = pEngine->UserWordUtt[TransCnt].NbrPhenoms;
        InitData.pPhenoms = pEngine->UserWordUtt[TransCnt].pPhenoms;
        InitData.NbrSilPhenoms = 1;
        InitData.pSilPhenoms = &pEngine->UserWordSilPhenom;
        Size = (InitData.NbrPhenoms + InitData.NbrSilPhenoms) * sizeof(*InitData.pPhenoms) +
               sizeof(InitData);
        if (_tosControl(&pEngine->Object, 0x17, 1, 0) != 0) {
            retValue = 1;
            goto end;
        }
        bSecondTime = 0;
        for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
            if ((int) pEngine->UserWordUtt[TransCnt].NbrPhenoms != phenomeCount[UttCnt]) {
                if (_tosControl(&pEngine->Object, 0x13, (u32) &InitData, Size) != 0) {
                    retValue = 1;
                    goto end;
                }
                if (DoLabelStart(pEngine) != 0) {
                    retValue = 1;
                    goto end;
                }
                if (SendLabelsToEngine(pEngine, LabelBlockIDs, UttCnt)) {
                    retValue = 1;
                    goto end;
                }
                if (DoLabelStop(pEngine) != 0) {
                    retValue = 1;
                    goto end;
                }
                pScore[UttCnt] = pEngine->LastUserWordRes.UWScore;
                TransScore += pEngine->LastUserWordRes.UWScore;
                pNormScore[TransCnt][UttCnt] = pEngine->LastUserWordRes.NormScore;
                pLength[TransCnt][UttCnt] = (u32) pEngine->LastUserWordRes.SpeechLength;
                casrStoreSegmentation(pEngine, TransCnt, UttCnt,
                                      pEngine->LastUserWordRes.pBackTrace);
                phenomeCount[UttCnt] = pEngine->LastUserWordRes.NbrPhenoms - 2;
                if ((u32) pEngine->LastUserWordRes.NbrPhenoms <
                    pEngine->UserWordUtt[TransCnt].NbrPhenoms + 2) {
                    if (!reducedListRecorded) {
                        struct _SCOREUWBACKTRACE *Bt = pEngine->LastUserWordRes.pBackTrace;
                        int locPhenomeCount;

                        if ((u32) pEngine->LastUserWordRes.NbrPhenoms < 2) {
                            locPhenomeCount = 0;
                        } else {
                            locPhenomeCount = pEngine->LastUserWordRes.NbrPhenoms - 2;
                        }
                        reducedUttIdx = UttCnt;
                        reducedListRecorded = 1;
                        reducedListPhenomeCount = locPhenomeCount;
                        while (--locPhenomeCount >= 0) {
                            pReducedPhenomeList[locPhenomeCount] = Bt->State;
                            Bt = Bt->Previous;
                        }
                    } else {
                        u8 sameList;
                        int currentNrPhenomes;
                        if ((u32) pEngine->LastUserWordRes.NbrPhenoms < 2) {
                            currentNrPhenomes = 0;
                        } else {
                            currentNrPhenomes = pEngine->LastUserWordRes.NbrPhenoms - 2;
                        }

                        if (reducedListPhenomeCount == currentNrPhenomes) {
                            struct _SCOREUWBACKTRACE *Bt = pEngine->LastUserWordRes.pBackTrace;
                            int locPhenomeCount = reducedListPhenomeCount;

                            sameList = 1;
                            while (--locPhenomeCount >= 0 && sameList) {
                                sameList = pReducedPhenomeList[locPhenomeCount] == Bt->State;
                                Bt = Bt->Previous;
                            }
                        } else {
                            sameList = 0;
                        }
                        if (sameList) {
                            multipleReduced = 1;
                        } else {
                            int UttCnt2;

                            for (UttCnt2 = 0; UttCnt2 < (long) pEngine->NbrUttForTrain; UttCnt2++) {
                                pRejectedUttrs[UttCnt2] = 1;
                            }
                            *anyRejected = 1;
                            goto end;
                        }
                    }
                }
            } else {
                TransScore += pScore[UttCnt];
            }
            if (UttCnt == (long) pEngine->NbrUttForTrain - 1 && reducedListRecorded) {
                if (multipleReduced) {
                    int locPhenomeCount = 0;

                    if (reducedListPhenomeCount == 0) {
                        *anyRejected = 1;
                        pRejectedUttrs[TransCnt] = 1;
                        TransScore = 0xFFFFFFFF;
                        break;
                    }
                    pEngine->UserWordUtt[TransCnt].NbrPhenoms = reducedListPhenomeCount;
                    pPhenoms = pEngine->UserWordUtt[TransCnt].pPhenoms;
                    /* The copy reads the incremented index, starting at one. */
                    while (locPhenomeCount++ < reducedListPhenomeCount) {
                        *pPhenoms++ = pReducedPhenomeList[locPhenomeCount];
                    }
                    InitData.NbrPhenoms = pEngine->UserWordUtt[TransCnt].NbrPhenoms;
                    Size = (InitData.NbrPhenoms + InitData.NbrSilPhenoms) *
                               sizeof(*InitData.pPhenoms) +
                           sizeof(InitData);
                } else if (_tosControl(&pEngine->Object, 0x17, 0, 0) != 0) {
                    retValue = 1;
                    goto end;
                }
                if (bSecondTime) {
                    *anyRejected = 1;
                    pRejectedUttrs[reducedUttIdx] = 1;
                    TransScore = 0xFFFFFFFF;
                } else {
                    TransScore = 0;
                    reducedListPhenomeCount = 0;
                    reducedListRecorded = 0;
                    multipleReduced = 0;
                    UttCnt = -1;
                    bSecondTime = 1;
                }
            }
        }
        if (TransScore < BestScore) {
            BestScore = TransScore;
            *BestTransIdx = TransCnt;
        }
    }
end:
    if (pReducedPhenomeList != NULL) {
        heap_Free(pEngine->hHeap, pReducedPhenomeList);
    }
    return retValue;
}

/* Rescores the utterances and tests the two highest confidence values against the rejection
 * thresholds. */
static u8 CheckConsistency(struct _ENGINE *pEngine, unsigned int BestTransIndx, u8 *pRejectedUttrs,
                           u8 *anyRejected) {
    int UttCnt;
    int maxConfIndx = -1;
    long confValue;
    long maxConfValue = -1000;
    long max2ndConfValue = -1000;
    unsigned long Size;
    struct _INITSCOREWUSERWORDDATA InitData;

    InitData.NbrPhenoms = pEngine->UserWordUtt[BestTransIndx].NbrPhenoms;
    InitData.pPhenoms = pEngine->UserWordUtt[BestTransIndx].pPhenoms;
    InitData.NbrSilPhenoms = 1;
    InitData.pSilPhenoms = &pEngine->UserWordSilPhenom;
    Size = (InitData.NbrPhenoms + InitData.NbrSilPhenoms) * sizeof(*InitData.pPhenoms) +
           sizeof(InitData);

    if (_tosControl(&pEngine->Object, 0x17, 0, 0) != 0) {
        return 1;
    }
    for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
        if (_tosControl(&pEngine->Object, 0x13, (u32) &InitData, Size) != 0) {
            return 1;
        }
        if (DoLabelStart(pEngine) != 0) {
            return 1;
        }
        if (SendLabelsToEngine(pEngine, LabelBlockIDs, UttCnt)) {
            return 1;
        }
        if (DoLabelStop(pEngine) != 0) {
            return 1;
        }
        confValue = pEngine->LastUserWordRes.ConfValue;
        if (confValue > maxConfValue) {
            max2ndConfValue = maxConfValue;
            maxConfValue = confValue;
            maxConfIndx = UttCnt;
        } else if (confValue > max2ndConfValue) {
            max2ndConfValue = confValue;
        }
    }
    if (maxConfValue > pEngine->ParamList.ArchiParam.UWConfThres1) {
        *anyRejected = 1;
        if (max2ndConfValue < pEngine->ParamList.ArchiParam.UWConfThres2) {
            pRejectedUttrs[maxConfIndx] = 1;
        } else {
            for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
                pRejectedUttrs[UttCnt] = 1;
            }
        }
    }
    return 0;
}

/* Rejected utterances are freed and removed from the training set. */
static void RejectTrainUtters(struct _ENGINE *pEngine, u8 *pRejectedUttrs) {
    int rejectCount = 0;
    int FromUttCnt;
    int UttCnt;
    int ToUttCnt;

    for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
        rejectCount += pRejectedUttrs[UttCnt];
    }
    if (rejectCount > 1) {
        rejectCount = pEngine->NbrUttForTrain;
        for (UttCnt = 0; UttCnt < (long) pEngine->NbrUttForTrain; UttCnt++) {
            pRejectedUttrs[UttCnt] = 1;
        }
    }

    for (ToUttCnt = FromUttCnt = 0; FromUttCnt < (long) pEngine->NbrUttForTrain; FromUttCnt++) {
        if (pRejectedUttrs[FromUttCnt] != 0) {
            heap_Free(pEngine->hHeap, pEngine->UserWordUtt[FromUttCnt].pPhenoms);
            pEngine->UserWordUtt[FromUttCnt].pPhenoms = 0;
            pEngine->UserWordUtt[FromUttCnt].NbrPhenoms = 0;
            pEngine->pWriteLab[FromUttCnt] = pEngine->UtteranceLabels[FromUttCnt];
        } else {
            if (ToUttCnt != FromUttCnt) {
                u8 *value;

                pEngine->UserWordUtt[ToUttCnt].pPhenoms = pEngine->UserWordUtt[FromUttCnt].pPhenoms;
                pEngine->UserWordUtt[FromUttCnt].pPhenoms = 0;
                pEngine->UserWordUtt[ToUttCnt].NbrPhenoms =
                    pEngine->UserWordUtt[FromUttCnt].NbrPhenoms;
                pEngine->UserWordUtt[FromUttCnt].NbrPhenoms = 0;
                value = pEngine->UtteranceLabels[ToUttCnt];
                pEngine->UtteranceLabels[ToUttCnt] = pEngine->UtteranceLabels[FromUttCnt];
                pEngine->UtteranceLabels[FromUttCnt] = value;
                value = pEngine->pWriteLab[ToUttCnt];
                pEngine->pWriteLab[ToUttCnt] = pEngine->pWriteLab[FromUttCnt];
                pEngine->pWriteLab[FromUttCnt] = value;
                value = pEngine->pEndLab[ToUttCnt];
                pEngine->pEndLab[ToUttCnt] = pEngine->pEndLab[FromUttCnt];
                pEngine->pEndLab[FromUttCnt] = value;
            }
            ToUttCnt++;
        }
    }
    pEngine->CurrUttIdx -= rejectCount;
    pEngine->DoPrompt = PROMPTTYPE_START;
    pEngine->UWUttNotAcceptedByEngine = 1;
}

/* Selects a transcription and models the accepted training utterances. */
u8 DoUserWordTraining(struct _ENGINE *pEngine) {
    u8 Rej;
    f32 pScore[3][3];
    f32 pLength[3][3];
    u32 BestTransIdx;
    u8 pRejectedUttrs[3];

    BestTransIdx = 0;
    {
        s32 index;

        for (index = 0; index < (long) pEngine->NbrUttForTrain; index++) {
            pRejectedUttrs[index] = 0;
        }
    }
    *pEngine->AddrUserWordBuffSize = 0;
    *(void **) pEngine->AddrUserWordBuff = NULL;
    pEngine->UWUttNotAcceptedByEngine = 0;
    if (ScoreTranscriptions(pEngine, pScore, pLength, &BestTransIdx, pRejectedUttrs,
                            &Rej) != 0) {
        return 1;
    }
    if (Rej != 0) {
        pEngine->NbrStates = 0;
        RejectTrainUtters(pEngine, pRejectedUttrs);
        _tosWarningLog(&pEngine->Object, 0x24);
        return 0;
    }
    pEngine->NbrStates = pEngine->UserWordUtt[BestTransIdx].NbrPhenoms;
    if (FindBestTranscription(pEngine, pScore, pLength, &BestTransIdx) != 0) {
        return 1;
    }
    pEngine->NbrStates = pEngine->UserWordUtt[BestTransIdx].NbrPhenoms;
    if (DoDurationModeling(pEngine, BestTransIdx) != 0) {
        return 1;
    }
    pEngine->NbrStates = pEngine->UserWordUtt[BestTransIdx].NbrPhenoms;
    if (TrainInit(pEngine) != 0) {
        return 1;
    }
    TrainModels(pEngine, BestTransIdx);
    CheckConsistency(pEngine, BestTransIdx, pRejectedUttrs, &Rej);
    if (Rej != 0) {
        pEngine->NbrStates = 0;
        RejectTrainUtters(pEngine, pRejectedUttrs);
        _tosWarningLog(&pEngine->Object, 0x24);
        return 0;
    }
    *pEngine->AddrUserWordBuffSize = pEngine->UserWordUtt[BestTransIdx].NbrPhenoms * 2;
    *(void **) pEngine->AddrUserWordBuff =
        heap_Alloc(pEngine->hHeap, *pEngine->AddrUserWordBuffSize);
    memcpy(*(void **) pEngine->AddrUserWordBuff, pEngine->UserWordUtt[BestTransIdx].pPhenoms,
           *pEngine->AddrUserWordBuffSize);
    return 0;
}
