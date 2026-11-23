/* Recognizer state, session records and scheduler callbacks. */
#ifndef GSSDK_ENGINE_H
#define GSSDK_ENGINE_H

#include "gssdk/casr.h"
#include "gssdk/engine_params.h"
#include "gssdk/langdata.h"
#include "gssdk/mqueue.h"

/* Fixed header of a version 2 recognition context. */
struct _ASRSPI_CONTEXTINFO {
    unsigned long DataType; /* Recognition context record type. */
    unsigned long Size; /* Byte extent stored in the packed header. */
    char VersionInfo[0x80]; /* Language version string stored in the header. */
    unsigned long TS; /* Terminal silence setting used for recognition. */
    unsigned long Accuracy; /* Recognition accuracy setting. */
    unsigned long Rejection; /* Recognition rejection setting. */
    unsigned long Garbage; /* Recognition garbage setting. */
    unsigned long NbrItem; /* Number of lexicon items. */
    unsigned long NbrWord; /* Number of words. */
    unsigned long NbrPron; /* Number of pronunciations. */
    unsigned long NbrFollower; /* Number of follower entries. */
    unsigned long NbrStartWord; /* Number of words allowed at the start of a recognition path. */
    unsigned long NbrStopWord; /* Number of words allowed at the end of a recognition path. */
    unsigned long GarbageWordID; /* Word ID for garbage results. */
    unsigned long ContextType; /* Context kind tested by the spelling and isolated-word
                                * accessors. */
};

/* An imported recognizer data record and its allocation heap. */
struct _DATAIMPORT {
    struct _DATA *pData; /* Imported language or session data. */
    void *hHeap; /* Heap that owns this handle. */
};


/* Loaded context data and the heap that owns it. */
struct _CTXIMPORT {
    /* Imported context record. */
    struct _DATA *pCtx;
    /* Heap used for this record's allocations. */
    void *hHeap;
};

/* Chinese context lexicon and pronunciation offsets used by the second pass. */
struct _CHINESE_SECONDPASSINFO {
    const unsigned short *pCtxLexicon; /* Packed lexicon of the active Chinese context. */
    const unsigned long *pCtxPronBegin; /* Start offsets indexed by pronunciation. */
};

/* Word ID and score in the decoder's result list. */
struct _WORDRESULT {
    long WordID; /* ID of the word represented by this result. */
    long Score;  /* Final score for the result. */
};

struct _DYNPROG_RESULT_NODE;

/* Ranked results and the running scores for the two garbage models. */
struct _DYNPROG_RESULT {
    struct _DYNPROG_RESULT_NODE *pResult; /* Ranked results retained from the utterance. */
    unsigned long GarbageSize; /* Number of frames for which both garbage-score arrays have room. */
    unsigned long *pSingleWordGarbage; /* Running single-word garbage score for each frame. */
    unsigned long *pSentenceGarbage;   /* Running sentence garbage score for each frame. */
};

/* Scores for the states of an active speech unit. */
struct _ISOWORD_TOKEN {
    u8 u8LastActState; /* Index of the last state reached by this token. */
    u8 u8LastState;    /* Index of the last state in the speech unit. */
    s16 pScore[1];     /* Variable-length scores for the states in this token. */
};

struct _ISOWORDTOKENDP;
struct _ISOWORD_NODE;

/* Reprunes the active word tree after token storage is exhausted. */
u8 RePruneToPho(struct _ISOWORDTOKENDP *This, struct _ISOWORD_NODE ***ppToP,
                s32 *pNrToCreated, s32 NrToPredicted, int BestScoreCurFr);

/* The engine's active context, spelling and second-pass resources. */
struct _CTXINFO {
    /* Dynamic-programming context for recognition. */
    struct _DYNPROG_CTXINFO *pDynProgCtxInfo;
    /* Dynamic-programming context for spelling. */
    struct _DYNPROG_CTXINFO *pSpellingDynProgCtxInfo;
    /* Context data used to rescore results. */
    struct _SECONDPASSINFO *pSecondPassInfo;
    /* Chinese lexicon and pronunciation data for rescoring. */
    struct _CHINESE_SECONDPASSINFO *pChineseSecondPassInfo;
    /* Dynamic-programming context for rescoring. */
    struct _DYNPROG_CTXINFO *pSecondPassDynProgCtxInfo;
    /* Maps the active word IDs back to their original IDs. */
    struct _NEWID2OLDID *pNewWordID2OldWordID;
    /* Root of the context word tree. */
    struct _ISOWORD_NODE *pCtxTree;
    /* Dynamic-programming context for extra events. */
    struct _DYNPROG_EXEV_CTXINFO *pExEvCtxInfo;
};

