/* Builds the Chinese second-pass word dictionary and pronunciation graph. */
#include "types.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/langdata.h"
#include "gssdk/secondpass.h"
#include <string.h>

/* Encodes every pronunciation belonging to one Chinese word in the packed dictionary. */
static void putWordInDict_ForChinese(unsigned long ItemNodeID_Minus1, unsigned short ItemType,
    unsigned long Item, unsigned long *pDictIndex, struct _DICTSTATE **ppState,
    const unsigned long *pSpeechUnitLex)
{
    struct _DICTSTATE *dictionaryCursor = *ppState;
    u16 encodedCount = 0;

    if (ItemType == 0) {
        dictionaryCursor++->StateIndex = (Item * 4) & 0xFFFC;
        encodedCount = 1;
    } else {
        const u8 *sourceBytes = (const u8 *)pSpeechUnitLex;
        s32 *offsets = (s32 *)sourceBytes;
        u16 *first = (u16 *)(sourceBytes + offsets[Item] * 2 + 4);
        u16 *last = (u16 *)(sourceBytes + offsets[Item + 1] * 2);
        for (; first < last; ++first) {
            dictionaryCursor++->StateIndex = (*first * 4) & 0xFFFC;
            ++encodedCount;
        }
    }
    dictionaryCursor[-1].StateIndex |= 1;
    dictionaryCursor[0].StateIndex = ItemNodeID_Minus1;
    dictionaryCursor[1].StateIndex = ItemNodeID_Minus1 >> 16;
    *pDictIndex += encodedCount + 2;
    *ppState = dictionaryCursor + 2;
}

/* Finds the longest pronunciation available in the enabled Chinese word lists. */
static unsigned long CountNbrOfPronsInResult_ForChinese(struct _ENGINE *pEngine,
    struct _DICTSTATE *pStateInDict, unsigned long *pBeginOfWords, unsigned long *pHiPron)
{
    u32 previousLength;
    struct _DYNPROG_RESULT_NODE *menu;
    struct _DYNPROG_RESULT_NODE *word;
    u32 maxLength;
    u32 resultCount;
    u32 multiplier;

    menu = pEngine->pRes->pResult;
    previousLength = 0;
    maxLength = *pHiPron;
    resultCount = 0;
    while (menu != NULL) {
        word = menu->pInfo;
        multiplier = 1;

        while (word != NULL) {
            struct _DICTSTATE *pState = pStateInDict + word->StateNbr;
            if ((pState[0].StateIndex & 1) == 0) {
                u32 rangeIndex = pState[2].StateIndex + ((u32)pState[3].StateIndex << 16);
                unsigned long *pRange = pBeginOfWords + rangeIndex;
                previousLength = pRange[1] - pRange[0];
                if (previousLength > maxLength) maxLength = previousLength;
                resultCount += previousLength * multiplier;
            } else {
                resultCount += multiplier;
                if (maxLength < 1) maxLength = 1;
            }
            multiplier = previousLength * multiplier;
            word = word->pNext;
        }
        menu = menu->pNext;
    }
    *pHiPron = maxLength;
    return resultCount;
}

