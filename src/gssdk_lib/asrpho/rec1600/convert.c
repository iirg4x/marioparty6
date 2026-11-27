/* Builds recognition dictionaries and second-pass context tables. */
#include "types.h"
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/secondpass.h"
#include "gssdk/tos.h"
#include "string.h"

/* Builds the bit mask that selects each word in the second-pass dictionary. */
u8 CreateMenuMasks(
    struct _ENGINE *pEngine, struct _DYNPROG_CTXINFO *pCtxInfo, unsigned long uNbrOfWords)
{
    u32 byteCount = ((uNbrOfWords - 1) >> 3) + 1;
    u8 *mask = heap_Alloc(pEngine->hHeap, byteCount);
    u32 index;

    pCtxInfo->pucMenuMasks = mask;
    if (mask == NULL) {
        return 1;
    }

    for (index = 0; index < byteCount - 1; index++) {
        mask[index] = 0xFF;
    }
    mask[index] = 0xFF << ((8 - (uNbrOfWords & 7)) & 7);
    return 0;
}

/* Builds the per-word offsets and speech-unit classes used by second-pass search. */
static u8 CreateSecondPassInfo(
    struct _ENGINE *pEngine, const u16 *pCtxLexicon, const u32 *pCtxPronBegin,
    const u32 *pDictPronBegin, unsigned int NbrPron)
{
    const u32 *pSpeechUnitLex = pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    u8 UserWordSpeechUnitClass = pEngine->pLang->langGetUserWordSpeechUnitClass(pEngine->pLang);

    /* The SDK leaves the speech-unit count at zero in this allocation. */
    u32 TotNbrSpeechUnits = 0;
    struct _SECONDPASSINFO *pSecondPassInfo = heap_Alloc(
        pEngine->hHeap, TotNbrSpeechUnits * sizeof(u16) + NbrPron * sizeof(struct _SECONDPASSINFO));
    unsigned int i;

    if (pSecondPassInfo == NULL) {
        return 1;
    }
    pEngine->CtxInfo.pSecondPassInfo = pSecondPassInfo;

    for (i = 0; i < NbrPron; i++) {
        const u16 *pLex = pCtxLexicon + pCtxPronBegin[i];
        u32 Type = *pLex++;
        const u16 *pLexEnd = pCtxLexicon + pCtxPronBegin[i + 1];
        s32 NbrUnits = pLexEnd - pLex;

        if (Type == 1) {
            u32 Size;
            u32 Skipped;

            pLexEnd--;

            pSecondPassInfo->LeftSpeechUnitClass =
                ((const u16 *)pSpeechUnitLex)[pSpeechUnitLex[*pLex]];
            pSecondPassInfo->RightSpeechUnitClass =
                ((const u16 *)pSpeechUnitLex)[pSpeechUnitLex[*pLexEnd] + 1];
            Size = 0;
            Skipped = 0;
            if (NbrUnits > 2) {
                Skipped = pSpeechUnitLex[*pLex + 1] -
                    pSpeechUnitLex[*pLex] - 2;
                pLex++;
                pLexEnd--;
            }
            pSecondPassInfo->StateDictIndex = *pDictPronBegin + Skipped;
            pDictPronBegin++;
            for (; pLex <= pLexEnd; pLex++) {
                u16 SpeechUnit = *pLex;

                Size += (u16)(pSpeechUnitLex[SpeechUnit + 1] - pSpeechUnitLex[SpeechUnit] - 2);
            }
            pSecondPassInfo->Size = Size;
        } else {
            pSecondPassInfo->StateDictIndex = *pDictPronBegin++;
            pSecondPassInfo->RightSpeechUnitClass = UserWordSpeechUnitClass;
            pSecondPassInfo->LeftSpeechUnitClass = UserWordSpeechUnitClass;
            pSecondPassInfo->Size = NbrUnits;
        }
        pSecondPassInfo++;
    }
    return 0;
}