/* Word ID and the index of its predecessor in a word-tree path. */
struct _ISOWORD_BACKTRACE {
    /* Word ID recorded at this path entry. */
    long s32WordID;
    /* Index of the preceding entry in the path. */
    unsigned long u32PrevWordIdx;
};

/* A node keyed by a speech-unit identifier, or a leaf holding a word ID. */
struct _ISOWORD_NODE {
    union {
        /* First follower in a branch node. */
        struct _ISOWORD_NODE *pFirstFol;
        /* Context word ID in a leaf node. */
        long WordID;
    } u;
    /* Recognition token associated with the node. */
    struct _ISOWORD_TOKEN *pToken;
    /* Speech-unit identifier used to index the speech-unit lexicon. */
    unsigned short PhoID;
    /* Number of followers at the node. */
    unsigned short NrFol;
};

typedef struct _SCOREUWBACKTRACE _SCOREUWBACKTRACE;

/* Backtrace node with user-word and garbage scores at one frame. */
struct _SCOREUWBACKTRACE {
    /* Number of references to this backtrace node. */
    s32 Count;
    /* Previous node in this backtrace. */
    _SCOREUWBACKTRACE *Previous;
    /* User-word score saved at the node. */
    u32 Score;
    /* Garbage-model score saved alongside the user-word score. */
    u32 GarbScore;
    /* State index recorded by the scorer. */
    u16 State;
    /* Frame index at the state exit. */
    u32 ExitTime;
};

typedef struct _SCOREUSERWORDRES _SCOREUSERWORDRES;

/* Scores, boundaries and backtrace for user-word recognition. */
struct _SCOREUSERWORDRES {
    /* Score for the user-word path. */
    unsigned long UWScore;
    /* Score for the unrestricted-speech path. */
    unsigned long SpeechScore;
    /* Frame count for the speech segment. */
    unsigned long SpeechLength;
    /* Normalized score reported for this utterance. */
    float NormScore;
    /* Confidence value compared against the training thresholds. */
    long ConfValue;
    /* First frame index of the scored segment. */
    unsigned long StartIdx;
    /* End frame index of the scored segment. */
    unsigned long StopIdx;
    /* Number of distribution entries in the sequence. */
    unsigned long NbrPhenoms;
    /* Backtrace used to extract utterance segmentation. */
    struct _SCOREUWBACKTRACE *pBackTrace;
};

/* Distributions needed to initialize user-word generation. */
struct _INITUSERWORDDATA {
    u32 NbrPhenoms; /* Number of distributions specified by the initialization record. */
    u16 *pPhenoms;  /* Distribution sequence supplied for user-word training. */
    u16 SilPhenome; /* Distribution used for silence states. */
};

/* Word and silence phoneme lists supplied to the scoring block. */
typedef struct _INITSCOREWUSERWORDDATA {
    /* Number of distribution entries in the sequence. */
    unsigned long NbrPhenoms;
    /* Number of silence distribution entries. */
    unsigned long NbrSilPhenoms;
    /* Distribution identifiers in path order. */
    unsigned short *pPhenoms;
    /* Distribution identifiers supplied for silence. */
    unsigned short *pSilPhenoms;
} _INITSCOREWUSERWORDDATA;

/* Result with averaged score and a trailing distribution list. */
typedef struct _USERWORDRES {
    f32 NormScore; /* Score normalized by the number of speech frames. */
    u32 SpeechFrames; /* Frame intervals summed over the selected non-silence path. */
    u32 NbrPhenoms; /* Number of entries in the trailing list. */
    u16 pPhenoms[1]; /* Distributions in path order. */
} _USERWORDRES;

/* Stored training utterance and its distribution sequence. */
struct _USERWORDUTT {
    /* Frame count for this utterance. */
    unsigned long SpeechFrames;
    /* Number of distribution entries in the sequence. */
    unsigned long NbrPhenoms;
    /* Distribution identifiers in path order. */
    unsigned short *pPhenoms;
};

/* Codebook counts and row sums for phoneme states. */
struct _COUNTMATRIX {
    /* Codebook occurrence counts for a phoneme state. */
    unsigned short *Counts;
    /* Size of the codebook counts storage. */
    unsigned long CountsSize;
    /* Totals for the count rows. */
    unsigned short *SumCounts;
    /* Size of the totals storage. */
    unsigned long SumCountsSize;
    /* Next state in the list of count matrices. */
    struct _COUNTMATRIX *Next;
};

