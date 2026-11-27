/* Reads recognition language metadata and locates tables in its packed records. */
#include "types.h"

#include "gssdk/langdata.h"

#include <string.h>

/* Computes the byte extent of a packed codebook. */
static u32 _cdbGetCodeBookSize(struct _LANGUAGE *pLang, struct _CDBINFO *pCdb);

/* Computes the byte extent of packed transition-word data. */
static u32 _transwGetSize(struct _LANGUAGE *pLang, struct _TRANSWORDINFO *pTransWord);

/* Checks whether the resource has the version-two language type. */
static BOOL _langCheckDataType(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->DataType == 0x1003;
}

/* Reads the stored size of the language resource. */
static u32 _langGetSize(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->Size;
}

/* Locates the language resource's version information. */
static u8 *_langGetVersionInfo(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->VersionInfo;
}

/* Reads the number of codebooks in the language. */
static u32 _langGetNbrCodeBook(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrCodeBook;
}

/* Reads the number of states in the language. */
static u32 _langGetNbrState(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrState;
}

/* Computes the total number of ergodic and nonergodic phonemes. */
static u32 _langGetNbrPhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrErgodicPhenomes +
           pLanguage->NbrNonErgodicPhenomes;
}

/* Reads the index of the state used for silence adaptation. */
static u32 _langGetAdaptSilState(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->AdaptSilState;
}

/* Reads the number of ergodic states. */
static u32 _langGetNbrErgodicStates(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrErgodicStates;
}

/* Reads the state count for the ergodic-state entries. */
static u32 _langGetNbrStateErgodicStates(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrStateErgodicStates;
}

/* Reads the number of ergodic phonemes. */
static u32 _langGetNbrErgodicPhenomes(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrErgodicPhenomes;
}

/* Reads the state count for the ergodic phonemes. */
static u32 _langGetNbrStateErgodicPhenomes(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrStateErgodicPhenomes;
}

/* Reads the number of nonergodic phonemes. */
static u32 _langGetNbrNonErgodicPhenomes(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrNonErgodicPhenomes;
}

/* Reads the silence phoneme for recognition. */
static u32 _langGetSilencePhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->SilencePhenome;
}

/* Reads the silence phoneme for user words. */
static u32 _langGetUWSilencePhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->UWSilencePhenome;
}

/* Reads the garbage phoneme for single-word recognition. */
static u32 _langGetSingleWordGarbagePhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->SingleWordGarbagePhenome;
}

/* Reads the garbage phoneme for sentence recognition. */
static u32 _langGetSentenceGarbagePhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->SentenceGarbagePhenome;
}

/* Reads the garbage phoneme for unrestricted speech. */
static u32 _langGetAnySpeechGarbagePhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->AnySpeechGarbagePhenome;
}

/* Reads the number of speech units in the lexicon. */
static u32 _langGetNbrSpeechUnit(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrSpeechUnit;
}

/* Reads the number of frequency warp factors. */
static u32 _langGetNbrWarpFactors(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrWarpFactors;
}

/* Reads the number of speech-unit classes. */
static u32 _langGetNbrSpeechUnitClass(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrSpeechUnitClass;
}

/* Returns the number of tones in the recognition language. */
static u32 _langGetNbrTones(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrTones;
}

/* Reads the speech-unit class reserved for user words. */
static u32 _langGetUserWordSpeechUnitClass(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->UserWordSpeechUnitClass;
}

/* Reads the number of transition words. */
static u32 _langGetNbrTransWord(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrTransWord;
}

/* Returns the number of phonemes available for user-word training. */
static u32 _langGetNbrPhenUserWordTraining(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->NbrPhenUserWordTraining;
}

/* Reads the recognition word transition penalty. */
static s16 _langGetRecogWTP(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->RecogWTP;
}

/* Reads the spelling word transition penalty. */
static s16 _langGetSpellingWTP(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->SpellingWTP;
}

/* Reads the codebook vector dimension. */
static u32 _cdbGetCodeBookDim(struct _CDBINFO *pCdb)
{
    return pCdb->CodeBookDim;
}

/* Reads the number of vectors in the first codebook. */
static u32 _cdbGetFirstCdbSize(struct _CDBINFO *pCdb)
{
    return pCdb->FirstCdbSize;
}