/* Builds the expanded word and offset tables for a recognition context. */
u8 CreateExEvCtxInfo(struct _ENGINE *pEngine)
{
    void *pExevContextInfo = pEngine->pLang->langGetpExtraEventContext(pEngine->pLang);
    unsigned long NBS_RejectionWordIndex =
        pEngine->pLang->exevGetRejection_NBS_WordIndex(pExevContextInfo);
    unsigned long ExtraEventSilencePhenomeID =
        pEngine->pLang->exevGetExtraEventSilencePhenome(pExevContextInfo);
    const unsigned long *const pCtxBeginOfWords =
        pEngine->pLang->exevGetpBeginOfWords(pExevContextInfo);
    const unsigned short *const pCtxLexicon =
        pEngine->pLang->exevGetpBeginOfItems(pExevContextInfo);
    const unsigned short *pCtxLex = NULL;
    const unsigned short *pCtxLexEnd = NULL;
    unsigned long uNbrOfProns_RejectionWord = 0;
    const unsigned long *const pCtxPron_NBS_Rejection =
        pCtxBeginOfWords + pCtxBeginOfWords[NBS_RejectionWordIndex];
    const unsigned long *const pCtxPronEnd_NBS_Rejection =
        pCtxBeginOfWords + pCtxBeginOfWords[NBS_RejectionWordIndex + 1];
    const unsigned long *pCtxPron = pCtxPron_NBS_Rejection;
    const unsigned long *const pLngSpeechUnitLex =
        pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    unsigned long DictSize = 0;
    unsigned short *pBeginOfPronsInDict;
    struct _DICTSTATE *pStateInDict;
    struct _DICTSTATE *pDictState;

    pEngine->CtxInfo.pExEvCtxInfo =
        heap_Alloc(pEngine->hHeap, sizeof(struct _DYNPROG_EXEV_CTXINFO));
    if (pEngine->CtxInfo.pExEvCtxInfo == NULL) {
        goto failed;
    }
    while (pCtxPron < pCtxPronEnd_NBS_Rejection) {
        pCtxLex = pCtxLexicon + *pCtxPron++;
        pCtxLexEnd = pCtxLexicon + *pCtxPron;
        while (pCtxLex < pCtxLexEnd) {
            unsigned short SpeechUnit = *pCtxLex++;

            DictSize +=
                (pLngSpeechUnitLex[SpeechUnit + 1] & 0xFFFF) - pLngSpeechUnitLex[SpeechUnit] - 2;
        }
        uNbrOfProns_RejectionWord++;
    }
    DictSize += 2;
    pEngine->CtxInfo.pExEvCtxInfo->pBeginOfPronunciationsInDict =
        heap_Alloc(pEngine->hHeap, (uNbrOfProns_RejectionWord + 2) * sizeof(unsigned short));
    if (pEngine->CtxInfo.pExEvCtxInfo->pBeginOfPronunciationsInDict == NULL) {
        goto failed;
    }
    pEngine->CtxInfo.pExEvCtxInfo->pStateInDict =
        heap_Alloc(pEngine->hHeap, DictSize * sizeof(struct _DICTSTATE));
    if (pEngine->CtxInfo.pExEvCtxInfo->pStateInDict == NULL) {
        goto failed;
    }
    pStateInDict = pEngine->CtxInfo.pExEvCtxInfo->pStateInDict;
    pDictState = pStateInDict;
    pBeginOfPronsInDict = pEngine->CtxInfo.pExEvCtxInfo->pBeginOfPronunciationsInDict;
    *pBeginOfPronsInDict++ = 0;
    pDictState->StateIndex = ExtraEventSilencePhenomeID;
    pDictState++;
    pCtxPron = pCtxPron_NBS_Rejection;
    while (pCtxPron < pCtxPronEnd_NBS_Rejection) {
        *pBeginOfPronsInDict++ = pDictState - pStateInDict;
        pCtxLex = pCtxLexicon + *pCtxPron++;
        pCtxLexEnd = pCtxLexicon + *pCtxPron;
        while (pCtxLex < pCtxLexEnd) {
            unsigned short SpeechUnit = *pCtxLex++;
            const unsigned short *pPhen =
                (const unsigned short *)pLngSpeechUnitLex + pLngSpeechUnitLex[SpeechUnit] + 2;
            const unsigned short *const pPhenEnd =
                (const unsigned short *)pLngSpeechUnitLex + pLngSpeechUnitLex[SpeechUnit + 1];

            while (pPhen < pPhenEnd) {
                pDictState->StateIndex = *pPhen++;
                pDictState++;
            }
        }
    }
    pDictState->StateIndex = ExtraEventSilencePhenomeID;
    *pBeginOfPronsInDict = pDictState - pStateInDict;
    pEngine->CtxInfo.pExEvCtxInfo->NbrOfPronunciations_RejectionWord = uNbrOfProns_RejectionWord;
    pEngine->CtxInfo.pExEvCtxInfo->Rejection_NBS_penalty =
        pEngine->pLang->exevGetRejection_NBS_penalty(pExevContextInfo);
    pEngine->CtxInfo.pExEvCtxInfo->Rejection_NBS_path_penalty =
        pEngine->pLang->exevGetRejection_NBS_path_penalty(pExevContextInfo);
    pEngine->CtxInfo.pExEvCtxInfo->uNrStatesInDict = DictSize;
    return 0;
failed:
    FreeCtxInfo(pEngine);
    return 1;
}

