#include "types.h"

#include "gssdk/langdata.h"
#include "string.h"

static BOOL _langCheckDataType(LanguageData *language)
{
    return language->data->dataType == 0x1003;
}

static u32 _langGetSize(LanguageData *language)
{
    return language->data->size;
}

static void *_langGetVersionInfo(LanguageData *language)
{
    return language->data->versionInfo;
}

static u32 _langGetNbrCodeBook(LanguageData *language)
{
    return language->data->nbrCodeBook;
}

static u32 _langGetNbrState(LanguageData *language)
{
    return language->data->nbrState;
}

static u32 _langGetNbrPhenome(LanguageData *language)
{
    return language->data->nbrErgodicPhenomes +
           language->data->nbrNonErgodicPhenomes;
}

static u32 _langGetAdaptSilState(LanguageData *language)
{
    return language->data->adaptSilState;
}

static u32 _langGetNbrErgodicStates(LanguageData *language)
{
    return language->data->nbrErgodicStates;
}

static u32 _langGetNbrStateErgodicStates(LanguageData *language)
{
    return language->data->nbrStateErgodicStates;
}

static u32 _langGetNbrErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrErgodicPhenomes;
}

static u32 _langGetNbrStateErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrStateErgodicPhenomes;
}

static u32 _langGetNbrNonErgodicPhenomes(LanguageData *language)
{
    return language->data->nbrNonErgodicPhenomes;
}

static u32 _langGetSilencePhenome(LanguageData *language)
{
    return language->data->silencePhenome;
}

static u32 _langGetUWSilencePhenome(LanguageData *language)
{
    return language->data->userWordSilencePhenome;
}

static u32 _langGetSingleWordGarbagePhenome(LanguageData *language)
{
    return language->data->singleWordGarbagePhenome;
}

static u32 _langGetSentenceGarbagePhenome(LanguageData *language)
{
    return language->data->sentenceGarbagePhenome;
}

static u32 _langGetAnySpeechGarbagePhenome(LanguageData *language)
{
    return language->data->anySpeechGarbagePhenome;
}

static u32 _langGetNbrSpeechUnit(LanguageData *language)
{
    return language->data->nbrSpeechUnit;
}

static u32 _langGetNbrWarpFactors(LanguageData *language)
{
    return language->data->nbrWarpFactors;
}

static u32 _langGetNbrSpeechUnitClass(LanguageData *language)
{
    return language->data->nbrSpeechUnitClass;
}

static u32 _langGetNbrTones(LanguageData *language)
{
    return language->data->nbrTones;
}

static u32 _langGetUserWordSpeechUnitClass(LanguageData *language)
{
    return language->data->userWordSpeechUnitClass;
}

static u32 _langGetNbrTransWord(LanguageData *language)
{
    return language->data->nbrTransWord;
}

static u32 _langGetNbrPhenUserWordTraining(LanguageData *language)
{
    return language->data->nbrPhenUserWordTraining;
}

static s32 _langGetRecogWTP(LanguageData *language)
{
    return language->data->recogWTP;
}

static s32 _langGetSpellingWTP(LanguageData *language)
{
    return language->data->spellingWTP;
}

static u32 _cdbGetCodeBookDim(CodeBookData *codeBook)
{
    return codeBook->dimension;
}

static u32 _cdbGetFirstCdbSize(CodeBookData *codeBook)
{
    return codeBook->firstSize;
}

static u32 _cdbGetNbrInSecSearch(CodeBookData *codeBook)
{
    return codeBook->nbrInSecondSearch;
}

static u32 _cdbGetSecondCdbSize(CodeBookData *codeBook)
{
    return codeBook->secondSize;
}

static u32 _cdbGetCompStart(CodeBookData *codeBook)
{
    return codeBook->compressedStart;
}

static u32 _cdbGetNbrGastone(CodeBookData *codeBook)
{
    return codeBook->nbrGastone;
}

