/* Word and pronunciation data for the second recognition pass. */
#ifndef GSSDK_SECONDPASS_H
#define GSSDK_SECONDPASS_H

#include "types.h"

struct _ASRSPI_CONTEXT;
struct _ENGINE;

/* A word and pronunciation in the temporary second-pass graph. */
struct _NODE {
    unsigned long OldWordID; /* Word ID in the source context. */
    unsigned long OldPronID; /* Pronunciation ID in the source context. */
    unsigned short iVertex; /* First outgoing edge, or 0xFFFF at a terminal node. */
};

/* An edge in the temporary Chinese pronunciation graph. */
struct _CHINESE_VERTEX {
    unsigned short iNode;
    unsigned short iVertex;
};
/* A speech unit and its pronunciation IDs in the translated graph. */
struct _TRANSLATED_NODE {
    unsigned short Type;
    unsigned long OldWordID;
    unsigned long OldPronID;
    unsigned short Item;
    unsigned short Tone;
    unsigned long iVertex;
};
/* An edge in the translated speech-unit graph. */
struct _TRANSLATED_VERTEX {
    unsigned long iNode;
    unsigned long iVertex;
};

/* The tables and tone data shared by the recursive Chinese graph builders. */
struct _GROUPED_ARGUMENTS_FOR_RECURSIVE_FUNCTIONS {
    struct _ENGINE *pEngine;
    struct _NODE *pNodeTable;
    struct _CHINESE_VERTEX *pVertexTable;
    struct _TRANSLATED_NODE *pTranslated_NodeTable;
    struct _TRANSLATED_VERTEX *pTranslated_VertexTable;
    struct _TRANSLATED_NODE *pFinalSwapped_Translated_NodeTable;
    struct _TRANSLATED_VERTEX *pFinalSwapped_Translated_VertexTable;
    const unsigned short *pCtxLexicon;
    const unsigned long *pCtxPronBegin;
    const unsigned long *pSpeechUnitLex;
    const unsigned short *pTCM;
    unsigned short NbrOfFinals;
    unsigned short NbrOfFinals_x_NbrOfLeftTones;
    unsigned short UnknownTone;
};

/* A pronunciation graph edge with a destination and an optional joining translation. */
struct _VERTEX {
    unsigned short iNode; /* Destination node in the graph. */
    unsigned short iVertex; /* Next edge from the same source, or 0xFFFF at the end. */
    unsigned short TransWordIndex; /* 0xFFFF marks direct traversal. */
};

/* A state index with terminal flags in the low bits in the packed pronunciation dictionary. */
struct _DICTSTATE {
    unsigned short StateIndex; /* State index packed with the entry's flags. */
};

/* Packed dictionary and transition costs for extra-event recognition. */
struct _DYNPROG_EXEV_CTXINFO {
    unsigned long uNrStatesInDict; /* Number of entries in the state dictionary. */
    struct _DICTSTATE *pStateInDict; /* Dictionary states used by the transition chains. */
    unsigned short *pBeginOfPronunciationsInDict; /* State offsets delimiting the chains. */
    unsigned long NbrOfPronunciations_RejectionWord; /* Number of rejection chains. */
    long Rejection_NBS_penalty; /* Cost added when a transition chain advances. */
    long Rejection_NBS_path_penalty; /* Initial cost assigned to the first state. */
};

/* Encoded dictionary offset, length and endpoint classes for a pronunciation. */
struct _SECONDPASSINFO {
    unsigned long StateDictIndex; /* Offset into the packed state dictionary. */
    unsigned short Size; /* Number of entries for this pronunciation. */
    unsigned char LeftSpeechUnitClass; /* Class of the first speech unit. */
    unsigned char RightSpeechUnitClass; /* Class of the last speech unit. */
};

/* Packed dictionary and search arrays used by a dynamic-programming recognition pass. */
struct _DYNPROG_CTXINFO {
    int iOptionalSilence; /* Word index used for optional silence. */
    unsigned int uNbrOfWords; /* Number of words in the pass dictionary. */
    unsigned int uNrStatesInDict; /* Number of entries in the packed state dictionary. */
    unsigned short *pWordProps; /* Recognition properties indexed by word. */
    struct _DICTSTATE *pStateInDict; /* Packed pronunciation state entries. */
    signed short *pWordPenalties; /* Recognition penalties indexed by word. */
    unsigned long *pSyntax; /* Follower lists for the vocabulary. */
    unsigned long *pBeginOfWords; /* Dictionary start offsets indexed by word. */
    unsigned char *pucMenuMasks; /* One active-menu bit per word. */
};

/* Source dictionary IDs for a second-pass entry. Joining translations use all-ones IDs. */
struct _NEWID2OLDID {
    unsigned long OldWordID; /* Word ID in the source context. */
    unsigned long OldPronID; /* Pronunciation ID in the source context. */
};

/* First-pass result node. Sentence nodes link to the associated word results. */
struct _DYNPROG_RESULT_NODE {
    unsigned long StateNbr; /* State number used to identify the dictionary entry. */
    unsigned long Score; /* Accumulated score of the path through this node. */
    unsigned long ExitTime; /* Frame at which the path left this state. */
    struct _DYNPROG_RESULT_NODE *pInfo; /* Backtrace entries belonging to this result. */
    struct _DYNPROG_RESULT_NODE *pNext; /* Next node at this level of the result list. */
};

/* Builds a second-pass dictionary from the active recognition context. */
unsigned char CreateSecondPassContext(struct _ENGINE *pEngine);
/* Builds a second-pass dictionary from the active isolated-word tree. */
u8 CreateSecondPassTreeContext(struct _ENGINE *pEngine);
/* Releases the engine's second-pass dictionary and its lookup arrays. */
void FreeSecondPassContext(struct _ENGINE *pEngine);
/* Builds the active-menu masks for the second-pass vocabulary. */
u8 CreateMenuMasks(struct _ENGINE *pEngine, struct _DYNPROG_CTXINFO *pSecondCtxInfo,
                   u32 NbrWordsSecondPass);
/* Extracts the packed state offsets and endpoint classes for one pronunciation. */
u8 GetSecondPassInfo(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx,
                     struct _SECONDPASSINFO *pSecondPassInfo, unsigned int PronID);

/* Runs the spelling pass over the saved utterance labels. */
u8 casrDoSpelling(struct _ENGINE *pEngine);

/* Rescores an isolated-word utterance using the saved labels. */
u8 casrDoIsoWordSecondPass(struct _ENGINE *pEngine);

/* Rescores a continuous recognition utterance using the saved labels. */
u8 casrDoNormalSecondPass(struct _ENGINE *pEngine);

/* Dispatches the second pass for the active recognition mode. */
u8 casrDoSecondPass(struct _ENGINE *pEngine);

/* Builds the continuous second-pass context for Chinese. */
u8 CreateSecondPassContext_ForChinese(struct _ENGINE *pEngine);

/* Builds the isolated-word second-pass context for Chinese. */
u8 CreateSecondPassTreeContext_ForChinese(struct _ENGINE *pEngine);

#endif