/* Adds the dictionary states represented by a word and follows its child entries. */
static void Build_Tree_ForChinese(struct _ENGINE *pEngine, struct _NODE *pNodeTable,
    struct _CHINESE_VERTEX *pVertexTable, const struct _DICTSTATE *pStateInDict,
    unsigned long *pBeginOfWords, unsigned long StateDictIndex,
    unsigned long *pVertexIndex, unsigned long *pNodeIndex)
{
    struct _DYNPROG_RESULT_NODE *pSent, *pRes;
    const struct _DICTSTATE *pDictStatePointer;
    unsigned long PrevNbrPronounces;
    unsigned long VertexIndex;
    unsigned long NodeIndex;
    unsigned long iVertex;
    unsigned int i, j;
    unsigned long iNode;
    unsigned long OldPronID;
    unsigned long OldWordID;
    unsigned long PrevExitTime;
    unsigned long NbrPronounces;

    VertexIndex = 0;
    NodeIndex = 1;
    for (pSent = pEngine->pRes->pResult; pSent != NULL;
        pSent = pSent->pNext) {
        iNode = 0;
        OldPronID = 0;
        OldWordID = 0;
        pRes = pSent->pInfo;
        if (pRes != NULL) {
        PrevExitTime = pRes->ExitTime;
        while (pRes != NULL) {
            pDictStatePointer = pStateInDict + pRes->StateNbr;
            iVertex = pNodeTable[iNode].iVertex;
            if (pDictStatePointer->StateIndex & 1) {
                if (pRes->ExitTime == PrevExitTime) {
                    pRes = pRes->pNext;
                    continue;
                }
                if ((pDictStatePointer[-1].StateIndex >> 2)
                    == (pDictStatePointer->StateIndex >> 2)) {
                    pRes = pRes->pNext;
                    continue;
                }
                if (pRes->pNext != NULL && (pDictStatePointer->StateIndex >> 2) ==
                    (pStateInDict[pRes->pNext->StateNbr].StateIndex >> 2)) {
                    pRes = pRes->pNext;
                    continue;
                }
                /* The silence node uses reserved pronunciation and word IDs. */
                OldPronID = 0xFFFFFFFE;
                /* The reserved word ID bypasses the word-to-pronunciation table. */
                OldWordID = 0xFFFFFFFE;
            } else {

                OldWordID =
                    pDictStatePointer[2].StateIndex + (pDictStatePointer[3].StateIndex << 16);
                OldPronID = pBeginOfWords[OldWordID] - StateDictIndex;
                PrevExitTime = pRes->ExitTime;
            }
            while (iVertex != 0xFFFFU) {
                if (OldPronID == pNodeTable[pVertexTable[iVertex].iNode].OldPronID) {
                    iNode = pVertexTable[iVertex].iNode;
                    pRes = pRes->pNext;
                    break;
                }
                iVertex = pVertexTable[iVertex].iVertex;
            }
            if (iVertex == 0xFFFFU) {
                /* The root and silence nodes each have a single pronunciation. */
                PrevNbrPronounces = iNode != 0 && pNodeTable[iNode].OldWordID != 0xFFFFFFFE
                                        ? pBeginOfWords[pNodeTable[iNode].OldWordID + 1]
                                              - pBeginOfWords[pNodeTable[iNode].OldWordID]
                                        : 1;
                break;
            }
        }
        iVertex = pNodeTable[iNode].iVertex;
        if (pRes != NULL) do {
            /* Silence has no pronunciation range in the context lexicon. */
            if (OldWordID == 0xFFFFFFFE) NbrPronounces = 1;
            else NbrPronounces = pBeginOfWords[OldWordID + 1] - pBeginOfWords[OldWordID];
            for (i = 0; i < NbrPronounces; i++) {
                for (j = 0; j < PrevNbrPronounces; j++) {
                    pVertexTable[VertexIndex + j].iNode = NodeIndex + j * NbrPronounces + i;
                    pVertexTable[VertexIndex + j].iVertex =
                        iVertex == 0xFFFFU ? 0xFFFFU : iVertex + j;
                }
                iVertex = VertexIndex;
                if (i == NbrPronounces - 1) {
                    for (j = 0; j < PrevNbrPronounces; j++)
                        pNodeTable[iNode + j].iVertex = VertexIndex + j;
                }
                VertexIndex = (unsigned short)(VertexIndex + PrevNbrPronounces);
                for (j = 0; j < PrevNbrPronounces; j++) {
                    pNodeTable[NodeIndex + j * NbrPronounces + i].OldPronID = OldPronID + i;
                    pNodeTable[NodeIndex + j * NbrPronounces + i].OldWordID = OldWordID;
                }
            }
            iNode = NodeIndex;
            NodeIndex = (unsigned short)(NodeIndex + NbrPronounces * PrevNbrPronounces);
            PrevNbrPronounces = NbrPronounces * PrevNbrPronounces;
            pRes = pRes->pNext;
            if (pRes == NULL) break;
            pDictStatePointer = pStateInDict + pRes->StateNbr;
            if (pDictStatePointer->StateIndex & 1) {
                if (pRes->ExitTime == PrevExitTime
                    || (pDictStatePointer[-1].StateIndex >> 2)
                           == (pDictStatePointer->StateIndex >> 2)
                    || (pRes->pNext != NULL
                        && (pDictStatePointer->StateIndex >> 2)
                               == (pStateInDict[pRes->pNext->StateNbr].StateIndex >> 2))) {
                    pRes = pRes->pNext;
                    if (pRes == NULL) break;
                    pDictStatePointer = pStateInDict + pRes->StateNbr;

                    OldWordID =
                        pDictStatePointer[2].StateIndex + (pDictStatePointer[3].StateIndex << 16);
                    OldPronID = pBeginOfWords[OldWordID] - StateDictIndex;
                    PrevExitTime = pRes->ExitTime;
                } else {
                    /* An unrecognized result is represented by the reserved silence ID. */
                    OldPronID = 0xFFFFFFFE;
                /* Use the reserved word ID for the silence node as well. */
                OldWordID = 0xFFFFFFFE;
                }
            } else {

                OldWordID =
                    pDictStatePointer[2].StateIndex + (pDictStatePointer[3].StateIndex << 16);
                OldPronID = pBeginOfWords[OldWordID] - StateDictIndex;
                PrevExitTime = pRes->ExitTime;
            }
            iVertex = 0xFFFFU;
        } while (1);
        for (i = 0; i < PrevNbrPronounces; i++) pNodeTable[iNode++].iVertex = 0xFFFFU;
        }
    }
    *pVertexIndex = VertexIndex;
    *pNodeIndex = NodeIndex;
}

/* Counts the dictionary states represented by a node and its descendants. */
static void Calculate_NbrOfStatesInDict_ForChinese(
    unsigned long ItemNodeID, unsigned long *pDictSize,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs)
{
    u32 nodeIndex;

    if (ItemNodeID != 0) {
        if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Type == 0) {
            ++*pDictSize;
        } else {
            *pDictSize +=
                pGroupedArgs->pSpeechUnitLex[
                    pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item + 1]
                - pGroupedArgs->pSpeechUnitLex
                      [pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item]
                - 2;
        }
    }

    nodeIndex = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex;
    while (nodeIndex != 0xFFFFU) {
        Calculate_NbrOfStatesInDict_ForChinese(
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[nodeIndex].iNode, pDictSize,
            pGroupedArgs);
        nodeIndex = pGroupedArgs->pFinalSwapped_Translated_VertexTable[nodeIndex].iVertex;
    }
}

