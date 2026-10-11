/* Builds the word dictionary and pronunciation graph for speech recognition's tree search. */
#include "types.h"
#include "gssdk/langdata.h"
#include <string.h>

/* 0xffff marks an absent graph link or unavailable joining translation. */
enum { NO_GRAPH_INDEX = 65535 };

typedef struct TreeWord {
    s32 wordID;
    /* Index of the next sequence word in wordSequences; -1 terminates the chain. */
    u32 next;
} TreeWord;

typedef struct TreeItem {
    /* Context word ID, or -1 - sequence index for a linked word sequence. */
    s32 wordID;
    u32 reservedItemData;
} TreeItem;

typedef struct WordGraphNode {
    s32 wordID;
    /* Pronunciation index relative to the context dictionary's base offset. */
    u32 dictionaryIndex;
    /* Head transition index for this node; NO_GRAPH_INDEX marks no outgoing edge. */
    u16 firstTransition;
    u16 reservedNodeData;
} WordGraphNode;

/* Destination node, next edge index (NO_GRAPH_INDEX terminates the list), and joining
 * translation ID (NO_GRAPH_INDEX means direct traversal). */
typedef struct WordGraphTransition {
    u16 destination;
    u16 next;
    u16 translationID;
} WordGraphTransition;

/* Assembled second-pass state and packed arrays; wordCount includes joining words,
 * and tokenCount includes two index words stored per pronunciation. */
typedef struct SecondPassContext {
    s32 state;
    u32 wordCount;
    u32 tokenCount;
    u16 *wordFlags;
    u16 *dictionary;
    s16 *wordPenalties;
    u32 *transitions;
    u32 *wordOffsets;
    void *menuMasks;
} SecondPassContext;

/* Pronunciation metadata: dictionary offset, encoded token count,
 * and leading/trailing speech-unit classes. */
typedef struct DictionaryWord {
    u32 transcriptionOffset;
    u16 tokenCount;
    u8 leadingClass;
    u8 trailingClass;
} DictionaryWord;

/* Pronunciation metadata retained while resolving a graph node's joining word. */
typedef struct NodeWordInfo {
    u16 tokenCount;
    u8 leadingClass;
    u8 trailingClass;
} NodeWordInfo;

typedef struct RecognitionContext {
    u8 reservedHeader[564];
    /* Passed to the speech allocator, which currently uses heap 0 and ignores this handle. */
    void *heap;
    u32 reservedState;
    /* Points to the V2 context data pointer used to fill the local dictionaryContext table. */
    void *dictionarySource;
    u32 reservedPassSetup;
    void *firstPassData;
    u32 reservedDictionaryState;
    DictionaryWord *dictionaryWords;
    u32 reservedSearchState;
    SecondPassContext *secondPass;
    /* Maps source word and pronunciation IDs; joining translation entries store -1,-1. */
    s32 *cachedData;
    u32 reservedPassState[2];
    LanguageData *language;
    u8 reservedMenuState[64];
    /* Input tree items; negative word IDs refer to linked entries in wordSequences. */
    TreeItem **treeWords;
    u32 treeWordCount;
    u8 reservedRecognitionState[384];
    /* Penalty copied to entries whose word flag has bit 7 set. */
    s16 wordPenalty;
    u16 reservedTreeFlags;
    u32 reservedTreeState;
    /* Linked sequence words indexed by the negative TreeItem wordID encoding. */
    TreeWord *wordSequences;
    u32 reservedSequenceState;
    u32 maxSequenceLength;
} RecognitionContext;

extern void *heap_Alloc(void *heap, u32 size);
extern void heap_Free(void *heap, void *memory);
extern void FreeSecondPassContext(RecognitionContext *context);
extern u8 CreateMenuMasks(RecognitionContext *context, SecondPassContext *secondPass, u32 count);

typedef struct ContextV2 ContextV2;
typedef void (*ContextMethod)(void);

struct ContextV2 {
    void *data;
    ContextMethod checkDataType;
    ContextMethod getSize;
    ContextMethod getLanguageVersionInfo;
    ContextMethod getTS;
    ContextMethod getAccuracy;
    ContextMethod getRejection;
    ContextMethod getGarbage;
    ContextMethod getNbrItem;
    u32 (*getNbrWord)(ContextV2 *context);
    ContextMethod getNbrPron;
    ContextMethod getNbrFollower;
    ContextMethod getNbrStartWord;
    ContextMethod getNbrStopWord;
    ContextMethod getGarbageWordID;
    ContextMethod isSpellingContext;
    u16 *(*getLex)(ContextV2 *context);
    u32 *(*getWordOffsets)(ContextV2 *context);
    u16 *(*getWordProperties)(ContextV2 *context);
    ContextMethod getSyntax;
    ContextMethod isIsoWordContext;
};

