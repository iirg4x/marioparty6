#include "types.h"

#include <stddef.h>
#include <string.h>

typedef struct ContextDataV2 {
    u32 dataType;
    u32 size;
    u8 languageVersionInfo[0x80];
    u32 ts;
    u32 accuracy;
    u32 rejection;
    u32 garbage;
    u32 nbrItem;
    u32 nbrWord;
    u32 nbrPron;
    u32 nbrFollower;
    u32 nbrStartWord;
    u32 nbrStopWord;
    u32 garbageWordID;
    u32 contextType;
    u16 lex[1];
} ContextDataV2;

typedef struct ContextData {
    ContextDataV2 *data;
} ContextData;

typedef void (*ContextV2Function)(void);

static BOOL _contextCheckDataType(ContextData *pCtx)
{
    return pCtx->data->dataType == 6;
}

static u32 _contextGetSize(ContextData *pCtx)
{
    return pCtx->data->size;
}

static void *_contextGetLanguageVersionInfo(ContextData *pCtx)
{
    return pCtx->data->languageVersionInfo;
}

static u32 _contextGetTS(ContextData *pCtx)
{
    return pCtx->data->ts;
}

static u32 _contextGetAccuracy(ContextData *pCtx)
{
    return pCtx->data->accuracy;
}

static u32 _contextGetRejection(ContextData *pCtx)
{
    return pCtx->data->rejection;
}

static u32 _contextGetGarbage(ContextData *pCtx)
{
    return pCtx->data->garbage;
}

static u32 _contextGetNbrItem(ContextData *pCtx)
{
    return pCtx->data->nbrItem;
}

static u32 _contextGetNbrWord(ContextData *pCtx)
{
    return pCtx->data->nbrWord;
}

static u32 _contextGetNbrPron(ContextData *pCtx)
{
    return pCtx->data->nbrPron;
}

static u32 _contextGetNbrFollower(ContextData *pCtx)
{
    return pCtx->data->nbrFollower;
}

static u32 _contextGetNbrStartWord(ContextData *pCtx)
{
    return pCtx->data->nbrStartWord;
}

static u32 _contextGetNbrStopWord(ContextData *pCtx)
{
    return pCtx->data->nbrStopWord;
}

static u32 _contextGetGarbageWordID(ContextData *pCtx)
{
    return pCtx->data->garbageWordID;
}

static u8 _contextIsSpellingContext(ContextData *pCtx)
{
    return pCtx->data->contextType == 1;
}

static u8 _contextIsIsoWordContext(ContextData *pCtx)
{
    return pCtx->data->contextType == 2;
}

static u16 *_contextGetpLex(ContextData *pCtx)
{
    return pCtx->data->lex;
}

static u16 *_contextGetpBeginOfWords(ContextData *pCtx)
{
    ContextDataV2 *data = pCtx->data;
    u32 count = data->nbrItem + data->nbrPron;

    count += count & 1;
    return _contextGetpLex(pCtx) + count;
}

static void *_contextGetpWordProp(ContextData *pCtx)
{
    ContextDataV2 *data = pCtx->data;
    char *wordProp = (char *)_contextGetpBeginOfWords(pCtx);
    wordProp += ((data->nbrPron + 2) + data->nbrWord) * sizeof(u32);
    return wordProp;
}

static void *_contextGetpSyntax(ContextData *pCtx)
{
    ContextDataV2 *data = pCtx->data;
    u32 syntaxCount = data->nbrWord;

    syntaxCount += syntaxCount & 1;
    return (u16 *)_contextGetpWordProp(pCtx) + syntaxCount;
}

static const ContextV2Function _ContextV2VirtualTable[] = {
    NULL,
    (ContextV2Function)_contextCheckDataType,
    (ContextV2Function)_contextGetSize,
    (ContextV2Function)_contextGetLanguageVersionInfo,
    (ContextV2Function)_contextGetTS,
    (ContextV2Function)_contextGetAccuracy,
    (ContextV2Function)_contextGetRejection,
    (ContextV2Function)_contextGetGarbage,
    (ContextV2Function)_contextGetNbrItem,
    (ContextV2Function)_contextGetNbrWord,
    (ContextV2Function)_contextGetNbrPron,
    (ContextV2Function)_contextGetNbrFollower,
    (ContextV2Function)_contextGetNbrStartWord,
    (ContextV2Function)_contextGetNbrStopWord,
    (ContextV2Function)_contextGetGarbageWordID,
    (ContextV2Function)_contextIsSpellingContext,
    (ContextV2Function)_contextGetpLex,
    (ContextV2Function)_contextGetpBeginOfWords,
    (ContextV2Function)_contextGetpWordProp,
    (ContextV2Function)_contextGetpSyntax,
    (ContextV2Function)_contextIsIsoWordContext,
};

void FillContextV2VirtualTable(ContextV2Function *pCtx)
{
    memcpy(pCtx, _ContextV2VirtualTable, sizeof(_ContextV2VirtualTable));
}
