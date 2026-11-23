/* Builds, traverses and releases word trees for speech recognition contexts. */
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "types.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Carries the lexicon and active entries through recursive tree construction. */
struct _ADDFOLL_STATIC {
    u32 LastBacktraceIndex; /* Next entry in the backtrace. */
    u16 SUSilence;          /* First speech unit of the stop word, used in follower nodes. */
    struct _ENGINE *pEngine; /* Engine receiving the tree. */
    const unsigned long *pSyntax; /* Follower lists in the context. */
    const unsigned long *pCtxBeginOfWords; /* Context pronunciation offsets. */
    u16 *pCtxLexicon; /* Packed context pronunciations. */
    u32 NbrCurWords;     /* Words on the current recursive path. */
    u8 *pActiveWordMask; /* One bit per word enabled to contribute followers. */
    unsigned short UserWordSpeechUnits[50]; /* Speech units of the current user word. */
};

/* Builds the context tree from the lexicon and active-word list. */
static u8 intCreateCtxTree(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx, u32 *pidWords,
                           u32 NbrWords);

/* Weights used to estimate recognition tree size by number of states. */
static const f32 aNodesFr[13] = { 0.135781f, 0.165789f, 0.202428f, 0.247165f, 0.301789f,
                                  0.368485f, 0.44992f, 0.549353f, 0.670761f, 0.819f,
                                  1.0f, 1.0f, 1.0f };

/* Grows a node's child array and returns the previous array to its allocator. */
static struct _ISOWORD_NODE *AllocateNodeAndFreePrevious(struct _TOS *pTos, int NrFoll,
                                                         struct _ISOWORD_NODE **pCur)
{
    struct _ISOWORD_NODE *pNewNodes;

    if (NrFoll < 8) {
        FASTALLO_TAKE(&pTos->FastAllocHeap,
                      ((NrFoll + 1) * sizeof(struct _ISOWORD_NODE) + 3) >> 2, &pNewNodes)
    } else {
        pNewNodes = heap_Alloc(pTos->hHeap, (NrFoll + 1) * sizeof(struct _ISOWORD_NODE));
    }
    if (pNewNodes == NULL) {
        return NULL;
    }

    pNewNodes[NrFoll].NrFol = 0;
    pNewNodes[NrFoll].pToken = 0;
    if (NrFoll != 0) {
        struct _ISOWORD_NODE *pCurFoll = *pCur;
        memcpy(pNewNodes, pCurFoll, (NrFoll * sizeof(struct _ISOWORD_NODE)));
        if (NrFoll <= 8) {
            struct _FASTALLO_FREE_NODE **freeList =
                &pTos->FastAllocHeap.pFreeNodes[
                    (NrFoll * sizeof(struct _ISOWORD_NODE) + 3) >> 2];
            struct _ISOWORD_NODE *previousFollowers = pCurFoll;

            ((struct _FASTALLO_FREE_NODE *) previousFollowers)->pNext = *freeList;
            *freeList = (struct _FASTALLO_FREE_NODE *) previousFollowers;
        } else {
            heap_Free(pTos->hHeap, pCurFoll);
        }
    }

    *pCur = pNewNodes;
    return pNewNodes;
}

/* Initializes the context tree without a word list. */
unsigned char CreateCtxTree(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx)
{
    return intCreateCtxTree(pEngine, pCtx, NULL, 0);
}