/* Engine callbacks for cached and calculated distances. */
struct _DISTCALCFUNCS {
    /* Object passed to the distance callbacks. */
    void *pThis;
    /* Calculates a distance for the supplied index. */
    signed short (*CalculateDistance)(void *, unsigned int);
    /* Initializes the distance calculation for a frame. */
    void (*InitCalculateDistances)(void *, signed short *, unsigned char *, unsigned char, int);
    /* Returns the cached distances. */
    signed short *(*GetpCachedDistances)(void *);
    /* Returns the offset applied to distances. */
    long (*GetDistanceOffset)(void *);
};

/* Recognition engine state shared by tree construction and user-word training. */
struct _ENGINE {
    /* Task scheduler associated with the recognizer. */
    struct _TOS Tos;
    /* Object record used by the engine handle. */
    struct _OBJECT Object;
    /* Input connection used to read queued labels. */
    struct _INPUTQUEUE LabelInQueue;
    /* Engine output connection. */
    struct _OUTPUTQUEUE OutQueue;
    /* Connection for output sample frames. */
    struct _OUTPUTQUEUE SampleOutQueue;
    /* Connection for silence-block output. */
    struct _OUTPUTQUEUE SilOutQueue;
    /* Connection for cepstral vectors. */
    struct _OUTPUTQUEUE CepstraOutQueue;
    /* Connection for label output. */
    struct _OUTPUTQUEUE LabelOutQueue;
    /* Current recognition parameters. */
    struct _PARAMLIST ParamList;
    /* Callbacks registered through the engine API. */
    struct _CRECOGCALLBACKS CRecogCallBacks;
    /* Dispatches a recognition result to its handler. */
    unsigned char (*cbResultAvailable)(struct _TOS *, unsigned char, unsigned char, unsigned long,
                                       unsigned long);
    /* Dispatches a user-word training event to its handler. */
    unsigned char (*cbTrain)(struct _TOS *, unsigned char, unsigned char, unsigned long,
                             unsigned long);
    /* Per-model enable bytes used when activating models. */
    unsigned char GenderActive[3];
    /* Value checked when resolving an engine handle. */
    unsigned long ValidEngine;
    /* Data retained for engine callbacks. */
    struct _USERDATA UserData;
    /* Heap used for this record's allocations. */
    void *hHeap;
    /* Currently selected language data. */
    struct _DATAIMPORT *ActivatedLanguage;
    /* Currently selected recognition context. */
    struct _CTXIMPORT *ActivatedContext;
    /* Controls recognition through the spelling context. */
    unsigned char IsActiveContextSpelling;
    /* The incremented tree word count must be at most this byte plus one. */
    unsigned char IsActiveContextIsoWord;
    /* Structures derived from the selected context. */
    struct _CTXINFO CtxInfo;
    /* Selected language record. */
    struct _LANGUAGE *pLang;
    /* Distribution identifier used for unrestricted speech. */
    unsigned long AnySpeechPhenom;
    /* Format of the input samples. */
    enum _CSAMPLEFORMAT CSampleFormat;
    /* Size of a frame processed by the engine. */
    unsigned long FrameSize;
    /* List of recognition blocks to run. */
    const unsigned char *pActiveBlocks;
    /* Label buffers for the training utterances. */
    unsigned char *UtteranceLabels[3];
    /* Current write position in each utterance label buffer. */
    unsigned char *pWriteLab[3];
    /* End of each utterance label buffer. */
    unsigned char *pEndLab[3];
    /* Running recognition frame count. */
    unsigned long FrameCount;
    /* Records detection of terminal silence. */
    unsigned char TSDetected;
    /* Records detection of the speech start. */
    unsigned char StartDetected;
    /* Records detection of a DTMF tone. */
    unsigned char DTMFDetected;
    /* Controls whether the engine runs a second pass. */
    unsigned char DoSecondPass;
    /* Prompt selection used by the recognizer. */
    enum _PROMPTTYPE DoPrompt;
    /* Result produced by the dynamic-programming search. */
    struct _DYNPROG_RESULT *pRes;
    /* Number of ranked results requested. */
    unsigned int NBest;
    /* Sample bytes held between engine calls. */
    unsigned char SampleBuf[0xda];
    /* Number of sample bytes held in the buffer. */
    unsigned short SampleBufSize;
    /* Storage for the signal level reported by the engine. */
    signed short *pSignalLevel;
    /* Storage for the reported begin time. */
    unsigned long *pBeginTime;
    /* Indicates that user-word training is active. */
    unsigned char bUserWordTraining;
    /* Address of the caller's output buffer pointer. */
    void *AddrUserWordBuff;
    /* Address of the caller's output buffer size. */
    unsigned long *AddrUserWordBuffSize;
    /* Number of utterances requested for training. */
    unsigned long NbrUttForTrain;
    /* Index of the current training utterance. */
    unsigned short CurrUttIdx;
    /* Recognition state saved before training. */
    void *pRecogSavedState;
    /* Records rejection of the current training utterance. */
    unsigned char UWUttNotAcceptedByEngine;
    /* Scores and boundaries from the last training result. */
    struct _SCOREUSERWORDRES LastUserWordRes;
    /* Stored utterances used to train a user word. */
    struct _USERWORDUTT UserWordUtt[3];
    /* Distribution identifier used for user-word silence. */
    unsigned short UserWordSilPhenom;
    /* List of counts accumulated for model training. */
    struct _COUNTMATRIX *Counts;
    /* Number of states in the model being trained. */
    unsigned short NbrStates;
    /* Tables used to smooth the training counts. */
    float *ppSmoother[5];
    /* Storage for the smoothed state values. */
    float *pSmoothedState;
    /* Smoothing weight used during model training. */
    float Epsilon;
    /* Maximum distance used during model training. */
    float MaxDist;
    /* Temporary probability storage during model training. */
    float *TmpProbs;
    /* Largest codebook size used for training storage. */
    unsigned short MaxCodebookSize;
    /* Segmentation arrays indexed by transcription and utterance. */
    unsigned short **ppUserwordSegmentation;
    /* Score penalty applied to a recognized word. */
    signed short WordPenalty;
    /* Error status retained from a callback. */
    long CallbackErrLog;
    /* Path entries for the context word tree. */
    struct _ISOWORD_BACKTRACE *pBacktraceArray;
    /* Number of allocated backtrace entries. */
    unsigned long NbrBacktraceAllocated;
    /* Word count limit used in sentence recognition. */
    unsigned long MaxNbrWordsInISOWordSentence;
    /* Controls selection of the optimal parameter set. */
    int bSetOptimalParameters;
    /* Distance calculation callbacks used by the engine. */
    struct _DISTCALCFUNCS DistCalcFuncs;
    /* Speech-unit lexicon for the user-word language. */
    unsigned short *pUserWordLngSpeechUnitLex;
    /* First distribution index for each user-word speech unit. */
    unsigned long *pFirstPhenomOfUserWordSpeechUnit;
    /* Number of speech units in the user-word lexicon. */
    unsigned short NbrUserWordSpeechUnits;
    /* Allocated distribution entries for user words. */
    unsigned long NbrUserWordPhenomsAllocated;
};