/* Read token counts and joining classes for graph and edge construction.
 * Type 1 expands speech units, skipping the first and last units when there are more than two;
 * other types use direct tokens and the user-word class. Always returns 0. */
u8 GetSecondPassInfo(RecognitionContext *context, ContextV2 *dictionaryContext,
                      DictionaryWord *wordInfo, u32 pronunciationIndex)
{
    u32 *speechUnitOffsets = ((u32 *(*)(LanguageData *))
        context->language->reservedA8[0])(context->language);
    u8 defaultClass = context->language->getUserWordSpeechUnitClass(context->language);
    u16 *dictionary = dictionaryContext->getLex(dictionaryContext);
    u32 *wordOffsets = dictionaryContext->getWordOffsets(dictionaryContext);
    unsigned int wordType;
    u16 *pronunciation;
    u16 *end;
    int length;

    wordOffsets += *wordOffsets;
    pronunciation = dictionary + wordOffsets[pronunciationIndex];
    wordType = *pronunciation++;
    end = dictionary + wordOffsets[pronunciationIndex + 1];
    length = end - pronunciation;
    if (wordType == 1) {
        u32 tokenCount = 0;

        wordInfo->leadingClass = ((u16 *)speechUnitOffsets)[speechUnitOffsets[*pronunciation]];
        --end;
        wordInfo->trailingClass = ((u16 *)speechUnitOffsets)[speechUnitOffsets[*end] + 1];
        if (length > 2) {
            ++pronunciation;
            --end;
        }
        while (pronunciation <= end) {
            u16 speechUnit = *pronunciation;
            tokenCount += speechUnitOffsets[speechUnit + 1] - speechUnitOffsets[speechUnit] - 2;
            ++pronunciation;
        }
        wordInfo->tokenCount = tokenCount;
    } else {
        wordInfo->trailingClass = defaultClass;
        wordInfo->leadingClass = defaultClass;
        wordInfo->tokenCount = length;
    }
    return 0;
}

/* Append joining-translation pronunciations with token IDs shifted left two bits.
 * Nonfinal tokens set bit 1; the final token sets bit 0 instead. Record dictionary offsets
 * and append the low and high 16-bit halves of the second-pass word index. */
static void putTransWordInDict(u32 **wordOffsetList, u32 *wordOffset,
                        u16 **encodedWordList, u32 secondPassWordIndex,
                        u16 *translationWordIndices, u16 *wordLimit, u16 *dictionary)
{
    u16 wordIndexLow = secondPassWordIndex;
    u32 *offsetCursor = *wordOffsetList;
    u16 *encodedCursor = *encodedWordList;
    u16 *indexCursor = translationWordIndices;
    u16 *dictionaryCursor = dictionary + *indexCursor;
    u16 *nextWord;

    secondPassWordIndex >>= 16;
    while (indexCursor < wordLimit) {
        *offsetCursor++ = *wordOffset;
        nextWord = dictionary + *++indexCursor;
        *wordOffset += nextWord - dictionaryCursor + 2;
        while (dictionaryCursor < nextWord - 1) {
            *encodedCursor++ = (*dictionaryCursor++ * 4) | 2;
        }
        encodedCursor[0] = (*dictionaryCursor++ * 4) | 1;
        encodedCursor[1] = wordIndexLow;
        encodedCursor[2] = secondPassWordIndex;
        encodedCursor += 3;
    }

    *encodedWordList = encodedCursor;
    *wordOffsetList = offsetCursor;
}

/* Pack a context pronunciation as token IDs shifted left two bits. Type 1 expands speech
 * units, skipping the first and last units when there are more than two. Set bit 0 on the
 * final token and append the low and high 16-bit halves of its second-pass word index. */
