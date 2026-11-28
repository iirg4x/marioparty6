/* Builds the word and transition dictionaries used by speech recognition's second pass. */
#include "gssdk/secondpass.h"
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "types.h"
#include "gssdk/langdata.h"
#include <string.h>

/* Packs joining translations and appends the two halves of the second-pass word ID. */
static void putTransWordInDict(u32 **ppBeginOfPron, u32 *pDictIndex, struct _DICTSTATE **ppState,
                               u32 WordID, u16 *pIndex, u16 *pIndexEnd, u16 *pTransWordLex)
{
    u16 wordIndexLow = WordID;
    u32 *pBeginOfPron = *ppBeginOfPron;
    struct _DICTSTATE *pState = *ppState;
    u16 *indexCursor = pIndex;
    u16 *pPhen = pTransWordLex + *indexCursor;
    u16 *pPhenEnd;

    WordID >>= 16;
    while (indexCursor < pIndexEnd) {
        *pBeginOfPron++ = *pDictIndex;
        pPhenEnd = pTransWordLex + *++indexCursor;
        *pDictIndex += pPhenEnd - pPhen + 2;
        while (pPhen < pPhenEnd - 1) {
            pState++->StateIndex = (*pPhen++ * 4) | 2;
        }
        pState[0].StateIndex = (*pPhen++ * 4) | 1;
        pState[1].StateIndex = wordIndexLow;
        pState[2].StateIndex = WordID;
        pState += 3;
    }

    *ppState = pState;
    *ppBeginOfPron = pBeginOfPron;
}

/*
 * Builds the second-pass pronunciation graph and packed dictionary from the first-pass results.
 * Called by the recognizer when it prepares second-pass search data from first-pass results.
 */