static u16 _langGetSilPhenome(LanguageData *language)
{
    return language->data->silencePhenome;
}

static void *_langGetpWarpFactors(LanguageData *language)
{
    return language->data->payload;
}

static void *_langGetpGastone(LanguageData *language)
{
    return language->data->payload + language->data->nbrWarpFactors * 8;
}

static void *_langGetpErgodicStates(LanguageData *language)
{
    LanguageDataV2 *data = language->data;

    return data->payload + data->nbrWarpFactors * 8 +
           data->nbrState * data->nbrCodeBook * 2;
}

static void *_langGetpErgodicPhenomes(LanguageData *language)
{
    LanguageDataV2 *data = language->data;

    return data->payload + data->nbrWarpFactors * 8 +
           data->nbrState * data->nbrCodeBook * 2 +
           (data->nbrErgodicStates + 1) * 2 +
           data->nbrStateErgodicStates * 2;
}

static void *_langGetpErgodicPenalty(LanguageData *language)
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

static u32 _cdbGetCodeBookSize(LanguageData *language, CodeBookData *codeBook);
static u32 _transwGetSize(LanguageData *language, TransWordData *words);

static CodeBookData *_langGetpCodeBook(LanguageData *language, u32 index)
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

static u32 *_langGetpSpeechUnit(LanguageData *language)
{
    u32 count = language->data->nbrCodeBook;
    if (language->data->flags & 4) {
        count++;
    }
    return (u32 *)_langGetpCodeBook(language, count);
}

static u32 _langGetSizeSpeechUnits(LanguageData *language)
{
    LanguageDataV2 *data = language->data;
    return _langGetpSpeechUnit(language)[data->nbrSpeechUnit];
}

static TransWordData *_langGetpTransWord(LanguageData *language)
{
    LanguageDataV2 *data = language->data;
    u32 *speechUnits = _langGetpSpeechUnit(language);
    return (TransWordData *)((u8 *)data + (((u8 *)speechUnits
        + speechUnits[data->nbrSpeechUnit] * 2 - (u8 *)data + 3) & ~3));
}

static u16 *_langGetpPhenUserWordTraining(LanguageData *language)
{
    TransWordData *words = _langGetpTransWord(language);
    return (u16 *)((u8 *)words + _transwGetSize(language, words));
}

static u16 *_langGetpDimensionsOfToneConversionMatrix(LanguageData *language)
{
    u8 *end = (u8 *)(_langGetpPhenUserWordTraining(language)
        + language->data->nbrPhenUserWordTraining);
    if (language->checkBitField(language, 4)) {
        return (u16 *)(end + _cdbGetCodeBookSize(language, (CodeBookData *)end));
    }
    return (u16 *)end;
}

static u16 *_langGetpOffsetForFinals(LanguageData *language)
{
    return _langGetpDimensionsOfToneConversionMatrix(language) + 3;
}

static u16 *_langGetpToneConversionMatrix(LanguageData *language)
{
    return _langGetpOffsetForFinals(language) + 1;
}

static ExtraEventData *_langGetpExtraEventContext(LanguageData *language)
{
    if (language->data->nbrTones != 0) {
        u16 *dimensions = _langGetpDimensionsOfToneConversionMatrix(language);
        u32 count = (u16)(dimensions[0] * dimensions[1] * dimensions[2]);
        count = (count + 3) & ~3;
        return (ExtraEventData *)(_langGetpToneConversionMatrix(language) + (u16)count);
    } else {
        u8 *end = (u8 *)_langGetpDimensionsOfToneConversionMatrix(language);
        u32 offset = (u32)end - (u32)language;
        return (ExtraEventData *)((u8 *)language + ((offset + 3) & ~3));
    }
}

static BOOL _langIsNormalPhenome(LanguageData *language, u16 phenome)
{
    if (phenome < language->data->nbrErgodicPhenomes) {
        return FALSE;
    }
    return TRUE;
}