/* Appends a user's pronunciation and its segment boundaries to the context. */
static int AddUserWordToLngSpeechUnitLex(struct _ADDFOLL_STATIC *pStatic, const u16 *pCtxLex,
                                         const u16 *pCtxLexEnd, u16 *pNbrSpeechUnitsInWord)
{
    struct _ENGINE *pEngine = pStatic->pEngine;
    u16 NbrPhenomsInWord = pCtxLexEnd - pCtxLex;
    s32 RetVal = 0;
    u32 boundaryCount;
    u32 *pNew2;
    u16 *pRunUserWordLngSpeechUnitLex;

    if (pEngine->NbrUserWordPhenomsAllocated == 0 ||
        NbrPhenomsInWord +
            pEngine->pFirstPhenomOfUserWordSpeechUnit[pEngine->NbrUserWordSpeechUnits] + 1 >
            pEngine->NbrUserWordPhenomsAllocated) {
        s32 capacityIncrement = NbrPhenomsInWord + 1;
        u16 *pNew;

        pEngine->NbrUserWordPhenomsAllocated += capacityIncrement < 256 ? 256 : capacityIncrement;
        pNew = heap_Realloc(pEngine->hHeap, pEngine->pUserWordLngSpeechUnitLex,
                            pEngine->NbrUserWordPhenomsAllocated * sizeof(u16));

        if (pNew == NULL) {
            if (capacityIncrement < 256) {
                capacityIncrement = 256;
            }
            pEngine->NbrUserWordPhenomsAllocated -= capacityIncrement;
            RetVal = 1;
            goto done;
        } else {
            pEngine->pUserWordLngSpeechUnitLex = pNew;
        }
    }

    if (NbrPhenomsInWord <= 8) {
        *pNbrSpeechUnitsInWord = 1;
    } else if (NbrPhenomsInWord <= 16) {
        *pNbrSpeechUnitsInWord = 2;
    } else {
        *pNbrSpeechUnitsInWord = ((NbrPhenomsInWord - 17) / 255) + 3;
    }

    boundaryCount = pEngine->NbrUserWordSpeechUnits + *pNbrSpeechUnitsInWord;
    boundaryCount++;
    pNew2 = heap_Realloc(pEngine->hHeap, pEngine->pFirstPhenomOfUserWordSpeechUnit,
                         boundaryCount * sizeof(*pNew2));
    if (pNew2 == NULL) {
        RetVal = 1;
    } else {
        u32 FirstIndex;

        pEngine->pFirstPhenomOfUserWordSpeechUnit = pNew2;
        pNew2[0] = 0;
        FirstIndex =
            pEngine->pFirstPhenomOfUserWordSpeechUnit[pEngine->NbrUserWordSpeechUnits];
        pRunUserWordLngSpeechUnitLex = pEngine->pUserWordLngSpeechUnitLex + FirstIndex;
        while (pCtxLex < pCtxLexEnd) {
            *pRunUserWordLngSpeechUnitLex++ = *pCtxLex++;
        }

        pEngine->NbrUserWordSpeechUnits++;
        pEngine->pFirstPhenomOfUserWordSpeechUnit[pEngine->NbrUserWordSpeechUnits] =
            FirstIndex + (NbrPhenomsInWord > 8 ? 8 : NbrPhenomsInWord);
        if (NbrPhenomsInWord > 8) {
            pEngine->NbrUserWordSpeechUnits++;
            pEngine->pFirstPhenomOfUserWordSpeechUnit[pEngine->NbrUserWordSpeechUnits] =
                FirstIndex + (NbrPhenomsInWord > 16 ? 16 : NbrPhenomsInWord);
            if (NbrPhenomsInWord > 16) {
                s32 end;

                NbrPhenomsInWord -= 16;
                end = FirstIndex + 16;
                do {
                    pEngine->NbrUserWordSpeechUnits++;
                    end += NbrPhenomsInWord > 255 ? 255 : NbrPhenomsInWord;
                    pEngine->pFirstPhenomOfUserWordSpeechUnit[pEngine->NbrUserWordSpeechUnits] =
                        end;
                    if (NbrPhenomsInWord > 255) {
                        NbrPhenomsInWord -= 255;
                    } else {
                        NbrPhenomsInWord = 0;
                    }
                } while (NbrPhenomsInWord != 0);
            }
        }
    }

done:
    return RetVal;
}

/* Appends a word and its predecessor index to the backtrace array. */
static unsigned long AddWordToBacktrace(struct _ADDFOLL_STATIC *pStatic, long Word,
                                        unsigned long PrevIndex)
{
    unsigned long NewWordIndex = 0xFFFFFFFFUL;
    struct _ISOWORD_BACKTRACE *pNew;
    if (pStatic->LastBacktraceIndex == pStatic->pEngine->NbrBacktraceAllocated) {
        pStatic->pEngine->NbrBacktraceAllocated += 1024;
        pNew = heap_Realloc(pStatic->pEngine->hHeap, pStatic->pEngine->pBacktraceArray,
                            pStatic->pEngine->NbrBacktraceAllocated *
                                sizeof(struct _ISOWORD_BACKTRACE));
        if (pNew == NULL) {
            pStatic->pEngine->NbrBacktraceAllocated -= 1024;
            goto return_index;
        }
        pStatic->pEngine->pBacktraceArray = pNew;
    }
    NewWordIndex = pStatic->LastBacktraceIndex;
    pStatic->pEngine->pBacktraceArray[pStatic->LastBacktraceIndex].s32WordID = Word;
    pStatic->pEngine->pBacktraceArray[pStatic->LastBacktraceIndex].u32PrevWordIdx = PrevIndex;
    pStatic->LastBacktraceIndex++;
return_index:
    return NewWordIndex;
}

