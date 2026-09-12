#include "types.h"

#include "gssdk/langdata.h"

BOOL _langCheckDataType(LanguageData *language)
{
    return language->data->dataType == 0x1003;
}

u32 _langGetSize(LanguageData *language)
{
    return language->data->size;
}

void *_langGetVersionInfo(LanguageData *language)
{
    return language->data->versionInfo;
}

u32 _langGetNbrCodeBook(LanguageData *language)
{
    return language->data->nbrCodeBook;
}

u32 _langGetNbrState(LanguageData *language)
{
    return language->data->nbrState;
}

u32 _langGetNbrPhenome(LanguageData *language)
{
    return language->data->nbrErgodicPhenomes +
           language->data->nbrNonErgodicPhenomes;
}

u32 _langGetAdaptSilState(LanguageData *language)
{
    return language->data->adaptSilState;
}

u32 _langGetNbrErgodicStates(LanguageData *language)
{
    return language->data->nbrErgodicStates;
}

u32 _langGetNbrStateErgodicStates(LanguageData *language)
{
    return language->data->nbrStateErgodicStates;
}

u32 _langGetNbrErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrErgodicPhenomes;
}

u32 _langGetNbrStateErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrStateErgodicPhenomes;
}

u32 _langGetNbrNonErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrNonErgodicPhenomes;
}

u32 _langGetSilencePhenome(LanguageData *language)
{
    return language->data->silencePhenome;
}

u32 _langGetUWSilencePhenome(LanguageData *language)
{
    return language->data->userWordSilencePhenome;
}

u32 _langGetSingleWordGarbagePhenome(LanguageData *language)
{
    return language->data->singleWordGarbagePhenome;
}

u32 _langGetSentenceGarbagePhenome(LanguageData *language)
{
    return language->data->sentenceGarbagePhenome;
}

u32 _langGetAnySpeechGarbagePhenome(LanguageData *language)
{
    return language->data->anySpeechGarbagePhenome;
}

u32 _langGetNbrSpeechUnit(LanguageData *language)
{
    return language->data->nbrSpeechUnit;
}

u32 _langGetNbrWarpFactors(LanguageData *language)
{
    return language->data->nbrWarpFactors;
}

u32 _langGetNbrSpeechUnitClass(LanguageData *language)
{
    return language->data->nbrSpeechUnitClass;
}

u32 _langGetNbrTones(LanguageData *language)
{
    return language->data->nbrTones;
}

u32 _langGetUserWordSpeechUnitClass(LanguageData *language)
{
    return language->data->userWordSpeechUnitClass;
}

u32 _langGetNbrTransWord(LanguageData *language)
{
    return language->data->nbrTransWord;
}

u32 _langGetNbrPhenUserWordTraining(LanguageData *language)
{
    return language->data->nbrPhenUserWordTraining;
}

s32 _langGetRecogWTP(LanguageData *language)
{
    return language->data->recogWTP;
}

s32 _langGetSpellingWTP(LanguageData *language)
{
    return language->data->spellingWTP;
}

u32 _cdbGetCodeBookDim(CodeBookData *codeBook)
{
    return codeBook->dimension;
}

u32 _cdbGetFirstCdbSize(CodeBookData *codeBook)
{
    return codeBook->firstSize;
}

u32 _cdbGetNbrInSecSearch(CodeBookData *codeBook)
{
    return codeBook->nbrInSecondSearch;
}

u32 _cdbGetSecondCdbSize(CodeBookData *codeBook)
{
    return codeBook->secondSize;
}

u32 _cdbGetCompStart(CodeBookData *codeBook)
{
    return codeBook->compressedStart;
}

u32 _cdbGetNbrGastone(CodeBookData *codeBook)
{
    return codeBook->nbrGastone;
}

u16 _langGetSilPhenome(LanguageData *language)
{
    return language->data->silencePhenome;
}

void *_langGetpWarpFactors(LanguageData *language)
{
    return language->data->payload;
}

void *_langGetpGastone(LanguageData *language)
{
    return language->data->payload + language->data->nbrWarpFactors * 8;
}

void *_langGetpErgodicStates(LanguageData *language)
{
    LanguageDataV2 *data = language->data;

    return data->payload + data->nbrWarpFactors * 8 +
           data->nbrState * data->nbrCodeBook * 2;
}

void *_langGetpErgodicPhenomes(LanguageData *language)
{
    LanguageDataV2 *data = language->data;

    return data->payload + data->nbrWarpFactors * 8 +
           data->nbrState * data->nbrCodeBook * 2 +
           (data->nbrErgodicStates + 1) * 2 +
           data->nbrStateErgodicStates * 2;
}

