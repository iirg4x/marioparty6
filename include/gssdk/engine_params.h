/* Parameter records for the recognizer and its processing blocks. */
#ifndef GSSDK_ENGINE_PARAMS_H
#define GSSDK_ENGINE_PARAMS_H

/* Engine-wide timing, recognition and user-word settings. */
struct _ARCHIPARAM {
    /* Recognition timeout setting. */
    unsigned long TimeOut;
    /* Enables recognition rescoring in the second pass. */
    unsigned long SecondPassEnable;
    /* Minimum user-word speech duration. */
    unsigned long UWMinSpeechDur;
    /* Threshold for the highest training confidence. */
    long UWConfThres1;
    /* Threshold for the second-highest training confidence. */
    long UWConfThres2;
    /* Enables recognition of extra events. */
    unsigned long ExtraEventEnable;
};

/* Selects the gender models available to the recognizer. */
struct _GENDERPARAM {
    /* Mask of gender models available to the recognizer. */
    unsigned long EnabledGendersMask;
};

/* Distance penalties used to score unrestricted speech and rejection. */
struct _DISTPARAM {
    /* Distance added for unrestricted speech. */
    signed short AnySpeechPenalty;
    /* Distance added for rejection. */
    signed short RejectionPenalty;
};

/* Dynamic-programming and result limits. */
struct _DYNPROG_PARAM {
    /* Accuracy setting for dynamic-programming recognition. */
    unsigned long Accuracy;
    /* Configured maximum number of ranked results. */
    unsigned long MaxNBest;
    /* Enables terminal-silence detection in the recognizer. */
    unsigned long TSEnable;
    /* Terminal-silence duration converted to frames by the recognizer. */
    unsigned long TSCount;
    /* Controls whether recognition can report a preliminary result. */
    unsigned long PremRes;
};

/* Control payload shared between the engine and the voice-activity detector. */
typedef struct {
    /* Controls the voice-activity detector. */
    unsigned long On;
    /* Far-talk setting passed to the voice-activity detector. */
    unsigned long FarTalk;
    /* Timeout setting passed to the voice-activity detector. */
    unsigned long TimeOut;
} VadControlParameters;

/* Control payload shared between the engine and the speech trigger. */
typedef struct {
    /* Enables detection of the start of speech. */
    unsigned long DoStartDetection;
    /* Enables detection of the end of speech. */
    unsigned long DoStopDetection;
    /* Terminal-silence duration passed to the trigger. */
    unsigned long TSCount;
    /* Sensitivity setting passed to the trigger. */
    unsigned long Sensitivity;
    /* Minimum speech duration passed to the trigger. */
    unsigned long MinSpeechDur;
    /* Absolute threshold passed to the trigger. */
    long AbsTriggThreshold;
} TriggerControlParameters;

/* Parameters used to configure the recognition architecture and its blocks. */
struct _PARAMLIST {
    /* Settings shared across recognition blocks. */
    struct _ARCHIPARAM ArchiParam;
    /* Settings for the voice-activity detector. */
    VadControlParameters VadParam;
    /* Settings for the speech trigger. */
    TriggerControlParameters TriggerParam;
    /* Model selection settings. */
    struct _GENDERPARAM GenderParam;
    /* Distance penalties applied by the recognition engine. */
    struct _DISTPARAM DistParam;
    /* Settings for the dynamic-programming search. */
    struct _DYNPROG_PARAM DynProgParam;
};

#endif