/* Attaches each allowed following word or pronunciation to the tree recursively. */
static int AddFollowers(struct _ADDFOLL_STATIC *pStatic, struct _ISOWORD_NODE *pTree, s32 StartWord,
                        u32 PrevWordIndex)
{
    const unsigned long *pFollowers = pStatic->pSyntax + pStatic->pSyntax[StartWord];
    const unsigned long *pFollowersEnd = pStatic->pSyntax + pStatic->pSyntax[StartWord + 1];
    struct _ISOWORD_NODE *pOriTree = pTree;
    struct _ISOWORD_NODE *pNextWordTree = pTree;
    unsigned long NewWordIndex = 0xFFFFFFFFUL;
    struct _TOS *pTos = &pStatic->pEngine->Tos;
    int RetVal = 0;
    unsigned short NbrUserWordSpeechUnitsInWord;
    unsigned short i;
    unsigned short *pRunUserWordSpeechUnits;
    long WordNbr;

    pStatic->NbrCurWords++;
    /* The next word would exceed the maximum isolated-word context path length. */
    if (pStatic->NbrCurWords > pStatic->pEngine->IsActiveContextIsoWord + 1) {
        RetVal = 1;
    } else {
        for (; pFollowers < pFollowersEnd; pFollowers++) {
            WordNbr = *pFollowers;
            if (pStatic->pSyntax[WordNbr + 1] - pStatic->pSyntax[WordNbr] == 0) {
                pTree = pOriTree;
                if (pStatic->NbrCurWords == 2) {
                    unsigned int PhonID = pStatic->SUSilence;
                    struct _ISOWORD_NODE *pFoll = pTree->u.pFirstFol;
                    int NrFoll = pTree->NrFol;
                    int j;
                    struct _ISOWORD_NODE *pNewNodes;
                    for (j = 0; j < NrFoll; j++) {
                        if (PhonID == pFoll[j].PhoID && pFoll[j].NrFol == 0) {
                            break;
                        }
                    }
                    if (j >= NrFoll || StartWord != pFoll[j].u.WordID) {
                        pNewNodes =
                            AllocateNodeAndFreePrevious(pTos, NrFoll, &pTree->u.pFirstFol);
                        pNewNodes[NrFoll].PhoID = PhonID;
                        pTree->NrFol = NrFoll + 1;
                        pNewNodes[NrFoll].NrFol = 0;
                        pNewNodes[NrFoll].u.WordID = StartWord;
                    }
                } else {
                    NewWordIndex = AddWordToBacktrace(pStatic, StartWord, PrevWordIndex);
                    if (NewWordIndex == 0xFFFFFFFFUL) {
                        RetVal = 1;
                        goto leave_word_path;
                    }
                    {
                        unsigned int PhonID = pStatic->SUSilence;
                        struct _ISOWORD_NODE *pFoll = pTree->u.pFirstFol;
                        int NrFoll = pTree->NrFol;
                        int j;
                        struct _ISOWORD_NODE *pNewNodes;
                        for (j = 0; j < NrFoll; j++) {
                            if (PhonID == pFoll[j].PhoID && pFoll[j].NrFol == 0) {
                                break;
                            }
                        }
                        if (j >= NrFoll || -(long) (NewWordIndex + 1) != pFoll[j].u.WordID) {
                            pNewNodes =
                                AllocateNodeAndFreePrevious(pTos, NrFoll, &pTree->u.pFirstFol);
                            pNewNodes[NrFoll].PhoID = PhonID;
                            pTree->NrFol = NrFoll + 1;
                            pNewNodes[NrFoll].NrFol = 0;
                            pNewNodes[NrFoll].u.WordID = -(long) (NewWordIndex + 1);
                        }
                    }
                }
            } else if (pStatic->pActiveWordMask[WordNbr >> 3] & (1 << (7 - (WordNbr & 7)))) {
                const unsigned long *pCtxPron =
                    pStatic->pCtxBeginOfWords + pStatic->pCtxBeginOfWords[WordNbr];
                const unsigned long *pCtxPronEnd =
                    pStatic->pCtxBeginOfWords + pStatic->pCtxBeginOfWords[WordNbr + 1];
                if (pStatic->NbrCurWords > 1 && pOriTree == pNextWordTree) {
                    NewWordIndex = AddWordToBacktrace(pStatic, StartWord, PrevWordIndex);
                    if (NewWordIndex == 0xFFFFFFFFUL) {
                        RetVal = 1;
                        goto leave_word_path;
                    }
                    pNextWordTree = pOriTree;
                    {
                        unsigned int PhonID = pStatic->SUSilence;
                        struct _ISOWORD_NODE *pFoll = pNextWordTree->u.pFirstFol;
                        int j;
                        int NrFoll = pNextWordTree->NrFol;
                        struct _ISOWORD_NODE *pNewNodes;
                        for (j = 0; j < NrFoll; j++) {
                            if (PhonID == pFoll[j].PhoID && pFoll[j].NrFol != 0) {
                                break;
                            }
                        }
                        if (j < NrFoll) {
                            pNextWordTree = &pFoll[j];
                        } else {
                            pNewNodes = AllocateNodeAndFreePrevious(pTos, NrFoll,
                                                                    &pNextWordTree->u.pFirstFol);
                            pNewNodes[NrFoll].PhoID = PhonID;
                            pNextWordTree->NrFol = NrFoll + 1;
                            pNextWordTree = &pNewNodes[NrFoll];
                            pNextWordTree->NrFol = 0;
                        }
                    }
                }
                for (; pCtxPron < pCtxPronEnd; pCtxPron++) {
                    const unsigned short *pCtxLex = pStatic->pCtxLexicon + *pCtxPron;
                    const unsigned short *pCtxLexEnd = pStatic->pCtxLexicon + pCtxPron[1];
                    pTree = pNextWordTree;
                    if (*pCtxLex++ != 1) {
                        pStatic->UserWordSpeechUnits[0] =
                            pStatic->pEngine->NbrUserWordSpeechUnits +
                            pStatic->pEngine->pLang->langGetNbrSpeechUnit(pStatic->pEngine->pLang) +
                            1;
                        if (AddUserWordToLngSpeechUnitLex(pStatic, pCtxLex, pCtxLexEnd,
                                                          &NbrUserWordSpeechUnitsInWord) != 0) {
                            RetVal = 1;
                            goto leave_word_path;
                        }
                        pCtxLex = pStatic->UserWordSpeechUnits;
                        pCtxLexEnd = pCtxLex + NbrUserWordSpeechUnitsInWord;
                        pRunUserWordSpeechUnits = pStatic->UserWordSpeechUnits;
                        for (i = 1; i < NbrUserWordSpeechUnitsInWord; i++) {
                            pRunUserWordSpeechUnits[i] = *pCtxLex + i;
                        }
                    }
                    while (pCtxLex < pCtxLexEnd) {
                        unsigned int PhonID = *pCtxLex++;
                        struct _ISOWORD_NODE *pFoll = pTree->u.pFirstFol;
                        int j;
                        int NrFoll = pTree->NrFol;
                        struct _ISOWORD_NODE *pNewNodes;
                        for (j = 0; j < NrFoll; j++) {
                            if (PhonID == pFoll[j].PhoID && pFoll[j].NrFol != 0) {
                                break;
                            }
                        }
                        if (j < NrFoll) {
                            pTree = &pFoll[j];
                        } else {
                            pNewNodes =
                                AllocateNodeAndFreePrevious(pTos, NrFoll, &pTree->u.pFirstFol);
                            pNewNodes[NrFoll].PhoID = PhonID;
                            pTree->NrFol = NrFoll + 1;
                            pTree = &pNewNodes[NrFoll];
                            pTree->NrFol = 0;
                        }
                    }
                    if (AddFollowers(pStatic, pTree, WordNbr, NewWordIndex) != 0) {
                        RetVal = 1;
                        goto leave_word_path;
                    }
                }
            }
        }
    }
leave_word_path:
    pStatic->NbrCurWords--;
    return RetVal;
}