unsigned char CreateSecondPassContext(struct _ENGINE *pEngine)
{
    int speechUnitClassCount;
    struct _DICTSTATE *dictionary;
    struct _DYNPROG_RESULT_NODE *sentence;
    u32 firstPassOffsetBase;
    u16 *wordAttributes;
    s32 rootTransitionCount;
    u32 initialGraphWordCount;
    s32 tokenCount;
    struct _SECONDPASSINFO *dictionaryWords;
    u16 *translationTokenLexicon;
    struct _DYNPROG_CTXINFO *secondPass;
    u32 *pronunciationOffsetTable;
    u32 encodedWordOffset;
    struct _DICTSTATE *encodedPronunciationDictionary;
    u32 maxPronunciationsPerWord;
    s32 graphPronunciationCount;
    struct _NODE *graphNodes;
    struct _VERTEX *transitions;
    u32 nextGraphTransition;
    u32 nextGraphNode;
    struct _DYNPROG_CTXINFO *firstPassData;
    u32 *wordOffsets;
    struct _DYNPROG_RESULT_NODE *pSent;
    struct _DYNPROG_RESULT_NODE *word;
    struct _TRANSWORDINFO *translationDictionary;
    u16 *translationPronunciationOffsets;
    u32 previousBranchCount;
    u32 wordCount;
    u32 totalPronunciationCount;
    u32 nextTranslationWordIndex;
    u32 transitionCount;
    u16 *wordFlagCursor;
    s16 *wordPenaltyCursor;
    u32 *wordBoundaryCursor;
    u32 *packedTransitionCursor;
    u32 *transitionBoundaryCursor;
    u32 *allPronunciationOffsets;
    struct _NEWID2OLDID *firstPassWordMapCursor;
    struct _NODE *graphNodeCursor;
    u32 graphNodeIndex;
    u16 translationID;

    maxPronunciationsPerWord = 0;
    graphPronunciationCount = 0;
    graphNodes = NULL;
    transitions = NULL;
    nextGraphTransition = 0;
    nextGraphNode = 1;
    pSent = pEngine->pRes->pResult;
    firstPassData = pEngine->CtxInfo.pDynProgCtxInfo;
    wordOffsets = firstPassData->pBeginOfWords;
    firstPassOffsetBase = *wordOffsets;
    wordAttributes = firstPassData->pWordProps;
    dictionary = firstPassData->pStateInDict;
    dictionaryWords = pEngine->CtxInfo.pSecondPassInfo;
    rootTransitionCount = 0;
    tokenCount = 0;
    speechUnitClassCount = pEngine->pLang->langGetNbrSpeechUnitClass(pEngine->pLang);
    translationDictionary = pEngine->pLang->langGetpTransWord(pEngine->pLang);
    translationTokenLexicon = pEngine->pLang->transwGetpTransWordLex(translationDictionary);
    translationPronunciationOffsets =
        pEngine->pLang->transwGetpBeginOfTransWords(translationDictionary);
    encodedWordOffset = 0;
    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL) {
        FreeSecondPassContext(pEngine);
    }
    secondPass = heap_Alloc(pEngine->hHeap, sizeof(struct _DYNPROG_CTXINFO));
    if (secondPass == NULL) {
        goto failure;
    }
    memset(secondPass, 0, sizeof(struct _DYNPROG_CTXINFO));
    pEngine->CtxInfo.pSecondPassDynProgCtxInfo = secondPass;

    /* The first-pass pronunciation counts bound the temporary graph. */
    while (pSent != NULL) {
        word = pSent->pInfo;
        while (word != NULL) {
            struct _DICTSTATE *encodedWordEntry = dictionary + word->StateNbr;
            u32 *pronunciationBounds =
                wordOffsets + (encodedWordEntry[1].StateIndex +
                               (encodedWordEntry[2].StateIndex << 16));
            u32 count = pronunciationBounds[1] - pronunciationBounds[0];
            if (count > maxPronunciationsPerWord) {
                maxPronunciationsPerWord = count;
            }
            graphPronunciationCount += count;
            word = word->pNext;
        }
        pSent = pSent->pNext;
    }
    graphNodes = heap_Alloc(pEngine->hHeap, (graphPronunciationCount + 1) * sizeof(struct _NODE));
    transitions = heap_Alloc(pEngine->hHeap,
        graphPronunciationCount * maxPronunciationsPerWord * sizeof(struct _VERTEX));
    if (graphNodes == NULL || transitions == NULL) {
        goto failure;
    }
    graphNodes[0].iVertex = 0xFFFF;
    /*
     * Enabled menus with common pronunciation paths share graph nodes; each distinct pronunciation
     * extends the search path with its own node.
     */
    for (sentence = pEngine->pRes->pResult; sentence != NULL;
         sentence = sentence->pNext) {
        u32 rootNodeIndex = 0;
        u32 dictionaryIndex = 0;
        s32 wordID = 0;
        word = sentence->pInfo;
        if (word != NULL) {
            struct _VERTEX *transition;
            u32 searchLink;
            u32 link;
            while (word != NULL) {
                struct _NODE *node = graphNodes + rootNodeIndex;
                struct _DICTSTATE *encodedWordEntry = dictionary + word->StateNbr;
                searchLink = node->iVertex;
                wordID = encodedWordEntry[1].StateIndex + (encodedWordEntry[2].StateIndex << 16);
                dictionaryIndex = wordOffsets[wordID] - firstPassOffsetBase;
                for (;;) {
                    if (searchLink == 0xFFFF) {
                        u32 count;
                        if (rootNodeIndex != 0) {
                            u32 *pronunciationBounds = wordOffsets + node->OldWordID;
                            count = pronunciationBounds[1] - pronunciationBounds[0];
                        } else {
                            count = 1;
                        }
                        previousBranchCount = count;
                        break;
                    } else {
                        u16 destination;
                        transition = transitions + searchLink;
                        destination = transition->iNode;
                        if (dictionaryIndex == graphNodes[destination].OldPronID) {
                            word = word->pNext;
                            rootNodeIndex = destination;
                            break;
                        }
                        searchLink = transition->iVertex;
                    }
                }
                if (searchLink == 0xFFFF) {
                    break;
                }
            }
            link = graphNodes[rootNodeIndex].iVertex;
            if (word != NULL) {
                for (;;) {
                    u32 count;
                    u32 *pronunciationBounds = wordOffsets + wordID;
                    u32 pronunciation;
                    count = pronunciationBounds[1] - pronunciationBounds[0];
                    for (pronunciation = 0; pronunciation < count; pronunciation++) {
                        u32 destination = nextGraphNode + pronunciation;
                        u32 previousNodeIndex;
                        for (previousNodeIndex = 0; previousNodeIndex < previousBranchCount;
                             previousNodeIndex++) {
                            transitions[nextGraphTransition + previousNodeIndex].iNode =
                                (u16) destination;
                            transitions[nextGraphTransition + previousNodeIndex].iVertex =
                                link == 0xFFFF ? 0xFFFF : link + previousNodeIndex;
                        }
                        link = nextGraphTransition;
                        if (pronunciation == count - 1) {
                            for (previousNodeIndex = 0; previousNodeIndex < previousBranchCount;
                                 previousNodeIndex++) {
                                graphNodes[rootNodeIndex + previousNodeIndex].iVertex =
                                    nextGraphTransition + previousNodeIndex;
                            }
                        }
                        graphNodes[destination].OldPronID = dictionaryIndex + pronunciation;
                        nextGraphTransition = (u16)(nextGraphTransition + previousBranchCount);
                        graphNodes[destination].OldWordID = wordID;
                    }
                    word = word->pNext;
                    rootNodeIndex = nextGraphNode;
                    previousBranchCount = count;
                    nextGraphNode = (u16)(nextGraphNode + count);
                    if (word == NULL) {
                        break;
                    } else {
                        struct _DICTSTATE *encodedWordEntry;
                        link = 0xFFFF;
                        encodedWordEntry = dictionary + word->StateNbr;
                        wordID = encodedWordEntry[1].StateIndex +
                                 (encodedWordEntry[2].StateIndex << 16);
                        dictionaryIndex = wordOffsets[wordID] - firstPassOffsetBase;
                    }
                }
            }
            {
                u32 previousNodeIndex;
                for (previousNodeIndex = 0; previousNodeIndex < previousBranchCount;
                     previousNodeIndex++) {
                    graphNodes[rootNodeIndex++].iVertex = 0xFFFF;
                }
            }
        }
    }

    {
        u32 link = graphNodes[0].iVertex;
        while (link != 0xFFFF) {
            link = transitions[link].iVertex;
            rootTransitionCount++;
        }
    }
    wordCount = nextGraphNode - 1;
    initialGraphWordCount = totalPronunciationCount = wordCount;
    nextTranslationWordIndex = wordCount;
    transitionCount = nextGraphTransition - rootTransitionCount;
    graphNodeCursor = graphNodes + 1;
    /*
     * Resolves each edge's joining word from the source trailing class and destination leading
     * class. Edges without an available translation are marked for direct traversal.
     */
    for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
         graphNodeCursor++, graphNodeIndex++) {
        u32 link = graphNodeCursor->iVertex;
        if (link != 0) {
            struct _SECONDPASSINFO *pInfo = dictionaryWords + graphNodeCursor->OldPronID;
            u16 trailingClass = pInfo->RightSpeechUnitClass;
            u16 wordTokenCount = pInfo->Size;
            tokenCount += wordTokenCount;
            while (link != 0xFFFF) {
                u16 destination = transitions[link].iNode;
                s32 destinationWord = graphNodes[destination].OldPronID;
                u16 leadingClass = dictionaryWords[destinationWord].LeftSpeechUnitClass;
                u16 first;
                u16 last;
                int count;
                translationID = leadingClass + trailingClass * speechUnitClassCount;
                first = translationPronunciationOffsets[translationID];
                last = translationPronunciationOffsets[translationID + 1];
                count = translationPronunciationOffsets[last] -
                    translationPronunciationOffsets[first];
                if (count != 0) {
                    tokenCount = tokenCount + count;
                    totalPronunciationCount += last - first;
                    wordCount++;
                    transitionCount++;
                } else {
                    translationID = 0xFFFF;
                }
                transitions[link].TransWordIndex = translationID;
                link = transitions[link].iVertex;
            }
        }
    }
    /* Builds the menu selection masks for this word list. */
    if (CreateMenuMasks(pEngine, secondPass, wordCount) != 0) {
        goto failure;
    }
    secondPass->pSyntax = heap_Alloc(pEngine->hHeap,
        (wordCount + 1 + transitionCount) * sizeof(*secondPass->pSyntax));
    if (secondPass->pSyntax == NULL) {
        goto failure;
    }
    transitionBoundaryCursor = secondPass->pSyntax;
    packedTransitionCursor = secondPass->pSyntax + wordCount + 1;
    secondPass->pWordProps =
        heap_Alloc(pEngine->hHeap, wordCount * sizeof(*secondPass->pWordProps));
    if (secondPass->pWordProps == NULL) {
        goto failure;
    }
    wordFlagCursor = secondPass->pWordProps;
    memset(wordFlagCursor, 0, wordCount * sizeof(*secondPass->pWordProps));
    secondPass->pWordPenalties = heap_Alloc(pEngine->hHeap,
        wordCount * sizeof(*secondPass->pWordPenalties));
    if (secondPass->pWordPenalties == NULL) {
        goto failure;
    }
    wordPenaltyCursor = secondPass->pWordPenalties;
    secondPass->pBeginOfWords = heap_Alloc(pEngine->hHeap,
        (wordCount + totalPronunciationCount + 2) * sizeof(*secondPass->pBeginOfWords));
    if (secondPass->pBeginOfWords == NULL) {
        goto failure;
    }
    allPronunciationOffsets = secondPass->pBeginOfWords;
    pronunciationOffsetTable = allPronunciationOffsets + wordCount + 1;
    wordBoundaryCursor = allPronunciationOffsets;
    tokenCount += totalPronunciationCount * 2;
    secondPass->pStateInDict = heap_Alloc(pEngine->hHeap, tokenCount * sizeof(struct _DICTSTATE));
    if (secondPass->pStateInDict == NULL) {
        goto failure;
    }
    encodedPronunciationDictionary = secondPass->pStateInDict;
    pEngine->CtxInfo.pNewWordID2OldWordID =
        heap_Alloc(pEngine->hHeap, wordCount * sizeof(struct _NEWID2OLDID));
    if (pEngine->CtxInfo.pNewWordID2OldWordID == NULL) {
        goto failure;
    }
    firstPassWordMapCursor = pEngine->CtxInfo.pNewWordID2OldWordID;
    encodedWordOffset = 0;
    {
        u32 link = graphNodes[0].iVertex;
        while (link != 0xFFFF) {
            struct _VERTEX *transition = transitions + link;
            wordFlagCursor[transition->iNode - 1] = 1;
            link = transition->iVertex;
        }
    }
    /* Packs graph pronunciations into the second-pass arrays and records their first-pass IDs. */
    graphNodeCursor = graphNodes + 1;
    for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
         graphNodeCursor++, graphNodeIndex++) {
        s32 wordID = graphNodeCursor->OldWordID;
        u32 link = graphNodeCursor->iVertex;
        u32 wordAttributeFlags = wordAttributes[wordID];
        struct _SECONDPASSINFO *pInfo;
        u16 pronunciationTokenCount;
        struct _DICTSTATE *wordTokenEnd;
        u32 dictionaryWordIndex;
        firstPassWordMapCursor->OldWordID = wordID;
        firstPassWordMapCursor->OldPronID = graphNodeCursor->OldPronID;
        firstPassWordMapCursor++;
        *transitionBoundaryCursor++ = packedTransitionCursor - secondPass->pSyntax;
        *pronunciationOffsetTable = encodedWordOffset;
        *wordBoundaryCursor++ = pronunciationOffsetTable++ - allPronunciationOffsets;
        pInfo = dictionaryWords + graphNodeCursor->OldPronID;
        pronunciationTokenCount = pInfo->Size;
        wordTokenEnd = encodedPronunciationDictionary;
        memcpy(wordTokenEnd, dictionary + pInfo->StateDictIndex,
               pronunciationTokenCount * sizeof(*wordTokenEnd));
        wordTokenEnd += pronunciationTokenCount;
        wordTokenEnd[-1].StateIndex |= 1;
        dictionaryWordIndex = graphNodeIndex - 1;
        wordTokenEnd[0].StateIndex = dictionaryWordIndex;
        wordTokenEnd[1].StateIndex = dictionaryWordIndex >> 16;
        encodedPronunciationDictionary = wordTokenEnd + 2;
        encodedWordOffset += pronunciationTokenCount + 2;
        if (link != 0) {
            if (link == 0xFFFF) {
                *wordFlagCursor |= 2;
            } else if (*wordFlagCursor != 1) {
                *wordFlagCursor = wordAttributeFlags;
            }
            if (*wordFlagCursor++ & 0x80) {
                *wordPenaltyCursor++ = pEngine->WordPenalty;
            } else {
                *wordPenaltyCursor++ = 0;
            }
            while (link != 0xFFFF) {
                struct _VERTEX *transition = transitions + link;
                if (transition->TransWordIndex == 0xFFFF) {
                    *packedTransitionCursor++ = transition->iNode - 1;
                } else {
                    *packedTransitionCursor++ = nextTranslationWordIndex++;
                }
                link = transition->iVertex;
            }
        }
    }
    /* Appends available joining translations and connects each one to its destination word. */
    graphNodeCursor = graphNodes + 1;
    nextTranslationWordIndex = initialGraphWordCount;
    *transitionBoundaryCursor++ = packedTransitionCursor - secondPass->pSyntax;
    *wordBoundaryCursor++ = pronunciationOffsetTable - allPronunciationOffsets;
    for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
         graphNodeCursor++, graphNodeIndex++) {
        u32 link = graphNodeCursor->iVertex;
        if (link != 0) {
            while (link != 0xFFFF) {
                struct _VERTEX *transition = transitions + link;
                u16 translationID = transition->TransWordIndex;
                if (translationID != 0xFFFF) {
                    firstPassWordMapCursor->OldWordID = -1;
                    firstPassWordMapCursor->OldPronID = -1;
                    firstPassWordMapCursor++;
                    putTransWordInDict(&pronunciationOffsetTable, &encodedWordOffset,
                                       &encodedPronunciationDictionary,
                                       nextTranslationWordIndex++,
                                       translationPronunciationOffsets +
                                           translationPronunciationOffsets[translationID],
                                       translationPronunciationOffsets +
                                           translationPronunciationOffsets[translationID + 1],
                                       translationTokenLexicon);
                    *wordBoundaryCursor++ = pronunciationOffsetTable - allPronunciationOffsets;
                    *packedTransitionCursor++ = transition->iNode - 1;
                    *transitionBoundaryCursor++ =
                        packedTransitionCursor - secondPass->pSyntax;
                    *wordPenaltyCursor++ = 0;
                }
                link = transition->iVertex;
            }
        }
    }
    *pronunciationOffsetTable++ = encodedWordOffset;
    heap_Free(pEngine->hHeap, graphNodes);
    heap_Free(pEngine->hHeap, transitions);
    secondPass->iOptionalSilence = 0;
    secondPass->uNbrOfWords = wordCount;
    secondPass->uNrStatesInDict = tokenCount;
    return 0;

failure:
    if (graphNodes != NULL) {
        heap_Free(pEngine->hHeap, graphNodes);
    }
    if (transitions != NULL) {
        heap_Free(pEngine->hHeap, transitions);
    }
    FreeSecondPassContext(pEngine);
    return 1;
}

/*
 * Releases cached and second-pass search data.
 * CreateSecondPassContext calls this before a rebuild and when memory setup fails.
 */
void FreeSecondPassContext(struct _ENGINE *pEngine)
{
    if (pEngine->CtxInfo.pNewWordID2OldWordID != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pNewWordID2OldWordID);
        pEngine->CtxInfo.pNewWordID2OldWordID = NULL;
    }

    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL) {
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pBeginOfWords != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pBeginOfWords);
        }
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pStateInDict != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pStateInDict);
        }
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pSyntax != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pSyntax);
        }
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordPenalties != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordPenalties);
        }
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordProps != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordProps);
        }
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pucMenuMasks != NULL) {
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pucMenuMasks);
        }
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo);
        pEngine->CtxInfo.pSecondPassDynProgCtxInfo = NULL;
    }
}
