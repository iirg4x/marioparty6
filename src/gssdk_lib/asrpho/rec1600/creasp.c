/* Builds the word and transition dictionaries used by speech recognition's second pass. */
#include "types.h"
#include "gssdk/langdata.h"
#include <string.h>

enum { NO_GRAPH_INDEX = 65535 };

extern void heap_Free(void *heap, void *ptr);
extern void *heap_Alloc(void *heap, u32 size);

typedef struct RecognitionWordNode {
    /* Offset of this word's pronunciation data in the first-pass dictionary. */
    s32 transcriptionOffset;
    /* Additional menu-node data carried with the recognition entry. */
    u32 reservedNodeData[2];
    /* Child entries and sibling links in the enabled-menu word list. */
    struct RecognitionWordNode *children;
    struct RecognitionWordNode *next;
} RecognitionWordNode;

typedef struct DictionaryWord {
    /* Offset of the encoded pronunciation in the first-pass dictionary. */
    s32 transcriptionOffset;
    /* Number of encoded speech units in this pronunciation. */
    u16 tokenCount;
    /* Speech-unit classes used to select a joining translation. */
    u8 leadingClass;
    u8 trailingClass;
} DictionaryWord;

typedef struct FirstPassContext {
    /* First-pass state carried with the recognition context. */
    u32 reservedState[3];
    /* Per-word attributes and pronunciation data produced by the first pass. */
    u16 *wordAttributes;
    u16 *dictionary;
    /* Additional first-pass context storage with unknown role. */
    u32 reservedLimits[2];
    /* Pronunciation start offsets indexed by first-pass word ID. */
    u32 *wordOffsets;
} FirstPassContext;

typedef struct TranslationDictionary {
    /* Translation-word count and encoded lexicon size. */
    u32 wordCount;
    u32 lexiconSize;
    /* Encoded speech-unit lexicon followed by its offset tables. */
    u16 data[];
} TranslationDictionary;

typedef struct WordGraphNode {
    /* First-pass word ID and pronunciation index for this graph node. */
    s32 wordID;
    s32 dictionaryIndex;
    /* Head transition index; 65535 marks a node with no outgoing edge. */
    u16 firstTransition;
    /* Additional data carried with the graph node. */
    u16 reservedNodeData;
} WordGraphNode;

typedef struct WordGraphTransition {
    /* Destination graph node and next edge in the source node's list. */
    u16 destination;
    u16 next;
    /* Joining translation ID, or 65535 when no translation exists. */
    u16 translationID;
} WordGraphTransition;

typedef struct SecondPassContext {
    /* State and sizes of the assembled second-pass recognition data. */
    s32 state;
    u32 wordCount;
    s32 tokenCount;
    /* Per-word search flags, encoded pronunciations, and penalties. */
    u16 *wordFlags;
    u16 *dictionary;
    s16 *wordPenalties;
    /* Word transition offsets followed by the packed transition indices. */
    u32 *transitions;
    /* Boundaries of each word's pronunciation entries. */
    u32 *wordOffsets;
    /* Per-menu selection masks used during second-pass search. */
    void *menuMasks;
} SecondPassContext;

typedef struct RecognitionContext {
    /* Recognizer header storage whose fields are owned by the speech engine. */
    u8 reservedHeader[564];
    /* Accepted by the speech allocator, which uses the current speech group in heap 0 instead. */
    void *heap;
    /* Recognizer state carried with the recognition context. */
    u32 reservedState[3];
    /* First-pass words, attributes, and pronunciation offsets. */
    FirstPassContext *firstPassData;
    /* Recognizer dictionary state carried with the context. */
    u32 reservedDictionaryState;
    /* Metadata for each first-pass pronunciation. */
    DictionaryWord *dictionaryWords;
    /* Recognizer search state carried with the context. */
    u32 reservedSearchState;
    /* Search data assembled for the second recognition pass. */
    SecondPassContext *secondPass;
    /* Maps first-pass-backed second-pass words to their first-pass word and pronunciation IDs;
     * joining translations use -1. */
    s32 *cachedData;
    /* Recognizer pass state carried with the context. */
    u32 reservedPassState[2];
    /* Language-specific speech-unit classes and translation lexicon. */
    LanguageData *language;
    /* Menu selection state owned by the recognizer. */
    u8 reservedMenuState[64];
    /* Enabled menu roots and their linked word entries. */
    RecognitionWordNode **menus;
    /* Additional recognizer state carried with the context. */
    u8 reservedRecognitionState[388];
    /* Penalty copied to words marked for penalty handling. */
    s16 wordPenalty;
} RecognitionContext;