/* Retains the lexicon and entry offsets for the Chinese second pass. */
static u8 CreateSecondPassInfo_ForChinese_TreeCtx(struct _ENGINE *pEngine,
                                                  const unsigned short *pCtxLexicon,
                                                  const unsigned long *pCtxPronBegin)
{
    struct _CHINESE_SECONDPASSINFO *pChineseSecondPassInfo =
        heap_Alloc(pEngine->hHeap, sizeof(struct _CHINESE_SECONDPASSINFO));
    if (pChineseSecondPassInfo == NULL) {
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

/* Builds the context tree from the lexicon and active-word list. */
static u8 intCreateCtxTree(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx, u32 *pidWords,
                           u32 NbrWords)
{
    struct _TOS *pTos = &pEngine->Tos;
    struct _ISOWORD_NODE *pTree;
    const unsigned long *pSyntax = pCtx->contextGetpSyntax(pCtx);
    u16 *pWordProps = pCtx->contextGetpWordProp(pCtx);
    u32 uNbrOfWords = pCtx->contextGetNbrWord(pCtx);
    u16 *pCtxLexicon = pCtx->contextGetpLex(pCtx);
    const unsigned long *pCtxBeginOfWords = pCtx->contextGetpBeginOfWords(pCtx);
    const unsigned long *pCtxPron;
    u8 *pActiveWordMask; /* One bit per word enabled to contribute followers. */
    u32 i;
    s32 StartWord = -1;
    s32 StopWord = -1;
    u16 *pCtxLex;
    unsigned short SUStopWord;
    struct _ADDFOLL_STATIC FollowStatic;

    u32 ActiveWordMaskSize = ((uNbrOfWords - 1) >> 3) + 1;
    pActiveWordMask = heap_Alloc(pEngine->hHeap, ActiveWordMaskSize);
    if (pActiveWordMask == NULL) {
        return 1;
    }
    if (pidWords == NULL) {
        memset(pActiveWordMask, -1, ActiveWordMaskSize);
    } else {
        memset(pActiveWordMask, 0, ActiveWordMaskSize);
        for (i = 0; i < NbrWords; i++) {
            u32 index = pidWords[i];
            pActiveWordMask[index >> 3] |= 1 << (7 - (index & 7));
        }
    }

    for (i = 0; i < uNbrOfWords; i++) {
        if ((pWordProps[i] & 0x80) == 0) {
            pActiveWordMask[i >> 3] |= 1 << (7 - (i & 7));
        }
        if (pWordProps[i] & 1) {
            StartWord = i;
            if (StopWord != -1) {
                break;
            }
        }
        if (pWordProps[i] & 2) {
            StopWord = i;
            if (StartWord != -1) {
                break;
            }
        }
    }

    pCtxPron = pCtxBeginOfWords + pCtxBeginOfWords[StartWord];
    pCtxLex = pCtxLexicon + *pCtxPron;
    if (*pCtxLex++ != 1) {
        heap_Free(pEngine->hHeap, pActiveWordMask);
        return 1;
    }

    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(struct _ISOWORD_NODE) >> 2, &pTree)
    pTree->NrFol = 0;
    pTree->pToken = 0;
    pEngine->CtxInfo.pCtxTree = pTree;
    pTree->PhoID = *pCtxLex;

    pCtxPron = pCtxBeginOfWords + pCtxBeginOfWords[StopWord];
    pCtxLex = pCtxLexicon + *pCtxPron;
    if (*pCtxLex++ != 1) {
        heap_Free(pEngine->hHeap, pActiveWordMask);
        return 1;
    }

    SUStopWord = *pCtxLex;
    FollowStatic.pEngine = pEngine;
    FollowStatic.LastBacktraceIndex = 0;
    FollowStatic.SUSilence = SUStopWord;
    FollowStatic.pSyntax = pSyntax;
    FollowStatic.NbrCurWords = 0;
    FollowStatic.pCtxBeginOfWords = pCtxBeginOfWords;
    FollowStatic.pCtxLexicon = pCtxLexicon;
    FollowStatic.pActiveWordMask = pActiveWordMask;
    if (AddFollowers(&FollowStatic, pTree, StartWord, (u32) -1) != 0) {
        heap_Free(pEngine->hHeap, pActiveWordMask);
        return 1;
    }

    pEngine->CtxInfo.pDynProgCtxInfo = NULL;
    pEngine->CtxInfo.pNewWordID2OldWordID = NULL;
    if (pEngine->pLang->langCheckBitField(pEngine->pLang, 8) != 0) {
        if (CreateSecondPassInfo_ForChinese_TreeCtx(pEngine, pCtxLexicon,
                                                    pCtxBeginOfWords + *pCtxBeginOfWords) != 0) {
            heap_Free(pEngine->hHeap, pActiveWordMask);
            return 1;
        }
    }

    heap_Free(pEngine->hHeap, pActiveWordMask);
    return 0;
}

/* Returns each child array to the engine's small-node pool or its heap. */
static void _FreeTree(struct _TOS *pTos, struct _ISOWORD_NODE *pCur)
{
    u32 NrFoll = pCur->NrFol;
    u32 i;

    for (i = 0; i < NrFoll; i++) {
        _FreeTree(pTos, &pCur->u.pFirstFol[i]);
    }

    if (NrFoll != 0) {
        if (NrFoll <= 8) {
            struct _FASTALLO_FREE_NODE **freeList =
                &pTos->FastAllocHeap.pFreeNodes[(NrFoll * sizeof(*pCur) + 3) >> 2];
            ((struct _FASTALLO_FREE_NODE *) pCur->u.pFirstFol)->pNext = *freeList;
            *freeList = (struct _FASTALLO_FREE_NODE *) pCur->u.pFirstFol;
        } else {
            heap_Free(pTos->hHeap, pCur->u.pFirstFol);
        }
    }
}

/* Releases the active tree and word buffers owned by a speech context. */
void FreeTree(struct _ENGINE *pEngine)
{
    struct _FASTALLO_FREE_NODE **pTemp;

    _FreeTree(&pEngine->Tos, pEngine->CtxInfo.pCtxTree);

    pTemp = &pEngine->Tos.FastAllocHeap.pFreeNodes[sizeof(struct _ISOWORD_NODE) >> 2];
    ((struct _FASTALLO_FREE_NODE *) pEngine->CtxInfo.pCtxTree)->pNext = *pTemp;
    *pTemp = (struct _FASTALLO_FREE_NODE *) pEngine->CtxInfo.pCtxTree;
    pEngine->CtxInfo.pCtxTree = NULL;
    if (pEngine->pBacktraceArray != NULL) {
        heap_Free(pEngine->hHeap, pEngine->pBacktraceArray);
        pEngine->pBacktraceArray = NULL;
        pEngine->NbrBacktraceAllocated = 0;
    }
    if (pEngine->pUserWordLngSpeechUnitLex != NULL) {
        heap_Free(pEngine->hHeap, pEngine->pUserWordLngSpeechUnitLex);
        pEngine->pUserWordLngSpeechUnitLex = NULL;
    }
    pEngine->NbrUserWordPhenomsAllocated = 0;
    if (pEngine->pFirstPhenomOfUserWordSpeechUnit != NULL) {
        heap_Free(pEngine->hHeap, pEngine->pFirstPhenomOfUserWordSpeechUnit);
        pEngine->pFirstPhenomOfUserWordSpeechUnit = NULL;
    }
    pEngine->NbrUserWordSpeechUnits = 0;
}

/* Adds the node weight and recurses into followers with the remaining state budget. */
static f32 CalcNrNodes(const unsigned long *pLngSpeechUnitLex,
                       const unsigned long *pFirstPhenomOfUwSu, u16 LngNbrSpeechUnits,
                       struct _ISOWORD_NODE *pNode, s32 NrStates)
{
    float NrNodes;
    unsigned long NrStatesInNode;
    unsigned short UserWordSuID;
    struct _ISOWORD_NODE *pFol;
    struct _ISOWORD_NODE *pFolEnd;
    NrNodes = aNodesFr[NrStates];
    if (pNode->PhoID < LngNbrSpeechUnits) {
        NrStatesInNode = pLngSpeechUnitLex[pNode->PhoID + 1] - pLngSpeechUnitLex[pNode->PhoID] - 2;
    } else {
        UserWordSuID = pNode->PhoID - LngNbrSpeechUnits - 1;
        NrStatesInNode = pFirstPhenomOfUwSu[UserWordSuID + 1] - pFirstPhenomOfUwSu[UserWordSuID];
    }
    NrStates -= NrStatesInNode;
    if (NrStates > 0) {
        pFol = pNode->u.pFirstFol;
        pFolEnd = pFol + pNode->NrFol;
        for (; pFol < pFolEnd; pFol++) {
            NrNodes += CalcNrNodes(pLngSpeechUnitLex, pFirstPhenomOfUwSu, LngNbrSpeechUnits, pFol,
                                   NrStates);
        }
    }
    return NrNodes;
}

/* Tracks the largest branch and the largest weighted node-count estimate. */
static f32 CalcNrNodes10(const unsigned long *pLngSpeechUnitLex,
                         const unsigned long *pFirstPhenomOfUwSu, u16 LngNbrSpeechUnits,
                         struct _ISOWORD_NODE *pNode, f32 MaxNrNodes10, u32 *pMaxBranch)
{
    float NrNodes10;
    struct _ISOWORD_NODE *pFol;
    struct _ISOWORD_NODE *pFolEnd;
    if (pNode->NrFol > 0.1 * *pMaxBranch) {
        if (pNode->NrFol > *pMaxBranch)
            *pMaxBranch = pNode->NrFol;
        NrNodes10 =
            CalcNrNodes(pLngSpeechUnitLex, pFirstPhenomOfUwSu, LngNbrSpeechUnits, pNode, 12);
        if (NrNodes10 > MaxNrNodes10) {
            MaxNrNodes10 = NrNodes10;
        }
    }
    pFol = pNode->u.pFirstFol;
    pFolEnd = pFol + pNode->NrFol;
    for (; pFol < pFolEnd; pFol++) {
        MaxNrNodes10 = CalcNrNodes10(pLngSpeechUnitLex, pFirstPhenomOfUwSu, LngNbrSpeechUnits, pFol,
                                     MaxNrNodes10, pMaxBranch);
    }
    return MaxNrNodes10;
}

/* Rounds the maximum weighted estimate across the tree and adds the fixed allowance. */
unsigned long CalcAccuracyCtxTree(struct _ENGINE *pEngine)
{
    unsigned long NrNodes10;
    const unsigned long *pLngSpeechUnitLex;
    struct _ISOWORD_NODE *pTree;
    unsigned long MaxBranch = 0;
    unsigned short LngNbrSpeechUnits;
    const unsigned long *pFirstPhenomOfUwSu;
    pTree = pEngine->CtxInfo.pCtxTree;
    pLngSpeechUnitLex = pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    LngNbrSpeechUnits = pEngine->pLang->langGetNbrSpeechUnit(pEngine->pLang);
    pFirstPhenomOfUwSu = pEngine->pFirstPhenomOfUserWordSpeechUnit;
    NrNodes10 = 0.5f + CalcNrNodes10(pLngSpeechUnitLex, pFirstPhenomOfUwSu, LngNbrSpeechUnits,
                                     pTree, 0.0f, &MaxBranch);
    return NrNodes10 + 135;
}

/* Counts the word IDs represented by a leaf and its backtrace. */
static unsigned long _CountWordsInTree(struct _ISOWORD_BACKTRACE *pArr, struct _ISOWORD_NODE *pCur)
{
    unsigned long NrWords = 0;
    u16 NrFoll = pCur->NrFol;
    u32 i;

    if (NrFoll != 0) {
        for (i = 0; i < NrFoll; i++) {
            NrWords += _CountWordsInTree(pArr, pCur->u.pFirstFol + i);
        }
        return NrWords;
    }

    if (pCur->u.WordID >= 0) {
        return 1;
    }

    {
        u32 Index = -1 - pCur->u.WordID;
        s32 WordCount = 0;

        while (Index != 0xFFFFFFFFU) {
            Index = pArr[Index].u32PrevWordIdx;
            WordCount++;
        }
        return WordCount;
    }
}

/* Appends the word IDs represented by a leaf and its backtrace. */
static void _GetWordsInTree(struct _ISOWORD_BACKTRACE *pArr, struct _ISOWORD_NODE *pCur,
                            unsigned long **ppidWords)
{
    u16 NrFoll = pCur->NrFol;
    u32 i;

    if (NrFoll != 0) {
        for (i = 0; i < NrFoll; i++) {
            _GetWordsInTree(pArr, pCur->u.pFirstFol + i, ppidWords);
        }
        return;
    }

    if (pCur->u.WordID >= 0) {
        *(*ppidWords)++ = pCur->u.WordID;
    } else {
        u32 Index = -1 - pCur->u.WordID;

        while (Index != 0xFFFFFFFFU) {
            *(*ppidWords)++ = pArr[Index].s32WordID;
            Index = pArr[Index].u32PrevWordIdx;
        }
    }
}

/* Compares word IDs by value. */
int CompareIDs(const void *A, const void *B)
{
    const long *idA = A;
    const long *idB = B;

    return *idA - *idB;
}

/* Counts the words whose terminal flag is clear. */
static unsigned long GetNbrNonTermWords(struct _ASRSPI_CONTEXT *pCtx)
{
    const unsigned short *pWordProps = pCtx->contextGetpWordProp(pCtx);
    unsigned long uNbrOfWords = pCtx->contextGetNbrWord(pCtx);
    unsigned long Nbr = 0;
    unsigned long i;
    for (i = 0; i < uNbrOfWords; i++) {
        if ((pWordProps[i] & 0x80) == 0) {
            Nbr++;
        }
    }
    return Nbr;
}

/* Copies the IDs of words that are marked as nonterminal. */
static void CopyNonTerminalWords(struct _ASRSPI_CONTEXT *pCtx, unsigned long **ppidWordsTemp)
{
    unsigned long *pidWords = *ppidWordsTemp;
    const unsigned short *pWordProps = pCtx->contextGetpWordProp(pCtx);
    unsigned long uNbrOfWords = pCtx->contextGetNbrWord(pCtx);
    unsigned long i;
    for (i = 0; i < uNbrOfWords; i++) {
        if ((pWordProps[i] & 0x80) == 0) {
            *pidWords++ = i;
        }
    }
    *ppidWordsTemp = pidWords;
}

/* Collects, sorts and deduplicates the word IDs available to isolated-word recognition. */
int GetIsolatedWordDpActiveWords(struct _ENGINE *pEngine, unsigned long **ppidWords,
                                 unsigned long *pNbrWords)
{
    unsigned long *pidWords;
    unsigned long NbrWords;
    struct _ASRSPI_CONTEXT Ctx;
    unsigned long *pidWordsTemp;
    unsigned long *pRead;
    unsigned long *pWrite;
    unsigned long OldNbrWords;
    unsigned long i;
    struct _ISOWORD_BACKTRACE *pArr;
    if (pEngine->CtxInfo.pCtxTree != NULL) {
        FillContextV2VirtualTable(&Ctx);
        Ctx.pCtx = pEngine->ActivatedContext->pCtx;
        if (pEngine->CtxInfo.pCtxTree->NrFol != 0) {
            pArr = pEngine->pBacktraceArray;
            NbrWords = _CountWordsInTree(pArr, pEngine->CtxInfo.pCtxTree);
            NbrWords += GetNbrNonTermWords(&Ctx);
            pidWords = heap_Alloc(pEngine->hHeap, NbrWords * sizeof(unsigned long));
            if (pidWords == NULL) {
                return _tosErrorLog(&pEngine->Object, 1);
            }
            pidWordsTemp = pidWords;
            CopyNonTerminalWords(&Ctx, &pidWordsTemp);
            _GetWordsInTree(pArr, pEngine->CtxInfo.pCtxTree, &pidWordsTemp);
            qsort(pidWords, NbrWords, sizeof(*pidWords), CompareIDs);
            pRead = pWrite = pidWords;
            OldNbrWords = NbrWords;
            for (i = 0; i < OldNbrWords - 1; i++) {
                if (pRead != pWrite) {
                    *pWrite = *pRead;
                }
                if (*pRead != pRead[1]) {
                    pWrite++;
                } else {
                    NbrWords--;
                }
                pRead++;
            }
            if (pRead != pWrite) {
                *pWrite = *pRead;
            }
        } else {
            NbrWords = GetNbrNonTermWords(&Ctx);
            pidWords = heap_Alloc(pEngine->hHeap, NbrWords * sizeof(unsigned long));
            if (pidWords == NULL) {
                return _tosErrorLog(&pEngine->Object, 1);
            }
            {
                unsigned long *pidWordsTemp = pidWords;
                CopyNonTerminalWords(&Ctx, &pidWordsTemp);
            }
        }
        *pNbrWords = NbrWords;
        *ppidWords = pidWords;
        return 0;
    }
    return _tosErrorLog(&pEngine->Object, 4);
}

/* Rebuilds the isolated-word context tree for a speech context. */
int SetIsolatedWordDpActiveWords(struct _ENGINE *pEngine, u32 *pidWords, u32 NbrWords)
{
    struct ISOWORD_USERWORD_LNG UserWordLng;
    struct _ASRSPI_CONTEXT treeContext;
    void *treeData;
    s32 result;
    u32 resultCode;

    if (pEngine->CtxInfo.pCtxTree != NULL) {
        if (_tosControl(&pEngine->Object, 0x29, 0, 0) != 0) {
            return pEngine->Tos.ErrorLog;
        }
        FreeTree(pEngine);
        if (NbrWords == 0) {
            /* A non-null empty list keeps the builder from enabling every word. */
            pidWords = (u32 *) pEngine;
        }
        FillContextV2VirtualTable(&treeContext);
        treeData = pEngine->ActivatedContext->pCtx;
        treeContext.pCtx = treeData;
        result = intCreateCtxTree(pEngine, &treeContext, pidWords, NbrWords);
        resultCode = (u8) result;
        if (resultCode != 0) {
            return resultCode;
        }
        UserWordLng.pUserWordLngSpeechUnitLex = pEngine->pUserWordLngSpeechUnitLex;
        UserWordLng.pFirstPhenomOfUserWordSpeechUnit = pEngine->pFirstPhenomOfUserWordSpeechUnit;
        if (_tosControl(&pEngine->Object, 0x24, (u32) pEngine->CtxInfo.pCtxTree,
                        (u32) &UserWordLng) != 0) {
            FreeCtxInfo(pEngine);
            return pEngine->Tos.ErrorLog;
        }
        return 0;
    }
    return _tosErrorLog(&pEngine->Object, 4);
}
