#include "gssdk/gsapi.h"
#include "gssdk/asrspi.h"
#include "gssdk/heap.h"
#include "gssdk/gsapi_internal.h"

#define GS_ERROR_FLAG (1U << 31)
#define GS_CONTEXT_ERROR_BASE (GS_ERROR_FLAG | (204U << 16))
#define GS_CONTEXT_ERROR_3 (GS_CONTEXT_ERROR_BASE | 3U)
#define GS_CONTEXT_ERROR_8 (GS_CONTEXT_ERROR_BASE | 8U)
#define GS_CONTEXT_ERROR_9 (GS_CONTEXT_ERROR_BASE | 9U)
#define GS_CONTEXT_OUT_OF_MEMORY (GS_CONTEXT_ERROR_BASE | 134U)
#define GS_SESSION_IMPORT_FAILURE (204U << 16)
#define GS_GCD_CHUNK_TYPE ('G' | ('C' << 8) | ('D' << 16) | (' ' << 24))

struct _GSAPI_ACTIONARRAY {
    unsigned long u32NbrItems; /* Number of indexed actions. */
    unsigned long *Action; /* Byte offset to the action array. */
};

typedef struct {
    unsigned long u32Pos; /* Position in the recognized word sequence. */
    unsigned long WordId; /* Word ID required at that position. */
} GSActionWordWithPos;

/* The action block stores the array addresses as offsets from its start. */
struct _GSAPI_ACTIONSTRUCT {
    unsigned long u32Type;
    unsigned long u32Size;
    union {
        struct _GSAPI_ACTIONARRAY Array;
        struct {
            unsigned long u32NbrItems;
            unsigned long *WordId;
            unsigned long *Action;
        } List;
        struct {
            unsigned long u32NbrItems;
            GSActionWordWithPos *WordWithPos;
            unsigned long *Action;
        } PosList;
    } Actions;
};

/* Saved recognition session with its payload stored immediately after this record. */
struct _GSAPI_SESSIONDATA {
    void *pData;
    unsigned long DataSize;
};

/* Allocates and exports the engine's current session. */
long SessionDataExport(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA **ppSessionData)
{
    unsigned long Size;
    long Error;
    struct _GSAPI_SESSIONDATA *pSessionData;

    if (ppSessionData != NULL) {
        *ppSessionData = NULL;
    }
    Error = asrspi_ExportSessionData(pEngine->hAsrSpiEngine, ASRBASE_SESSIONDATA, NULL, &Size);
    if (Error < 0) {
        return Error;
    }
    pSessionData = heap_Alloc(gGSAPI.hHeap, Size + sizeof(struct _GSAPI_SESSIONDATA));
    pSessionData->DataSize = Size;
    pSessionData->pData = pSessionData + 1;
    if (pSessionData == NULL) {
        return GS_CONTEXT_OUT_OF_MEMORY;
    }
    Error = asrspi_ExportSessionData(pEngine->hAsrSpiEngine, ASRBASE_SESSIONDATA,
                                     pSessionData->pData, &Size);
    if (Error < 0) {
        heap_Free(gGSAPI.hHeap, pSessionData);
        return Error;
    }
    gGSAPI.NumSessionDataBuffersLoaded++;
    if (ppSessionData != NULL) {
        *ppSessionData = pSessionData;
    }
    return Error;
}

/* Restores a saved recognition session and optionally releases its storage. */
long SessionDataImport(
    struct _GSAPI_ENGINE *pEngine, struct _GSAPI_SESSIONDATA *pSessionData, int bFreeMemory)
{
    long Error = asrspi_ActivateSessionData(pEngine->hAsrSpiEngine, pSessionData->pData);

    if (IsError(Error)) {
        Error = GS_SESSION_IMPORT_FAILURE;
    }
    if (bFreeMemory) {
        SessionDataFree(pSessionData);
    }
    return Error;
}

long SessionDataFree(struct _GSAPI_SESSIONDATA *pSessionData)
{
    if (pSessionData != 0) {
        heap_Free(gGSAPI.hHeap, pSessionData);
        gGSAPI.NumSessionDataBuffersLoaded--;
    }

    return 0;
}

/* Activates the context and restores the previous session when one was saved. */
long ContextActivate(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext)
{
    struct _GSAPI_SESSIONDATA *session = NULL;
    long result;

    if (pEngine->DoSessionDataSave) {
        result = SessionDataExport(pEngine, &session);
        if (result < 0) {
            return result;
        }
    }
    result = asrspi_ActivateCtx(pEngine->hAsrSpiEngine, pContext->pCtx, 1);
    if (result >= 0) {
        pEngine->pCurrentContext = pContext;
        pEngine->hActiveContext = pContext;
        result = ContextSetActiveWords(pEngine, pContext);
        if (result >= 0) {
            result = ActivateDefaultParams(pEngine);
        }
        if (result >= 0) {
            result = ContextActivateParams(pEngine, pContext);
        }
        if (pEngine->DoSessionDataSave && result >= 0) {
            long sessionStatus = SessionDataImport(pEngine, session, 0);

            result = sessionStatus;
        }
        pEngine->DoSessionDataSave = 1;
    }
    SessionDataFree(session);
    return result;
}