void *_langGetpErgodicPenalty(LanguageData *language)
{
    LanguageDataV2 *data = language->data;

    u32 bytes = data->nbrWarpFactors * 8
        + data->nbrState * data->nbrCodeBook * 2
        + (data->nbrErgodicStates + 1) * 2
        + data->nbrStateErgodicStates * 2
        + (data->nbrErgodicPhenomes + 1) * 2
        + data->nbrStateErgodicPhenomes * 2;
    return (u8 *)data + bytes + sizeof(LanguageDataV2);
}

/* Layout names are descriptive; field widths and boundaries are target-backed. */
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


u32 _cdbGetCodeBookSize(LanguageData *language, CodeBookData *codeBook);
u32 _transwGetSize(LanguageData *language, TransWordData *words);

CodeBookData *_langGetpCodeBook(LanguageData *language, u32 index)
{
    LanguageDataV2 *data = language->data;
    u32 bytes;
    CodeBookData *codeBook;
    u32 i;

    bytes = data->nbrWarpFactors * 8
        + data->nbrState * data->nbrCodeBook * 2
        + (data->nbrErgodicStates + 1) * 2
        + data->nbrStateErgodicStates * 2
        + (data->nbrErgodicPhenomes + 1) * 2
        + data->nbrStateErgodicPhenomes * 4 + sizeof(LanguageDataV2);
    codeBook = (CodeBookData *)((u8 *)data + ((bytes + 3) & ~3));
    for (i = 0; i < index; i++) {
        bytes = _cdbGetCodeBookSize(language, codeBook);
        codeBook = (CodeBookData *)((u8 *)codeBook + ((bytes + 3) & ~3));
    }
    return codeBook;
}

u32 *_langGetpSpeechUnit(LanguageData *language)
{
    u32 count = language->data->nbrCodeBook;
    if (language->data->flags & 4) {
        count++;
    }
    return (u32 *)_langGetpCodeBook(language, count);
}

u32 _langGetSizeSpeechUnits(LanguageData *language)
{
    LanguageDataV2 *data = language->data;
    return _langGetpSpeechUnit(language)[data->nbrSpeechUnit];
}

TransWordData *_langGetpTransWord(LanguageData *language)
{
    LanguageDataV2 *data = language->data;
    u32 *speechUnits = _langGetpSpeechUnit(language);
    return (TransWordData *)((u8 *)data + (((u8 *)speechUnits
        + speechUnits[data->nbrSpeechUnit] * 2 - (u8 *)data + 3) & ~3));
}

u16 *_langGetpPhenUserWordTraining(LanguageData *language)
{
    TransWordData *words = _langGetpTransWord(language);
    return (u16 *)((u8 *)words + _transwGetSize(language, words));
}

u16 *_langGetpDimensionsOfToneConversionMatrix(LanguageData *language)
{
    u8 *end = (u8 *)(_langGetpPhenUserWordTraining(language)
        + language->data->nbrPhenUserWordTraining);
    if (language->checkBitField(language, 4)) {
        return (u16 *)(end + _cdbGetCodeBookSize(language, (CodeBookData *)end));
    }
    return (u16 *)end;
}

u16 *_langGetpOffsetForFinals(LanguageData *language)
{
    return _langGetpDimensionsOfToneConversionMatrix(language) + 3;
}

u16 *_langGetpToneConversionMatrix(LanguageData *language)
{
    return _langGetpDimensionsOfToneConversionMatrix(language) + 4;
}

ExtraEventData *_langGetpExtraEventContext(LanguageData *language)
{
    if (language->data->nbrTones != 0) {
        u16 *dimensions = _langGetpDimensionsOfToneConversionMatrix(language);
        u16 count = dimensions[0] * dimensions[1] * dimensions[2];
        count = (count + 3) & ~3;
        return (ExtraEventData *)(_langGetpToneConversionMatrix(language) + count);
    } else {
        u8 *end = (u8 *)_langGetpDimensionsOfToneConversionMatrix(language);
        u32 offset = (u32)end - (u32)language;
        return (ExtraEventData *)((u8 *)language + ((offset + 3) & ~3));
    }
}

BOOL _langIsNormalPhenome(LanguageData *language, u16 phenome)
{
    if (phenome < language->data->nbrErgodicPhenomes) {
        return FALSE;
    }
    return TRUE;
}

u32 _langCheckBitField(LanguageData *language, u32 bitField)
{
    return language->data->flags & bitField;
}

u16 *_transwGetpTransWordLex(TransWordData *words)
{
    return words->lex;
}

u16 *_transwGetpBeginOfTransWords(TransWordData *words)
{
    return _transwGetpTransWordLex(words) + words->nbrItems;
}

u32 _transwGetSize(LanguageData *language, TransWordData *words)
{
    u32 bytes = sizeof(TransWordData) + words->nbrItems * sizeof(u16);
    bytes += (language->data->nbrTransWord + 1) * sizeof(u16);
    bytes += (words->nbrWords + 1) * sizeof(u16);
    return bytes;
}

