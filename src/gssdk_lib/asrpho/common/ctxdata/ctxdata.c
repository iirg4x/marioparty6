/* Reads recognition context metadata and locates the tables in a context record. */
#include "gssdk/context.h"
#include "gssdk/engine.h"
#include "types.h"

#include <stddef.h>
#include <string.h>

/* Checks that the record contains recognition context data. */
static int _contextCheckDataType(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->DataType == 0x6;
}

/* Returns the recorded size of the recognition context. */
static unsigned long _contextGetSize(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->Size;
}

/* Locates the language version string for the context. */
static char *_contextGetLanguageVersionInfo(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->VersionInfo;
}

/* Returns the context setting for terminal silence. */
static unsigned long _contextGetTS(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->TS;
}

/* Returns the context's accuracy setting. */
static unsigned long _contextGetAccuracy(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->Accuracy;
}

/* Returns the context's rejection setting. */
static unsigned long _contextGetRejection(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->Rejection;
}

/* Returns the context's garbage setting. */
static unsigned long _contextGetGarbage(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->Garbage;
}

/* Returns the number of items recorded in the context. */
static unsigned long _contextGetNbrItem(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrItem;
}

/* Returns the number of words in the context. */
static unsigned long _contextGetNbrWord(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrWord;
}

/* Returns the number of pronunciations in the context. */
static unsigned long _contextGetNbrPron(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrPron;
}

/* Returns the number of follower entries in the context. */
static unsigned long _contextGetNbrFollower(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrFollower;
}

/* Returns the number of words that can start a recognition path. */
static unsigned long _contextGetNbrStartWord(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrStartWord;
}

/* Returns the number of words that can end a recognition path. */
static unsigned long _contextGetNbrStopWord(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->NbrStopWord;
}

/* Returns the word ID reserved for garbage recognition. */
static unsigned long _contextGetGarbageWordID(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->GarbageWordID;
}

/* Tests whether the context is used for spelling recognition. */
static unsigned char _contextIsSpellingContext(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->ContextType == 1;
}

/* Tests whether the context is used for isolated-word recognition. */
static unsigned char _contextIsIsoWordContext(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return pContext->ContextType == 0x2;
}

/* Locates the lexicon in the packed context data. */
static unsigned short *_contextGetpLex(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;

    return (unsigned short *)(pContext + 1);
}

/* Locates the table of word offsets in the context data. */
static unsigned long *_contextGetpBeginOfWords(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;
    unsigned long NbrItem = pContext->NbrItem + pContext->NbrPron;

    NbrItem += NbrItem & 1;
    return (unsigned long *)(_contextGetpLex(pCtx) + NbrItem);
}

/* Locates the word property table in the context data. */
static unsigned short *_contextGetpWordProp(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;
    char *Ptr = (char *)_contextGetpBeginOfWords(pCtx);

    Ptr += ((pContext->NbrPron + 2) + pContext->NbrWord) * sizeof(u32);
    return (unsigned short *)Ptr;
}

/* Locates the syntax table in the context data. */
static unsigned long *_contextGetpSyntax(struct _ASRSPI_CONTEXT *pCtx)
{
    struct _ASRSPI_CONTEXTINFO *pContext = pCtx->pCtx;
    unsigned long NbrWord = pContext->NbrWord;

    NbrWord += NbrWord & 1;
    return (unsigned long *)(_contextGetpWordProp(pCtx) + NbrWord);
}

/* Accessors for the version-two packed context format. */
static const struct _ASRSPI_CONTEXT _ContextV2VirtualTable = {
    NULL, _contextCheckDataType,
    _contextGetSize, _contextGetLanguageVersionInfo,
    _contextGetTS, _contextGetAccuracy,
    _contextGetRejection, _contextGetGarbage,
    _contextGetNbrItem, _contextGetNbrWord,
    _contextGetNbrPron, _contextGetNbrFollower,
    _contextGetNbrStartWord, _contextGetNbrStopWord,
    _contextGetGarbageWordID, _contextIsSpellingContext,
    _contextGetpLex, _contextGetpBeginOfWords,
    _contextGetpWordProp, _contextGetpSyntax,
    _contextIsIsoWordContext,
};

/* Installs the version-two context accessors in the caller's context record. */
void FillContextV2VirtualTable(struct _ASRSPI_CONTEXT *pCtx)
{
    memcpy(pCtx, &_ContextV2VirtualTable, sizeof(_ContextV2VirtualTable));
}