/* Counts second-pass items and the deepest item chain for the Chinese dictionary. */
static unsigned long Calculate_NbrOfItems_In_Words_Of_SecPass(unsigned long NbrWordsSecondPass,
    struct _NODE *pNodeTable, struct _CHINESE_VERTEX *pVertexTable,
    const struct _CHINESE_SECONDPASSINFO *pChineseSecondPassInfo,
    unsigned long *pMaxNbr_Of_OutputVertices)
{
    unsigned int i;
    unsigned long NbrOfItems;
    unsigned long NbrOfVertices;
    unsigned long MaxNbr_Of_OutputVertices;
    unsigned long iVertex;
    const unsigned short *pLex;
    const unsigned short *pLexEnd;
    const unsigned short *pCtxLexicon;
    const unsigned long *pCtxPronBegin;
    pCtxLexicon = pChineseSecondPassInfo->pCtxLexicon;
    NbrOfItems = 0;
    pCtxPronBegin = pChineseSecondPassInfo->pCtxPronBegin;
    MaxNbr_Of_OutputVertices = 0;
    iVertex = pNodeTable->iVertex;
    while (iVertex != 0xFFFFU) {
        iVertex = pVertexTable[iVertex].iVertex;
        MaxNbr_Of_OutputVertices++;
    }
    for (i = 1; i <= NbrWordsSecondPass; i++) {
        /* The silence pronunciation contributes one item directly. */
        if (pNodeTable[i].OldPronID == 0xFFFFFFFE) {
            NbrOfItems++;
        } else {
            pLex = pCtxLexicon + pCtxPronBegin[pNodeTable[i].OldPronID];
            pLex++;
            pLexEnd = pCtxLexicon + pCtxPronBegin[pNodeTable[i].OldPronID + 1];
            NbrOfItems += pLexEnd - pLex;
        }
        iVertex = pNodeTable[i].iVertex;
        NbrOfVertices = 0;
        while (iVertex != 0xFFFFU) {
            iVertex = pVertexTable[iVertex].iVertex;
            NbrOfVertices++;
        }
        if (NbrOfVertices > MaxNbr_Of_OutputVertices) MaxNbr_Of_OutputVertices = NbrOfVertices;
    }
    *pMaxNbr_Of_OutputVertices = MaxNbr_Of_OutputVertices;
    return NbrOfItems;
}

/* Expands one pronunciation chain into word and final-tree nodes. */
static void Translate_Prons_Into_Items_And_Tones(
    unsigned long NodeID, unsigned long *pItemNodeID, unsigned long *pItemVertexID,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs)
{
    unsigned short iVertex;
    unsigned long LastVertex_Leaving_TranslatedSourceNode;
    unsigned char To_Be_Translated_Node_Is_EndNode;
    const unsigned short *pLex;
    const unsigned short *pLexEnd;
    unsigned short Type;

    if (NodeID == 0) {
        iVertex = pGroupedArgs->pNodeTable[0].iVertex;
        pGroupedArgs->pTranslated_NodeTable[0].Type = 0;
        pGroupedArgs->pTranslated_NodeTable[0].Tone = 0xFFFFU;
        pGroupedArgs->pTranslated_NodeTable[0].iVertex = 0;
        pGroupedArgs->pTranslated_VertexTable[0].iNode = 1;
    } else {
        s32 rangeIndex;

        To_Be_Translated_Node_Is_EndNode = 0;
        if (pGroupedArgs->pNodeTable[NodeID].iVertex == 0xFFFFU)
            To_Be_Translated_Node_Is_EndNode = 1;
        rangeIndex = pGroupedArgs->pNodeTable[NodeID].OldPronID;
        /* Silence is expanded using the language module's silence phone. */
        if ((unsigned long)rangeIndex != 0xFFFFFFFE) {
            pLex = pGroupedArgs->pCtxLexicon + pGroupedArgs->pCtxPronBegin[rangeIndex];
            pLexEnd = pGroupedArgs->pCtxLexicon + pGroupedArgs->pCtxPronBegin[rangeIndex + 1];
            Type = *pLex++;

            while (pLex < pLexEnd) {
                u16 isFinal;
                if (Type == 1)
                    isFinal = 1;
                else
                    isFinal = 0;
                pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Type = isFinal;
                if (pLex == pLexEnd - 1) {
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldWordID =
                        pGroupedArgs->pNodeTable[NodeID].OldWordID;
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldPronID =
                        pGroupedArgs->pNodeTable[NodeID].OldPronID;
                } else {
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldWordID = -4;
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldPronID = -4;
                }
                pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Item = *pLex;
                if (pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Type == 1) {
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Tone =
                        ((const u16 *)pGroupedArgs->pSpeechUnitLex)[
                            pGroupedArgs->pSpeechUnitLex[*pLex] + 1];
                } else {
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Tone =
                        pGroupedArgs->UnknownTone;
                }
                if (To_Be_Translated_Node_Is_EndNode != 0 && pLex == pLexEnd - 1) {
                    pGroupedArgs->pTranslated_NodeTable[(*pItemNodeID)++].iVertex = 0xFFFFU;
                } else {
                    pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].iVertex = *pItemVertexID;
                    pGroupedArgs->pTranslated_VertexTable[*pItemVertexID].iNode = ++*pItemNodeID;
                    pGroupedArgs->pTranslated_VertexTable[(*pItemVertexID)++].iVertex = 0xFFFFU;
                }
                ++pLex;
            }
        } else {
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Type = 0;
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldWordID =
                pGroupedArgs->pNodeTable[NodeID].OldWordID;
            rangeIndex = pGroupedArgs->pNodeTable[NodeID].OldPronID;
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].OldPronID = rangeIndex;
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Item =
                pGroupedArgs->pEngine->pLang->langGetSilPhenome(pGroupedArgs->pEngine->pLang);
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Tone = 0;
            if (To_Be_Translated_Node_Is_EndNode != 0) {
                pGroupedArgs->pTranslated_NodeTable[(*pItemNodeID)++].iVertex = 0xFFFFU;
            } else {
                pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].iVertex = *pItemVertexID;
                pGroupedArgs->pTranslated_VertexTable[*pItemVertexID].iNode = ++*pItemNodeID;
                pGroupedArgs->pTranslated_VertexTable[(*pItemVertexID)++].iVertex = 0xFFFFU;
            }
        }
        if (To_Be_Translated_Node_Is_EndNode == 0) --*pItemVertexID;
        iVertex = pGroupedArgs->pNodeTable[NodeID].iVertex;
    }

    while (iVertex != 0xFFFFU) {
        NodeID = pGroupedArgs->pVertexTable[iVertex].iNode;
        LastVertex_Leaving_TranslatedSourceNode = (*pItemVertexID)++;
        Translate_Prons_Into_Items_And_Tones(
            NodeID, pItemNodeID, pItemVertexID, pGroupedArgs);
        iVertex = pGroupedArgs->pVertexTable[iVertex].iVertex;
        if (iVertex == 0xFFFFU)
            pGroupedArgs->pTranslated_VertexTable[
                LastVertex_Leaving_TranslatedSourceNode].iVertex = 0xFFFFU;
        else {
            pGroupedArgs->pTranslated_VertexTable[
                LastVertex_Leaving_TranslatedSourceNode].iVertex = *pItemVertexID;
            pGroupedArgs->pTranslated_VertexTable[*pItemVertexID].iNode = *pItemNodeID;
        }
    }
}