u32 _cdbGetCodeBookSize(LanguageData *language, CodeBookData *codeBook)
{
    u32 probColumns;
    u32 bytes;

    if (_langCheckBitField(language, 1)) {
        probColumns = (codeBook->nbrGastone + 1) / 2;
    } else {
        probColumns = codeBook->nbrGastone;
    }
    bytes = codeBook->dimension * codeBook->firstSize * sizeof(f32);
    bytes += codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32);
    bytes += codeBook->dimension * codeBook->secondSize * sizeof(f32);
    bytes += codeBook->secondSize * sizeof(u32);
    bytes += codeBook->secondSize * probColumns;
    bytes += sizeof(CodeBookData);
    if (_langCheckBitField(language, 2)) {
        bytes = (bytes + 3) & ~3;
        bytes += codeBook->secondSize * codeBook->secondSize * sizeof(f32);
    }
    return bytes;
}

f32 *_cdbGetpFirstCdb(CodeBookData *codeBook)
{
    return (f32 *)(codeBook + 1);
}

u32 *_cdbGetpIndexInSecCdb(CodeBookData *codeBook)
{
    return (u32 *)((u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32));
}

f32 *_cdbGetpSecondCdb(CodeBookData *codeBook)
{
    return (f32 *)((u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32)
        + codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32));
}

u8 *_cdbGetpProbMatrix(CodeBookData *codeBook)
{
    return (u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32)
        + codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32)
        + codeBook->dimension * codeBook->secondSize * sizeof(f32)
        + codeBook->secondSize * sizeof(u32);
}

f32 *_cdbGetpSmoothMatrix(LanguageData *language, CodeBookData *codeBook)
{
    u32 probColumns;
    u32 bytes;

    if (!(language->data->flags & 2)) {
        return NULL;
    }
    if (language->data->flags & 1) {
        probColumns = (codeBook->nbrGastone + 1) / 2;
    } else {
        probColumns = codeBook->nbrGastone;
    }
    bytes = codeBook->dimension * codeBook->firstSize * sizeof(f32);
    bytes += codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32);
    bytes += codeBook->dimension * codeBook->secondSize * sizeof(f32);
    bytes += codeBook->secondSize * sizeof(u32);
    bytes += codeBook->secondSize * probColumns;
    bytes += sizeof(CodeBookData);
    bytes = (bytes + 3) & ~3;
    return (f32 *)((u8 *)codeBook + bytes);
}

u32 _cdbGetProbMatrixSize(LanguageData *language, CodeBookData *codeBook)
{
    u32 probColumns;

    if (language->data->flags & 1) {
        probColumns = (codeBook->nbrGastone + 1) / 2;
    } else {
        probColumns = codeBook->nbrGastone;
    }
    return codeBook->secondSize * probColumns;
}

u32 _cdbGetSmoothMatrixSize(CodeBookData *codeBook)
{
    return codeBook->secondSize * codeBook->secondSize * sizeof(f32);
}

s32 _exevGetLeading_NBS_penalty(ExtraEventData *data) { return data->leadingPenalty; }
s32 _exevGetTrailing_NBS_penalty(ExtraEventData *data) { return data->trailingPenalty; }
s32 _exevGetRejection_NBS_penalty(ExtraEventData *data) { return data->rejectionPenalty; }
s32 _exevGetRejection_NBS_path_penalty(ExtraEventData *data) { return data->rejectionPathPenalty; }
u32 _exevGetExtraEventSilencePhenome(ExtraEventData *data) { return data->silencePhenome; }
u32 _exevGetNbrItemsInLexicon(ExtraEventData *data) { return data->nbrItems; }
u32 _exevGetNbrOfPronunciations(ExtraEventData *data) { return data->nbrPronunciations; }
u32 _exevGetLeading_NBS_WordIndex(ExtraEventData *data) { return data->leadingWordIndex; }
u32 _exevGetTrailing_NBS_WordIndex(ExtraEventData *data) { return data->trailingWordIndex; }
u32 _exevGetRejection_NBS_WordIndex(ExtraEventData *data) { return data->rejectionWordIndex; }

u16 *_exevGetpBeginOfItems(ExtraEventData *data)
{
    return data->items;
}

u32 *_exevGetpBeginOfWords(ExtraEventData *data)
{
    u32 count = data->nbrItems;
    count += count & 1;
    return (u32 *)((u8 *)data + sizeof(ExtraEventData) + count * sizeof(u16));
}

void *_exevGetpBeginOfProns(ExtraEventData *data)
{
    return NULL;
}

s32 _GetNbrStatesInPhenome(u16 *states, u16 phenome)
{
    return states[phenome + 1] - states[phenome];
}

s32 _ConvPhenomesToStates(u16 *states, u16 phenome, u16 **output)
{
    *output = states + states[phenome];
    return states[phenome + 1] - states[phenome];
}
