/* Builds the Chinese second-pass dictionary from an isolated-word tree. */
#include "gssdk/engine.h"
#include "gssdk/context.h"
#include "gssdk/heap.h"
#include "gssdk/langdata.h"
#include "gssdk/secondpass.h"

#include <string.h>

typedef struct _NODE NODE;

/* Packs the states and terminal word ID for one speech unit. */
static void putWordInDict_ForChinese_TreeCtx(unsigned long ItemNodeID_Minus1, u16 ItemType,
                                               unsigned long Item, unsigned long *pDictIndex,
    struct _DICTSTATE **ppState, const unsigned long *pSpeechUnitLex)
{
    struct _DICTSTATE *output = *ppState;
    u16 wordsWritten = 0;

    if (ItemType == 0) {
        output++->StateIndex = (Item * 4) & 0xFFFC;
        wordsWritten = 1;
    } else {
        const unsigned short *source = (const unsigned short *)pSpeechUnitLex
            + pSpeechUnitLex[Item] + 2;
        const unsigned short *end = (const unsigned short *)pSpeechUnitLex
            + pSpeechUnitLex[Item + 1];
        for (; source < end; source++) {
            output++->StateIndex = (*source * 4) & 0xFFFC;
            wordsWritten++;
        }
    }

    output[-1].StateIndex |= 1;
    output[0].StateIndex = ItemNodeID_Minus1;
    output[1].StateIndex = ItemNodeID_Minus1 >> 16;
    *pDictIndex += wordsWritten + 2;
    *ppState = output + 2;
}

/* Builds the pronunciation tree from the first-pass word results. */
static void Build_Tree_ForChinese_TreeCtx(NODE *pNodeTable, struct _CHINESE_VERTEX *pVertexTable,
    long StartWord, long StopWord, const unsigned long *pCtxBeginOfWords,
    unsigned long NBest, const long *pResult, unsigned long *pVertexIndex,
    unsigned long *pNodeIndex)
{
    unsigned int Sent;
    const long *pRes;
    unsigned long iVertex;
    unsigned long PronIDsStart;
    unsigned int i, j, k;
    unsigned long VertexIndex;
    unsigned long NodeIndex;
    unsigned long iNode;
    unsigned long OldPronID;
    long OldWordID;
    unsigned long PrevNbrPronounces;

    pRes = pResult;
    PronIDsStart = pCtxBeginOfWords[0];
    VertexIndex = *pVertexIndex;
    NodeIndex = *pNodeIndex;
    for (Sent = 0; Sent < NBest; Sent++) {
        iNode = 0;
        OldWordID = *pRes;
        iVertex = pNodeTable[iNode].iVertex;
        OldPronID = pCtxBeginOfWords[StartWord] - PronIDsStart;
        while (1) {
            if (iVertex == 0xFFFFU) {
                PrevNbrPronounces = iNode != 0 ? pCtxBeginOfWords[pNodeTable[iNode].OldWordID + 1]
                                                     - pCtxBeginOfWords[pNodeTable[iNode].OldWordID]
                                               : 1;
                break;
            }
            if (OldPronID == pNodeTable[pVertexTable[iVertex].iNode].OldPronID) {
                iNode = pVertexTable[iVertex].iNode;
                break;
            }
            iVertex = pVertexTable[iVertex].iVertex;
        }
        if (iVertex == 0xFFFFU) {
            unsigned long NbrPronounces =
                pCtxBeginOfWords[StartWord + 1] - pCtxBeginOfWords[StartWord];
            iVertex = pNodeTable[iNode].iVertex;
            for (i = 0; i < NbrPronounces; i++) {
                for (j = 0; j < PrevNbrPronounces; j++) {
                    pVertexTable[VertexIndex + j].iNode = NodeIndex + i;
                    pVertexTable[VertexIndex + j].iVertex =
                        iVertex == 0xFFFFU ? 0xFFFFU : iVertex + j;
                }
                iVertex = VertexIndex;
                if (i == NbrPronounces - 1) {
                    for (j = 0; j < PrevNbrPronounces; j++)
                        pNodeTable[iNode + j].iVertex = VertexIndex + j;
                }
                VertexIndex = (unsigned short)(VertexIndex + PrevNbrPronounces);
                pNodeTable[NodeIndex + i].OldPronID = OldPronID + i;
                pNodeTable[NodeIndex + i].OldWordID = StartWord;
            }
            iNode = NodeIndex;
            NodeIndex = (unsigned short)(NodeIndex + NbrPronounces);
            PrevNbrPronounces = NbrPronounces;
            iVertex = 0xFFFFU;
        } else {
            iVertex = pNodeTable[iNode].iVertex;
        }
        OldPronID = pCtxBeginOfWords[OldWordID] - PronIDsStart;
        if (iVertex != 0xFFFFU) {
            while (1) {
                if (iVertex == 0xFFFFU) {
                    PrevNbrPronounces = iNode != 0
                                            ? pCtxBeginOfWords[pNodeTable[iNode].OldWordID + 1]
                                                  - pCtxBeginOfWords[pNodeTable[iNode].OldWordID]
                                            : 1;
                    break;
                }
                if (OldPronID == pNodeTable[pVertexTable[iVertex].iNode].OldPronID) {
                    iNode = pVertexTable[iVertex].iNode;
                    break;
                }
                iVertex = pVertexTable[iVertex].iVertex;
            }
            iVertex = pNodeTable[iNode].iVertex;
        }
        for (k = 0; k < 2; k++) {
            unsigned long NbrPronounces =
                pCtxBeginOfWords[OldWordID + 1] - pCtxBeginOfWords[OldWordID];
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
            OldWordID = StopWord;
            OldPronID = pCtxBeginOfWords[OldWordID] - PronIDsStart;
            iVertex = 0xFFFFU;
        }
        for (i = 0; i < PrevNbrPronounces; i++) pNodeTable[iNode++].iVertex = 0xFFFFU;
        pRes += 2;
    }
    *pVertexIndex = VertexIndex;
    *pNodeIndex = NodeIndex;
}