/* Reads the number of second-search indexes per first-codebook vector. */
static u32 _cdbGetNbrInSecSearch(struct _CDBINFO *pCdb)
{
    return pCdb->NbrInSecSearch;
}

/* Reads the number of vectors in the second codebook. */
static u32 _cdbGetSecondCdbSize(struct _CDBINFO *pCdb)
{
    return pCdb->SecondCdbSize;
}

/* Reads the starting dimension for compressed codebook data. */
static u32 _cdbGetCompStart(struct _CDBINFO *pCdb)
{
    return pCdb->CompStart;
}

/* Reads the number of codebook probability entries per vector. */
static u32 _cdbGetNbrGastone(struct _CDBINFO *pCdb)
{
    return pCdb->NbrGastone;
}

/* Reads the silence phoneme as a sixteen-bit index. */
static u16 _langGetSilPhenome(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->SilencePhenome;
}

/* Locates the frequency warp factors at the start of the language payload. */
static struct _WARPINFO *_langGetpWarpFactors(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return (struct _WARPINFO *)(pLanguage + 1);
}

/* Locates the Gastone table following the language's warp factors. */
static u16 *_langGetpGastone(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return (u16 *)((u8 *)pLanguage + sizeof(struct _LANGUAGEINFO) +
        pLanguage->NbrWarpFactors * sizeof(struct _WARPINFO));
}

/* Locates the ergodic state table in the language data. */
static u16 *_langGetpErgodicStates(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return (u16 *)((u8 *)pLanguage + sizeof(struct _LANGUAGEINFO) +
           pLanguage->NbrWarpFactors * sizeof(struct _WARPINFO) +
           pLanguage->NbrState * pLanguage->NbrCodeBook * sizeof(u16));
}

/* Locates the ergodic phoneme table in the language data. */
static u16 *_langGetpErgodicPhenomes(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return (u16 *)((u8 *)pLanguage + sizeof(struct _LANGUAGEINFO) +
           pLanguage->NbrWarpFactors * sizeof(struct _WARPINFO) +
           pLanguage->NbrState * pLanguage->NbrCodeBook * sizeof(u16) +
           (pLanguage->NbrErgodicStates + 1) * sizeof(u16) +
           pLanguage->NbrStateErgodicStates * sizeof(u16));
}

/* Locates the ergodic penalty table in the language data. */
static s16 *_langGetpErgodicPenalty(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    unsigned long Size;

    Size = sizeof(*pLanguage) + pLanguage->NbrWarpFactors * sizeof(struct _WARPINFO) +
           pLanguage->NbrState * pLanguage->NbrCodeBook * sizeof(u16) +
           (pLanguage->NbrErgodicStates + 1) * sizeof(u16) +
           pLanguage->NbrStateErgodicStates * sizeof(u16) +
           (pLanguage->NbrErgodicPhenomes + 1) * sizeof(u16) +
           pLanguage->NbrStateErgodicPhenomes * sizeof(u16);
    /* The penalty table follows the packed ergodic phoneme-state entries. */
    return (s16 *)((u8 *)pLanguage + Size);
}

/* Locates the requested codebook by stepping past the preceding codebooks. */
static void *_langGetpCodeBook(struct _LANGUAGE *pLang, unsigned long Cdb)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    unsigned long Size;
    char *pCdb;
    unsigned int i;
    unsigned long CodeBookSize;

    Size = sizeof(*pLanguage) + pLanguage->NbrWarpFactors * sizeof(struct _WARPINFO) +
           pLanguage->NbrState * pLanguage->NbrCodeBook * sizeof(u16) +
           (pLanguage->NbrErgodicStates + 1) * sizeof(u16) +
           pLanguage->NbrStateErgodicStates * sizeof(u16) +
           (pLanguage->NbrErgodicPhenomes + 1) * sizeof(u16) +
           pLanguage->NbrStateErgodicPhenomes * (sizeof(u16) + sizeof(s16));
    Size = (Size + 3) & ~3UL;
    pCdb = (char *)pLanguage + Size;
    for (i = 0; i < Cdb; i++) {
        CodeBookSize = _cdbGetCodeBookSize(pLang, (struct _CDBINFO *)pCdb);
        CodeBookSize = (CodeBookSize + 3) & ~3UL;
        pCdb += CodeBookSize;
    }
    return pCdb;
}