/* Saves the lexicon and pronunciation offsets for the Chinese second pass. */
static unsigned char CreateSecondPassInfo_ForChinese(
    struct _ENGINE *pEngine, const unsigned short *pCtxLexicon, const unsigned long *pCtxPronBegin)
{
    struct _CHINESE_SECONDPASSINFO *pChineseSecondPassInfo =
        heap_Alloc(pEngine->hHeap, sizeof(struct _CHINESE_SECONDPASSINFO));

    if (pChineseSecondPassInfo == NULL) {
        _tosErrorLog(&pEngine->Object, 1);
        return 1;
    }
    pChineseSecondPassInfo->pCtxLexicon = pCtxLexicon;
    pChineseSecondPassInfo->pCtxPronBegin = pCtxPronBegin;
    if (pEngine->CtxInfo.pChineseSecondPassInfo != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pChineseSecondPassInfo);
    }
    pEngine->CtxInfo.pChineseSecondPassInfo = pChineseSecondPassInfo;
    return 0;
}

/* Builds the engine tables for a recognition context. */
u8 CreateCtxInfo(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx)
{
    unsigned int i;
    unsigned short OptSilPhenom = (pEngine->pLang->langGetSilPhenome(pEngine->pLang) << 2) | 1;
    const unsigned long *const pLngSpeechUnitLex =
        pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    unsigned long uNbrOfWords = pCtx->contextGetNbrWord(pCtx);
    unsigned long uNbrOfPronounciations = pCtx->contextGetNbrPron(pCtx);
    const unsigned short *const pCtxLexicon = pCtx->contextGetpLex(pCtx);
    const unsigned short *pCtxLex;
    const unsigned long *const pCtxBeginOfWords = pCtx->contextGetpBeginOfWords(pCtx);
    const unsigned long *pCtxPron = pCtxBeginOfWords + uNbrOfWords + 1;
    unsigned long PronCount = uNbrOfWords + 1;
    unsigned long DictSize = 0;
    unsigned long NbrSpeechUnitsInCtx = 0;
    struct _DICTSTATE *pDictState;
    struct _DICTSTATE *pStateInDict;
    unsigned long *pBeginOfWords;
    unsigned long *pBeginOfProns;
    signed short *pWordPenalties;
    unsigned short *pWordProps;
    unsigned long *pSyntax;
    unsigned short ENDBACKTRACEMASK = 0;
    unsigned long SecondPassEnable;

    if (casrGetParam((struct HCASR__ *)pEngine, 0xF, &SecondPassEnable) != 0) {
        return 1;
    }
    if (SecondPassEnable != 0 && pEngine->pLang->langCheckBitField(pEngine->pLang, 8) != 0) {
        ENDBACKTRACEMASK = 2;
    }
    pEngine->CtxInfo.pDynProgCtxInfo = heap_Alloc(pEngine->hHeap, sizeof(struct _DYNPROG_CTXINFO));
    if (pEngine->CtxInfo.pDynProgCtxInfo == NULL) {
        return 0;
    }
    pEngine->CtxInfo.pNewWordID2OldWordID = NULL;
    pCtxLex = pCtxLexicon;
    for (i = 1; i <= uNbrOfPronounciations; i++) {
        const unsigned short *const pCtxLexEnd = pCtxLexicon + pCtxPron[i];

        if (*pCtxLex++ == 1) {
            NbrSpeechUnitsInCtx += pCtxLexEnd - pCtxLex;
            while (pCtxLex < pCtxLexEnd) {
                unsigned int SpeechUnit = *pCtxLex++;

                DictSize += pLngSpeechUnitLex[SpeechUnit + 1] - pLngSpeechUnitLex[SpeechUnit];
            }
        } else {
            DictSize += pCtxLexEnd - pCtxLex;
            pCtxLex = pCtxLexEnd;
        }
    }
    DictSize -= NbrSpeechUnitsInCtx * 2;
    DictSize += uNbrOfPronounciations;
    DictSize += uNbrOfPronounciations * 2;
    pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords = heap_Alloc(
        pEngine->hHeap, (uNbrOfWords + uNbrOfPronounciations + 2) * sizeof(unsigned long));
    if (pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords == NULL) {
        goto failed;
    }
    pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict =
        heap_Alloc(pEngine->hHeap, DictSize * sizeof(struct _DICTSTATE));
    if (pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict == NULL) {
        goto failed;
    }
    pStateInDict = pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict;
    pDictState = pStateInDict;
    pBeginOfWords = pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords;
    pBeginOfProns = pBeginOfWords + uNbrOfWords + 1;
    *pBeginOfWords++ = PronCount;
    *pBeginOfProns++ = 0;
    pCtxLex = pCtxLexicon;
    pCtxPron++;
    for (i = 0; i < uNbrOfWords; i++) {
        const unsigned long *const pCtxPronEnd = pCtxBeginOfWords + pCtxBeginOfWords[i + 1];

        while (pCtxPron <= pCtxPronEnd) {
            const unsigned short *const pCtxLexEnd = pCtxLexicon + *pCtxPron;

            if (*pCtxLex++ == 1) {
                while (pCtxLex < pCtxLexEnd) {
                    unsigned int SpeechUnit = *pCtxLex++;
                    const unsigned short *pPhen = (const unsigned short *)pLngSpeechUnitLex
                                                  + pLngSpeechUnitLex[SpeechUnit] + 2;
                    const unsigned short *const pPhenEnd = (const unsigned short *)pLngSpeechUnitLex
                                                           + pLngSpeechUnitLex[SpeechUnit + 1];

                    while (pPhen < pPhenEnd) {
                        pDictState++->StateIndex = *pPhen++ << 2;
                    }
                }
            } else {
                while (pCtxLex < pCtxLexEnd) {
                    pDictState++->StateIndex = *pCtxLex++ << 2;
                }
            }
            pCtxPron++;
            PronCount++;
            pDictState[-1].StateIndex |= ENDBACKTRACEMASK;
            pDictState++->StateIndex = OptSilPhenom;
            pDictState++->StateIndex = i & 0xFFFF;
            pDictState++->StateIndex = i >> 0x10;
            *pBeginOfProns++ = pDictState - pStateInDict;
        }
        *pBeginOfWords++ = PronCount;
    }
    pSyntax = pCtx->contextGetpSyntax(pCtx);
    pWordPenalties = heap_Alloc(pEngine->hHeap, uNbrOfWords * sizeof(signed short));
    pEngine->CtxInfo.pDynProgCtxInfo->pWordPenalties = pWordPenalties;
    if (pWordPenalties == NULL) {
        goto failed;
    }
    pWordProps = pCtx->contextGetpWordProp(pCtx);
    for (i = 0; i < uNbrOfWords; i++) {
        pWordPenalties[i] = pWordProps[i] & 0x80 ? pEngine->WordPenalty : 0;
    }
    if (CreateMenuMasks(pEngine, pEngine->CtxInfo.pDynProgCtxInfo, uNbrOfWords) != 0) {
        goto failed;
    }
    pEngine->CtxInfo.pDynProgCtxInfo->uNrStatesInDict = DictSize;
    pEngine->CtxInfo.pDynProgCtxInfo->uNbrOfWords = uNbrOfWords;
    pEngine->CtxInfo.pDynProgCtxInfo->pSyntax = pSyntax;
    pEngine->CtxInfo.pDynProgCtxInfo->pWordProps = pWordProps;
    pEngine->CtxInfo.pDynProgCtxInfo->iOptionalSilence = 1;
    if (pEngine->pLang->langCheckBitField(pEngine->pLang, 8) != 0) {
        if (CreateSecondPassInfo_ForChinese(
                pEngine, pCtxLexicon, pCtxBeginOfWords + *pCtxBeginOfWords)
            != 0) {
            goto failed;
        }
    } else if (CreateSecondPassInfo(pEngine, pCtxLexicon,
                   pCtxBeginOfWords + *pCtxBeginOfWords,
                   pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords + uNbrOfWords + 1,
                   uNbrOfPronounciations)
               != 0) {
        goto failed;
    }
    if (pCtx->contextIsSpellingContext(pCtx) != 0) {
        unsigned short *pWordProperties;
        unsigned long *pSpellSyntax;

        pEngine->CtxInfo.pSpellingDynProgCtxInfo =
            heap_Alloc(pEngine->hHeap, sizeof(struct _DYNPROG_CTXINFO));
        if (pEngine->CtxInfo.pSpellingDynProgCtxInfo == NULL) {
            goto failed;
        }
        memcpy(pEngine->CtxInfo.pSpellingDynProgCtxInfo, pEngine->CtxInfo.pDynProgCtxInfo,
            sizeof(struct _DYNPROG_CTXINFO));
        pWordPenalties = heap_Alloc(pEngine->hHeap, uNbrOfWords * sizeof(signed short));
        if (pWordPenalties == NULL) {
            goto failed;
        }
        pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordPenalties = pWordPenalties;
        for (i = 0; i < uNbrOfWords; i++) {
            pWordPenalties[i] = 0;
        }
        pWordProperties = heap_Alloc(pEngine->hHeap, uNbrOfWords * sizeof(unsigned short));
        if (pWordProperties == NULL) {
            goto failed;
        }
        pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordProps = pWordProperties;
        for (i = 0; i < uNbrOfWords; i++) {
            if (pWordProps[i] & 0x80) {
                pWordProperties[i] = 0x83;
            } else {
                pWordProperties[i] = 0;
            }
        }
        pSpellSyntax = heap_Alloc(pEngine->hHeap, (uNbrOfWords + 1) * sizeof(unsigned long));
        if (pSpellSyntax == NULL) {
            goto failed;
        }
        pEngine->CtxInfo.pSpellingDynProgCtxInfo->pSyntax = pSpellSyntax;
        memset(pSpellSyntax, 0, (uNbrOfWords + 1) * sizeof(unsigned long));
    } else {
        pEngine->CtxInfo.pSpellingDynProgCtxInfo = NULL;
    }
    return 0;
failed:
    FreeCtxInfo(pEngine);
    return 1;
}