/* Maps the context's active parameter IDs to their SPI equivalents. */
long ContextActivateParams(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext)
{
    struct _ASRBASE_PARAM ParamList[17];
    s32 index;
    u32 mask;
    unsigned long count = 0;

    if (pContext->pParams == NULL) {
        return 0;
    }
    mask = pContext->pParams->u32UsedParams;
    for (index = 0; index <= 17; index++, mask >>= 1) {
        if (mask & 1) {
            enum _ASRBASE_PARAMID asrspiParamId =
                TranslateGsapiParamId2AsrSpiParamId((enum _GSAPI_PARAMID)index);

            if (asrspiParamId != 0xFF) {
                ParamList[count].ID = asrspiParamId;
                ParamList[count].Value = pContext->pParams->Params[index];
                count++;
            }
        }
    }
    return asrspi_SetParamList(pEngine->hAsrSpiEngine, ParamList, count);
}

/* Clears the GSAPI context pointers. */
long ContextAPIDeActivate(struct _GSAPI_ENGINE *pEngine)
{
    pEngine->pCurrentContext = 0;
    pEngine->hActiveContext = 0;
    return 0;
}

/* Deactivates the SPI context before clearing the cached context pointers. */
long ContextDeActivate(struct _GSAPI_ENGINE *pEngine)
{
    long lhError = asrspi_ActivateCtx(pEngine->hAsrSpiEngine, NULL, 0);

    if (IsError(lhError)) {
        return lhError;
    }
    pEngine->pCurrentContext = 0;
    pEngine->hActiveContext = 0;
    return 0;
}

long ContextGetAction(struct _GSAPI_CONTEXT *pContext, struct _ASRBASE_RESULT *pResult,
                      unsigned long *pAction)
{
    u32 count;
    struct _GSAPI_ACTIONSTRUCT *table;
    u32 key;

    *pAction = 0;
    if (pContext == NULL) {
        return 0;
    }
    table = pContext->pEvents;
    if (table == NULL) {
        return GS_CONTEXT_ERROR_9;
    }
    key = pResult->pSentences->pWords[1].pAlternatives->idWord;
    switch (table->u32Type) {
    case 2:
        if (key < table->Actions.Array.u32NbrItems) {
            *pAction = ((u32 *)((u8 *)table + (u32)table->Actions.Array.Action))[key];
        }
        break;
    case 3: {
        u32 index;
        u32 *actions;
        u32 *keys;
        u32 limit;

        limit = table->Actions.List.u32NbrItems;
        index = 0;
        actions = (u32 *)((u8 *)table + (u32)table->Actions.List.Action);
        keys = (u32 *)((u8 *)table + (u32)table->Actions.List.WordId);

        for (; index < limit; index++) {
            if (key == keys[index]) {
                *pAction = actions[index];
                break;
            }
        }
        break;
    }
    case 4: {
        u32 *actions;
        GSActionWordWithPos *entries;
        u32 index;

        count = table->Actions.PosList.u32NbrItems;
        entries = (GSActionWordWithPos *)((u8 *)table + (u32)table->Actions.PosList.WordWithPos);
        actions = (u32 *)((u8 *)table + (u32)table->Actions.PosList.Action);

        for (index = 0; index < count; index++) {
            if (pResult->pSentences->iNbrWords > (long)entries[index].u32Pos &&
                key == entries[index].WordId) {
                *pAction = actions[index];
            }
        }
        break;
    }
    }
    return 0;
}

long ContextGetParam(struct _GSAPI_CONTEXT *pContext, enum _GSAPI_PARAMID ParamId,
                     unsigned long *pValue)
{
    struct GSAPIParamBlock *parameters = pContext->pParams;

    if (parameters == 0) {
        return GS_CONTEXT_ERROR_8;
    }
    if ((parameters->u32UsedParams & (1U << ParamId)) == 0) {
        return GS_CONTEXT_ERROR_8;
    }

    *pValue = parameters->Params[ParamId];
    return 0;
}

/* Activates every word for a context that supplies a word ID list. */
long ContextSetActiveWords(struct _GSAPI_ENGINE *pEngine, struct _GSAPI_CONTEXT *pContext)
{
    if (pContext->pWordIds == NULL) {
        return 0;
    }

    /* GP passes the all-words selector when activating a context. */
    return asrspi_ActivateWords(pEngine->hAsrSpiEngine, pContext->pWordIds, 0xFFFFFFFFU);
}

