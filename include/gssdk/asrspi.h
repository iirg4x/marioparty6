/* SPI engine declarations shared by the recognizer and game speech API. */
#ifndef GSSDK_ASRSPI_H
#define GSSDK_ASRSPI_H

#include "gssdk/casr.h"
#include "gssdk/safeh.h"

/* Abnormal input conditions delivered by the SPI callback. */
enum _ASRBASE_ABNORMCOND {
    ASRBASE_BADSNR = 0x0,
    ASRBASE_OVERLOAD = 0x1,
    ASRBASE_TOOQUIET = 0x2,
    ASRBASE_NOSIGNAL = 0x3,
    ASRBASE_GARBLEDSOUND = 0x4,
    ASRBASE_POORMIC = 0x5,
};

/* Parameter IDs accepted by the SPI interface. */
enum _ASRBASE_PARAMID {
    ASRBASE_PARAM_TS_ENABLE = 0x0,
    ASRBASE_PARAM_START_ENABLE = 0x1,
    ASRBASE_PARAM_ENABLEPREMRES = 0x2,
    ASRBASE_PARAM_TIMEOUT = 0x3,
    ASRBASE_PARAM_SECONDPASS_ENABLE = 0x4,
    ASRBASE_PARAM_SPEECHTHRESHOLD = 0x5,
    ASRBASE_PARAM_MINSPEECH = 0x6,
    ASRBASE_PARAM_MAXNBEST = 0x7,
    ASRBASE_PARAM_FARTALK = 0x8,
    ASRBASE_PARAM_AGCON = 0x9,
    ASRBASE_PARAM_SELECTGENDER = 0xa,
    ASRBASE_PARAM_ABSOLUTE_THRESHOLD = 0xb,
    ASRBASE_PARAM_USETREEDP = 0xc,
    ASRBASE_CTXPARAM_TS = 0xd,
    ASRBASE_CTXPARAM_ACCURACY = 0xe,
    ASRBASE_CTXPARAM_REJECTION = 0xf,
    ASRBASE_CTXPARAM_GARBAGE = 0x10,
    ASRBASE_PARAM_SENSITIVITY = 0x11,
    ASRBASE_PARAM_EXTRAEVENT_ENABLE = 0x12,
};

/* One SPI parameter ID and its value. */
struct _ASRBASE_PARAM {
    /* Selects the SPI parameter to change. */
    enum _ASRBASE_PARAMID ID;
    /* Value passed to the engine for this parameter. */
    unsigned long Value;
};

/* Data formats selected for session import and export. */
enum _ASRBASE_DATATYPE {
    ASRBASE_SESSIONDATA = 0x1,
    ASRBASE_SESSIONDATA_FIXED = 0x2,
};

/* Recognition outcomes sent to the client. */
enum _ASRBASE_RESULTTYPE {
    ASRBASE_RESULTTYPE_FINAL = 0x0,
    ASRBASE_RESULTTYPE_PREMATURE = 0x1,
    ASRBASE_RESULTTYPE_TIMEOUT = 0x2,
    ASRBASE_RESULTTYPE_REJECTED = 0x3,
};

/* States used to validate SPI operations. */
enum _ASRSPI_STATE {
    ASRBASE_BOOT = 0,
    ASRBASE_LANGREADY = 1,
    ASRBASE_IDLE = 2,
    ASRBASE_RECOGNIZING = 3,
    ASRBASE_RECORDING = 4,
    ASRBASE_ACCEPT = 5,
    ASRBASE_USWTRAINED = 6,
};

/* Outcomes of recording a user-word utterance. */
enum _ASRBASE_RECORDINGTYPE {
    ASRBASE_RECORDINGTYPE_ACCEPTED = 0,
    ASRBASE_RECORDINGTYPE_REJECTED_BY_USER = 1,
    ASRBASE_RECORDINGTYPE_TOO_SHORT = 2,
    ASRBASE_RECORDINGTYPE_INCONSISTENT = 3,
};

/* Sample and feature formats accepted by the engine. */
enum _ASRBASE_SAMPLEFORMAT {
    ASRBASE_PCM_16_11KHZ = 0x0,
    ASRBASE_PCM_16_8KHZ = 0x1,
    ASRBASE_UTTFORMAT_CEPSTRA_11KHZ = 0x2,
    ASRBASE_UTTFORMAT_CEPSTRA_8KHZ = 0x3,
    ASRBASE_UTTFORMAT_LABELS_11KHZ = 0x4,
    ASRBASE_UTTFORMAT_LABELS_8KHZ = 0x5,
    ASRBASE_UTTFORMAT_UNKNOWN = 0x6,
};

/* Beginning and end times recorded for a recognition span. */
struct _ASRBASE_TIMEINFO {
    /* Beginning of the recognized span. */
    unsigned long TimeBegin;
    /* End of the recognized span. */
    unsigned long TimeEnd;
};

/* A recognition alternative for one word. */
struct _ASRBASE_ALTERNATIVE {
    /* Confidence of this word alternative. */
    int iConf;
    /* Recognition score for this word alternative. */
    long lScore;
    /* Word ID assigned by the recognition context. */
    unsigned long idWord;
};

/* A recognized word span and the alternatives for that span. */
struct _ASRBASE_RECWORD {
    /* Number of alternatives in pAlternatives. */
    int iNbrAlternatives;
    /* Time span shared by the word alternatives. */
    struct _ASRBASE_TIMEINFO TimeInfo;
    /* Word IDs and scores for this span. */
    struct _ASRBASE_ALTERNATIVE *pAlternatives;
};

