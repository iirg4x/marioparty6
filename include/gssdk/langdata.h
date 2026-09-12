#ifndef GSSDK_LANGDATA_H
#define GSSDK_LANGDATA_H

#include "types.h"

typedef struct LanguageDataV2 {
    u32 dataType;
    u32 size;
    u8 versionInfo[0x80];
    u32 nbrCodeBook;
    u32 nbrState;
    u32 nbrNonErgodicPhenomes;
    u32 nbrErgodicStates;
    u32 nbrStateErgodicStates;
    u32 nbrErgodicPhenomes;
    u32 nbrStateErgodicPhenomes;
    u32 silencePhenome;
    u32 userWordSilencePhenome;
    u32 adaptSilState;
    u32 singleWordGarbagePhenome;
    u32 sentenceGarbagePhenome;
    u32 anySpeechGarbagePhenome;
    u32 nbrSpeechUnit;
    u32 nbrSpeechUnitClass;
    u32 nbrTones;
    u32 userWordSpeechUnitClass;
    u32 nbrWarpFactors;
    u32 nbrTransWord;
    u32 nbrPhenUserWordTraining;
    u32 flags;
    s16 recogWTP;
    s16 spellingWTP;
    u8 payload[];
} LanguageDataV2;

typedef struct CodeBookData {
    u32 dimension;
    u32 firstSize;
    u32 nbrInSecondSearch;
    u32 secondSize;
    u32 compressedStart;
    u32 nbrGastone;
} CodeBookData;

typedef struct TransWordData {
    u32 nbrWords;
    u32 nbrItems;
    u16 lex[];
} TransWordData;

typedef struct ExtraEventData {
    s32 leadingPenalty;
    s32 trailingPenalty;
    s32 rejectionPenalty;
    s32 rejectionPathPenalty;
    u32 silencePhenome;
    u32 nbrItems;
    u32 nbrPronunciations;
    u32 leadingWordIndex;
    u32 trailingWordIndex;
    u32 rejectionWordIndex;
    u16 items[];
} ExtraEventData;

typedef struct LanguageData LanguageData;
typedef void (*LanguageDataMethod)(void);