long ContextSetCtxData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32CtxData)
{
    pContext->pCtx = (void *)pu32CtxData;
    return 0;
}

long ContextSetGcdData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32GcdData)
{
    unsigned long GcdFileType;
    unsigned long GcdFileSize;
    unsigned char *pBlock;

    if (pContext->pParams != NULL) {
        if (pContext->pParams->u32Size == 0) {
            heap_Free(gGSAPI.hHeap, pContext->pParams);
        }
        pContext->pParams = NULL;
    }
    if (pu32GcdData == NULL) {
        pContext->pGcd = NULL;
        pContext->pEvents = NULL;
        return 0;
    }
    GcdFileType = pu32GcdData[0];
    if (GcdFileType != GS_GCD_CHUNK_TYPE) {
        return GS_CONTEXT_ERROR_3;
    }
    pContext->pGcd = (void *)pu32GcdData;
    pContext->pEvents = NULL;
    GcdFileSize = ((const unsigned long *)pContext->pGcd)[1];
    if (GcdFileSize <= 0x8) {
        return GS_CONTEXT_ERROR_3;
    }
    pBlock = (unsigned char *)pContext->pGcd + 0x8;
    while (pBlock != (unsigned char *)pContext->pGcd + GcdFileSize) {
        unsigned long u32BlockSize = ((const unsigned long *)pBlock)[1];
        unsigned long u32BlockType = ((const unsigned long *)pBlock)[0];

        if (u32BlockSize == 0) {
            return GS_CONTEXT_ERROR_3;
        }
        switch (u32BlockType) {
        case 1:
            pContext->pParams = (struct GSAPIParamBlock *)pBlock;
            if (pContext->pParams->u32Size != sizeof(struct GSAPIParamBlock)) {
                return GS_CONTEXT_ERROR_3;
            }
            break;
        case 2:
            pContext->pEvents = (struct _GSAPI_ACTIONSTRUCT *)pBlock;
            break;
        case 3:
            pContext->pEvents = (struct _GSAPI_ACTIONSTRUCT *)pBlock;
            break;
        case 4:
            pContext->pEvents = (struct _GSAPI_ACTIONSTRUCT *)pBlock;
            break;
        default:
            return GS_CONTEXT_ERROR_3;
        }
        pBlock += u32BlockSize;
    }
    return 0;
}

long ContextSetParam(struct _GSAPI_CONTEXT *pContext, enum _GSAPI_PARAMID ParamId,
                     unsigned long Value)
{
    if (pContext->pParams == 0) {
        pContext->pParams = heap_Calloc(gGSAPI.hHeap, 1, sizeof(struct GSAPIParamBlock));
        if (pContext->pParams == 0) {
            return GS_CONTEXT_OUT_OF_MEMORY;
        }
    }
    pContext->pParams->u32UsedParams |= 1U << ParamId;
    pContext->pParams->Params[ParamId] = Value;
    return 0;
}

long ContextSetWrdData(struct _GSAPI_CONTEXT *pContext, const unsigned long *pu32WrdData)
{
    pContext->pWrd = (void *)pu32WrdData;
    if (pContext->pWordList != NULL) {
        WrdDestroyWordList(gGSAPI.hHeap, pContext->pWordList);
    }
    if (pContext->pWrd != NULL) {
        if (pContext->pWordList == NULL) {
            pContext->pWordList = heap_Alloc(gGSAPI.hHeap, sizeof(struct tag_WRD_WORDLIST));
            if (pContext->pWordList == NULL) {
                return GS_CONTEXT_OUT_OF_MEMORY;
            }
        }
        WrdCreateWordList(gGSAPI.hHeap, pContext->pWordList, pContext->pWrd);
    } else if (pContext->pWordList != NULL) {
        heap_Free(gGSAPI.hHeap, pContext->pWordList);
        pContext->pWordList = NULL;
    }
    return 0;
}

long ContextUnLoad(struct _GSAPI_CONTEXT *pContext)
{
    pContext->pWrd = NULL;
    pContext->pCtx = NULL;
    if (pContext->pParams != NULL && pContext->pParams->u32Size == 0) {
        heap_Free(gGSAPI.hHeap, pContext->pParams);
    }
    pContext->pGcd = NULL;
    pContext->pParams = NULL;
    pContext->pEvents = NULL;
    if (pContext->pWordList != NULL) {
        WrdDestroyWordList(gGSAPI.hHeap, pContext->pWordList);
        heap_Free(gGSAPI.hHeap, pContext->pWordList);
        pContext->pWordList = NULL;
    }
    if (pContext->pWordIds != NULL) {
        heap_Free(gGSAPI.hHeap, pContext->pWordIds);
        pContext->pWordIds = NULL;
    }
    return 0;
}