/* Checks whether the tone identifies a final. */
static unsigned char Tone_Of_Final(unsigned short Tone, unsigned short UnknownTone)
{
    if (Tone != 0 && Tone < UnknownTone)
        return 1;
    else
        return 0;
}

/* Counts the extra nodes required when a final leads to several branches. */
static void Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree(unsigned long ItemNodeID,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs, unsigned long *pNbrOfItemNodes)
{
    unsigned long iVertex;

    iVertex = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    if (iVertex != 0xFFFFU
        && pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex != 0xFFFFU
        && Tone_Of_Final(
            pGroupedArgs->pTranslated_NodeTable[ItemNodeID].Tone, pGroupedArgs->UnknownTone)) {
        iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
        while (iVertex != 0xFFFFU) {
            ++*pNbrOfItemNodes;
            iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
        }
        iVertex = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    }
    while (iVertex != 0xFFFFU) {
        Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree(
            pGroupedArgs->pTranslated_VertexTable[iVertex].iNode, pGroupedArgs, pNbrOfItemNodes);
        iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
    }
}

/* Copies a translated node's type, item, tone, and original word and pronunciation IDs. */
static void Copy_All_Fields_Except_iVertex(
    struct _TRANSLATED_NODE *pFinalSwapped_Translated_NodeTable, unsigned long SwappedItemNodeID,
    const struct _TRANSLATED_NODE *pTranslated_NodeTable, unsigned long ItemNodeID)
{
    pFinalSwapped_Translated_NodeTable[SwappedItemNodeID].Type =
        pTranslated_NodeTable[ItemNodeID].Type;
    pFinalSwapped_Translated_NodeTable[SwappedItemNodeID].OldWordID =
        pTranslated_NodeTable[ItemNodeID].OldWordID;
    pFinalSwapped_Translated_NodeTable[SwappedItemNodeID].OldPronID =
        pTranslated_NodeTable[ItemNodeID].OldPronID;
    pFinalSwapped_Translated_NodeTable[SwappedItemNodeID].Item =
        pTranslated_NodeTable[ItemNodeID].Item;
    pFinalSwapped_Translated_NodeTable[SwappedItemNodeID].Tone =
        pTranslated_NodeTable[ItemNodeID].Tone;
}