typedef struct _CHINESE_SECONDPASSINFO CHINESE_SECONDPASSINFO;

/* Counts the dictionary states represented by a node and its descendants. */
static void Calculate_NbrOfStatesInDict_ForChinese_TreeCtx(
    unsigned long ItemNodeID, unsigned long *pDictSize,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs)
{
    unsigned long iVertex;

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
    iVertex = pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].iVertex;
    while (iVertex != 0xFFFFU) {
        Calculate_NbrOfStatesInDict_ForChinese_TreeCtx(
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[iVertex].iNode, pDictSize,
            pGroupedArgs);
        iVertex = pGroupedArgs->pFinalSwapped_Translated_VertexTable[iVertex].iVertex;
    }
}

/* Counts the second-pass items and tracks the longest item chain. */
static unsigned long Calculate_NbrOfItems_In_Words_Of_SecPass_TreeCtx(
    unsigned long NbrWordsSecondPass,
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
    NbrOfVertices = 0;
    iVertex = pNodeTable->iVertex;
    while (iVertex != 0xFFFFU) {
        iVertex = pVertexTable[iVertex].iVertex;
        NbrOfVertices++;
    }
    MaxNbr_Of_OutputVertices = NbrOfVertices;
    for (i = 1; i <= NbrWordsSecondPass; i++) {
        NbrOfVertices = 0;
        iVertex = pNodeTable[i].iVertex;
        pLex = pCtxLexicon + pCtxPronBegin[pNodeTable[i].OldPronID];
        pLex++;
        pLexEnd = pCtxLexicon + pCtxPronBegin[pNodeTable[i].OldPronID + 1];
        NbrOfItems += pLexEnd - pLex;
        while (iVertex != 0xFFFFU) {
            iVertex = pVertexTable[iVertex].iVertex;
            NbrOfVertices++;
        }
        if (NbrOfVertices > MaxNbr_Of_OutputVertices) MaxNbr_Of_OutputVertices = NbrOfVertices;
    }
    *pMaxNbr_Of_OutputVertices = MaxNbr_Of_OutputVertices;
    return NbrOfItems;
}