extern u8 CreateMenuMasks(RecognitionContext *, SecondPassContext *, u32);
void FreeSecondPassContext(RecognitionContext *context);

/*
* Adds each pronunciation of a joining translation to the second-pass dictionary.
* CreateSecondPassContext calls this while packing translation pronunciations and their transitions.
*/
static void putTransWordInDict(
    u32 **wordOffsetTable, u32 *encodedOffset, u16 **encodedDictionary,
    u32 secondPassWordIndex, u16 *translationWordIndices, u16 *translationWordLimit,
    u16 *translationTokenLexicon)
{
    u16 wordIndexLow = secondPassWordIndex;
    u32 *wordBoundaryCursor = *wordOffsetTable;
    u16 *dictionaryCursor = *encodedDictionary;
    u16 *translationIndexCursor = translationWordIndices;
    u16 *lexiconCursor = translationTokenLexicon + *translationIndexCursor;
    u16 *nextPronunciation;

    secondPassWordIndex >>= 16;

    while (translationIndexCursor < translationWordLimit) {
        *wordBoundaryCursor++ = *encodedOffset;
        nextPronunciation = translationTokenLexicon + *++translationIndexCursor;
        *encodedOffset += nextPronunciation - lexiconCursor + 2;
        while (lexiconCursor < nextPronunciation - 1) {
            *dictionaryCursor++ = (*lexiconCursor++ * 4) | 2;
        }
        dictionaryCursor[0] = (*lexiconCursor++ * 4) | 1;
        dictionaryCursor[1] = wordIndexLow;
        dictionaryCursor[2] = secondPassWordIndex;
        dictionaryCursor += 3;
    }

    *encodedDictionary = dictionaryCursor;
    *wordOffsetTable = wordBoundaryCursor;
}

/*
 * Builds the second-pass pronunciation graph and packed dictionary for the enabled menus.
 * Called by the recognizer when it prepares second-pass search data from first-pass results.
 */