/* Copies and splits final-bearing branches into the output tree. */
static unsigned long SplitUp_Finals(
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs, unsigned long ItemNodeID,
    unsigned long *pSwappedItemNodeID, unsigned long *pSwappedItemVertexID)
{
    unsigned long ItemVertexID;
    unsigned long LastVertex_CommingOutOf_SwappedItemNode;
    unsigned char FirstEncounteredNode_Was_NodeWithMultipleBranches = 1;
    unsigned long Branch_ItemNodeID;
    unsigned long ReturnValue;

    ItemVertexID = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    while (ItemVertexID != 0xFFFFU
           && pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iVertex == 0xFFFFU) {
        Copy_All_Fields_Except_iVertex(
            pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
            pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex =
            *pSwappedItemVertexID;
        pGroupedArgs->pFinalSwapped_Translated_VertexTable[*pSwappedItemVertexID].iNode =
            *pSwappedItemNodeID;
        pGroupedArgs->pFinalSwapped_Translated_VertexTable[(*pSwappedItemVertexID)++].iVertex =
            0xFFFFU;
        ItemNodeID = pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iNode;
        ItemVertexID = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
        if (FirstEncounteredNode_Was_NodeWithMultipleBranches)
            FirstEncounteredNode_Was_NodeWithMultipleBranches = 0;
    }
    if (ItemVertexID == 0xFFFFU) {
        Copy_All_Fields_Except_iVertex(
            pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
            pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex = 0xFFFFU;
    } else if (Tone_Of_Final(
                   pGroupedArgs->pTranslated_NodeTable[ItemNodeID].Tone,
                   pGroupedArgs->UnknownTone)) {
        LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID - 1;
        while (ItemVertexID != 0xFFFFU) {
            Copy_All_Fields_Except_iVertex(
                pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
                pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
            pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex =
                *pSwappedItemVertexID;
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[*pSwappedItemVertexID].iNode =
                *pSwappedItemNodeID;
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                (*pSwappedItemVertexID)++].iVertex = 0xFFFFU;
            SplitUp_Finals(pGroupedArgs, pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iNode,
                pSwappedItemNodeID, pSwappedItemVertexID);
            ItemVertexID = pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iVertex;
            if (ItemVertexID == 0xFFFFU) {
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    LastVertex_CommingOutOf_SwappedItemNode].iVertex =
                    0xFFFFU;
                if (FirstEncounteredNode_Was_NodeWithMultipleBranches)
                    return LastVertex_CommingOutOf_SwappedItemNode;
                else return 0;
            }
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                LastVertex_CommingOutOf_SwappedItemNode].iVertex =
                *pSwappedItemVertexID;
            LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID;
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[(*pSwappedItemVertexID)++].iNode =
                *pSwappedItemNodeID;
        }
    } else {
        Copy_All_Fields_Except_iVertex(
            pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
            pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex =
            *pSwappedItemVertexID;
        LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID;
        pGroupedArgs->pFinalSwapped_Translated_VertexTable[(*pSwappedItemVertexID)++].iNode =
            *pSwappedItemNodeID;
        while (ItemVertexID != 0xFFFFU) {
            ReturnValue =
                SplitUp_Finals(pGroupedArgs,
                    pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iNode,
                pSwappedItemNodeID, pSwappedItemVertexID);
            if (ReturnValue != 0) LastVertex_CommingOutOf_SwappedItemNode = ReturnValue;
            ItemVertexID = pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iVertex;
            if (ItemVertexID == 0xFFFFU)
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    LastVertex_CommingOutOf_SwappedItemNode].iVertex =
                    0xFFFFU;
            else {
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    LastVertex_CommingOutOf_SwappedItemNode].iVertex =
                    *pSwappedItemVertexID;
                LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID;
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    (*pSwappedItemVertexID)++].iNode =
                    *pSwappedItemNodeID;
            }
        }
    }
    return 0;
}

/* Applies final substitutions while walking the final tree for a word. */
static u8 Swap_Finals(unsigned short LeftTone, unsigned long ItemNodeID_Of_PreviousFinal,
    unsigned long ItemNodeID, struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs)
{
    unsigned short RightTone;
    unsigned long SU_ID =
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item;
    unsigned long ItemVertexID =
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex;
    unsigned short i;
    while (ItemVertexID != 0xFFFFU) {
        RightTone = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Tone;
        if (Tone_Of_Final(RightTone, pGroupedArgs->UnknownTone)) {
            if (ItemNodeID_Of_PreviousFinal != 0) {
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item =
                    pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                                 + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones *
                                                     RightTone];
                LeftTone =
                    pGroupedArgs->pFinalSwapped_Translated_NodeTable[
                        ItemNodeID_Of_PreviousFinal].Tone;
                ItemNodeID_Of_PreviousFinal = ItemNodeID;
                SU_ID = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item;
            } else if (LeftTone == 0xFFFFU) {
                SU_ID = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item;
                ItemNodeID_Of_PreviousFinal = ItemNodeID;
                LeftTone = 0;
            } else {
                SU_ID = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item;
                ItemNodeID_Of_PreviousFinal = ItemNodeID;
            }
        } else if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Type == 0) {
            if (RightTone == 0) {
                if (ItemNodeID_Of_PreviousFinal != 0)
                    pGroupedArgs->pFinalSwapped_Translated_NodeTable[
                        ItemNodeID_Of_PreviousFinal].Item =
                        pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone];
                ItemNodeID_Of_PreviousFinal = 0;
                LeftTone = 0;
            } else if (ItemNodeID != 0) {
                if (ItemNodeID_Of_PreviousFinal != 0)
                    pGroupedArgs->pFinalSwapped_Translated_NodeTable[
                        ItemNodeID_Of_PreviousFinal].Item =
                        pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                                     + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones
                                                           * pGroupedArgs->UnknownTone];
                ItemNodeID_Of_PreviousFinal = 0;
                LeftTone = pGroupedArgs->UnknownTone;
            }
        }
        if (pGroupedArgs->pFinalSwapped_Translated_VertexTable[ItemVertexID].iVertex == 0xFFFFU) {
            ItemNodeID =
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex].iNode;
            ItemVertexID = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex;
        } else break;
    }
    if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex == 0xFFFFU) {
        RightTone = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Tone;
        if (Tone_Of_Final(RightTone, pGroupedArgs->UnknownTone)) {
            if (ItemNodeID_Of_PreviousFinal != 0) {
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item =
                    pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                                 + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones *
                                                     RightTone];
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item =
                    pGroupedArgs->pTCM[
                        pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item
                            + pGroupedArgs->NbrOfFinals
                                  * pGroupedArgs->pFinalSwapped_Translated_NodeTable
                                      [ItemNodeID_Of_PreviousFinal].Tone];
            } else {
                if (LeftTone == 0xFFFFU) LeftTone = 0;
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item =
                    pGroupedArgs->pTCM[
                        pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item
                                                 + pGroupedArgs->NbrOfFinals * LeftTone];
            }
        } else if (ItemNodeID_Of_PreviousFinal != 0) {
            if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Type == 0 &&
                RightTone != 0)
                i = pGroupedArgs->UnknownTone;
            else i = 0;
            pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item =
                pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                             + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones * i];
        }
    } else {
    for (i = 0; ItemVertexID != 0xFFFFU; i++) {
        if (i != 0 && ItemNodeID_Of_PreviousFinal != 0) {
            LeftTone =
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[
                    ItemNodeID_Of_PreviousFinal].Tone;
            ItemNodeID_Of_PreviousFinal = 0;
        }
        if (Swap_Finals(LeftTone, ItemNodeID_Of_PreviousFinal,
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[ItemVertexID].iNode,
                    pGroupedArgs)
            != 0)
            return 1;
        ItemVertexID = pGroupedArgs->pFinalSwapped_Translated_VertexTable[ItemVertexID].iVertex;
    }
    }
    return 0;
}