/* Expands pronunciation nodes into speech units and tone values. */
static void Translate_Prons_Into_Items_And_Tones_TreeCtx(
    unsigned long NodeID, unsigned long *pItemNodeID, unsigned long *pItemVertexID,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs)
{
    unsigned short iVertex;
    unsigned long LastVertex_Leaving_TranslatedSourceNode;
    const unsigned short *pLex;
    const unsigned short *pLexEnd;
    unsigned char To_Be_Translated_Node_Is_EndNode;
    unsigned short Type;

    if (NodeID == 0) {
        iVertex = pGroupedArgs->pNodeTable[0].iVertex;
        pGroupedArgs->pTranslated_NodeTable[0].Type = 0;
        pGroupedArgs->pTranslated_NodeTable[0].Tone = 0xFFFFU;
        pGroupedArgs->pTranslated_NodeTable[0].iVertex = 0;
        pGroupedArgs->pTranslated_VertexTable[0].iNode = 1;
    } else {
        pLex = pGroupedArgs->pCtxLexicon
               + pGroupedArgs->pCtxPronBegin[pGroupedArgs->pNodeTable[NodeID].OldPronID];
        pLexEnd = pGroupedArgs->pCtxLexicon
                  + pGroupedArgs->pCtxPronBegin[pGroupedArgs->pNodeTable[NodeID].OldPronID + 1];
        Type = *pLex++;
        To_Be_Translated_Node_Is_EndNode = 0;
        if (pGroupedArgs->pNodeTable[NodeID].iVertex == 0xFFFFU)
            To_Be_Translated_Node_Is_EndNode = 1;
        while (pLex < pLexEnd) {
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Type = Type == 1 ? 1 : 0;
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
            pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].Tone =
                ((const u16 *)pGroupedArgs->pSpeechUnitLex)[
                    pGroupedArgs->pSpeechUnitLex[*pLex] + 1];
            if (To_Be_Translated_Node_Is_EndNode && pLex == pLexEnd - 1) {
                pGroupedArgs->pTranslated_NodeTable[(*pItemNodeID)++].iVertex = 0xFFFFU;
            } else {
                pGroupedArgs->pTranslated_NodeTable[*pItemNodeID].iVertex = *pItemVertexID;
                pGroupedArgs->pTranslated_VertexTable[*pItemVertexID].iNode = ++*pItemNodeID;
                pGroupedArgs->pTranslated_VertexTable[(*pItemVertexID)++].iVertex = 0xFFFFU;
            }
            pLex++;
        }
        if (!To_Be_Translated_Node_Is_EndNode) --*pItemVertexID;
        iVertex = pGroupedArgs->pNodeTable[NodeID].iVertex;
    }
    while (iVertex != 0xFFFFU) {
        NodeID = pGroupedArgs->pVertexTable[iVertex].iNode;
        LastVertex_Leaving_TranslatedSourceNode = (*pItemVertexID)++;
        Translate_Prons_Into_Items_And_Tones_TreeCtx(
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
static unsigned char Tone_Of_Final_TreeCtx(unsigned short Tone, unsigned short UnknownTone)
{
    if (Tone != 0 && Tone < UnknownTone)
        return 1;
    else
        return 0;
}

/* Counts the extra nodes required when a final leads to several branches. */
static void Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree_TreeCtx(unsigned long ItemNodeID,
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs, unsigned long *pNbrOfItemNodes)
{
    unsigned long iVertex;

    iVertex = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    if (iVertex != 0xFFFFU && pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex != 0xFFFFU
        && Tone_Of_Final_TreeCtx(
            pGroupedArgs->pTranslated_NodeTable[ItemNodeID].Tone, pGroupedArgs->UnknownTone)) {
        iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
        while (iVertex != 0xFFFFU) {
            ++*pNbrOfItemNodes;
            iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
        }
        iVertex = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    }
    while (iVertex != 0xFFFFU) {
        Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree_TreeCtx(
            pGroupedArgs->pTranslated_VertexTable[iVertex].iNode, pGroupedArgs, pNbrOfItemNodes);
        iVertex = pGroupedArgs->pTranslated_VertexTable[iVertex].iVertex;
    }
}

/* Copies a translated node's type, item, tone, and original word and pronunciation IDs. */
static void Copy_All_Fields_Except_iVertex_TreeCtx(
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
static unsigned long SplitUp_Finals_TreeCtx(
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs, unsigned long ItemNodeID,
    unsigned long *pSwappedItemNodeID, unsigned long *pSwappedItemVertexID)
{
    unsigned long ItemVertexID;
    unsigned long LastVertex_CommingOutOf_SwappedItemNode;
    unsigned char FirstEncounteredNode_Was_NodeWithMultipleBranches = 1;
    unsigned long Branch_ItemNodeID;
    unsigned long ReturnValue;

    ItemVertexID = pGroupedArgs->pTranslated_NodeTable[ItemNodeID].iVertex;
    while (ItemVertexID != 0xFFFFU && pGroupedArgs->pTranslated_VertexTable
        [ItemVertexID].iVertex == 0xFFFFU) {
        Copy_All_Fields_Except_iVertex_TreeCtx(
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
        Copy_All_Fields_Except_iVertex_TreeCtx(
            pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
            pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex = 0xFFFFU;
    } else if (Tone_Of_Final_TreeCtx(
                   pGroupedArgs->pTranslated_NodeTable[ItemNodeID].Tone,
                   pGroupedArgs->UnknownTone)) {
        LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID - 1;
        while (ItemVertexID != 0xFFFFU) {
            Copy_All_Fields_Except_iVertex_TreeCtx(
                pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
                pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
            pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex =
                *pSwappedItemVertexID;
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[*pSwappedItemVertexID].iNode =
                *pSwappedItemNodeID;
            pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                (*pSwappedItemVertexID)++].iVertex = 0xFFFFU;
            SplitUp_Finals_TreeCtx(pGroupedArgs,
                pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iNode,
                pSwappedItemNodeID, pSwappedItemVertexID);
            ItemVertexID = pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iVertex;
            if (ItemVertexID == 0xFFFFU) {
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    LastVertex_CommingOutOf_SwappedItemNode].iVertex = 0xFFFFU;
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
        Copy_All_Fields_Except_iVertex_TreeCtx(
            pGroupedArgs->pFinalSwapped_Translated_NodeTable, *pSwappedItemNodeID,
            pGroupedArgs->pTranslated_NodeTable, ItemNodeID);
        pGroupedArgs->pFinalSwapped_Translated_NodeTable[(*pSwappedItemNodeID)++].iVertex =
            *pSwappedItemVertexID;
        LastVertex_CommingOutOf_SwappedItemNode = *pSwappedItemVertexID;
        pGroupedArgs->pFinalSwapped_Translated_VertexTable[(*pSwappedItemVertexID)++].iNode =
            *pSwappedItemNodeID;
        while (ItemVertexID != 0xFFFFU) {
            ReturnValue = SplitUp_Finals_TreeCtx(pGroupedArgs,
                pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iNode, pSwappedItemNodeID,
                pSwappedItemVertexID);
            if (ReturnValue != 0) LastVertex_CommingOutOf_SwappedItemNode = ReturnValue;
            ItemVertexID = pGroupedArgs->pTranslated_VertexTable[ItemVertexID].iVertex;
            if (ItemVertexID == 0xFFFFU)
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[
                    LastVertex_CommingOutOf_SwappedItemNode].iVertex = 0xFFFFU;
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

/* Traverses the translated tree and swaps finals using the preceding tone. */
static u8 Swap_Finals_TreeCtx(unsigned short LeftTone, unsigned long ItemNodeID_Of_PreviousFinal,
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
        if (Tone_Of_Final_TreeCtx(RightTone, pGroupedArgs->UnknownTone)) {
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
        } else if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Type == 0 &&
            ItemNodeID != 0) {
            if (ItemNodeID_Of_PreviousFinal != 0)
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item =
                    pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                           + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones *
                                               pGroupedArgs->UnknownTone];
            ItemNodeID_Of_PreviousFinal = 0;
            LeftTone = pGroupedArgs->UnknownTone;
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
        if (Tone_Of_Final_TreeCtx(RightTone, pGroupedArgs->UnknownTone)) {
            if (ItemNodeID_Of_PreviousFinal != 0) {
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID_Of_PreviousFinal].Item =
                    pGroupedArgs->pTCM[SU_ID + pGroupedArgs->NbrOfFinals * LeftTone
                                           + pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones *
                                               RightTone];
                pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item =
                    pGroupedArgs
                        ->pTCM[pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Item
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
            if (pGroupedArgs->pFinalSwapped_Translated_NodeTable[ItemNodeID].Type == 0)
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
        if (Swap_Finals_TreeCtx(LeftTone, ItemNodeID_Of_PreviousFinal,
                pGroupedArgs->pFinalSwapped_Translated_VertexTable[ItemVertexID].iNode,
                    pGroupedArgs)
            != 0)
            return 1;
        ItemVertexID = pGroupedArgs->pFinalSwapped_Translated_VertexTable[ItemVertexID].iVertex;
    }
    }
    return 0;
}

/* Writes the packed state dictionary, syntax and word properties. */
static u8 FillOut_Syntax_Statedict_And_Wordprops_ForChinese_TreeCtx(
    struct _ENGINE *pEngine, const struct _TRANSLATED_NODE *pNodeTable,
    const struct _TRANSLATED_VERTEX *pVertexTable,
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

    pSecondCtxInfo->pWordProps = heap_Alloc(pEngine->hHeap, NbrNodes * sizeof(*pWordProp));
    pWordProp = pSecondCtxInfo->pWordProps;
    pSecondCtxInfo->pWordPenalties = heap_Alloc(pEngine->hHeap, NbrNodes * sizeof(*pWordProp));
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

    memset(pWordProp, 0, NbrNodes * sizeof(*pWordProp));
    iVertex = pNodeTable[0].iVertex;
    while (iVertex != 0xFFFFU) {
        iNode = pVertexTable[iVertex].iNode;
        pWordProp[iNode - 1] = 1;
        iVertex = pVertexTable[iVertex].iVertex;
    }

    for (i = 1; i <= NbrNodes; i++) {
        pNewID2OldID->OldWordID = pNodeTable[i].OldWordID;
        pNewID2OldID->OldPronID = pNodeTable[i].OldPronID;
        pNewID2OldID++;
        *pSyntax = pFollowers - pSecondCtxInfo->pSyntax;
        pSyntax++;
        *pBeginOfPron = DictIndex;
        *pBegin++ = pBeginOfPron++ - pBeginOfWords;
        putWordInDict_ForChinese_TreeCtx(i - 1, pNodeTable[i].Type, pNodeTable[i].Item,
                                      &DictIndex, &pState, pSpeechUnitLex);

        iVertex = pNodeTable[i].iVertex;
        if (iVertex != 0) {
            if (iVertex == 0xFFFFU) {
                pWordProp[0] |= 2;
            } else if (pWordProp[0] != 1 &&
                       pNodeTable[i].OldPronID != 0xFFFFFFFC) { /* Intermediate node ID. */
                pWordProp[0] |= 0x80;
            }

            if ((*pWordProp++ & 0x80) != 0) {
                *pWordPenalty++ = pEngine->WordPenalty;
            } else {
                *pWordPenalty++ = 0;
            }
            while (iVertex != 0xFFFFU) {
                iNode = pVertexTable[iVertex].iNode;
                *pFollowers++ = iNode - 1;
                iVertex = pVertexTable[iVertex].iVertex;
            }
        }
    }

    *pSyntax = pFollowers - pSecondCtxInfo->pSyntax;
    *pBegin = pBeginOfPron - pBeginOfWords;
    return 0;
}

/* Releases the Chinese dictionary and isolated-word search arrays. */
void FreeSecondPassContext_ForChinese_TreeCtx(struct _ENGINE *pEngine)
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
        heap_Free(pEngine->hHeap, pEngine->CtxInfo.pSecondPassDynProgCtxInfo);
        pEngine->CtxInfo.pSecondPassDynProgCtxInfo = NULL;
    }
}

/* Checks whether a word allowed at the start has a final in its pronunciation. */
static unsigned char StartWordsContainFinals(const unsigned short *pWordProps,
    unsigned long uNbrOfWords, CHINESE_SECONDPASSINFO *pChineseSecondPassInfo,
    const unsigned long *pSpeechUnitLex, unsigned short UnknownTone)
{
    unsigned int i;
    const unsigned short *pCtxLexicon = pChineseSecondPassInfo->pCtxLexicon;
    const unsigned long *pCtxPronBegin = pChineseSecondPassInfo->pCtxPronBegin;
    const unsigned short *pLex;
    const unsigned short *pLexEnd;
    for (i = 0; i < uNbrOfWords; i++) {
        if (pWordProps[i] & 1) {
            pLex = pCtxLexicon + pCtxPronBegin[i];
            pLexEnd = pCtxLexicon + pCtxPronBegin[i + 1];
            for (; pLex < pLexEnd; pLex++) {
                if (Tone_Of_Final_TreeCtx(
                        ((const unsigned short *)pSpeechUnitLex)[pSpeechUnitLex[*pLex] + 1],
                        UnknownTone))
                    return 1;
            }
        }
    }
    return 0;
}

/* Counts result pronunciations and tracks the largest count for any result word. */
static unsigned long CountNbrOfPronsInResult_ForChinese_TreeCtx(
    const long *pRes, unsigned long NBest, const unsigned long *pCtxBeginOfWords,
    unsigned long *pHiPron)
{
    unsigned int Sent;
    unsigned long HiPron = *pHiPron;
    unsigned long NbrPronsFirstPass = 0;
    long Word;
    unsigned long NrProns;
    for (Sent = 0; Sent < NBest; Sent++) {
        Word = *pRes;
        NrProns = pCtxBeginOfWords[Word + 1] - pCtxBeginOfWords[Word];
        if (NrProns > HiPron) HiPron = NrProns;
        NbrPronsFirstPass = NrProns + NbrPronsFirstPass;
        NbrPronsFirstPass += 2;
        pRes += 2;
    }
    *pHiPron = HiPron;
    return NbrPronsFirstPass;
}

/* Builds the Chinese second-pass context for an isolated-word result. */
unsigned char CreateSecondPassTreeContext_ForChinese(struct _ENGINE *pEngine)
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
    unsigned long NbrOfItemVertices_In_FinalSwappedTranslatedTree = 0;
    unsigned long SwappedItemNodeID = 0;
    unsigned long SwappedItemVertexID = 0;
    struct _DYNPROG_CTXINFO *pSecondCtxInfo;
    struct _LANGUAGE *pLang;
    unsigned short *pDimensionsOfToneConversionMatrix;
    NODE *pNodeTable;
    struct _CHINESE_VERTEX *pVertexTable;
    struct _TRANSLATED_NODE *pTranslated_NodeTable;
    struct _TRANSLATED_VERTEX *pTranslated_VertexTable;
    struct _TRANSLATED_NODE *pFinalSwapped_Translated_NodeTable;
    struct _TRANSLATED_VERTEX *pFinalSwapped_Translated_VertexTable;
    CHINESE_SECONDPASSINFO *pChineseSecondPassInfo;
    struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS *pGroupedArgs;
    struct _ASRSPI_CONTEXT Ctx;
    struct _DYNPROG_RESULT *pRecogResult;
    const long *pResult;
    unsigned long NBest;
    const long *pRes;
    const unsigned long *pCtxBeginOfWords;
    long StartWord;
    long StopWord;
    unsigned long uNbrOfWords;
    const unsigned short *pWordProps;
    unsigned int i;

    pLang = pEngine->pLang;
    pDimensionsOfToneConversionMatrix = pLang->langGetpDimensionsOfToneConversionMatrix(pLang);
    pChineseSecondPassInfo = pEngine->CtxInfo.pChineseSecondPassInfo;
    pRecogResult = pEngine->pRes;
    pResult = (const long *)pRecogResult->pResult;
    NBest = pEngine->NBest;
    pRes = pResult;
    StartWord = -1;
    StopWord = -1;
    if (pEngine->CtxInfo.pSecondPassDynProgCtxInfo != NULL)
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
    pSecondCtxInfo = heap_Alloc(pEngine->hHeap, 0x24);
    if (pSecondCtxInfo == NULL) {
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
        return 1;
    }
    memset(pSecondCtxInfo, 0, 0x24);
    pEngine->CtxInfo.pSecondPassDynProgCtxInfo = pSecondCtxInfo;
    pGroupedArgs = heap_Alloc(pEngine->hHeap, sizeof(*pGroupedArgs));
    if (pGroupedArgs == NULL) {
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
        return 1;
    }
    pGroupedArgs->pEngine = pEngine;
    pGroupedArgs->pCtxLexicon = pChineseSecondPassInfo->pCtxLexicon;
    pGroupedArgs->pCtxPronBegin = pChineseSecondPassInfo->pCtxPronBegin;
    pGroupedArgs->pSpeechUnitLex = (const unsigned long *)pLang->langGetpSpeechUnit(pLang);
    pGroupedArgs->pTCM = (const unsigned short *)pLang->langGetpToneConversionMatrix(pLang)
                         - *(const unsigned short *)pLang->langGetOffsetForFinals(pLang);
    pGroupedArgs->NbrOfFinals = pDimensionsOfToneConversionMatrix[2];
    pGroupedArgs->NbrOfFinals_x_NbrOfLeftTones =
        pDimensionsOfToneConversionMatrix[2] * pDimensionsOfToneConversionMatrix[1];
    pGroupedArgs->UnknownTone = pLang->langGetNbrTones(pLang) + 1;
    FillContextV2VirtualTable(&Ctx);
    Ctx.pCtx = pEngine->ActivatedContext->pCtx;
    uNbrOfWords = Ctx.contextGetNbrWord(&Ctx);
    pWordProps = Ctx.contextGetpWordProp(&Ctx);
    pCtxBeginOfWords = Ctx.contextGetpBeginOfWords(&Ctx);
    for (i = 0; i < uNbrOfWords; i++) {
        if (pWordProps[i] & 1) {
            StartWord = i;
            if (StopWord != -1) break;
        }
        if (pWordProps[i] & 2) {
            StopWord = i;
            if (StartWord != -1) break;
        }
    }
    NbrPronsFirstPass =
        CountNbrOfPronsInResult_ForChinese_TreeCtx(pRes, NBest, pCtxBeginOfWords, &HiPron);
    pNodeTable = pGroupedArgs->pNodeTable =
        heap_Alloc(pEngine->hHeap, (NbrPronsFirstPass + 1) * sizeof(*pNodeTable));
    pVertexTable = pGroupedArgs->pVertexTable =
        heap_Alloc(pEngine->hHeap, NbrPronsFirstPass * HiPron * sizeof(*pVertexTable));
    if (pNodeTable == NULL || pVertexTable == NULL) {
        if (pNodeTable) heap_Free(pEngine->hHeap, pNodeTable);
        if (pVertexTable) heap_Free(pEngine->hHeap, pVertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
        return 1;
    }
    pNodeTable[0].iVertex = 0xFFFFU;
    Build_Tree_ForChinese_TreeCtx(pNodeTable, pVertexTable, StartWord, StopWord, pCtxBeginOfWords,
        NBest, pResult, &VertexIndex, &NodeIndex);
    for (iVertex = pNodeTable[0].iVertex; iVertex != 0xFFFFU;
        iVertex = pVertexTable[iVertex].iVertex)
        NbrStartWords++;
    NbrWordsSecondPass = NodeIndex - 1;
    NbrOfItems = Calculate_NbrOfItems_In_Words_Of_SecPass_TreeCtx(NbrWordsSecondPass, pNodeTable,
        pVertexTable, pChineseSecondPassInfo, &MaxNbr_Of_OutputVertices);
    pTranslated_NodeTable = pGroupedArgs->pTranslated_NodeTable =
        heap_Alloc(pEngine->hHeap, (NbrOfItems + 1) * sizeof(*pTranslated_NodeTable));
    pTranslated_VertexTable = pGroupedArgs->pTranslated_VertexTable = heap_Alloc(
        pEngine->hHeap, NbrOfItems * MaxNbr_Of_OutputVertices * sizeof(*pTranslated_VertexTable));
    if (pTranslated_NodeTable == NULL || pTranslated_VertexTable == NULL) {
        heap_Free(pEngine->hHeap, pNodeTable);
        heap_Free(pEngine->hHeap, pVertexTable);
        if (pTranslated_NodeTable) heap_Free(pEngine->hHeap, pTranslated_NodeTable);
        if (pTranslated_VertexTable) heap_Free(pEngine->hHeap, pTranslated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
        return 1;
    }
    Translate_Prons_Into_Items_And_Tones_TreeCtx(
        0, &ItemNodeID, &ItemVertexID, pGroupedArgs);
    heap_Free(pEngine->hHeap, pNodeTable);
    heap_Free(pEngine->hHeap, pVertexTable);
    if (StartWordsContainFinals(pWordProps, uNbrOfWords, pChineseSecondPassInfo,
            pGroupedArgs->pSpeechUnitLex, pGroupedArgs->UnknownTone)) {
        NbrOfItemNodes_In_FinalSwappedTranslatedTree = NbrOfItems;
        Calculate_NbrOfItemNodes_In_FinalSwappedTranslatedTree_TreeCtx(
            0, pGroupedArgs, &NbrOfItemNodes_In_FinalSwappedTranslatedTree);
        pFinalSwapped_Translated_NodeTable = pGroupedArgs->pFinalSwapped_Translated_NodeTable =
            heap_Alloc(pEngine->hHeap, MaxNbr_Of_OutputVertices * (NbrOfItems + 1)
                                           * sizeof(*pFinalSwapped_Translated_NodeTable));
        pFinalSwapped_Translated_VertexTable = pGroupedArgs->pFinalSwapped_Translated_VertexTable =
            heap_Alloc(pEngine->hHeap, MaxNbr_Of_OutputVertices
                                           * (MaxNbr_Of_OutputVertices * NbrOfItems)
                                           * sizeof(*pFinalSwapped_Translated_VertexTable));
        if (pFinalSwapped_Translated_NodeTable == NULL
            || pFinalSwapped_Translated_VertexTable == NULL) {
            heap_Free(pEngine->hHeap, pTranslated_NodeTable);
            heap_Free(pEngine->hHeap, pTranslated_VertexTable);
            if (pFinalSwapped_Translated_NodeTable)
                heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
            if (pFinalSwapped_Translated_VertexTable)
                heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
            heap_Free(pEngine->hHeap, pGroupedArgs);
            FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
            return 1;
        }
        SplitUp_Finals_TreeCtx(
            pGroupedArgs, 0, &SwappedItemNodeID, &SwappedItemVertexID);
        heap_Free(pEngine->hHeap, pTranslated_NodeTable);
        heap_Free(pEngine->hHeap, pTranslated_VertexTable);
    } else {
        pFinalSwapped_Translated_NodeTable = pGroupedArgs->pFinalSwapped_Translated_NodeTable =
            pTranslated_NodeTable;
        pFinalSwapped_Translated_VertexTable = pGroupedArgs->pFinalSwapped_Translated_VertexTable =
            pTranslated_VertexTable;
        NbrOfItemNodes_In_FinalSwappedTranslatedTree = NbrOfItems;
    }
    if (Swap_Finals_TreeCtx(0xFFFFU, 0, 0, pGroupedArgs)) {
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
        return 1;
    }
    Calculate_NbrOfStatesInDict_ForChinese_TreeCtx(0, &DictSize, pGroupedArgs);
    DictSize += NbrOfItemNodes_In_FinalSwappedTranslatedTree * 2;
    NbrOfItemVertices_In_FinalSwappedTranslatedTree =
        ((ItemVertexID + 1) - NbrStartWords)
        + (NbrOfItemNodes_In_FinalSwappedTranslatedTree - NbrOfItems);
    if (FillOut_Syntax_Statedict_And_Wordprops_ForChinese_TreeCtx(pEngine,
            pFinalSwapped_Translated_NodeTable,
            pFinalSwapped_Translated_VertexTable, DictSize,
            NbrOfItemNodes_In_FinalSwappedTranslatedTree,
            NbrOfItemVertices_In_FinalSwappedTranslatedTree,
            pSecondCtxInfo)) {
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_NodeTable);
        heap_Free(pEngine->hHeap, pFinalSwapped_Translated_VertexTable);
        heap_Free(pEngine->hHeap, pGroupedArgs);
        FreeSecondPassContext_ForChinese_TreeCtx(pEngine);
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
