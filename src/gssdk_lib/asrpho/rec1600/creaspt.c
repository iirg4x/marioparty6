/* Builds the word dictionary and pronunciation graph for speech recognition's tree search. */
#include "gssdk/secondpass.h"
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "types.h"
#include "gssdk/langdata.h"
#include <string.h>

/* Pronunciation metadata retained while resolving a graph node's joining word. */
typedef struct NodeWordInfo {
    u16 tokenCount; /* Number of pronunciation tokens. */
    u8 leadingClass; /* Speech-unit class at the beginning. */
    u8 trailingClass; /* Speech-unit class at the end. */
} NodeWordInfo;

/* Reads token counts and joining classes for graph and edge construction.
 * Type 1 expands speech units, skipping the first and last units when there are more than two;
 * other types use direct tokens and the user-word class. Always returns 0. */
u8 GetSecondPassInfo(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx,
                     struct _SECONDPASSINFO *pSecondPassInfo, unsigned int PronID)
{
    const u32 *pSpeechUnitLex = pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    u8 defaultClass = pEngine->pLang->langGetUserWordSpeechUnitClass(pEngine->pLang);
    u16 *dictionary = pCtx->contextGetpLex(pCtx);
    u32 *wordOffsets = pCtx->contextGetpBeginOfWords(pCtx);
    unsigned int wordType;
    u16 *pronunciation;
    u16 *end;
    int length;

    wordOffsets += *wordOffsets;
    pronunciation = dictionary + wordOffsets[PronID];
    wordType = *pronunciation++;
    end = dictionary + wordOffsets[PronID + 1];
    length = end - pronunciation;
    if (wordType == 1) {
        u32 tokenCount = 0;

        pSecondPassInfo->LeftSpeechUnitClass =
            ((const u16 *)pSpeechUnitLex)[pSpeechUnitLex[*pronunciation]];
        --end;
        pSecondPassInfo->RightSpeechUnitClass =
            ((const u16 *)pSpeechUnitLex)[pSpeechUnitLex[*end] + 1];
        if (length > 2) {
            ++pronunciation;
            --end;
        }
        while (pronunciation <= end) {
            u16 speechUnit = *pronunciation;
            tokenCount += pSpeechUnitLex[speechUnit + 1] - pSpeechUnitLex[speechUnit] - 2;
            ++pronunciation;
        }
        pSecondPassInfo->Size = tokenCount;
    } else {
        pSecondPassInfo->RightSpeechUnitClass = defaultClass;
        pSecondPassInfo->LeftSpeechUnitClass = defaultClass;
        pSecondPassInfo->Size = length;
    }
    return 0;
}

/* Packs joining translations and appends the two halves of the second-pass word ID. */
static void putTransWordInDict(u32 **ppBeginOfPron, u32 *pDictIndex, u16 **ppState,
                               u32 WordID, u16 *pIndex, u16 *pIndexEnd, u16 *pTransWordLex)
{
    u16 wordIndexLow = WordID;
    u32 *pBeginOfPron = *ppBeginOfPron;
    u16 *pState = *ppState;
    u16 *indexCursor = pIndex;
    u16 *pPhen = pTransWordLex + *indexCursor;
    u16 *pPhenEnd;

    WordID >>= 16;
    while (indexCursor < pIndexEnd) {
        *pBeginOfPron++ = *pDictIndex;
        pPhenEnd = pTransWordLex + *++indexCursor;
        *pDictIndex += pPhenEnd - pPhen + 2;
        while (pPhen < pPhenEnd - 1) {
            *pState++ = (*pPhen++ * 4) | 2;
        }
        pState[0] = (*pPhen++ * 4) | 1;
        pState[1] = wordIndexLow;
        pState[2] = WordID;
        pState += 3;
    }

    *ppState = pState;
    *ppBeginOfPron = pBeginOfPron;
}

/* Packs a context pronunciation as token IDs shifted left two bits. Type 1 expands speech
 * units, skipping the first and last units when there are more than two. Sets bit 0 on the
 * final token and appends the low and high 16-bit halves of its second-pass word index. */