/* Builds the Chinese second-pass arrays from the translated word tree. */

static u8 FillOut_Syntax_Statedict_And_Wordprops_ForChinese(struct _ENGINE *pEngine,
    const struct _TRANSLATED_NODE *pNodeTable, const struct _TRANSLATED_VERTEX *pVertexTable,
    unsigned long DictSize, unsigned long NbrNodes, unsigned long NbrVertices,
    struct _DYNPROG_CTXINFO *pSecondCtxInfo)
{
    unsigned short *pWordProp;
    signed short *pWordPenalty;
    struct _NEWID2OLDID *pNewID2OldID;
    unsigned long *pSyntax;
    unsigned long *pFollowers;
    unsigned long *pBeginOfWords;
    unsigned long *pBegin;
    unsigned long *pBeginOfPron;
    const unsigned long *pSpeechUnitLex;
    struct _DICTSTATE *pState;
    unsigned long iVertex;
    unsigned long DictIndex = 0;
    unsigned int i;
    unsigned long iNode;

    if (CreateMenuMasks(pEngine, pSecondCtxInfo, NbrNodes) != 0) {
        return 1;
    }

    pSecondCtxInfo->pWordProps = heap_Alloc(pEngine->hHeap, NbrNodes * 2);
    pWordProp = pSecondCtxInfo->pWordProps;
    pSecondCtxInfo->pWordPenalties = heap_Alloc(pEngine->hHeap, NbrNodes * 2);
    pWordPenalty = pSecondCtxInfo->pWordPenalties;
    pEngine->CtxInfo.pNewWordID2OldWordID = heap_Alloc(pEngine->hHeap, NbrNodes * 8);
    pNewID2OldID = pEngine->CtxInfo.pNewWordID2OldWordID;
    pSecondCtxInfo->pSyntax = heap_Alloc(pEngine->hHeap, (NbrNodes + 1 + NbrVertices) * 4);
    pSyntax = pSecondCtxInfo->pSyntax;
    pFollowers = pSecondCtxInfo->pSyntax + NbrNodes + 1;
    pSecondCtxInfo->pBeginOfWords = heap_Alloc(pEngine->hHeap, (NbrNodes + NbrNodes + 2) * 4);
    pBeginOfWords = pSecondCtxInfo->pBeginOfWords;
    pBegin = pBeginOfWords;
    pBeginOfPron = pBeginOfWords + NbrNodes + 1;
    pSpeechUnitLex = pEngine->pLang->langGetpSpeechUnit(pEngine->pLang);
    pSecondCtxInfo->pStateInDict = heap_Alloc(pEngine->hHeap, DictSize * 2);
    pState = pSecondCtxInfo->pStateInDict;

    if (pWordProp == NULL || pWordPenalty == NULL || pNewID2OldID == NULL ||
        pSyntax == NULL || pBeginOfWords == NULL || pState == NULL) {
        return 1;
    }

    memset(pWordProp, 0, NbrNodes * 2);
    iVertex = pNodeTable[0].iVertex;
    while (iVertex != 0xFFFFU) {
        i = pVertexTable[iVertex].iNode;
        pWordProp[i - 1] = 1;
        iVertex = pVertexTable[iVertex].iVertex;
    }

    for (iNode = 1; iNode <= NbrNodes; iNode++) {

        pNewID2OldID->OldWordID = pNodeTable[iNode].OldWordID;
        pNewID2OldID->OldPronID = pNodeTable[iNode].OldPronID;
        pNewID2OldID++;
        *pSyntax = pFollowers - pSecondCtxInfo->pSyntax;
        pSyntax++;
        *pBeginOfPron = DictIndex;
        *pBegin++ = pBeginOfPron++ - pBeginOfWords;
        putWordInDict_ForChinese(iNode - 1, pNodeTable[iNode].Type, pNodeTable[iNode].Item,
                                      &DictIndex, &pState, pSpeechUnitLex);

        iVertex = pNodeTable[iNode].iVertex;
        if (iVertex != 0) {
            if (iVertex == 0xFFFFU) {
                pWordProp[0] |= 2;
            } else if (pWordProp[0] != 1) {
                /* Only context pronunciations receive the word penalty here. */
                if (pNodeTable[iNode].OldPronID != 0xFFFFFFFC &&
                    pNodeTable[iNode].OldPronID != 0xFFFFFFFE) { /* Reserved silence ID. */
                    pWordProp[0] |= 0x80;
                }
            }

            if ((*pWordProp++ & 0x80) != 0) {
                *pWordPenalty++ = pEngine->WordPenalty;
            } else {
                *pWordPenalty++ = 0;
            }
            while (iVertex != 0xFFFFU) {
                *pFollowers++ = pVertexTable[iVertex].iNode - 1;
                iVertex = pVertexTable[iVertex].iVertex;
            }
        }
    }

