/* CASR states, recognition callbacks and engine operations. */
#ifndef GSSDK_CASR_H
#define GSSDK_CASR_H

#include "types.h"

/* Recognition engine states. */
enum _CASRSTATE {
    CASR_BOOT = 0x0,
    CASR_DATAREADY = 0x1,
    CASR_IDLE = 0x2,
    CASR_SLEEP = 0x3,
    CASR_RUN = 0x4,
    CASR_RECOVER = 0x5,
    CASR_SECONDPASS = 0x6,
    CASR_SPELLSLEEP = 0x7,
    CASR_SPELLRUN = 0x8,
    CASR_SPELLRECOVER = 0x9,
    CASR_SPELLSECONDPASS = 0xa,
    CASR_ISOWORDSLEEP = 0xb,
    CASR_ISOWORDRUN = 0xc,
    CASR_ISOWORDRECOVER = 0xd,
    CASR_ISOWORDSECONDPASS = 0xe,
    CASR_PROMPT = 0xf,
    CASR_RECORDSLEEP = 0x10,
    CASR_RECORD = 0x11,
    CASR_ACCEPT = 0x12,
    CASR_CONFIRM = 0x13,
    CASR_UWTRAINED = 0x14,
};

/* Formats accepted by the acquisition interface. */
enum _CSAMPLEFORMAT {
    PCM_16_11KHZ = 0x0,
    UTTFORMAT_CEPSTRA = 0x1,
    UTTFORMAT_LABELS = 0x2,
    UTTFORMAT_LABELSRAW = 0x3,
    UTTFORMAT_UNKNOWN = 0x4,
};

/* Prompts issued during recognition and training. */
enum _PROMPTTYPE {
    PROMPTTYPE_START = 0x0,
    PROMPTTYPE_ACCEPT = 0x1,
    PROMPTTYPE_CONFIRM = 0x2,
    PROMPTTYPE_TRAINEND = 0x3,
    PROMPTTYPE_NOPROMPT = 0x4,
};

/* Abnormal input conditions reported by the recognizer. */
enum _ABNORMCOND {
    BADSNR = 0x0,
    OVERLOAD = 0x1,
    TOOQUIET = 0x2,
    NOSIGNAL = 0x3,
    GARBLEDSOUND = 0x4,
    POORMIC = 0x5,
};

/* Outcomes reported with recognition results. */
enum _CASRRESULTTYPE {
    CASRRESULTTYPE_FINAL = 0x0,
    CASRRESULTTYPE_PREMATURE = 0x1,
    CASRRESULTTYPE_TIMEOUT = 0x2,
    CASRRESULTTYPE_DTMF = 0x3,
    CASRRESULTTYPE_REJECTED = 0x4,
};

/* User data passed to the recognition callbacks. */
struct _USERDATA {
    /* User data supplied to the SPI. */
    unsigned long dwSPIUserData;
    /* User data supplied to the engine. */
    unsigned long dwEngineUserData;
};

/* Start and end times of a word or sentence in a recognition result. */
struct _TIMEINFO {
    unsigned long TimeBegin; /* Start time of the result. */
    unsigned long TimeEnd; /* End time of the result. */
};

/* One candidate for a recognized word. */
struct _ALTERNATIVE {
    int iConf; /* Confidence of this candidate. */
    long lScore; /* Accumulated score of this candidate. */
    unsigned long idWord; /* Word ID in the active context. */
};

/* Ranked alternatives and timing of a recognized word. */
struct _RECWORD {
    int iNbrAlternatives; /* Number of alternatives for this word. */
    struct _TIMEINFO TimeInfo; /* Time interval of the word. */
    struct _ALTERNATIVE *pAlternatives; /* Array of candidate words. */
};

/* One recognition hypothesis with timing, scores and words. */
struct _SENTENCE {
    int iNbrWords; /* Number of words in the hypothesis. */
    int iConf; /* Confidence of the whole hypothesis. */
    long lScore; /* Accumulated score of the whole hypothesis. */
    struct _TIMEINFO TimeInfo; /* Time interval of the sentence. */
    struct _RECWORD *pWords; /* Recognized words in the hypothesis. */
};

/* Recognition result and its candidate sentences. */
struct _CASRRESULT {
    /* Number of candidate sentences. */
    int iNbr;
    /* Candidate sentences returned by the recognizer. */
    struct _SENTENCE *pSentences;
    /* Outcome associated with these candidates. */
    enum _CASRRESULTTYPE Type;
};

/* Callbacks invoked during recognition and training. */
struct _CRECOGCALLBACKS {
    /* Reports terminal silence detection. */
    long (*cbTSDetected)(unsigned long, struct _USERDATA *);
    /* Reports the start of speech. */
    long (*cbStartDetected)(unsigned long, struct _USERDATA *);
    /* Delivers the recognition result. */
    long (*cbResult)(struct _CASRRESULT *, struct _USERDATA *);
    /* Reports an engine state change. */
    long (*cbState)(enum _CASRSTATE, unsigned long, struct _USERDATA *);
    /* Delivers a training prompt. */
    long (*cbTrain)(enum _PROMPTTYPE, struct _USERDATA *);
    /* Reports an abnormal input condition. */
    long (*cbAbnorm)(enum _ABNORMCOND, unsigned long, struct _USERDATA *);
    /* Reports an automatic gain adjustment. */
    long (*cbAgc)(unsigned short, struct _USERDATA *);
    /* Queries the current input gain. */
    long (*cbAskCurrentGain)(struct _USERDATA *);
};