static void putWordInDict(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx,
                          unsigned int PronID, u32 *pDictIndex, u16 **ppState, u32 WordID)
{
    const u32 *pSpeechUnitLex = pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    u16 *dictionary = pCtx->contextGetpLex(pCtx);
    u32 *wordOffsets = pCtx->contextGetpBeginOfWords(pCtx);
    u32 offsetBase = *wordOffsets;
    u32 *pronunciationOffsets = wordOffsets + offsetBase;
    u16 *pronunciation = dictionary + pronunciationOffsets[PronID];
    unsigned int wordType = *pronunciation++;
    u16 *end = dictionary + pronunciationOffsets[PronID + 1];
    unsigned int length = end - pronunciation;
    u16 *output = *ppState;
    u32 tokensWritten = 0;

    if (wordType == 1) {
        --end;
        if (length > 2) {
            ++pronunciation;
            --end;
        }
        while (pronunciation <= end) {
            const u32 *bounds = pSpeechUnitLex + *pronunciation;
            u32 tokenCount = bounds[1] - bounds[0] - 2;
            u32 tokenIndex = 0;
            const u16 *pPhenoms = (const u16 *)pSpeechUnitLex + bounds[0] + 2;

            for (; tokenIndex < tokenCount; tokenIndex++) {
                *output++ = (pPhenoms[tokenIndex] * 4) & 0xFFFC;
            }
            tokensWritten += tokenCount;
            ++pronunciation;
        }
    } else {
        u32 tokenIndex = 0;
        tokensWritten = length;
        for (; tokenIndex < length; tokenIndex++) {
            *output++ = (pronunciation[tokenIndex] * 4) & 0xFFFC;
        }
    }
    output[-1] |= 1;
    output[0] = WordID;
    output[1] = WordID >> 16;
    *pDictIndex += tokensWritten + 2;
    *ppState = output + 2;
}

/* Builds the shared-prefix pronunciation graph and packed second-pass context
 * for configured tree words. */
