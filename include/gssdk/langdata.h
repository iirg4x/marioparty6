/* Packed language data and its recognizer accessors. */
#ifndef GSSDK_LANGDATA_H
#define GSSDK_LANGDATA_H

#include "types.h"

/* Fixed header of a version-two recognition language resource. */
struct _LANGUAGEINFO {
    u32 DataType; /* Packed resource type identifier. */
    u32 Size; /* Stored size of the language resource in bytes. */
    u8 VersionInfo[0x80]; /* Version information stored in the resource header. */
    u32 NbrCodeBook; /* Number of codebooks. */
    u32 NbrState; /* Total number of recognition states. */
    u32 NbrNonErgodicPhenomes; /* Number of nonergodic phonemes. */
    u32 NbrErgodicStates; /* Number of entries in the ergodic state table. */
    u32 NbrStateErgodicStates; /* State count associated with the ergodic state entries. */
    u32 NbrErgodicPhenomes; /* Number of ergodic phonemes. */
    u32 NbrStateErgodicPhenomes; /* State count associated with the ergodic phonemes. */
    u32 SilencePhenome; /* Silence phoneme index for recognition. */
    u32 UWSilencePhenome; /* Silence phoneme index for user words. */
    u32 AdaptSilState; /* State index used for silence adaptation. */
    u32 SingleWordGarbagePhenome; /* Garbage phoneme index for single-word recognition. */
    u32 SentenceGarbagePhenome; /* Garbage phoneme index for sentence recognition. */
    u32 AnySpeechGarbagePhenome; /* Garbage phoneme index for unrestricted speech. */
    u32 NbrSpeechUnit; /* Number of speech units. */
    u32 NbrSpeechUnitClass; /* Number of speech-unit classes. */
    u32 NbrTones; /* Number of tones. */
    u32 UserWordSpeechUnitClass; /* Speech-unit class for user words. */
    u32 NbrWarpFactors; /* Number of frequency warp factors. */
    u32 NbrTransWord; /* Number of transition words. */
    u32 NbrPhenUserWordTraining; /* Number of phonemes available for user-word training. */
    u32 BitField; /* Flags for optional tables in the resource. */
    s16 RecogWTP; /* Word transition penalty for recognition. */
    s16 SpellingWTP; /* Word transition penalty for spelling. */
};

/* Codebook dimensions, vectors and search indexes used for vector quantization. */
struct _CODEBOOK {
    u8 CodeBookDim; /* Number of dimensions in a codebook vector. */
    u8 FirstCdbSize; /* Number of vectors in the first codebook. */
    u8 NbrInSecSearch; /* Number of second-search indexes per first-codebook vector. */
    u8 SecondCdbSize; /* Number of vectors in the second codebook. */
    u16 CompStart; /* Starting component in the input vector. */
    f32 *pFirstCdb; /* First codebook vectors. */
    u32 *pIndexInSecCdb; /* Indexes for the second-stage codebook search. */
    f32 *pSecondCdb; /* Second codebook vectors. */
};

/* Fixed header before the codebook vectors and search indexes. */
struct _CDBINFO {
    u32 CodeBookDim; /* Number of dimensions in a codebook vector. */
    u32 FirstCdbSize; /* Number of vectors in the first codebook. */
    u32 NbrInSecSearch; /* Number of second-search indexes per first-codebook vector. */
    u32 SecondCdbSize; /* Number of vectors in the second codebook. */
    u32 CompStart; /* Starting component in the input vector. */
    u32 NbrGastone; /* Number of probability entries per codebook vector. */
};

/* Header before the transition-word lexicon. */
struct _TRANSWORDINFO {
    u32 NbrTransPron; /* Number of transition-word pronunciations. */
    u32 NbrPhenInLex; /* Number of phonemes in the transition-word lexicon. */
};