s32 CreateSecondPassContext(RecognitionContext *context)
{
    int speechUnitClassCount;
    u16 *dictionary;
    RecognitionWordNode *menuRoot;
    u32 firstPassOffsetBase;
    u16 *wordAttributes;
    s32 rootTransitionCount;
    u32 initialGraphWordCount;
    s32 tokenCount;
    DictionaryWord *dictionaryWords;
    u16 *translationTokenLexicon;
    SecondPassContext *secondPass;
    u32 *pronunciationOffsetTable;
    u32 encodedWordOffset;
    u16 *encodedPronunciationDictionary;
    u32 maxPronunciationsPerWord;
    s32 graphPronunciationCount;
    WordGraphNode *graphNodes;
    WordGraphTransition *transitions;
    u32 nextGraphTransition;
    u32 nextGraphNode;
    FirstPassContext *firstPassData;
    u32 *wordOffsets;
    RecognitionWordNode *enabledMenuCursor;
    RecognitionWordNode *word;
    TranslationDictionary *translationDictionary;
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
    s32 *firstPassWordMapCursor;
    WordGraphNode *graphNodeCursor;
    u32 graphNodeIndex;
    u16 translationID;

    maxPronunciationsPerWord = 0;
    graphPronunciationCount = 0;
    graphNodes = NULL;
    transitions = NULL;
    nextGraphTransition = 0;
    nextGraphNode = 1;
    enabledMenuCursor = *context->menus;
    firstPassData = context->firstPassData;
    wordOffsets = firstPassData->wordOffsets;
    firstPassOffsetBase = *wordOffsets;
    wordAttributes = firstPassData->wordAttributes;
    dictionary = firstPassData->dictionary;
    dictionaryWords = context->dictionaryWords;
    rootTransitionCount = 0;
    tokenCount = 0;
    speechUnitClassCount = context->language->getNbrSpeechUnitClass(context->language);
    translationDictionary = ((TranslationDictionary *(*)(LanguageData *))
        context->language->reservedA8[1])(context->language);
    translationTokenLexicon = ((u16 *(*)(TranslationDictionary *))
        context->language->reservedF4[3])(translationDictionary);
    translationPronunciationOffsets = ((u16 *(*)(TranslationDictionary *))
        context->language->reservedF4[4])(translationDictionary);
    encodedWordOffset = 0;
    if (context->secondPass != NULL) {
        FreeSecondPassContext(context);
    }
    secondPass = heap_Alloc(context->heap, sizeof(SecondPassContext));
    if (secondPass == NULL) {
        goto failure;
    }
    memset(secondPass, 0, sizeof(SecondPassContext));
    context->secondPass = secondPass;

    /* Bound the temporary graph by the pronunciation counts of the menu words. */
    while (enabledMenuCursor != NULL) {
        word = enabledMenuCursor->children;
        while (word != NULL) {
            u16 *encodedWordEntry = dictionary + word->transcriptionOffset;
            u32 *pronunciationBounds =
                wordOffsets + (encodedWordEntry[1] + (encodedWordEntry[2] << 16));
            u32 count = pronunciationBounds[1] - pronunciationBounds[0];
            if (count > maxPronunciationsPerWord) {
                maxPronunciationsPerWord = count;
            }
            graphPronunciationCount += count;
            word = word->next;
        }
        enabledMenuCursor = enabledMenuCursor->next;
    }
    graphNodes = heap_Alloc(context->heap, (graphPronunciationCount + 1) * sizeof(WordGraphNode));
    transitions = heap_Alloc(context->heap,
        graphPronunciationCount * maxPronunciationsPerWord * sizeof(WordGraphTransition));
    if (graphNodes == NULL || transitions == NULL) {
        goto failure;
    }
    graphNodes[0].firstTransition = NO_GRAPH_INDEX;
    /*
     * Enabled menus with common pronunciation paths share graph nodes; each distinct pronunciation
     * extends the search path with its own node.
     */
    for (menuRoot = *context->menus; menuRoot != NULL; menuRoot = menuRoot->next) {
        u32 rootNodeIndex = 0;
        u32 dictionaryIndex = 0;
        s32 wordID = 0;
        word = menuRoot->children;
        if (word != NULL) {
            WordGraphTransition *transition;
            u32 searchLink;
            u32 link;
            while (word != NULL) {
                WordGraphNode *node = graphNodes + rootNodeIndex;
                u16 *encodedWordEntry = dictionary + word->transcriptionOffset;
                searchLink = node->firstTransition;
                wordID = encodedWordEntry[1] + (encodedWordEntry[2] << 16);
                dictionaryIndex = wordOffsets[wordID] - firstPassOffsetBase;
                for (;;) {
                    if (searchLink == NO_GRAPH_INDEX) {
                        u32 count;
                        if (rootNodeIndex != 0) {
                            u32 *pronunciationBounds = wordOffsets + node->wordID;
                            count = pronunciationBounds[1] - pronunciationBounds[0];
                        } else {
                            count = 1;
                        }
                        previousBranchCount = count;
                        break;
                    } else {
                        u16 destination;
                        transition = transitions + searchLink;
                        destination = transition->destination;
                        if (dictionaryIndex == (u32)graphNodes[destination].dictionaryIndex) {
                            word = word->next;
                            rootNodeIndex = destination;
                            break;
                        }
                        searchLink = transition->next;
                    }
                }
                if (searchLink == NO_GRAPH_INDEX) {
                    break;
                }
            }
            link = graphNodes[rootNodeIndex].firstTransition;
            if (word != NULL) {
              for (;;) {
                u32 count;
                u32 *pronunciationBounds = wordOffsets + wordID;
                u32 pronunciation;
                count = pronunciationBounds[1] - pronunciationBounds[0];
                for (pronunciation = 0; pronunciation < count; pronunciation++) {
                    u32 destination = nextGraphNode + pronunciation;
                    u32 previousNodeIndex;
                    previousNodeIndex = 0;
                    for (; previousNodeIndex < previousBranchCount; previousNodeIndex++) {
                        transitions[nextGraphTransition + previousNodeIndex].destination =
                            (u16) destination;
                        transitions[nextGraphTransition + previousNodeIndex].next =
                            link == NO_GRAPH_INDEX ? NO_GRAPH_INDEX : link + previousNodeIndex;
                    }
                    link = nextGraphTransition;
                    if (pronunciation == count - 1) {
                        u32 previousNodeIndex;
                        for (previousNodeIndex = 0; previousNodeIndex < previousBranchCount;
                             previousNodeIndex++) {
                            graphNodes[rootNodeIndex + previousNodeIndex].firstTransition =
                                nextGraphTransition + previousNodeIndex;
                        }
                    }
                    graphNodes[destination].dictionaryIndex = dictionaryIndex + pronunciation;
                    nextGraphTransition = (u16)(nextGraphTransition + previousBranchCount);
                    graphNodes[destination].wordID = wordID;
                }
                word = word->next;
                rootNodeIndex = nextGraphNode;
                previousBranchCount = count;
                nextGraphNode = (u16)(nextGraphNode + count);
                if (word == NULL) {
                    break;
                } else {
                    u16 *encodedWordEntry;
                    link = NO_GRAPH_INDEX;
                    encodedWordEntry = dictionary + word->transcriptionOffset;
                    wordID = encodedWordEntry[1] + (encodedWordEntry[2] << 16);
                    dictionaryIndex = wordOffsets[wordID] - firstPassOffsetBase;
                }
              }
            }
            {
                u32 previousNodeIndex;
                for (previousNodeIndex = 0; previousNodeIndex < previousBranchCount;
                     previousNodeIndex++) {
                    graphNodes[rootNodeIndex++].firstTransition = NO_GRAPH_INDEX;
                }
            }
        }
    }

    {
        u32 link = graphNodes[0].firstTransition;
        while (link != NO_GRAPH_INDEX) {
            link = transitions[link].next;
            rootTransitionCount++;
        }
    }
    wordCount = nextGraphNode - 1;
    initialGraphWordCount = totalPronunciationCount = wordCount;
    nextTranslationWordIndex = wordCount;
    transitionCount = nextGraphTransition - rootTransitionCount;
    graphNodeCursor = graphNodes + 1;
    /*
    * Resolve each edge's joining word from the source trailing class and destination leading class.
    * Edges without an available translation are marked for direct traversal.
    */
    for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
         graphNodeCursor++, graphNodeIndex++) {
        u32 link = graphNodeCursor->firstTransition;
        if (link != 0) {
            DictionaryWord *word = dictionaryWords + graphNodeCursor->dictionaryIndex;
            u16 trailingClass = word->trailingClass;
            u16 wordTokenCount = word->tokenCount;
            tokenCount += wordTokenCount;
            while (link != NO_GRAPH_INDEX) {
                u16 destination = transitions[link].destination;
                s32 destinationWord = graphNodes[destination].dictionaryIndex;
                u16 leadingClass = dictionaryWords[destinationWord].leadingClass;
                u16 first;
                u16 last;
                int count;
                translationID = leadingClass + trailingClass * speechUnitClassCount;
                first = translationPronunciationOffsets[translationID];
                last = translationPronunciationOffsets[translationID + 1];
                count =
                    translationPronunciationOffsets[last] - translationPronunciationOffsets[first];
                if (count != 0) {
                    tokenCount = tokenCount + count;
                    totalPronunciationCount += last - first;
                    wordCount++;
                    transitionCount++;
                } else {
                    translationID = NO_GRAPH_INDEX;
                }
                transitions[link].translationID = translationID;
                link = transitions[link].next;
            }
        }
    }
    /* Build the menu selection masks for this word list. */
    if (CreateMenuMasks(context, secondPass, wordCount) != 0) {
        goto failure;
    }
    secondPass->transitions = heap_Alloc(context->heap,
        (wordCount + 1 + transitionCount) * sizeof(*secondPass->transitions));
    if (secondPass->transitions == NULL) {
        goto failure;
    }
    transitionBoundaryCursor = secondPass->transitions;
    packedTransitionCursor = secondPass->transitions + wordCount + 1;
    secondPass->wordFlags = heap_Alloc(context->heap, wordCount * 2);
    if (secondPass->wordFlags == NULL) {
        goto failure;
    }
    wordFlagCursor = secondPass->wordFlags;
    memset(wordFlagCursor, 0, wordCount * 2);
    secondPass->wordPenalties = heap_Alloc(context->heap, wordCount * 2);
    if (secondPass->wordPenalties == NULL) {
        goto failure;
    }
    wordPenaltyCursor = secondPass->wordPenalties;
    {
        u32 pronunciationEntries = totalPronunciationCount + 2;
        secondPass->wordOffsets = heap_Alloc(context->heap,
            (wordCount + pronunciationEntries) * sizeof(*secondPass->wordOffsets));
    }
    if (secondPass->wordOffsets == NULL) {
        goto failure;
    }
    allPronunciationOffsets = secondPass->wordOffsets;
    pronunciationOffsetTable = allPronunciationOffsets + wordCount + 1;
    wordBoundaryCursor = allPronunciationOffsets;
    tokenCount += totalPronunciationCount * 2;
    secondPass->dictionary = heap_Alloc(context->heap, tokenCount * 2);
    if (secondPass->dictionary == NULL) {
        goto failure;
    }
    encodedPronunciationDictionary = secondPass->dictionary;
    context->cachedData = heap_Alloc(context->heap, wordCount * 8);
    if (context->cachedData == NULL) {
        goto failure;
    }
    firstPassWordMapCursor = context->cachedData;
    encodedWordOffset = 0;
    {
        u32 link = graphNodes[0].firstTransition;
        while (link != NO_GRAPH_INDEX) {
            WordGraphTransition *transition = transitions + link;
            wordFlagCursor[transition->destination - 1] = 1;
            link = transition->next;
        }
    }
    /* Pack graph pronunciations into the second-pass arrays and record their first-pass IDs. */
    graphNodeCursor = graphNodes + 1;
    {
        for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
             graphNodeCursor++, graphNodeIndex++) {
            s32 wordID = graphNodeCursor->wordID;
            u32 link = graphNodeCursor->firstTransition;
            u32 wordAttributeFlags = wordAttributes[wordID];
            DictionaryWord *word;
            u16 pronunciationTokenCount;
            u16 *wordTokenEnd;
            u32 dictionaryWordIndex;
            firstPassWordMapCursor[0] = wordID;
            firstPassWordMapCursor[1] = graphNodeCursor->dictionaryIndex;
            firstPassWordMapCursor += 2;
            *transitionBoundaryCursor++ = packedTransitionCursor - secondPass->transitions;
            *pronunciationOffsetTable = encodedWordOffset;
            *wordBoundaryCursor++ = pronunciationOffsetTable++ - allPronunciationOffsets;
            word = dictionaryWords + graphNodeCursor->dictionaryIndex;
            pronunciationTokenCount = word->tokenCount;
            wordTokenEnd = encodedPronunciationDictionary;
            memcpy(wordTokenEnd, dictionary + word->transcriptionOffset,
                   pronunciationTokenCount * 2);
            wordTokenEnd += pronunciationTokenCount;
            wordTokenEnd[-1] |= 1;
            dictionaryWordIndex = graphNodeIndex - 1;
            wordTokenEnd[0] = dictionaryWordIndex;
            wordTokenEnd[1] = dictionaryWordIndex >> 16;
            encodedPronunciationDictionary = wordTokenEnd + 2;
            encodedWordOffset += pronunciationTokenCount + 2;
            if (link != 0) {
                if (link == NO_GRAPH_INDEX) {
                    *wordFlagCursor |= 2;
                } else if (*wordFlagCursor != 1) {
                    *wordFlagCursor = wordAttributeFlags;
                }
                if (*wordFlagCursor++ & (1 << 7)) {
                    *wordPenaltyCursor++ = context->wordPenalty;
                } else {
                    *wordPenaltyCursor++ = 0;
                }
                while (link != NO_GRAPH_INDEX) {
                    WordGraphTransition *transition = transitions + link;
                    if (transition->translationID == NO_GRAPH_INDEX) {
                        *packedTransitionCursor++ = transition->destination - 1;
                    } else {
                        *packedTransitionCursor++ = nextTranslationWordIndex++;
                    }
                    link = transition->next;
                }
            }
        }
    }
    /* Append available joining translations and connect each one to its destination word. */
    graphNodeCursor = graphNodes + 1;
    {
        nextTranslationWordIndex = initialGraphWordCount;
        *transitionBoundaryCursor++ = packedTransitionCursor - secondPass->transitions;
        *wordBoundaryCursor++ = pronunciationOffsetTable - allPronunciationOffsets;
        for (graphNodeIndex = 1; graphNodeIndex <= initialGraphWordCount;
             graphNodeCursor++, graphNodeIndex++) {
            u32 link = graphNodeCursor->firstTransition;
            if (link != 0) {
                while (link != NO_GRAPH_INDEX) {
                    WordGraphTransition *transition = transitions + link;
                    u16 translationID = transition->translationID;
                    if (translationID != NO_GRAPH_INDEX) {
                        firstPassWordMapCursor[0] = -1;
                        firstPassWordMapCursor[1] = -1;
                        firstPassWordMapCursor += 2;
                        putTransWordInDict(&pronunciationOffsetTable, &encodedWordOffset,
                                           &encodedPronunciationDictionary,
                                           nextTranslationWordIndex++,
                                           translationPronunciationOffsets +
                                               translationPronunciationOffsets[translationID],
                                           translationPronunciationOffsets +
                                               translationPronunciationOffsets[translationID + 1],
                                           translationTokenLexicon);
                        *wordBoundaryCursor++ = pronunciationOffsetTable - allPronunciationOffsets;
                        *packedTransitionCursor++ = transition->destination - 1;
                        *transitionBoundaryCursor++ =
                            packedTransitionCursor - secondPass->transitions;
                        *wordPenaltyCursor++ = 0;
                    }
                    link = transition->next;
                }
            }
        }
    }
    *pronunciationOffsetTable++ = encodedWordOffset;
    heap_Free(context->heap, graphNodes);
    heap_Free(context->heap, transitions);
    secondPass->state = 0;
    secondPass->wordCount = wordCount;
    secondPass->tokenCount = tokenCount;
    return 0;