u8 CreateSecondPassTreeContext(struct _ENGINE *pEngine)
{
    u16 *translationOffsets;
    u32 *wordOffsets;
    u32 speechUnitClassCount;
    s32 *sequenceCursor;
    struct _WORDRESULT *itemCursor;
    u32 treeItemIndex;
    u32 rootEdgeCount;
    u32 tokenCount;
    u32 treeWordCount;
    u32 pronunciationOffsetBase;
    u16 *translationTokens;
    u16 *wordProperties;
    s32 finalWord;
    struct _DYNPROG_CTXINFO *secondPass;
    unsigned int sequenceLength;
    struct _ASRSPI_CONTEXT dictionaryContext;
    NodeWordInfo sourceWordInfo;
    u32 *pronunciationOffsets;
    u32 encodedOffset;
    u16 *encodedDictionary;
    struct _SECONDPASSINFO wordInfo;
    struct _NODE *graphNodes;
    struct _VERTEX *edges;
    struct _WORDRESULT *treeWords;
    struct _WORDRESULT *treeWordCursor;
    u32 maximumPronunciations;
    u32 graphCapacity;
    u32 nextNode;
    u32 nextEdge;
    struct _TRANSWORDINFO *translationDictionary;
    u32 contextWordCount;
    s32 firstWord;
    s32 *sequence;
    u32 itemIndex;
    u32 previousPronunciationCount;
    u32 wordCount;
    u32 originalWordCount;
    u32 pronunciationCount;
    u32 transitionCount;
    u32 nextTranslationWordIndex;
    u16 *wordFlagCursor;
    s16 *penaltyCursor;
    u32 *wordBoundaries;
    u32 *packedTransitions;
    u32 *transitionBoundaries;
    u32 *allWordOffsets;
    struct _NEWID2OLDID *wordMap;
    struct _NODE *nodeCursor;
    u32 nodeIndex;
    unsigned int i;
    unsigned int iVertex;

    maximumPronunciations = 0;
    graphCapacity = 0;
    graphNodes = NULL;
    edges = NULL;
    nextEdge = 0;
    rootEdgeCount = 0;
    nextNode = 1;
    tokenCount = 0;
    speechUnitClassCount = pEngine->pLang->langGetNbrSpeechUnitClass(pEngine->pLang);
    treeWords = (struct _WORDRESULT *)pEngine->pRes->pResult;
    treeWordCursor = treeWords;
    treeWordCount = pEngine->NBest;
    translationDictionary = pEngine->pLang->langGetpTransWord(pEngine->pLang);
    translationTokens = pEngine->pLang->transwGetpTransWordLex(translationDictionary);
    translationOffsets = pEngine->pLang->transwGetpBeginOfTransWords(translationDictionary);
    encodedOffset = 0;
    firstWord = -1;
    finalWord = -1;
    /* Discards existing second-pass state and cached mappings before allocating a replacement. */
    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL) {
        FreeSecondPassContext(pEngine);
    }
    secondPass = heap_Alloc(pEngine->hHeap, sizeof(struct _DYNPROG_CTXINFO));
    if (secondPass == NULL) {
        goto failure;
    }
    memset(secondPass, 0, sizeof(struct _DYNPROG_CTXINFO));
    pEngine->CtxInfo.pSecondPassDynProgCtxInfo = secondPass;
    FillContextV2VirtualTable(&dictionaryContext);
    dictionaryContext.pCtx = pEngine->ActivatedContext->pCtx;
    contextWordCount = dictionaryContext.contextGetNbrWord(&dictionaryContext);
    wordProperties = dictionaryContext.contextGetpWordProp(&dictionaryContext);
    wordOffsets = dictionaryContext.contextGetpBeginOfWords(&dictionaryContext);
    pronunciationOffsetBase = *wordOffsets;

    /* Finds the context word IDs marked as first and final by their property bits. */
    for (i = 0; i < contextWordCount; i++) {
        if (wordProperties[i] & 1) {
            firstWord = i;
            if (finalWord != -1) {
                break;
            }
        }
        if (wordProperties[i] & 2) {
            finalWord = i;
            if (firstWord != -1) {
                break;
            }
        }
    }
    /* Bounds the temporary graph using the sequence pronunciations and inserted first and final
     * words. */
    for (itemIndex = 0; itemIndex < treeWordCount; itemIndex++, treeWordCursor++) {
        s32 wordID = treeWordCursor->WordID;
        if (wordID < 0) {
            u32 sequenceIndex = -1 - wordID;
            struct _ISOWORD_BACKTRACE *wordSequences = pEngine->pBacktraceArray;
            while (sequenceIndex != -1) {
                struct _ISOWORD_BACKTRACE *sequenceWord = wordSequences + sequenceIndex;
                u32 *pronunciationBounds = wordOffsets + sequenceWord->s32WordID;
                u32 count = pronunciationBounds[1] - pronunciationBounds[0];
                if (count > maximumPronunciations) {
                    maximumPronunciations = count;
                }
                graphCapacity += count;
                sequenceIndex = sequenceWord->u32PrevWordIdx;
            }
            graphCapacity += 2;
        } else {
            u32 count = wordOffsets[wordID + 1] - wordOffsets[wordID];
            if (count > maximumPronunciations) {
                maximumPronunciations = count;
            }
            graphCapacity = count + graphCapacity;
            graphCapacity += 2;
        }
    }
    graphNodes = heap_Alloc(pEngine->hHeap, (graphCapacity + 1) * sizeof(struct _NODE));
    edges = heap_Alloc(pEngine->hHeap,
        graphCapacity * maximumPronunciations * sizeof(struct _VERTEX));
    if (graphNodes == NULL || edges == NULL) {
        goto failure;
    }
    graphNodes[0].iVertex = 0xFFFF;
    sequence =
        heap_Alloc(pEngine->hHeap, (pEngine->MaxNbrWordsInISOWordSentence + 3) * sizeof(s32));
    if (sequence == NULL) {
        goto failure;
    }
    sequence[0] = firstWord;
    itemCursor = treeWords;
    treeItemIndex = 0;
    for (; treeItemIndex < treeWordCount; itemCursor++, treeItemIndex++) {
        u32 currentNode = 0;
        s32 inputWordID = itemCursor->WordID;
        u32 dictionaryIndex = 0;
        s32 wordID = 0;
        u32 step;
        u32 link;
        s32 *prefixCursor;
        u32 endNodeIndex;

        sequenceLength = 1;

        /* Copies a negative item's linked word IDs in reverse chain order between the first and
         * final words. */
        if (inputWordID < 0) {
            u32 sequenceIndex;
            struct _ISOWORD_BACKTRACE *wordSequences;
            u32 cursor;
            u32 length;
            cursor = sequenceIndex = -1 - inputWordID;
            wordSequences = pEngine->pBacktraceArray;
            while (cursor != -1) {
                ++sequenceLength;
                cursor = wordSequences[cursor].u32PrevWordIdx;
            }
            length = 0;
            while (sequenceIndex != -1) {
                struct _ISOWORD_BACKTRACE *sequenceWord = wordSequences + sequenceIndex;
                ++length;
                sequence[sequenceLength - length] = sequenceWord->s32WordID;
                sequenceIndex = sequenceWord->u32PrevWordIdx;
            }
        } else {
            sequence[sequenceLength++] = inputWordID;
        }
        prefixCursor = sequence;
        step = 0;
        sequence[sequenceLength++] = finalWord;

        /* Reuses the longest prefix already present in the pronunciation graph. */
        while (step < sequenceLength) {
            struct _NODE *node = graphNodes + currentNode;
            wordID = *prefixCursor;
            dictionaryIndex = wordOffsets[wordID] - pronunciationOffsetBase;
            link = node->iVertex;
            for (;;) {
                struct _VERTEX *edge;
                if (link == 0xFFFF) {
                    u32 count;
                    if (currentNode != 0) {
                        u32 *pronunciationBounds = wordOffsets + node->OldWordID;
                        count = pronunciationBounds[1] - pronunciationBounds[0];
                    } else {
                        count = 1;
                    }
                    previousPronunciationCount = count;
                    break;
                }
                edge = edges + link;
                if (dictionaryIndex == graphNodes[edge->iNode].OldPronID) {
                    currentNode = edge->iNode;
                    ++prefixCursor;
                    ++step;
                    break;
                }
                link = edge->iVertex;
            }
            if (link == 0xFFFF) {
                break;
            }
        }
        link = graphNodes[currentNode].iVertex;
        /* Adds every remaining pronunciation, connecting it to each preceding pronunciation. */
        if (step < sequenceLength) {
            sequenceCursor = sequence + step;
            for (;;) {
                u32 count = wordOffsets[wordID + 1] - wordOffsets[wordID];
                u32 pronunciation;
                for (pronunciation = 0; pronunciation < count; pronunciation++) {
                    u32 destination = nextNode + pronunciation;
                    u32 predecessor;

                    for (predecessor = 0; predecessor < previousPronunciationCount;
                         predecessor++) {
                        edges[nextEdge + predecessor].iNode = destination;
                        edges[nextEdge + predecessor].iVertex =
                            link == 0xFFFF ? 0xFFFF : link + predecessor;
                    }
                    link = nextEdge;
                    if (pronunciation == count - 1) {
                        for (predecessor = 0; predecessor < previousPronunciationCount;
                             predecessor++) {
                            graphNodes[currentNode + predecessor].iVertex =
                                nextEdge + predecessor;
                        }
                    }
                    graphNodes[destination].OldPronID = dictionaryIndex + pronunciation;
                    graphNodes[destination].OldWordID = wordID;
                    nextEdge = (u16)(nextEdge + previousPronunciationCount);
                }
                currentNode = nextNode;
                nextNode = (u16)(nextNode + count);
                previousPronunciationCount = count;
                ++step;
                ++sequenceCursor;
                if (step == sequenceLength) {
                    break;
                }
                wordID = *sequenceCursor;
                dictionaryIndex = wordOffsets[wordID] - pronunciationOffsetBase;
                link = 0xFFFF;
            }
        }
        for (endNodeIndex = 0; endNodeIndex < previousPronunciationCount; endNodeIndex++) {
            graphNodes[currentNode++].iVertex = 0xFFFF;
        }
    }
    heap_Free(pEngine->hHeap, sequence);
    /* Counts root edges separately because transitionCount excludes them from packed
     * transitions. */
    iVertex = graphNodes[0].iVertex;
    while (iVertex != 0xFFFF) {
        iVertex = edges[iVertex].iVertex;
        ++rootEdgeCount;
    }
    wordCount = nextNode - 1;
    originalWordCount = pronunciationCount = wordCount;
    nextTranslationWordIndex = wordCount;
    transitionCount = nextEdge - rootEdgeCount;
    /* Resolves each edge from the trailing and leading classes. Missing joins traverse directly. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        NodeWordInfo destinationWordInfo;
        u32 link;
        if ((link = nodeCursor->iVertex) != 0) {
            if (GetSecondPassInfo(pEngine, &dictionaryContext, &wordInfo,
                                  nodeCursor->OldPronID) != 0) {
                goto failure;
            }
            /* Retains the source token count and trailing class
             * while querying each destination pronunciation. */
            sourceWordInfo.tokenCount = wordInfo.Size;
            sourceWordInfo.trailingClass = wordInfo.RightSpeechUnitClass;
            tokenCount += sourceWordInfo.tokenCount;
            while (link != 0xFFFF) {
                struct _VERTEX *edge = edges + link;
                u16 translationID;
                u16 first;
                u16 last;
                int count;
                if (GetSecondPassInfo(pEngine, &dictionaryContext, &wordInfo,
                                      graphNodes[edge->iNode].OldPronID) != 0) {
                    goto failure;
                }
                destinationWordInfo.leadingClass = wordInfo.LeftSpeechUnitClass;
                translationID = destinationWordInfo.leadingClass +
                    sourceWordInfo.trailingClass * speechUnitClassCount;
                first = translationOffsets[translationID];
                last = translationOffsets[translationID + 1];
                count = translationOffsets[last] - translationOffsets[first];
                if (count != 0) {
                    tokenCount += count;
                    pronunciationCount += last - first;
                    wordCount++;
                    transitionCount++;
                } else {
                    translationID = 0xFFFF;
                }
                edge->TransWordIndex = translationID;
                link = edge->iVertex;
            }
        }
    }
    /* The packed-array allocations have no individual null checks. CreateMenuMasks reports setup
     * failure. */
    secondPass->pSyntax = heap_Alloc(pEngine->hHeap,
        (wordCount + 1 + transitionCount) * sizeof(u32));
    transitionBoundaries = secondPass->pSyntax;
    packedTransitions = transitionBoundaries + wordCount + 1;
    secondPass->pWordProps = heap_Alloc(pEngine->hHeap, wordCount * sizeof(u16));
    wordFlagCursor = secondPass->pWordProps;
    memset(wordFlagCursor, 0, wordCount * sizeof(u16));
    secondPass->pWordPenalties = heap_Alloc(pEngine->hHeap, wordCount * sizeof(s16));
    penaltyCursor = secondPass->pWordPenalties;
    secondPass->pBeginOfWords = heap_Alloc(pEngine->hHeap,
        (wordCount + pronunciationCount + 2) * sizeof(u32));
    allWordOffsets = secondPass->pBeginOfWords;
    wordBoundaries = allWordOffsets;
    pronunciationOffsets = allWordOffsets + wordCount + 1;
    tokenCount += pronunciationCount * 2;
    secondPass->pStateInDict = heap_Alloc(pEngine->hHeap, tokenCount * sizeof(u16));
    encodedDictionary = (u16 *)secondPass->pStateInDict;
    pEngine->CtxInfo.pNewWordID2OldWordID =
        heap_Alloc(pEngine->hHeap, wordCount * sizeof(struct _NEWID2OLDID));
    wordMap = pEngine->CtxInfo.pNewWordID2OldWordID;
    encodedOffset = 0;
    if (CreateMenuMasks(pEngine, secondPass, wordCount) != 0) {
        goto failure;
    }
    /* Sets the root-reachable flag on each graph destination. */
    iVertex = graphNodes[0].iVertex;
    while (iVertex != 0xFFFF) {
        struct _VERTEX *edge = edges + iVertex;
        wordFlagCursor[edge->iNode - 1] = 1;
        iVertex = edge->iVertex;
    }
    /* Packs the context words, followed by the joining translation pronunciations. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        u32 wordProperty = wordProperties[nodeCursor->OldWordID];
        u32 link = nodeCursor->iVertex;
        wordMap->OldWordID = nodeCursor->OldWordID;
        wordMap->OldPronID = nodeCursor->OldPronID;
        wordMap++;
        *transitionBoundaries++ = packedTransitions - secondPass->pSyntax;
        *pronunciationOffsets = encodedOffset;
        *wordBoundaries++ = pronunciationOffsets++ - allWordOffsets;
        putWordInDict(pEngine, &dictionaryContext, nodeCursor->OldPronID,
                      &encodedOffset, &encodedDictionary, nodeIndex - 1);
        /* The word penalty applies when the root, terminal, or context flags include bit 7. */
        if (link != 0) {
            if (link == 0xFFFF) {
                *wordFlagCursor |= 2;
            } else if (*wordFlagCursor != 1) {
                *wordFlagCursor = wordProperty;
            }
            if (*wordFlagCursor++ & 0x80) {
                *penaltyCursor++ = pEngine->WordPenalty;
            } else {
                *penaltyCursor++ = 0;
            }
            while (link != 0xFFFF) {
                struct _VERTEX *edge = edges + link;
                if (edge->TransWordIndex == 0xFFFF) {
                    *packedTransitions++ = edge->iNode - 1;
                } else {
                    *packedTransitions++ = nextTranslationWordIndex++;
                }
                link = edge->iVertex;
            }
        }
    }
    nextTranslationWordIndex = originalWordCount;
    *transitionBoundaries++ = packedTransitions - secondPass->pSyntax;
    *wordBoundaries++ = pronunciationOffsets - allWordOffsets;
    /* Appends each available joining translation and links it to its destination graph word. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        u32 link = nodeCursor->iVertex;
        if (link != 0) {
            while (link != 0xFFFF) {
                struct _VERTEX *edge = edges + link;
                u16 translationID = edge->TransWordIndex;
                if (translationID != 0xFFFF) {
                    wordMap->OldWordID = -1;
                    wordMap->OldPronID = -1;
                    wordMap++;
                    putTransWordInDict(&pronunciationOffsets, &encodedOffset,
                        &encodedDictionary, nextTranslationWordIndex++,
                        translationOffsets + translationOffsets[translationID],
                        translationOffsets + translationOffsets[translationID + 1],
                        translationTokens);
                    *wordBoundaries++ = pronunciationOffsets - allWordOffsets;
                    *packedTransitions++ = edge->iNode - 1;
                    *transitionBoundaries++ = packedTransitions - secondPass->pSyntax;
                    *penaltyCursor++ = 0;
                }
                link = edge->iVertex;
            }
        }
    }
    *pronunciationOffsets++ = encodedOffset;
    heap_Free(pEngine->hHeap, graphNodes);
    heap_Free(pEngine->hHeap, edges);
    secondPass->iOptionalSilence = 0;
    secondPass->uNbrOfWords = wordCount;
    secondPass->uNrStatesInDict = tokenCount;
    return 0;

/* Releases temporary graph storage and partially built second-pass state after a failure. */
failure:
    if (graphNodes != NULL) {
        heap_Free(pEngine->hHeap, graphNodes);
    }
    if (edges != NULL) {
        heap_Free(pEngine->hHeap, edges);
    }
    FreeSecondPassContext(pEngine);
    return 1;
}