static BOOL _langCheckBitField(LanguageData *language, u32 bitField)
{
    return language->data->flags & bitField;
}

static u16 *_transwGetpTransWordLex(TransWordData *words)
{
    return words->lex;
}

static u16 *_transwGetpBeginOfTransWords(TransWordData *words)
{
    return _transwGetpTransWordLex(words) + words->nbrItems;
}

static u32 _transwGetSize(LanguageData *language, TransWordData *words)
{
    u32 bytes = sizeof(TransWordData) + words->nbrItems * sizeof(u16);
    bytes += (language->data->nbrTransWord + 1) * sizeof(u16);
    bytes += (words->nbrWords + 1) * sizeof(u16);
    return bytes;
}

static u32 _cdbGetCodeBookSize(LanguageData *language, CodeBookData *codeBook)
{
    u32 probColumns;
    u32 bytes;

    probColumns = (_langCheckBitField(language, 1))
        ? (codeBook->nbrGastone + 1) / 2 : codeBook->nbrGastone;
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

static f32 *_cdbGetpFirstCdb(CodeBookData *codeBook)
{
    return (f32 *)(codeBook + 1);
}

static u32 *_cdbGetpIndexInSecCdb(CodeBookData *codeBook)
{
    return (u32 *)((u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32));
}

static f32 *_cdbGetpSecondCdb(CodeBookData *codeBook)
{
    return (f32 *)((u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32)
        + codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32));
}

static u8 *_cdbGetpProbMatrix(CodeBookData *codeBook)
{
    return (u8 *)codeBook + sizeof(CodeBookData)
        + codeBook->dimension * codeBook->firstSize * sizeof(f32)
        + codeBook->firstSize * codeBook->nbrInSecondSearch * sizeof(u32)
        + codeBook->dimension * codeBook->secondSize * sizeof(f32)
        + codeBook->secondSize * sizeof(u32);
}

static f32 *_cdbGetpSmoothMatrix(LanguageData *language, CodeBookData *codeBook)
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

static u32 _cdbGetProbMatrixSize(LanguageData *language, CodeBookData *codeBook)
{
    u32 probColumns;

    if (language->data->flags & 1) {
        probColumns = (codeBook->nbrGastone + 1) / 2;
    } else {
        probColumns = codeBook->nbrGastone;
    }
    return codeBook->secondSize * probColumns;
}

static u32 _cdbGetSmoothMatrixSize(CodeBookData *codeBook)
{
    return codeBook->secondSize * codeBook->secondSize * sizeof(f32);
}

static s32 _exevGetLeading_NBS_penalty(ExtraEventData *data) { return data->leadingPenalty; }
static s32 _exevGetTrailing_NBS_penalty(ExtraEventData *data) { return data->trailingPenalty; }
static s32 _exevGetRejection_NBS_penalty(ExtraEventData *data) { return data->rejectionPenalty; }
static s32 _exevGetRejection_NBS_path_penalty(ExtraEventData *data) { return data->rejectionPathPenalty; }
static u32 _exevGetExtraEventSilencePhenome(ExtraEventData *data) { return data->silencePhenome; }
static u32 _exevGetNbrItemsInLexicon(ExtraEventData *data) { return data->nbrItems; }
static u32 _exevGetNbrOfPronunciations(ExtraEventData *data) { return data->nbrPronunciations; }
static u32 _exevGetLeading_NBS_WordIndex(ExtraEventData *data) { return data->leadingWordIndex; }
static u32 _exevGetTrailing_NBS_WordIndex(ExtraEventData *data) { return data->trailingWordIndex; }
static u32 _exevGetRejection_NBS_WordIndex(ExtraEventData *data) { return data->rejectionWordIndex; }

static u16 *_exevGetpBeginOfItems(ExtraEventData *data)
{
    return data->items;
}