struct LanguageData {
    LanguageDataV2 *data;
    BOOL (*checkDataType)(LanguageData *language);
    u32 (*getSize)(LanguageData *language);
    void *(*getVersionInfo)(LanguageData *language);
    u32 (*getNbrCodeBook)(LanguageData *language);
    u32 (*getNbrState)(LanguageData *language);
    u32 (*getNbrPhenome)(LanguageData *language);
    LanguageDataMethod reserved1C;
    u32 (*getAdaptSilState)(LanguageData *language);
    u32 (*getNbrErgodicStates)(LanguageData *language);
    u32 (*getNbrStateErgodicStates)(LanguageData *language);
    u32 (*getNbrErgodicPhenomes)(LanguageData *language);
    u32 (*getNbrStateErgodicPhenomes)(LanguageData *language);
    u32 (*getNbrNonErgodicPhenomes)(LanguageData *language);
    u32 (*getSilencePhenome)(LanguageData *language);
    u32 (*getUWSilencePhenome)(LanguageData *language);
    u32 (*getSingleWordGarbagePhenome)(LanguageData *language);
    u32 (*getSentenceGarbagePhenome)(LanguageData *language);
    u32 (*getAnySpeechGarbagePhenome)(LanguageData *language);
    u32 (*getNbrWarpFactors)(LanguageData *language);
    u32 (*getNbrSpeechUnit)(LanguageData *language);
    u32 (*getNbrSpeechUnitClass)(LanguageData *language);
    u32 (*getNbrTones)(LanguageData *language);
    u32 (*getUserWordSpeechUnitClass)(LanguageData *language);
    u32 (*getNbrTransWord)(LanguageData *language);
    u32 (*getNbrPhenUserWordTraining)(LanguageData *language);
    s32 (*getRecogWTP)(LanguageData *language);
    s32 (*getSpellingWTP)(LanguageData *language);
    u32 (*getCodeBookDim)(CodeBookData *codeBook);
    u32 (*getFirstCdbSize)(CodeBookData *codeBook);
    u32 (*getNbrInSecSearch)(CodeBookData *codeBook);
    u32 (*getSecondCdbSize)(CodeBookData *codeBook);
    u32 (*getCompStart)(CodeBookData *codeBook);
    u32 (*getNbrGastone)(CodeBookData *codeBook);
    void *(*getpWarpFactors)(LanguageData *language);
    void *(*getpGastone)(LanguageData *language);
    LanguageDataMethod reserved90;
    u16 (*getSilPhenome)(LanguageData *language);
    void *(*getpErgodicStates)(LanguageData *language);
    void *(*getpErgodicPhenomes)(LanguageData *language);
    void *(*getpErgodicPenalty)(LanguageData *language);
    CodeBookData *(*getpCodeBook)(LanguageData *language, u32 index);
    u32 *(*getpSpeechUnit)(LanguageData *language);
    TransWordData *(*getpTransWord)(LanguageData *language);
    u32 (*getSizeSpeechUnits)(LanguageData *language);
    LanguageDataMethod reservedB4;
    u16 *(*getpPhenUserWordTraining)(LanguageData *language);
    u16 *(*getpDimensionsOfToneConversionMatrix)(LanguageData *language);
    u16 *(*getpOffsetForFinals)(LanguageData *language);
    u16 *(*getpToneConversionMatrix)(LanguageData *language);
    ExtraEventData *(*getpExtraEventContext)(LanguageData *language);
    BOOL (*isNormalPhenome)(LanguageData *language, u16 phenome);
    BOOL (*checkBitField)(LanguageData *language, u32 bitField);
    s32 (*getNbrStatesInPhenome)(u16 *states, u16 phenome);
    s32 (*convPhenomesToStates)(u16 *states, u16 phenome, u16 **output);
    u32 (*getCodeBookSize)(LanguageData *language, CodeBookData *codeBook);
    u32 (*getProbMatrixSize)(LanguageData *language, CodeBookData *codeBook);
    u32 (*getSmoothMatrixSize)(CodeBookData *codeBook);
    f32 *(*getpFirstCdb)(CodeBookData *codeBook);
    u32 *(*getpIndexInSecCdb)(CodeBookData *codeBook);
    f32 *(*getpSecondCdb)(CodeBookData *codeBook);
    u8 *(*getpProbMatrix)(CodeBookData *codeBook);
    f32 *(*getpSmoothMatrix)(LanguageData *language, CodeBookData *codeBook);
    u32 (*transWordGetSize)(LanguageData *language, TransWordData *words);
    u16 *(*transWordGetpLex)(TransWordData *words);
    u16 *(*transWordGetpBeginOfWords)(TransWordData *words);
    s32 (*exevGetLeadingPenalty)(ExtraEventData *data);
    s32 (*exevGetTrailingPenalty)(ExtraEventData *data);
    s32 (*exevGetRejectionPenalty)(ExtraEventData *data);
    s32 (*exevGetRejectionPathPenalty)(ExtraEventData *data);
    u32 (*exevGetSilencePhenome)(ExtraEventData *data);
    u32 (*exevGetNbrItems)(ExtraEventData *data);
    u32 (*exevGetNbrPronunciations)(ExtraEventData *data);
    u32 (*exevGetLeadingWordIndex)(ExtraEventData *data);
    u32 (*exevGetTrailingWordIndex)(ExtraEventData *data);
    u32 (*exevGetRejectionWordIndex)(ExtraEventData *data);
    u32 *(*exevGetpBeginOfWords)(ExtraEventData *data);
    void *(*exevGetpBeginOfProns)(ExtraEventData *data);
    u16 *(*exevGetpBeginOfItems)(ExtraEventData *data);
};

void FillLanguageVirtualTable(LanguageData *language);

#endif