/* Releases recognition buffers and saved copies that no longer share those buffers. */
void FreeCtxInfo(struct _ENGINE *pEngine)
{
    if (pEngine->CtxInfo.pDynProgCtxInfo != NULL) {
        if (pEngine->CtxInfo.pSpellingDynProgCtxInfo != NULL) {
            if (pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordPenalties
                != pEngine->CtxInfo.pDynProgCtxInfo->pWordPenalties) {
                heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordPenalties);
            }
            if (pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordProps
                != pEngine->CtxInfo.pDynProgCtxInfo->pWordProps) {
                heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSpellingDynProgCtxInfo->pWordProps);
            }
            if (pEngine->CtxInfo.pSpellingDynProgCtxInfo->pSyntax
                != pEngine->CtxInfo.pDynProgCtxInfo->pSyntax) {
                heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSpellingDynProgCtxInfo->pSyntax);
            }
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSpellingDynProgCtxInfo);
        }

        if (pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict);
        }
        if (pEngine->CtxInfo.pDynProgCtxInfo->pWordPenalties != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pDynProgCtxInfo->pWordPenalties);
        }
        if (pEngine->CtxInfo.pDynProgCtxInfo->pucMenuMasks != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pDynProgCtxInfo->pucMenuMasks);
        }
        if (pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords);
        }
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pDynProgCtxInfo);
        pEngine->CtxInfo.pDynProgCtxInfo = NULL;
    }
    if (pEngine->CtxInfo.pSecondPassInfo != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassInfo);
        pEngine->CtxInfo.pSecondPassInfo = NULL;
    }
    if (pEngine->CtxInfo.pChineseSecondPassInfo != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pChineseSecondPassInfo);
        pEngine->CtxInfo.pChineseSecondPassInfo = NULL;
    }
    if (pEngine->CtxInfo.pNewWordID2OldWordID != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pNewWordID2OldWordID);
        pEngine->CtxInfo.pNewWordID2OldWordID = NULL;
    }
    if (pEngine->CtxInfo.pCtxTree != 0) {
        FreeTree(pEngine);
    }
    if (pEngine->CtxInfo.pExEvCtxInfo != NULL) {
        if (pEngine->CtxInfo.pExEvCtxInfo->pBeginOfPronunciationsInDict != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pExEvCtxInfo->pBeginOfPronunciationsInDict);
        }
        if (pEngine->CtxInfo.pExEvCtxInfo->pStateInDict != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pExEvCtxInfo->pStateInDict);
        }
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pExEvCtxInfo);
        pEngine->CtxInfo.pExEvCtxInfo = NULL;
    }
}

/* Calculates the speech accuracy limit after first-pass recognition data is ready. */
u32 CalcAccuracyCtxInfo(struct _ENGINE *pEngine)
{
    struct _DYNPROG_CTXINFO *accuracyInfo = pEngine->CtxInfo.pDynProgCtxInfo;
    u32 largestGap = 0;
    u32 wordCount = accuracyInfo->uNbrOfWords;
    unsigned long *wordBoundaries = accuracyInfo->pSyntax;
    unsigned long *end = wordBoundaries + wordCount;
    u32 accuracy = (u32)((end[0] - wordBoundaries[0]) * 50) /
        (u32)(end - wordBoundaries);
    u32 minimumAccuracy;

    if (pEngine->IsActiveContextSpelling != 0) {
        return 0x5DC;
    }

    for (; wordBoundaries < end; wordBoundaries++) {
        u32 gap = wordBoundaries[1] - wordBoundaries[0];

        if (gap > largestGap) {
            largestGap = gap;
        }
    }

    minimumAccuracy = largestGap * 6;
    if (minimumAccuracy < accuracy) {
        minimumAccuracy = accuracy;
    }

    if (minimumAccuracy < 0x12C) {
        return 0x12C;
    }
    return minimumAccuracy;
}