static u32 *_exevGetpBeginOfWords(ExtraEventData *data)
{
    u32 count = data->nbrItems;
    count += count & 1;
    return (u32 *)((u8 *)data + sizeof(ExtraEventData) + count * sizeof(u16));
}

static void *_exevGetpBeginOfProns(ExtraEventData *data)
{
    return NULL;
}

static s32 _GetNbrStatesInPhenome(u16 *states, u16 phenome)
{
    return states[phenome + 1] - states[phenome];
}

static s32 _ConvPhenomesToStates(u16 *states, u16 phenome, u16 **output)
{
    *output = states + states[phenome];
    return states[phenome + 1] - states[phenome];
}

static const LanguageData _LangVirtualTable = {
    NULL,
    _langCheckDataType,
    _langGetSize,
    _langGetVersionInfo,
    _langGetNbrCodeBook,
    _langGetNbrState,
    _langGetNbrPhenome,
    NULL,
    _langGetAdaptSilState,
    _langGetNbrErgodicStates,
    _langGetNbrStateErgodicStates,
    _langGetNbrErgodicPhenomes,
    _langGetNbrStateErgodicPhenomes,
    _langGetNbrNonErgodicPhenomes,
    _langGetSilencePhenome,
    _langGetUWSilencePhenome,
    _langGetSingleWordGarbagePhenome,
    _langGetSentenceGarbagePhenome,
    _langGetAnySpeechGarbagePhenome,
    _langGetNbrWarpFactors,
    _langGetNbrSpeechUnit,
    _langGetNbrSpeechUnitClass,
    _langGetNbrTones,
    _langGetUserWordSpeechUnitClass,
    _langGetNbrTransWord,
    _langGetNbrPhenUserWordTraining,
    _langGetRecogWTP,
    _langGetSpellingWTP,
    _cdbGetCodeBookDim,
    _cdbGetFirstCdbSize,
    _cdbGetNbrInSecSearch,
    _cdbGetSecondCdbSize,
    _cdbGetCompStart,
    _cdbGetNbrGastone,
    _langGetpWarpFactors,
    _langGetpGastone,
    NULL,
    _langGetSilPhenome,
    _langGetpErgodicStates,
    _langGetpErgodicPhenomes,
    _langGetpErgodicPenalty,
    _langGetpCodeBook,
    _langGetpSpeechUnit,
    _langGetpTransWord,
    _langGetSizeSpeechUnits,
    NULL,
    _langGetpPhenUserWordTraining,
    _langGetpDimensionsOfToneConversionMatrix,
    _langGetpOffsetForFinals,
    _langGetpToneConversionMatrix,
    _langGetpExtraEventContext,
    _langIsNormalPhenome,
    _langCheckBitField,
    _GetNbrStatesInPhenome,
    _ConvPhenomesToStates,
    _cdbGetCodeBookSize,
    _cdbGetProbMatrixSize,
    _cdbGetSmoothMatrixSize,
    _cdbGetpFirstCdb,
    _cdbGetpIndexInSecCdb,
    _cdbGetpSecondCdb,
    _cdbGetpProbMatrix,
    _cdbGetpSmoothMatrix,
    _transwGetSize,
    _transwGetpTransWordLex,
    _transwGetpBeginOfTransWords,
    _exevGetLeading_NBS_penalty,
    _exevGetTrailing_NBS_penalty,
    _exevGetRejection_NBS_penalty,
    _exevGetRejection_NBS_path_penalty,
    _exevGetExtraEventSilencePhenome,
    _exevGetNbrItemsInLexicon,
    _exevGetNbrOfPronunciations,
    _exevGetLeading_NBS_WordIndex,
    _exevGetTrailing_NBS_WordIndex,
    _exevGetRejection_NBS_WordIndex,
    _exevGetpBeginOfWords,
    _exevGetpBeginOfProns,
    _exevGetpBeginOfItems,
};

void FillLanguageVirtualTable(LanguageData *language)
{
    memcpy(language, &_LangVirtualTable, sizeof(LanguageData));
}