/* Locates the speech unit table in the language data. */
static unsigned long *_langGetpSpeechUnit(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    unsigned int NbrCodeBooks = pLanguage->NbrCodeBook;

    if (pLanguage->BitField & 4) {
        NbrCodeBooks++;
    }
    return _langGetpCodeBook(pLang, NbrCodeBooks);
}

/* Computes the space occupied by the speech unit data. */
static u32 _langGetSizeSpeechUnits(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return _langGetpSpeechUnit(pLang)[pLanguage->NbrSpeechUnit];
}

/* Locates the transition-word data in the language record. */
static void *_langGetpTransWord(struct _LANGUAGE *pLang)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    unsigned int i;
    unsigned short *pSpeechUnits = (unsigned short *)_langGetpSpeechUnit(pLang);

    pSpeechUnits += ((unsigned long *)pSpeechUnits)[pLanguage->NbrSpeechUnit];
    /* Round the byte offset from the resource base to the next word. */
    i = (char *)pSpeechUnits - (char *)pLanguage;
    i = (i + 3) & ~3UL;
    return (char *)pLanguage + i;
}

/* Locates the phoneme table for user-word training. */
static u16 *_langGetpPhenUserWordTraining(struct _LANGUAGE *pLang)
{
    void *pTransWord = _langGetpTransWord(pLang);

    return (u16 *)((char *)pTransWord + _transwGetSize(pLang, (struct _TRANSWORDINFO *)pTransWord));
}

/* Locates the stored dimensions of the tone conversion matrix. */
static u16 *_langGetpDimensionsOfToneConversionMatrix(struct _LANGUAGE *pLang)
{
    void *pPhenUserWordTraining = _langGetpPhenUserWordTraining(pLang);
    char *pCandidate = pPhenUserWordTraining;

    pCandidate += _langGetNbrPhenUserWordTraining(pLang) * sizeof(u16);
    if (pLang->langCheckBitField(pLang, 4)) {
        return (u16 *)(pCandidate + _cdbGetCodeBookSize(pLang, (struct _CDBINFO *)pCandidate));
    }
    return (u16 *)(pCandidate);
}

/* Locates the offset for finals after the stored tone-matrix dimensions. */
static u16 *_langGetpOffsetForFinals(struct _LANGUAGE *pLang)
{
    return _langGetpDimensionsOfToneConversionMatrix(pLang) + 3;
}

/* Locates the tone conversion matrix after the finals offset. */
static u16 *_langGetpToneConversionMatrix(struct _LANGUAGE *pLang)
{
    return _langGetpOffsetForFinals(pLang) + 1;
}

/* Locates the context data used for extra recognition events. */
static void *_langGetpExtraEventContext(struct _LANGUAGE *pLang)
{
    unsigned short *pTCMdim;
    unsigned short TCMsize;
    unsigned int i;

    if (_langGetNbrTones(pLang)) {
        pTCMdim = _langGetpDimensionsOfToneConversionMatrix(pLang);
        TCMsize = pTCMdim[0] * pTCMdim[1] * pTCMdim[2];
        TCMsize = (TCMsize + 3) & ~3UL;
        return _langGetpToneConversionMatrix(pLang) + TCMsize;
    }
    pTCMdim = _langGetpDimensionsOfToneConversionMatrix(pLang);
    /* This offset is aligned relative to the accessor record. */
    i = (char *)pTCMdim - (char *)pLang;
    i = (i + 3) & ~3UL;
    return (char *)pLang + i;
}

/* Tests whether the phoneme lies in the normal phoneme range. */
static int _langIsNormalPhenome(struct _LANGUAGE *pLang, unsigned short PhenomeNbr)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    if (PhenomeNbr < pLanguage->NbrErgodicPhenomes) {
        return 0;
    }
    return 1;
}

/* Tests which of the requested language flags are set. */
static BOOL _langCheckBitField(struct _LANGUAGE *pLang, unsigned long bit)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;

    return pLanguage->BitField & bit;
}