/* Header for additional recognition events and their pronunciation lexicon. */
struct _EXTRAEVENTCTXINFO {
    s32 Leading_NBS_penalty; /* Penalty for a leading NBS event. */
    s32 Trailing_NBS_penalty; /* Penalty for a trailing NBS event. */
    s32 Rejection_NBS_penalty; /* Penalty for a rejection NBS event. */
    s32 Rejection_NBS_path_penalty; /* Path penalty for a rejection NBS event. */
    u32 ExtraEventSilencePhenome; /* Silence phoneme index for extra events. */
    u32 NbrItemsInLexicon; /* Number of items in the extra-event lexicon. */
    u32 NbrOfPronunciations; /* Number of pronunciations in the extra-event lexicon. */
    u32 Leading_NBS_WordIndex; /* Lexicon index for the leading NBS event. */
    u32 Trailing_NBS_WordIndex; /* Lexicon index for the trailing NBS event. */
    u32 Rejection_NBS_WordIndex; /* Lexicon index for the rejection NBS event. */
};

/* One frequency warp entry in the packed language. */
struct _WARPINFO {
    unsigned long GenderMask; /* Gender models supported by this warp factor. */
    float WarpingFactor; /* Frequency warp factor. */
};

/* A packed language resource and the operations used to read it. */
struct _LANGUAGE {
    /* Packed resource read by the language accessors. */
    void *pLang;
    /* Checks whether the resource has the version-two language type. */
    BOOL (*langCheckDataType)(struct _LANGUAGE *pLang);
    /* Reads the stored size of the language resource. */
    u32 (*langGetSize)(struct _LANGUAGE *pLang);
    /* Locates the language resource's version information. */
    u8 *(*langGetVersionInfo)(struct _LANGUAGE *pLang);
    /* Reads the number of codebooks in the language. */
    u32 (*langGetNbrCodeBook)(struct _LANGUAGE *pLang);
    /* Reads the number of states in the language. */
    u32 (*langGetNbrState)(struct _LANGUAGE *pLang);
    /* Computes the total number of ergodic and nonergodic phonemes. */
    u32 (*langGetNbrPhenome)(struct _LANGUAGE *pLang);
    /* Optional count of silence phonemes. This table leaves the slot empty. */
    u32 (*langGetNbrSilPhenome)(struct _LANGUAGE *pLang);
    /* Reads the index of the state used for silence adaptation. */
    u32 (*langGetAdaptSilState)(struct _LANGUAGE *pLang);
    /* Reads the number of ergodic states. */
    u32 (*langGetNbrErgodicStates)(struct _LANGUAGE *pLang);
    /* Reads the state count for the ergodic-state entries. */
    u32 (*langGetNbrStateErgodicStates)(struct _LANGUAGE *pLang);
    /* Reads the number of ergodic phonemes. */
    u32 (*langGetNbrErgodicPhenomes)(struct _LANGUAGE *pLang);
    /* Reads the state count for the ergodic phonemes. */
    u32 (*langGetNbrStateErgodicPhenomes)(struct _LANGUAGE *pLang);
    /* Reads the number of nonergodic phonemes. */
    u32 (*langGetNbrNonErgodicPhenomes)(struct _LANGUAGE *pLang);
    /* Reads the silence phoneme for recognition. */
    u32 (*langGetSilencePhenome)(struct _LANGUAGE *pLang);
    /* Reads the silence phoneme for user words. */
    u32 (*langGetUWSilencePhenome)(struct _LANGUAGE *pLang);
    /* Reads the garbage phoneme for single-word recognition. */
    u32 (*langGetSingleWordGarbagePhenome)(struct _LANGUAGE *pLang);
    /* Reads the garbage phoneme for sentence recognition. */
    u32 (*langGetSentenceGarbagePhenome)(struct _LANGUAGE *pLang);
    /* Reads the garbage phoneme for unrestricted speech. */
    u32 (*langGetAnySpeechGarbagePhenome)(struct _LANGUAGE *pLang);
    /* Reads the number of frequency warp factors. */
    u32 (*langGetNbrWarpFactors)(struct _LANGUAGE *pLang);
    /* Reads the number of speech units in the lexicon. */
    u32 (*langGetNbrSpeechUnit)(struct _LANGUAGE *pLang);
    /* Reads the number of speech-unit classes. */
    u32 (*langGetNbrSpeechUnitClass)(struct _LANGUAGE *pLang);
    /* Returns the number of tones in the recognition language. */
    u32 (*langGetNbrTones)(struct _LANGUAGE *pLang);
    /* Reads the speech-unit class reserved for user words. */
    u32 (*langGetUserWordSpeechUnitClass)(struct _LANGUAGE *pLang);
    /* Reads the number of transition words. */
    u32 (*langGetNbrTransWord)(struct _LANGUAGE *pLang);
    /* Returns the number of phonemes available for user-word training. */
    u32 (*langGetNbrPhenUserWordTraining)(struct _LANGUAGE *pLang);
    /* Reads the recognition word transition penalty. */
    s16 (*langGetRecogWtp)(struct _LANGUAGE *pLang);
    /* Reads the spelling word transition penalty. */
    s16 (*langGetSpellingWtp)(struct _LANGUAGE *pLang);
    /* Reads the codebook vector dimension. */
    u32 (*cdbGetCodeBookDim)(struct _CDBINFO *pCdb);
    /* Reads the number of vectors in the first codebook. */
    u32 (*cdbGetFirstCdbSize)(struct _CDBINFO *pCdb);
    /* Reads the number of second-search indexes per first-codebook vector. */
    u32 (*cdbGetNbrInSecSearch)(struct _CDBINFO *pCdb);
    /* Reads the number of vectors in the second codebook. */
    u32 (*cdbGetSecondCdbSize)(struct _CDBINFO *pCdb);
    /* Reads the starting dimension for compressed codebook data. */
    u32 (*cdbGetCompStart)(struct _CDBINFO *pCdb);
    /* Reads the number of codebook probability entries per vector. */
    u32 (*cdbGetNbrGastone)(struct _CDBINFO *pCdb);
    /* Locates the frequency warp factors at the start of the language payload. */
    struct _WARPINFO *(*langGetpWarpFactors)(struct _LANGUAGE *pLang);
    /* Locates the Gastone table following the language's warp factors. */
    u16 *(*langGetpGastone)(struct _LANGUAGE *pLang);
    /* Optional list of silence phonemes. This table leaves the slot empty. */
    u16 *(*langGetpSilPhenome)(struct _LANGUAGE *pLang);
    /* Reads the silence phoneme as a sixteen-bit index. */
    u16 (*langGetSilPhenome)(struct _LANGUAGE *pLang);
    /* Locates the ergodic state table in the language data. */
    u16 *(*langGetpErgodicStates)(struct _LANGUAGE *pLang);
    /* Locates the ergodic phoneme table in the language data. */
    u16 *(*langGetpErgodicPhenomes)(struct _LANGUAGE *pLang);
    /* Locates the ergodic penalty table in the language data. */
    s16 *(*langGetpErgodicPenalty)(struct _LANGUAGE *pLang);
    /* Locates the requested codebook by stepping past the preceding codebooks. */
    void *(*langGetpCodeBook)(struct _LANGUAGE *pLang, u32 Cdb);
    /* Locates the speech unit table in the language data. */
    u32 *(*langGetpSpeechUnit)(struct _LANGUAGE *pLang);
    /* Locates the transition-word data in the language record. */
    void *(*langGetpTransWord)(struct _LANGUAGE *pLang);
    /* Computes the space occupied by the speech unit data. */
    u32 (*langGetSizeSpeechUnits)(struct _LANGUAGE *pLang);
    /* Optional context-dependent phoneme lookup. This table leaves the slot empty. */
    void (*langGetCDPhenoms)(struct _LANGUAGE *pLang, u32 *pNbrCDPhenoms, u16 **ppCDPhenoms);
    /* Locates the phoneme table for user-word training. */
    u16 *(*langGetpPhenUserWordTraining)(struct _LANGUAGE *pLang);
    /* Locates the stored dimensions of the tone conversion matrix. */
    u16 *(*langGetpDimensionsOfToneConversionMatrix)(struct _LANGUAGE *pLang);
    /* Locates the offset for finals after the stored tone-matrix dimensions. */
    u16 *(*langGetOffsetForFinals)(struct _LANGUAGE *pLang);
    /* Locates the tone conversion matrix after the finals offset. */
    u16 *(*langGetpToneConversionMatrix)(struct _LANGUAGE *pLang);
    /* Locates the context data used for extra recognition events. */
    void *(*langGetpExtraEventContext)(struct _LANGUAGE *pLang);
    /* Tests whether the phoneme lies in the normal phoneme range. */
    int (*langIsNormalPhenome)(struct _LANGUAGE *pLang, u16 PhenomeNbr);
    /* Tests which of the requested language flags are set. */
    BOOL (*langCheckBitField)(struct _LANGUAGE *pLang, u32 bit);
    /* Computes a phoneme's state count from consecutive table offsets. */
    u16 (*GetNbrStatesInPhenome)(u16 *pPhenToStat, u16 PhenomeNbr);
    /* Finds the states associated with a phoneme in the conversion table. */
    u16 (*ConvPhenomesToStates)(u16 *pPhenToStat, u16 PhenomeNbr, u16 **ppStates);
    /* Computes the size of a codebook including its matrices and optional data. */
    u32 (*cdbGetCodeBookSize)(struct _LANGUAGE *pLang, struct _CDBINFO *pCdb);
    /* Computes the size of the codebook's probability matrix. */
    u32 (*cdbGetProbMatrixSize)(struct _LANGUAGE *pLang, struct _CDBINFO *pCdb);
    /* Computes the byte size of the square codebook smoothing matrix. */
    u32 (*cdbGetSmoothMatrixSize)(struct _CDBINFO *pCdb);
    /* Locates the first codebook vectors immediately after the header. */
    char *(*cdbGetpFirstCdb)(void *pCdb);
    /* Locates the second-search index table after the first codebook vectors. */
    char *(*cdbGetpIndexInSecCdb)(void *pCdb);
    /* Locates the second codebook after the first vectors and their index table. */
    char *(*cdbGetpSecondCdb)(void *pCdb);
    /* Locates the probability matrix after the codebook vectors and metadata. */
    char *(*cdbGetpProbMatrix)(void *pCdb);
    /* Locates the smoothing matrix when the language format includes it. */
    char *(*cdbGetpSmoothMatrix)(struct _LANGUAGE *pLang, void *pCdb);
    /* Computes the size of the packed transition-word data. */
    u32 (*transwGetSize)(struct _LANGUAGE *pLang, struct _TRANSWORDINFO *pTransWord);
    /* Locates the transition-word lexicon after its header. */
    u16 *(*transwGetpTransWordLex)(struct _TRANSWORDINFO *pTransWord);
    /* Locates the table of transition-word offsets. */
    u16 *(*transwGetpBeginOfTransWords)(struct _TRANSWORDINFO *pTransWord);
    /* Reads the penalty for a leading NBS event. */
    s32 (*exevGetLeading_NBS_penalty)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the penalty for a trailing NBS event. */
    s32 (*exevGetTrailing_NBS_penalty)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the penalty for a rejection NBS event. */
    s32 (*exevGetRejection_NBS_penalty)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the path penalty for a rejection NBS event. */
    s32 (*exevGetRejection_NBS_path_penalty)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the silence phoneme used by the extra-event context. */
    u32 (*exevGetExtraEventSilencePhenome)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the number of entries in the extra-event lexicon. */
    u32 (*exevGetNbrItemsInLexicon)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the number of pronunciations in the extra-event lexicon. */
    u32 (*exevGetNbrOfPronunciations)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the lexicon index for the leading NBS event. */
    u32 (*exevGetLeading_NBS_WordIndex)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the lexicon index for the trailing NBS event. */
    u32 (*exevGetTrailing_NBS_WordIndex)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Reads the lexicon index for the rejection NBS event. */
    u32 (*exevGetRejection_NBS_WordIndex)(struct _EXTRAEVENTCTXINFO *pExevCtx);
    /* Locates the extra-event context's table of word offsets. */
    u32 *(*exevGetpBeginOfWords)(void *pExevCtx);
    /* Returns the null pointer for the optional pronunciation-offset table. */
    u32 *(*exevGetpBeginOfPronunciations)(void *pExevCtx);
    /* Locates the first entry in the extra-event lexicon. */
    u16 *(*exevGetpBeginOfItems)(struct _EXTRAEVENTCTXINFO *pExevCtx);
};

/* Installs the language accessors in the supplied record. */
void FillLanguageVirtualTable(struct _LANGUAGE *pLang);

#endif