/* User-word speech-unit lexicon and its first phoneme indices. */
struct ISOWORD_USERWORD_LNG {
    /* Speech-unit lexicon for the user-word language. */
    u16 *pUserWordLngSpeechUnitLex;
    /* First distribution index for each user-word speech unit. */
    u32 *pFirstPhenomOfUserWordSpeechUnit;
};

struct _ASRSPI_CONTEXT;

/* Releases the engine's active context data. */
void FreeCtxInfo(struct _ENGINE *pEngine);

/* Builds a context tree with the context's default word list. */
unsigned char CreateCtxTree(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx);

/* Estimates the dynamic-programming accuracy needed for the context tree. */
unsigned long CalcAccuracyCtxTree(struct _ENGINE *pEngine);

/* Queues prerecorded labels for recognition. */
unsigned char SendLabelsToEngine(struct _ENGINE *pEngine, const unsigned char *pBlockIDs,
    unsigned int UttIdx);

/* Queues a frame range from one recorded utterance. */
unsigned char SendPartOfLabelsToEngine(struct _ENGINE *pEngine, const unsigned char *pBlockIDs,
    unsigned int UttIdx, unsigned int StartIdx, unsigned int EndIdx);

/* Applies a training utterance acceptance decision. */
unsigned char userwordAcknowledge(struct _ENGINE *pEngine, unsigned long Decision);

/* Requests the current microphone gain from the client. */
unsigned char cbAskCurrentGain(struct _TOS *pTos);

/* Stops user-word training and clears its output buffers. */
unsigned char cbCancelUserWordTraining(struct _TOS *pTos);

/* Saves or reports the first-pass recognition result. */
unsigned char cbDoSecondPassIfEnabled(
     struct _TOS *pTos, long BlockID, long ControlID, unsigned long CParam1, unsigned long CParam2);