/* A candidate sentence with its scores, time span and recognized words. */
struct _ASRBASE_SENTENCE {
    /* Number of word spans in pWords. */
    int iNbrWords;
    /* Confidence of this candidate sentence. */
    int iConf;
    /* Recognition score for this sentence. */
    long lScore;
    /* Time span covered by this candidate sentence. */
    struct _ASRBASE_TIMEINFO TimeInfo;
    /* Recognized spans in sentence order. */
    struct _ASRBASE_RECWORD *pWords;
};

/* Recognition outcome and its list of candidate sentences. */
struct _ASRBASE_RESULT {
    /* Outcome sent to the recognition callback. */
    enum _ASRBASE_RESULTTYPE Type;
    /* Number of candidates in pSentences. */
    int iNbr;
    /* Candidate sentences returned for this outcome. */
    struct _ASRBASE_SENTENCE *pSentences;
};

/* Client callbacks for microphone and signal events. */
struct _ASRBASE_SIGNALCALLBACKS {
    /* Reports an abnormal input condition with a time and the registered client data. */
    long (*cbAbnorm)(enum _ASRBASE_ABNORMCOND, unsigned long, unsigned long);
    /* Reports the time of speech detection and the registered client data. */
    long (*cbSpeechDetected)(unsigned long, unsigned long);
    /* Reports the time of terminal silence detection and the registered client data. */
    long (*cbTsDetected)(unsigned long, unsigned long);
    /* Reports the gain requested by AGC. */
    long (*cbAgc)(unsigned short, unsigned long);
    /* Reports the measured input energy. */
    long (*cbEnergyLevel)(int, unsigned long);
    /* Reports the measured signal-to-noise ratio. */
    long (*cbSNRLevel)(int, unsigned long);
};

/* Client callback registrations supplied when installing recognizer callbacks. */
struct _ASRSPI_RECOGCALLBACKS {
    /* Delivers a recognition result with the registered client data. */
    long (*cbResult)(struct _ASRBASE_RESULT *, unsigned long);
    /* Optional callbacks for events from the input signal. */
    struct _ASRBASE_SIGNALCALLBACKS *pSignalCallbacks;
};

/* Allocates and opens a recognition engine for the requested sample format. */
long asrspi_OpenEngine(SafeHandle hRsrc, enum _ASRBASE_SAMPLEFORMAT SampleFormat, void *phApiAsr);

/* Closes the recognition engine and releases its resources. */
long asrspi_CloseEngine(void *hAsrSpi);

/* Reads the current recognition state from the engine. */
long asrspi_GetState(void *hAsrSpi, enum _ASRSPI_STATE *pState);

/* Imports and activates the requested recognition language. */
long asrspi_ActivateLng(void *hAsrSpi, void *pAsrSpiLng);

/* Imports and activates a recognition context and optionally sets its preferred parameters. */
long asrspi_ActivateCtx(void *hAsrSpi, void *pAsrSpiCtx, int bSetOptimalParameters);

/* Applies a list of recognition parameters to the engine. */
long asrspi_SetParamList(void *hAsrSpi, const struct _ASRBASE_PARAM *pParamList, unsigned int Nbr);

/* Adds the requested words to the active set or activates every word. */
long asrspi_ActivateWords(void *hAsrSpi, unsigned long *pWordIDs, unsigned int cNewWords);

/* Starts recognition from idle and registers the client signal and result callbacks. */
long asrspi_Start(
    void *hAsrSpi, unsigned int Interval, struct _ASRSPI_RECOGCALLBACKS *pCallBacks,
    unsigned long dwUserData);

/* Stops recognition and checks whether the recognizer reached idle after an error. */
long asrspi_Stop(void *hAsrSpi);

/* Feeds a sample buffer to the recognizer and reports when acquisition finishes. */
long asrspi_Acquisition(void *hAsrSpi, void *pSampleBuf, unsigned long dwBufSize, int *pFinished);

/* Exports engine session data through the SPI handle into the caller's buffer. */
long asrspi_ExportSessionData(void *hAsrSpi, enum _ASRBASE_DATATYPE DataType, void *pBuf,
    unsigned long *pSize);

/* Imports and activates saved recognition session data. */
long asrspi_ActivateSessionData(void *hAsrSpi, void *pBuf);

/* Converts an abnormal condition and reports it to the SPI client. */
long ASRSPICBABNORM(enum _ABNORMCOND AbnormalCondition, unsigned long Time,
    struct _USERDATA *pUserData);

/* Delivers a microphone gain change to the SPI client. */
long ASRSPICBAGC(unsigned short NewGain, struct _USERDATA *pUserData);

/* Requests the current microphone gain from the SPI client. */
long ASRSPICBASKCURRENTGAIN(struct _USERDATA *pUserData);

/* Delivers a user-word training prompt to the SPI client. */
long ASRSPICBTRAIN(enum _PROMPTTYPE Type, struct _USERDATA *pUserData);

/* Reports terminal silence to the SPI client. */
long ASRSPICBTS(unsigned long Time, struct _USERDATA *pUserData);

/* Reports the start of speech to the SPI client. */
long ASRSPICBSTARTDET(unsigned long Time, struct _USERDATA *pUserData);

/* Reports recognition results to the SPI client. */
long ASRSPICBRESULT(struct _CASRRESULT *pSpiResult, struct _USERDATA *pUserData);

#endif