/* Locates the transition-word lexicon after its header. */
static u16 *_transwGetpTransWordLex(struct _TRANSWORDINFO *pTransWord)
{
    return (u16 *)(pTransWord + 1);
}

/* Locates the table of transition-word offsets. */
static u16 *_transwGetpBeginOfTransWords(struct _TRANSWORDINFO *pTransWord)
{
    u16 *Ptr = (u16 *)pTransWord;

    Ptr += pTransWord->NbrPhenInLex;
    return Ptr + sizeof(*pTransWord) / sizeof(*Ptr);
}

/* Computes the size of the packed transition-word data. */
static u32 _transwGetSize(struct _LANGUAGE *pLang, struct _TRANSWORDINFO *pTransWord)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    unsigned long Size = pTransWord->NbrPhenInLex * sizeof(u16) + sizeof(*pTransWord);

    Size += (pLanguage->NbrTransWord + 1) * sizeof(u16);
    Size += (pTransWord->NbrTransPron + 1) * sizeof(u16);
    return Size;
}

/* Computes the size of a codebook including its matrices and optional data. */
static u32 _cdbGetCodeBookSize(struct _LANGUAGE *pLang, struct _CDBINFO *pCdb)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    struct _CDBINFO *pCdbInfo = pCdb;
    unsigned long Size;

    /* A four-byte entry per second-codebook vector precedes the probability data. */
    Size = sizeof(*pCdbInfo) + pCdbInfo->CodeBookDim * pCdbInfo->FirstCdbSize * sizeof(f32) +
            pCdbInfo->FirstCdbSize * pCdbInfo->NbrInSecSearch * sizeof(u32) +
            pCdbInfo->CodeBookDim * pCdbInfo->SecondCdbSize * sizeof(f32) +
            pCdbInfo->SecondCdbSize * 4 +
            pCdbInfo->SecondCdbSize * ((pLanguage->BitField & 1)
                ? (pCdbInfo->NbrGastone + 1) >> 1 : pCdbInfo->NbrGastone);
    if (pLanguage->BitField & 2) {
        Size = (Size + 3) & ~3UL;
        Size += pCdbInfo->SecondCdbSize * pCdbInfo->SecondCdbSize * sizeof(f32);
    }
    return Size;
}

/* Locates the first codebook vectors immediately after the header. */
static char *_cdbGetpFirstCdb(void *pCdb)
{
    struct _CDBINFO *pCdbInfo = pCdb;

    return (char *)(pCdbInfo + 1);
}

/* Locates the second-search index table after the first codebook vectors. */
static char *_cdbGetpIndexInSecCdb(void *pCdb)
{
    struct _CDBINFO *pCdbInfo = pCdb;

    return (char *)pCdb + pCdbInfo->CodeBookDim * pCdbInfo->FirstCdbSize * sizeof(f32) +
           sizeof(*pCdbInfo);
}

/* Locates the second codebook after the first vectors and their index table. */
static char *_cdbGetpSecondCdb(void *pCdb)
{
    struct _CDBINFO *pCdbInfo = pCdb;

    return (char *)pCdb + sizeof(*pCdbInfo) +
           pCdbInfo->CodeBookDim * pCdbInfo->FirstCdbSize * sizeof(f32) +
           pCdbInfo->FirstCdbSize * pCdbInfo->NbrInSecSearch * sizeof(u32);
}

/* Locates the probability matrix after the codebook vectors and metadata. */
static char *_cdbGetpProbMatrix(void *pCdb)
{
    struct _CDBINFO *pCdbInfo = pCdb;

    return (char *)pCdb +
           (pCdbInfo->CodeBookDim * pCdbInfo->FirstCdbSize * sizeof(f32)) +
           (pCdbInfo->FirstCdbSize * pCdbInfo->NbrInSecSearch * sizeof(u32)) +
           (pCdbInfo->CodeBookDim * pCdbInfo->SecondCdbSize * sizeof(f32)) +
           (pCdbInfo->SecondCdbSize * 4) + sizeof(*pCdbInfo);
}