/* Trains the new word from the saved utterances. */
unsigned char cbDoUserWordTraining(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, void *CParam1,
    unsigned long CParam2);

/* Passes the requested microphone gain to the client. */
unsigned char cbGainRequest(
     struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1);

/* Disables the active gender and selects the remaining gender mask. */
unsigned char cbGenderDisable(
     struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Moves from the final accepted utterance to confirmation. */
unsigned char cbLastUtteranceAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2);

/* Resets acquisition for the next training utterance. */
unsigned char cbNewUserWordUtterance(struct _TOS *pTos);

/* Reports an empty result after recognition. */
unsigned char cbNothingRecognised(struct _TOS *pTos);

/* Saves the latest scores for a training utterance. */
unsigned long cbScoreUserWordResultAvailable(
    struct _TOS *pTos, unsigned char BlockId, unsigned long ControlId, void *CParam1,
    unsigned long CParam2);

/* Reports an empty result after the spelling search. */
unsigned char cbSpellNothingRecognised(struct _TOS *pTos);

/* Saves the spelling search result for subsequent processing. */
unsigned char cbSpellRecognised(
     struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Reports the elapsed time at the start of speech. */
unsigned char cbStartDetected(
     struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Reports an abnormal acoustic condition to the client. */
unsigned char cbTOSAbnormalCond(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Converts a decoder result and delivers it to the client. */
unsigned char cbTOSRecognised(
     struct _TOS *pTos, unsigned char BlockID, long ControlID, unsigned long CParam1);

/* Reports terminal silence with the recorded start offset. */
unsigned char cbTSAfterStartDetected(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Reports terminal silence and clears the active block selection. */
unsigned char cbTSDetected(
    struct _TOS *pTos, unsigned char BlockID, unsigned char ControlID, unsigned long CParam1,
    unsigned long CParam2);

/* Saves the phonemes and duration of the training utterance. */
unsigned long cbUserWordResultAvailable(
    struct _TOS *pTos, unsigned char BlockId, unsigned long ControlId, void *CParam1,
    unsigned long CParam2);

/* Advances to the next utterance after a usable recording. */
unsigned char cbUtteranceAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2);

/* Clears a rejected recording and prompts for another one. */
unsigned char cbUtteranceNotAccepted(
    struct _TOS *pTos, unsigned char BlockId, unsigned char ControlId, unsigned long CParam1,
    unsigned long CParam2);

/* Starts label delivery through the recognizer's input queue. */
u8 DoLabelStart(struct _ENGINE *pEngine);

/* Ends the current label sequence. */
u8 DoLabelStop();

/* Returns the isolated-word dictionary's active word IDs and their count. */
int GetIsolatedWordDpActiveWords(struct _ENGINE *pEngine, unsigned long **ppidWords,
                                 unsigned long *pNbrWords);

/* Rebuilds the isolated-word tree using the supplied word IDs. */
int SetIsolatedWordDpActiveWords(struct _ENGINE *pEngine, u32 *pidWords, u32 NbrWords);

/* Allocates the context used by extra-event recognition. */
extern u8 CreateExEvCtxInfo(struct _ENGINE *pEngine);

/* Builds the engine's lookup data for the active context. */
extern u8 CreateCtxInfo(struct _ENGINE *pEngine, struct _ASRSPI_CONTEXT *pCtx);

/* Returns the recognition accuracy required by the active context. */
extern u32 CalcAccuracyCtxInfo(struct _ENGINE *pEngine);

/* Reports the isolated-word result through the engine callback. */
unsigned char cbIsoWordRecognised(
    struct _TOS *pTos, long BlockID, long ControlID, unsigned long CParam1, unsigned long CParam2);

/* Exports the session records held by the engine. */
u8 casrGetSessionData(struct _ENGINE *pEngine, void *ppBuf, unsigned long *pSize);

/* Converts the difference between sentence and garbage scores into confidence. */
long CalcSentenceConfidence(unsigned long SentenceDuration, unsigned long SentenceScore,
                            unsigned long SentenceGarbage);

/* Selects the acquisition queue and scheduler blocks for the input format. */
u8 casrInitSampleFormat(struct _ENGINE *pEngine, enum _CSAMPLEFORMAT SampleFormat);

/* Releases the active tree and word buffers owned by a speech context. */
void FreeTree(struct _ENGINE *pEngine);

/* Compares word IDs by value. */
int CompareIDs(const void *A, const void *B);

#endif