failure:
    if (graphNodes != NULL) {
        heap_Free(context->heap, graphNodes);
    }
    if (transitions != NULL) {
        heap_Free(context->heap, transitions);
    }
    FreeSecondPassContext(context);
    return 1;
}

/*
 * Releases cached and second-pass search data.
 * CreateSecondPassContext calls this before a rebuild and when memory setup fails.
 */
void FreeSecondPassContext(RecognitionContext *context)
{
    if (context->cachedData != NULL) {
        heap_Free(context->heap, context->cachedData);
        context->cachedData = NULL;
    }

    if (context->secondPass != NULL) {
        if (context->secondPass->wordOffsets != NULL) {
            heap_Free(context->heap, context->secondPass->wordOffsets);
        }
        if (context->secondPass->dictionary != NULL) {
            heap_Free(context->heap, context->secondPass->dictionary);
        }
        if (context->secondPass->transitions != NULL) {
            heap_Free(context->heap, context->secondPass->transitions);
        }
        if (context->secondPass->wordPenalties != NULL) {
            heap_Free(context->heap, context->secondPass->wordPenalties);
        }
        if (context->secondPass->wordFlags != NULL) {
            heap_Free(context->heap, context->secondPass->wordFlags);
        }
        if (context->secondPass->menuMasks != NULL) {
            heap_Free(context->heap, context->secondPass->menuMasks);
        }
        heap_Free(context->heap, context->secondPass);
        context->secondPass = NULL;
    }
}