/* Locates the smoothing matrix when the language format includes it. */
static char *_cdbGetpSmoothMatrix(struct _LANGUAGE *pLang, void *pCdb)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    struct _CDBINFO *pCdbInfo;
    unsigned long Size;

    if ((pLanguage->BitField & 2) == 0) {
        return NULL;
    }

    pCdbInfo = pCdb;
    /* A four-byte entry per second-codebook vector precedes the probability data. */
    Size = sizeof(*pCdbInfo) + pCdbInfo->CodeBookDim * pCdbInfo->FirstCdbSize * sizeof(f32) +
            pCdbInfo->FirstCdbSize * pCdbInfo->NbrInSecSearch * sizeof(u32) +
            pCdbInfo->CodeBookDim * pCdbInfo->SecondCdbSize * sizeof(f32) +
            pCdbInfo->SecondCdbSize * 4 +
            pCdbInfo->SecondCdbSize * ((pLanguage->BitField & 1)
                ? (pCdbInfo->NbrGastone + 1) >> 1 : pCdbInfo->NbrGastone);
    Size = (Size + 3) & ~3UL;
    return (char *)pCdbInfo + Size;
}

/* Computes the size of the codebook's probability matrix. */
static u32 _cdbGetProbMatrixSize(struct _LANGUAGE *pLang, struct _CDBINFO *pCdb)
{
    struct _LANGUAGEINFO *pLanguage = pLang->pLang;
    u32 Size;

    if (pLanguage->BitField & 1) {
        Size = (pCdb->NbrGastone + 1) >> 1;
    } else {
        Size = pCdb->NbrGastone;
    }
    return pCdb->SecondCdbSize * Size;
}

/* Computes the byte size of the square codebook smoothing matrix. */
static u32 _cdbGetSmoothMatrixSize(struct _CDBINFO *pCdb)
{
    return pCdb->SecondCdbSize * pCdb->SecondCdbSize * sizeof(f32);
}

/* Reads the penalty for a leading NBS event. */
static s32 _exevGetLeading_NBS_penalty(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Leading_NBS_penalty;
}

/* Reads the penalty for a trailing NBS event. */
static s32 _exevGetTrailing_NBS_penalty(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Trailing_NBS_penalty;
}

/* Reads the penalty for a rejection NBS event. */
static s32 _exevGetRejection_NBS_penalty(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Rejection_NBS_penalty;
}

/* Reads the path penalty for a rejection NBS event. */
static s32 _exevGetRejection_NBS_path_penalty(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Rejection_NBS_path_penalty;
}

/* Reads the silence phoneme used by the extra-event context. */
static u32 _exevGetExtraEventSilencePhenome(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->ExtraEventSilencePhenome;
}

/* Reads the number of entries in the extra-event lexicon. */
static u32 _exevGetNbrItemsInLexicon(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->NbrItemsInLexicon;
}

/* Reads the number of pronunciations in the extra-event lexicon. */
static u32 _exevGetNbrOfPronunciations(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->NbrOfPronunciations;
}

/* Reads the lexicon index for the leading NBS event. */
static u32 _exevGetLeading_NBS_WordIndex(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Leading_NBS_WordIndex;
}

/* Reads the lexicon index for the trailing NBS event. */
static u32 _exevGetTrailing_NBS_WordIndex(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Trailing_NBS_WordIndex;
}

/* Reads the lexicon index for the rejection NBS event. */
static u32 _exevGetRejection_NBS_WordIndex(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return pExevCtx->Rejection_NBS_WordIndex;
}

/* Locates the first entry in the extra-event lexicon. */
static u16 *_exevGetpBeginOfItems(struct _EXTRAEVENTCTXINFO *pExevCtx)
{
    return (u16 *)(pExevCtx + 1);
}

/* Locates the extra-event context's table of word offsets. */
static u32 *_exevGetpBeginOfWords(void *pExevCtx)
{
    struct _EXTRAEVENTCTXINFO *pExEvCtxInfo = pExevCtx;
    u32 NbrItems = pExEvCtxInfo->NbrItemsInLexicon;

    NbrItems += NbrItems & 1;
    return (u32 *)((u8 *)pExEvCtxInfo + sizeof(struct _EXTRAEVENTCTXINFO) + NbrItems * sizeof(u16));
}

/* Returns the null pointer for the optional pronunciation-offset table. */
static u32 *_exevGetpBeginOfProns(void)
{
    return 0;
}