    *pSyntax = pFollowers - pSecondCtxInfo->pSyntax;
    *pBegin = pBeginOfPron - pBeginOfWords;
    return 0;
}

/* Releases the cached data and every allocation owned by the Chinese second pass. */
void FreeSecondPassContext_ForChinese(struct _ENGINE *pEngine)
{
    if (pEngine->CtxInfo.pNewWordID2OldWordID != NULL) {
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pNewWordID2OldWordID);
        pEngine->CtxInfo.pNewWordID2OldWordID = NULL;
    }
    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL) {
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pBeginOfWords != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pBeginOfWords);
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pStateInDict != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pStateInDict);
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pSyntax != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pSyntax);
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordPenalties != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordPenalties);
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordProps != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pWordProps);
        if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pucMenuMasks != NULL)
            heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo->pucMenuMasks);
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo);
        pEngine->CtxInfo.pSecondPassDynProgCtxInfo = NULL;
    }
}

/* Builds the second-pass dictionary and syntax from Chinese recognition results. */
unsigned char CreateSecondPassContext_ForChinese(struct _ENGINE *pEngine)
{
    unsigned long HiPron = 0;
    unsigned long NbrPronsFirstPass;
    unsigned long iVertex;
    unsigned long VertexIndex = 0;
    unsigned long NodeIndex = 1;
    unsigned long NbrStartWords = 0;
    unsigned long NbrWordsSecondPass;
    unsigned long DictSize = 0;
    unsigned long NbrOfItems;
    unsigned long MaxNbr_Of_OutputVertices = 0;
    unsigned long ItemNodeID = 1;
    unsigned long ItemVertexID = 0;
    unsigned long NbrOfItemNodes_In_FinalSwappedTranslatedTree = 0;
    unsigned long NbrOfItemVertices_In_FinalSwappedTranslatedTree;
    unsigned long SwappedItemNodeID = 0;
    unsigned long SwappedItemVertexID = 0;
    struct _DYNPROG_CTXINFO *pSecondCtxInfo;
    struct _LANGUAGE *pLang;
    struct _DICTSTATE *pStateInDict;
    unsigned long *pBeginOfWords;
    unsigned long StateDictIndex;
    unsigned short *pDimensionsOfToneConversionMatrix;
    struct _NODE *pNodeTable;
    struct _CHINESE_VERTEX *pVertexTable;
    struct _TRANSLATED_NODE *pTranslated_NodeTable;
    struct _TRANSLATED_VERTEX *pTranslated_VertexTable;
    struct _TRANSLATED_NODE *pFinalSwapped_Translated_NodeTable;
    struct _TRANSLATED_VERTEX *pFinalSwapped_Translated_VertexTable;
    struct _CHINESE_SECONDPASSINFO *pChineseSecondPassInfo;
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs;

    pLang = pEngine->pLang;
    pBeginOfWords = pEngine->CtxInfo.pDynProgCtxInfo->pBeginOfWords;
    pStateInDict = pEngine->CtxInfo.pDynProgCtxInfo->pStateInDict;
    StateDictIndex = *pBeginOfWords;
    pDimensionsOfToneConversionMatrix =
        pLang->langGetpDimensionsOfToneConversionMatrix(pLang);
    pChineseSecondPassInfo = pEngine->CtxInfo.pChineseSecondPassInfo;
    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL)
        FreeSecondPassContext_ForChinese(pEngine);
    pSecondCtxInfo = heap_Alloc(pEngine->hHeap, sizeof(*pSecondCtxInfo));
    if (pSecondCtxInfo == NULL) {
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    memset(pSecondCtxInfo, 0, sizeof(*pSecondCtxInfo));
    pEngine->CtxInfo.pSecondPassDynProgCtxInfo = pSecondCtxInfo;
    pGroupedArgs = heap_Alloc(pEngine->hHeap, sizeof(*pGroupedArgs));
    if (pGroupedArgs == NULL) {
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    pGroupedArgs->pEngine = pEngine;
    pGroupedArgs->pCtxLexicon = pChineseSecondPassInfo->pCtxLexicon;
    pGroupedArgs->pCtxPronBegin = pChineseSecondPassInfo->pCtxPronBegin;
    pGroupedArgs->pSpeechUnitLex = pLang->langGetpSpeechUnit(pLang);
    pGroupedArgs->pTCM = pLang->langGetpToneConversionMatrix(pLang)
                         - *pLang->langGetOffsetForFinals(pLang);
    pGroupedArgs->NbrOfFinals = pDimensionsOfToneConversionMatrix[2];
    pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones =
        pDimensionsOfToneConversionMatrix[2] * pDimensionsOfToneConversionMatrix[1];
    pGroupedArgs->UnknownTone = pLang->langGetNbrTones(pLang) + 1;
    NbrPronsFirstPass = CountNbrOfPronsInResult_ForChinese(
        pEngine, pStateInDict, pBeginOfWords, &HiPron);
    pNodeTable = pGroupedArgs->pNodeTable =
        heap_Alloc(pEngine->hHeap, (NbrPronsFirstPass + 1) * sizeof(*pNodeTable));
    pVertexTable = pGroupedArgs->pVertexTable =
        heap_Alloc(pEngine->hHeap, NbrPronsFirstPass * HiPron * sizeof(*pVertexTable));
    if (pNodeTable == NULL || pVertexTable == NULL) {
        if (pNodeTable != NULL) heap_Free(pEngine->hHeap, pNodeTable);
        if (pVertexTable != NULL) heap_Free(pEngine->hHeap, pVertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    pNodeTable[0].iVertex = 0xFFFFU;
    Build_Tree_ForChinese(pEngine, pNodeTable, pVertexTable, pStateInDict, pBeginOfWords,
        StateDictIndex, &VertexIndex, &NodeIndex);
    iVertex = pNodeTable[0].iVertex;
    while (iVertex != 0xFFFFU) {
        iVertex = pVertexTable[iVertex].iVertex;
        NbrStartWords++;
    }
    NbrWordsSecondPass = NodeIndex - 1;
    NbrOfItems = Calculate_NbrOfItems_In_Words_Of_SecPass(NbrWordsSecondPass, pNodeTable,
        pVertexTable, pChineseSecondPassInfo, &MaxNbr_Of_OutputVertices);
    pTranslated_NodeTable = pGroupedArgs->pTranslated_NodeTable =
        heap_Alloc(pEngine->hHeap, (NbrOfItems + 1) * sizeof(*pTranslated_NodeTable));
    pTranslated_VertexTable = pGroupedArgs->pTranslated_VertexTable = heap_Alloc(
        pEngine->hHeap, NbrOfItems * MaxNbr_Of_OutputVertices * sizeof(*pTranslated_VertexTable));
    if (pTranslated_NodeTable == NULL || pTranslated_VertexTable == NULL) {
        heap_Free(pEngine->hHeap, pNodeTable);
        heap_Free(pEngine->hHeap, pVertexTable);
        if (pTranslated_NodeTable != NULL) heap_Free(pEngine->hHeap, pTranslated_NodeTable);
        if (pTranslated_VertexTable != NULL) heap_Free(pEngine->hHeap, pTranslated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    Translate_Prons_Into_Items_And_Tones(
        0, &ItemNodeID, &ItemVertexID, pGroupedArgs);
    heap_Free(pEngine->hHeap, pNodeTable);
    heap_Free(pEngine->hHeap, pVertexTable);
    NbrOfItemNodes_In_FinalSwappedTranslatedTree = NbrOfItems;
    Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree(
        0, pGroupedArgs, &NbrOfItemNodes_In_FinalSwappedTranslatedTree);
    pFinalSwapped_Translated_NodeTable = pGroupedArgs->pFinalSwapped_Translated_NodeTable =
        heap_Alloc(pEngine->hHeap, MaxNbr_Of_OutputVertices * (NbrOfItems + 1)
                                       * sizeof(*pFinalSwapped_Translated_NodeTable));
    pFinalSwapped_Translated_VertexTable = pGroupedArgs->pFinalSwapped_Translated_VertexTable =
        heap_Alloc(
            pEngine->hHeap, (MaxNbr_Of_OutputVertices * (MaxNbr_Of_OutputVertices * NbrOfItems))
                                * sizeof(*pFinalSwapped_Translated_VertexTable));
    if (pFinalSwapped_Translated_NodeTable == NULL
        || pFinalSwapped_Translated_VertexTable == NULL) {
        heap_Free(pEngine->hHeap, pTranslated_NodeTable);
        heap_Free(pEngine->hHeap, pTranslated_VertexTable);
        if (pFinalSwapped_Translated_NodeTable != NULL)
            heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
        if (pFinalSwapped_Translated_VertexTable != NULL)
            heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    SplitUp_Finals(
        pGroupedArgs, 0, &SwappedItemNodeID, &SwappedItemVertexID);
    heap_Free(pEngine->hHeap, pTranslated_NodeTable);
    heap_Free(pEngine->hHeap, pTranslated_VertexTable);
    if (Swap_Finals(0xFFFFU, 0, 0, pGroupedArgs) != 0) {
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    Calculate_NbrOfStatesInDict_ForChinese(0, &DictSize, pGroupedArgs);
    DictSize += NbrOfItemNodes_In_FinalSwappedTranslatedTree * 2;
    NbrOfItemVertices_In_FinalSwappedTranslatedTree =
        ((ItemVertexID + 1) - NbrStartWords)
        + (NbrOfItemNodes_In_FinalSwappedTranslatedTree - NbrOfItems);
    if (FillOut_Syntax_Statedict_And_Wordprops_ForChinese(pEngine,
        pFinalSwapped_Translated_NodeTable, pFinalSwapped_Translated_VertexTable, DictSize,
        NbrOfItemNodes_In_FinalSwappedTranslatedTree,
        NbrOfItemVertices_In_FinalSwappedTranslatedTree, pSecondCtxInfo) != 0) {
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese(pEngine);
        return 1;
    }
    heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
    heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
    heap_Free(pEngine->hHeap, pGroupedArgs);
    pSecondCtxInfo->uNbrOfWords = NbrOfItemNodes_In_FinalSwappedTranslatedTree;
    pSecondCtxInfo->iOptionalSilence = 0;
    pSecondCtxInfo->uNrStatesInDict = DictSize;
    return 0;
}