struct HCASR__;
struct HCTX__;
struct HCDATA__;
struct _ENGINE;
struct _SCOREUWBACKTRACE;

/* Common header of serialized CASR data and context resources. */
struct _DATA {
    /* Resource format tag. */
    unsigned long DataType;
    /* Declared resource size. */
    unsigned long Size;
    /* Resource version text stored in the serialized header. */
    char VersionInfo[0x80];
};

/* Returns the last error recorded by the engine. */
unsigned long casrGetLastError(struct HCASR__ *hcAsr);

/* Creates the recognition engine with the caller's callbacks and heap. */
struct HCASR__ *casrOpen(void *hHeap,
    const struct _CRECOGCALLBACKS *pRecogCallBacks, unsigned long dwUserData);

/* Releases the recognition engine. */
int casrClose(struct HCASR__ *hcAsr);

/* Starts acquisition for the current recognition context. */
int casrStart(struct HCASR__ *hcAsr);

/* Stops acquisition and returns the engine to an idle state. */
int casrStop(struct HCASR__ *hcAsr);

/* Sets the word IDs available to recognition. */
int casrSetActiveWords(struct HCASR__ *hcAsr, unsigned long *pidWords, unsigned long NbrWords);

/* Returns the current active word list and its length. */
int casrGetActiveWords(struct HCASR__ *hcAsr, unsigned long **ppidWords, unsigned long *pNbrWords);

/* Reads the current value of the selected recognition parameter. */
int casrGetParam(struct HCASR__ *hcAsr, unsigned long ParamID, unsigned long *pParamValue);

/* Sets the parameter selected by ParamID. */
int casrSetParam(struct HCASR__ *hcAsr, unsigned long ParamID, unsigned long ParamValue);

/* Controls whether the engine queues the current utterance's labels. */
unsigned char casrEnableLabelInQueue(struct _ENGINE *pEngine, unsigned char Enable);

/* Initializes the segmentation records for user-word training. */
void casrInitSegmentation(struct _ENGINE *pEngine);

/* Locates the segmentation for a transcription and utterance. */
unsigned short *casrGetSegmStart(struct _ENGINE *pEngine, unsigned long TransCnt,
                                 unsigned long UttCnt);

/* Stores the segmentation from the scored utterance's backtrace. */
void casrStoreSegmentation(struct _ENGINE *pEngine, unsigned long TransCnt, unsigned long UttCnt,
                           struct _SCOREUWBACKTRACE *pBackTrace);

/* Passes the selected input gain to the engine. */
int casrGainSet(struct HCASR__ *hcAsr, unsigned short Gain);

/* Returns the engine's current recognition state. */
int casrGetState(struct HCASR__ *hcAsr, enum _CASRSTATE *pState);

/* Returns the measured input signal level. */
int casrGetSignalLevel(struct HCASR__ *hcAsr, signed short *pCasrLevel);

/* Returns the measured signal-to-noise ratio. */
int casrGetSNR(struct HCASR__ *hcAsr, signed short *pSNR);

/* Imports a serialized engine data record. */
struct HCDATA__ *casrImportData(void *hHeap, struct _DATA *pData);

/* Allocates a serialized record and writes its pointer through ppBuf. */
int casrExportData(struct HCASR__ *hcAsr, unsigned long DataType, void *ppBuf,
    unsigned long *pSize);

/* Activates data previously imported by the engine. */
int casrActivateData(struct HCASR__ *hcAsr, struct HCDATA__ *hcData);

/* Releases an imported engine data handle. */
int casrCloseData(struct HCDATA__ *hcData);

/* Imports a serialized recognition context. */
struct HCTX__ *casrImportContext(void *hHeap, struct _DATA *pCtx);

/* Releases an imported context handle. */
int casrCloseContext(struct HCTX__ *hCtx);

/* Activates a context and optionally selects its optimal parameters. */
int casrActivateContext(struct HCASR__ *hcAsr, struct HCTX__ *hCtx, int bSetOptimalParameters);

/* Feeds a buffer of samples or features to the engine. */
int casrAcquisition(struct HCASR__ *hcAsr, enum _CSAMPLEFORMAT SampleFormat,
    void *pSampleBuf, unsigned long dwBufSize);

/* Passes a training or recognition decision back to the engine. */
int casrAcknowledge(struct HCASR__ *hcAsr, unsigned long Decision);

/* Returns the error recorded by a recognition callback. */
int casrGetCallbackErr(struct HCASR__ *hcAsr, int *pCallbackErr);

#endif