/* Computes a phoneme's state count from consecutive table offsets. */
static u16 _GetNbrStatesInPhenome(u16 *pPhenToStat, unsigned short PhenomeNbr)
{
    return pPhenToStat[PhenomeNbr + 1] - pPhenToStat[PhenomeNbr];
}

/* Finds the states associated with a phoneme in the conversion table. */
static u16 _ConvPhenomesToStates(u16 *pPhenToStat, unsigned short PhenomeNbr, u16 **ppStates)
{
    *ppStates = pPhenToStat + pPhenToStat[PhenomeNbr];
    return pPhenToStat[PhenomeNbr + 1] - pPhenToStat[PhenomeNbr];
}

/* Accessors used by the version-two language resource. */
static const struct _LANGUAGE _LangVirtualTable = {
    0, _langCheckDataType,
    _langGetSize, _langGetVersionInfo,
    _langGetNbrCodeBook, _langGetNbrState,
    _langGetNbrPhenome, 0,
    _langGetAdaptSilState, _langGetNbrErgodicStates,
    _langGetNbrStateErgodicStates, _langGetNbrErgodicPhenomes,
    _langGetNbrStateErgodicPhenomes, _langGetNbrNonErgodicPhenomes,
    _langGetSilencePhenome, _langGetUWSilencePhenome,
    _langGetSingleWordGarbagePhenome, _langGetSentenceGarbagePhenome,
    _langGetAnySpeechGarbagePhenome, _langGetNbrWarpFactors,
    _langGetNbrSpeechUnit, _langGetNbrSpeechUnitClass,
    _langGetNbrTones, _langGetUserWordSpeechUnitClass,
    _langGetNbrTransWord, _langGetNbrPhenUserWordTraining,
    _langGetRecogWTP, _langGetSpellingWTP,
    _cdbGetCodeBookDim, _cdbGetFirstCdbSize,
    _cdbGetNbrInSecSearch, _cdbGetSecondCdbSize,
    _cdbGetCompStart, _cdbGetNbrGastone,
    _langGetpWarpFactors, _langGetpGastone,
    0, _langGetSilPhenome,
    _langGetpErgodicStates, _langGetpErgodicPhenomes,
    _langGetpErgodicPenalty, _langGetpCodeBook,
    _langGetpSpeechUnit, _langGetpTransWord,
    _langGetSizeSpeechUnits, 0,
    _langGetpPhenUserWordTraining, _langGetpDimensionsOfToneConversionMatrix,
    _langGetpOffsetForFinals, _langGetpToneConversionMatrix,
    _langGetpExtraEventContext, _langIsNormalPhenome,
    _langCheckBitField, _GetNbrStatesInPhenome,
    _ConvPhenomesToStates, _cdbGetCodeBookSize,
    _cdbGetProbMatrixSize, _cdbGetSmoothMatrixSize,
    _cdbGetpFirstCdb, _cdbGetpIndexInSecCdb,
    _cdbGetpSecondCdb, _cdbGetpProbMatrix,
    _cdbGetpSmoothMatrix, _transwGetSize,
    _transwGetpTransWordLex, _transwGetpBeginOfTransWords,
    _exevGetLeading_NBS_penalty, _exevGetTrailing_NBS_penalty,
    _exevGetRejection_NBS_penalty, _exevGetRejection_NBS_path_penalty,
    _exevGetExtraEventSilencePhenome, _exevGetNbrItemsInLexicon,
    _exevGetNbrOfPronunciations, _exevGetLeading_NBS_WordIndex,
    _exevGetTrailing_NBS_WordIndex, _exevGetRejection_NBS_WordIndex,
    _exevGetpBeginOfWords,
    /* _exevGetpBeginOfProns(void) fills the exevGetpBeginOfPronunciations(void *) slot. */
    (u32 *(*)(void *))_exevGetpBeginOfProns,
    _exevGetpBeginOfItems,
};

/* Installs the language resource accessors in the caller's language record. */
void FillLanguageVirtualTable(struct _LANGUAGE *pLang)
{
    memcpy(pLang, &_LangVirtualTable, sizeof(_LangVirtualTable));
}