static void putWordInDict(RecognitionContext *context, ContextV2 *dictionaryContext,
                   u32 pronunciationIndex, u32 *encodedOffset,
                   u16 **encodedDictionary, u32 wordIndex)
{
    u32 *speechUnitOffsets = ((u32 *(*)(LanguageData *))
        context->language->reservedA8[0])(context->language);
    u16 *dictionary = dictionaryContext->getLex(dictionaryContext);
    u32 *wordOffsets = dictionaryContext->getWordOffsets(dictionaryContext);
    u32 offsetBase = *wordOffsets;
    u32 *pronunciationOffsets = wordOffsets + offsetBase;
    u16 *pronunciation = dictionary + pronunciationOffsets[pronunciationIndex];
    unsigned int wordType = *pronunciation++;
    u16 *end = dictionary + pronunciationOffsets[pronunciationIndex + 1];
    unsigned int length = end - pronunciation;
    u16 *output = *encodedDictionary;
    u32 tokensWritten = 0;

    if (wordType == 1) {
        --end;
        if (length > 2U) {
            ++pronunciation;
            --end;
        }
        while (pronunciation <= end) {
            u32 *bounds = speechUnitOffsets + *pronunciation;
            u32 tokenCount = bounds[1] - bounds[0] - 2;
            u32 tokenIndex = 0;
            u16 *tokens = (u16 *)speechUnitOffsets + bounds[0] + 2;

            for (; tokenIndex < tokenCount; tokenIndex++) {
                *output++ = (tokens[tokenIndex] * 4) & 0xFFFC;
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
    output[0] = wordIndex;
    output[1] = wordIndex >> 16;
    *encodedOffset += tokensWritten + 2;
    *encodedDictionary = output + 2;
}

extern void FillContextV2VirtualTable(ContextV2 *context);

/* Build the shared-prefix pronunciation graph and packed second-pass context
 * for configured tree words. */
u8 CreateSecondPassTreeContext(RecognitionContext *context)
{
    u16 *translationOffsets;
    u32 *wordOffsets;
    u32 speechUnitClassCount;
    s32 *sequenceCursor;
    TreeItem *itemCursor;
    u32 treeItemIndex;
    u32 rootEdgeCount;
    u32 tokenCount;
    u32 treeWordCount;
    u32 pronunciationOffsetBase;
    u16 *translationTokens;
    u16 *wordProperties;
    s32 finalWord;
    SecondPassContext *secondPass;
    unsigned int sequenceLength;
    ContextV2 dictionaryContext;
    NodeWordInfo sourceWordInfo;
    u32 *pronunciationOffsets;
    u32 encodedOffset;
    u16 *encodedDictionary;
    DictionaryWord wordInfo;
    WordGraphNode *graphNodes;
    WordGraphTransition *edges;
    TreeItem *treeWords;
    TreeItem *treeWordCursor;
    u32 maximumPronunciations;
    u32 graphCapacity;
    u32 nextNode;
    u32 nextEdge;
    void *translationDictionary;
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
    s32 *wordMap;
    WordGraphNode *nodeCursor;
    u32 nodeIndex;

    maximumPronunciations = 0;
    graphCapacity = 0;
    graphNodes = NULL;
    edges = NULL;
    nextEdge = 0;
    rootEdgeCount = 0;
    nextNode = 1;
    tokenCount = 0;
    speechUnitClassCount = context->language->getNbrSpeechUnitClass(context->language);
    treeWords = *context->treeWords;
    treeWordCursor = *context->treeWords;
    treeWordCount = context->treeWordCount;
    translationDictionary = ((void *(*)(LanguageData *))
        context->language->reservedA8[1])(context->language);
    translationTokens = ((u16 *(*)(void *))
        context->language->reservedF4[3])(translationDictionary);
    translationOffsets = ((u16 *(*)(void *))
        context->language->reservedF4[4])(translationDictionary);
    encodedOffset = 0;
    firstWord = -1;
    finalWord = -1;
    /* Discard existing second-pass state and cached mappings before allocating a replacement. */
    if (context->secondPass != NULL) {
        FreeSecondPassContext(context);
    }
    secondPass = heap_Alloc(context->heap, sizeof(SecondPassContext));
    if (secondPass == NULL) {
        goto failure;
    }
    memset(secondPass, 0, sizeof(SecondPassContext));
    context->secondPass = secondPass;
    FillContextV2VirtualTable(&dictionaryContext);
    dictionaryContext.data = *(void **)context->dictionarySource;
    contextWordCount = dictionaryContext.getNbrWord(&dictionaryContext);
    wordProperties = dictionaryContext.getWordProperties(&dictionaryContext);
    wordOffsets = dictionaryContext.getWordOffsets(&dictionaryContext);
    pronunciationOffsetBase = *wordOffsets;

    /* Find the context word IDs marked as first and final by their property bits. */
    {
        u32 propertyIndex = 0;
        u16 *propertyCursor = wordProperties;
        for (; propertyIndex < contextWordCount; propertyCursor++, propertyIndex++) {
            if (*propertyCursor & 1) {
                firstWord = propertyIndex;
                if (finalWord != -1) {
                    break;
                }
            }
            if (*propertyCursor & 2) {
                finalWord = propertyIndex;
                if (firstWord != -1) {
                    break;
                }
            }
        }
    }
    /* Bound the temporary graph from each sequence's pronunciations
     * and its inserted first/final words. */
    for (itemIndex = 0; itemIndex < treeWordCount; itemIndex++, treeWordCursor++) {
        s32 wordID = treeWordCursor->wordID;
        if (wordID < 0) {
            u32 sequenceIndex = -1 - wordID;
            TreeWord *wordSequences = context->wordSequences;
            while (sequenceIndex != -1) {
                TreeWord *sequenceWord = wordSequences + sequenceIndex;
                u32 *pronunciationBounds = wordOffsets + sequenceWord->wordID;
                u32 count = pronunciationBounds[1] - pronunciationBounds[0];
                if (count > maximumPronunciations) {
                    maximumPronunciations = count;
                }
                graphCapacity += count;
                sequenceIndex = sequenceWord->next;
            }
            graphCapacity += 2;
        } else {
            u32 *pronunciationBounds = wordOffsets + wordID;
            u32 count = pronunciationBounds[1] - pronunciationBounds[0];
            if (count > maximumPronunciations) {
                maximumPronunciations = count;
            }
            graphCapacity = count + graphCapacity;
            graphCapacity += 2;
        }
    }
    graphNodes = heap_Alloc(context->heap, (graphCapacity + 1) * sizeof(WordGraphNode));
    edges = heap_Alloc(context->heap,
        graphCapacity * maximumPronunciations * sizeof(WordGraphTransition));
    if (graphNodes == NULL || edges == NULL) {
        goto failure;
    }
    graphNodes[0].firstTransition = NO_GRAPH_INDEX;
    sequence = heap_Alloc(context->heap, (context->maxSequenceLength + 3) * sizeof(s32));
    if (sequence == NULL) {
        goto failure;
    }
    sequence[0] = firstWord;
    itemCursor = treeWords;
    treeItemIndex = 0;
    for (; treeItemIndex < treeWordCount; itemCursor++, treeItemIndex++) {
        u32 currentNode = 0;
        s32 inputWordID = itemCursor->wordID;
        u32 dictionaryIndex = 0;
        s32 wordID = 0;
        u32 step;
        u32 link;
        s32 *prefixCursor;

        sequenceLength = 1;

        /* Copy a negative item's linked word IDs in reverse chain order between the first/final
         * words. */
        if (inputWordID < 0) {
            u32 sequenceIndex;
            TreeWord *wordSequences;
            u32 cursor;
            u32 length;
            cursor = sequenceIndex = -1 - inputWordID;
            wordSequences = context->wordSequences;
            while (cursor != -1) {
                ++sequenceLength;
                cursor = wordSequences[cursor].next;
            }
            length = 0;
            while (sequenceIndex != -1) {
                TreeWord *sequenceWord = wordSequences + sequenceIndex;
                ++length;
                sequence[sequenceLength - length] = sequenceWord->wordID;
                sequenceIndex = sequenceWord->next;
            }
        } else {
            sequence[sequenceLength++] = inputWordID;
        }
        prefixCursor = sequence;
        step = 0;
        sequence[sequenceLength++] = finalWord;

        /* Reuse the longest prefix already present in the pronunciation graph. */
        while (step < sequenceLength) {
            WordGraphNode *node = graphNodes + currentNode;
            wordID = *prefixCursor;
            dictionaryIndex = wordOffsets[wordID] - pronunciationOffsetBase;
            link = node->firstTransition;
            for (;;) {
                WordGraphTransition *edge;
                if (link == NO_GRAPH_INDEX) {
                    u32 count;
                    if (currentNode != 0) {
                        u32 *pronunciationBounds = wordOffsets + node->wordID;
                        count = pronunciationBounds[1] - pronunciationBounds[0];
                    } else {
                        count = 1;
                    }
                    previousPronunciationCount = count;
                    break;
                }
                edge = edges + link;
                if (dictionaryIndex == graphNodes[edge->destination].dictionaryIndex) {
                    currentNode = edge->destination;
                    ++prefixCursor;
                    ++step;
                    break;
                }
                link = edge->next;
            }
            if (link == NO_GRAPH_INDEX) {
                break;
            }
        }
        link = graphNodes[currentNode].firstTransition;
        /* Add every remaining pronunciation, connecting it to each preceding pronunciation. */
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
                        edges[nextEdge + predecessor].destination = (u16)destination;
                        edges[nextEdge + predecessor].next =
                            link == NO_GRAPH_INDEX ? NO_GRAPH_INDEX : link + predecessor;
                    }
                    link = nextEdge;
                    if (pronunciation == count - 1) {
                        for (predecessor = 0; predecessor < previousPronunciationCount;
                             predecessor++) {
                            graphNodes[currentNode + predecessor].firstTransition =
                                nextEdge + predecessor;
                        }
                    }
                    graphNodes[destination].dictionaryIndex = dictionaryIndex + pronunciation;
                    graphNodes[destination].wordID = wordID;
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
                link = NO_GRAPH_INDEX;
            }
        }
        {
            u32 endNodeIndex;
            for (endNodeIndex = 0; endNodeIndex < previousPronunciationCount; endNodeIndex++) {
                graphNodes[currentNode++].firstTransition = NO_GRAPH_INDEX;
            }
        }
    }
    heap_Free(context->heap, sequence);
    /* Count root edges separately because transitionCount excludes them from packed transitions. */
    {
        u32 link = graphNodes[0].firstTransition;
        while (link != NO_GRAPH_INDEX) {
            link = edges[link].next;
            ++rootEdgeCount;
        }
    }
    wordCount = nextNode - 1;
    originalWordCount = pronunciationCount = wordCount;
    nextTranslationWordIndex = wordCount;
    transitionCount = nextEdge - rootEdgeCount;
    /* Resolve each edge from source trailing and destination leading classes;
     * mark missing joins for direct traversal. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        NodeWordInfo destinationWordInfo;
        u32 link;
        if ((link = nodeCursor->firstTransition) != 0) {
            if (GetSecondPassInfo(context, &dictionaryContext, &wordInfo,
                                  nodeCursor->dictionaryIndex) != 0) {
                goto failure;
            }
            /* Retain the source token count and trailing class
             * while querying each destination pronunciation. */
            sourceWordInfo.tokenCount = wordInfo.tokenCount;
            sourceWordInfo.trailingClass = wordInfo.trailingClass;
            tokenCount += sourceWordInfo.tokenCount;
            while (link != NO_GRAPH_INDEX) {
                WordGraphTransition *edge = edges + link;
                u16 translationID;
                u16 first;
                u16 last;
                int count;
                if (GetSecondPassInfo(context, &dictionaryContext, &wordInfo,
                                      graphNodes[edge->destination].dictionaryIndex) != 0) {
                    goto failure;
                }
                destinationWordInfo.leadingClass = wordInfo.leadingClass;
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
                    translationID = NO_GRAPH_INDEX;
                }
                edge->translationID = translationID;
                link = edge->next;
            }
        }
    }
    /* Use these packed-array allocations without individual NULL checks; only the subsequent
     * CreateMenuMasks call reports a setup failure. */
    secondPass->transitions = heap_Alloc(context->heap,
        (wordCount + 1 + transitionCount) * sizeof(u32));
    transitionBoundaries = secondPass->transitions;
    packedTransitions = transitionBoundaries + wordCount + 1;
    secondPass->wordFlags = heap_Alloc(context->heap, wordCount * sizeof(u16));
    wordFlagCursor = secondPass->wordFlags;
    memset(wordFlagCursor, 0, wordCount * sizeof(u16));
    secondPass->wordPenalties = heap_Alloc(context->heap, wordCount * sizeof(s16));
    penaltyCursor = secondPass->wordPenalties;
    {
        u32 pronunciationEntries = pronunciationCount + 2;
        secondPass->wordOffsets = heap_Alloc(context->heap,
            (wordCount + pronunciationEntries) * sizeof(u32));
    }
    allWordOffsets = secondPass->wordOffsets;
    wordBoundaries = allWordOffsets;
    pronunciationOffsets = allWordOffsets + wordCount + 1;
    tokenCount += pronunciationCount * 2;
    secondPass->dictionary = heap_Alloc(context->heap, tokenCount * sizeof(u16));
    encodedDictionary = secondPass->dictionary;
    context->cachedData = heap_Alloc(context->heap, wordCount * 2 * sizeof(s32));
    wordMap = context->cachedData;
    encodedOffset = 0;
    if (CreateMenuMasks(context, secondPass, wordCount) != 0) {
        goto failure;
    }
    /* Set the root-reachable flag on each graph destination. */
    {
        u32 link = graphNodes[0].firstTransition;
        while (link != NO_GRAPH_INDEX) {
            WordGraphTransition *edge = edges + link;
            wordFlagCursor[edge->destination - 1] = 1;
            link = edge->next;
        }
    }
    /* Pack the context words, followed by the joining translation pronunciations. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        u32 wordProperty = wordProperties[nodeCursor->wordID];
        u32 link = nodeCursor->firstTransition;
        wordMap[0] = nodeCursor->wordID;
        wordMap[1] = nodeCursor->dictionaryIndex;
        wordMap += 2;
        *transitionBoundaries++ = packedTransitions - secondPass->transitions;
        *pronunciationOffsets = encodedOffset;
        *wordBoundaries++ = pronunciationOffsets++ - allWordOffsets;
        putWordInDict(context, &dictionaryContext, nodeCursor->dictionaryIndex,
                      &encodedOffset, &encodedDictionary, nodeIndex - 1);
        /* Preserve root/terminal status or word properties,
         * then apply wordPenalty when flag bit 7 is set. */
        if (link != 0) {
            if (link == NO_GRAPH_INDEX) {
                *wordFlagCursor |= 2;
            } else if (*wordFlagCursor != 1) {
                *wordFlagCursor = wordProperty;
            }
            if (*wordFlagCursor++ & (1 << 7)) {
                *penaltyCursor++ = context->wordPenalty;
            } else {
                *penaltyCursor++ = 0;
            }
            while (link != NO_GRAPH_INDEX) {
                WordGraphTransition *edge = edges + link;
                if (edge->translationID == NO_GRAPH_INDEX) {
                    *packedTransitions++ = edge->destination - 1;
                } else {
                    *packedTransitions++ = nextTranslationWordIndex++;
                }
                link = edge->next;
            }
        }
    }
    nextTranslationWordIndex = originalWordCount;
    *transitionBoundaries++ = packedTransitions - secondPass->transitions;
    *wordBoundaries++ = pronunciationOffsets - allWordOffsets;
    /* Append each available joining translation and link it to its destination graph word. */
    nodeCursor = graphNodes + 1;
    for (nodeIndex = 1; nodeIndex <= originalWordCount; nodeCursor++, nodeIndex++) {
        u32 link = nodeCursor->firstTransition;
        if (link != 0) {
            while (link != NO_GRAPH_INDEX) {
                WordGraphTransition *edge = edges + link;
                u16 translationID = edge->translationID;
                if (translationID != NO_GRAPH_INDEX) {
                    wordMap[0] = -1;
                    wordMap[1] = -1;
                    wordMap += 2;
                    putTransWordInDict(&pronunciationOffsets, &encodedOffset,
                        &encodedDictionary, nextTranslationWordIndex++,
                        translationOffsets + translationOffsets[translationID],
                        translationOffsets + translationOffsets[translationID + 1],
                        translationTokens);
                    *wordBoundaries++ = pronunciationOffsets - allWordOffsets;
                    *packedTransitions++ = edge->destination - 1;
                    *transitionBoundaries++ = packedTransitions - secondPass->transitions;
                    *penaltyCursor++ = 0;
                }
                link = edge->next;
            }
        }
    }
    *pronunciationOffsets++ = encodedOffset;
    heap_Free(context->heap, graphNodes);
    heap_Free(context->heap, edges);
    secondPass->state = 0;
    secondPass->wordCount = wordCount;
    secondPass->tokenCount = tokenCount;
    return 0;

/* Release temporary graph storage and partially built second-pass state after a failure. */
failure:
    if (graphNodes != NULL) {
        heap_Free(context->heap, graphNodes);
    }
    if (edges != NULL) {
        heap_Free(context->heap, edges);
    }
    FreeSecondPassContext(context);
    return 1;
}
