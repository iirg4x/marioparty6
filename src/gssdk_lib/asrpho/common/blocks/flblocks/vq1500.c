/* Labels acoustic feature vectors by searching the configured codebooks. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <float.h>

#include "gssdk/langdata.h"
#include "gssdk/tos.h"

/* Processing block and the language codebooks used by vector quantization. */
struct _VQ1500 {
    struct _BLOCK BaseBlock;          /* Scheduler-owned block header. */
    u8 NbrOfCodeBooks;               /* Number of codebooks loaded from the active language. */
    struct _CODEBOOK *pTheCodeBooks; /* Queue-label search data for each codebook. */
};

/* Releases the loaded codebook data. */
static void FreeCodeBooks(struct _TOS *pTos, struct _VQ1500 *This);
/* Releases the vector quantizer resources. */
static void FreeVq1500(struct _VQ1500 *This);

/* Sets queue sizes and initializes codebook state. */
static unsigned char InitVq1500Process(struct _BLOCK *_This)
{
    _This->InputQueues->Size = _tosGetProfileU32(&_This->Object, 1, 38) * sizeof(f32);
    _This->OutputQueues->Size = (u8) _tosGetProfileU32(&_This->Object, 2, 4);
    return 0;
}

/* Loads vector codebooks associated with the active language. */
static unsigned char LoadCodeBooks(struct _VQ1500 *This, struct _LANGUAGE *pLang)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    struct _CODEBOOK *pActCodeBook;
    u32 i;
    struct _CDBINFO *pCodeBookInfo;

    This->NbrOfCodeBooks = pLang->langGetNbrCodeBook(pLang);
    FreeCodeBooks(pTos, This);
    This->pTheCodeBooks = heap_Alloc(pTos->hHeap, This->NbrOfCodeBooks * sizeof(struct _CODEBOOK));
    if (This->pTheCodeBooks == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 2);
    }

    pActCodeBook = This->pTheCodeBooks;
    for (i = 0; i < This->NbrOfCodeBooks; pActCodeBook++, i++) {
        pCodeBookInfo = pLang->langGetpCodeBook(pLang, i);
        pActCodeBook->CodeBookDim = pLang->cdbGetCodeBookDim(pCodeBookInfo);
        pActCodeBook->FirstCdbSize = pLang->cdbGetFirstCdbSize(pCodeBookInfo);
        pActCodeBook->NbrInSecSearch = pLang->cdbGetNbrInSecSearch(pCodeBookInfo);
        pActCodeBook->SecondCdbSize = pLang->cdbGetSecondCdbSize(pCodeBookInfo);
        pActCodeBook->CompStart = (u8) pLang->cdbGetCompStart(pCodeBookInfo);
        pActCodeBook->pFirstCdb = (f32 *)pLang->cdbGetpFirstCdb(pCodeBookInfo);
        pActCodeBook->pIndexInSecCdb = (u32 *)pLang->cdbGetpIndexInSecCdb(pCodeBookInfo);
        pActCodeBook->pSecondCdb = (f32 *)pLang->cdbGetpSecondCdb(pCodeBookInfo);
    }
    return 0;
}

/* Finds the closest codebook vector and returns its label. */
static u8 GetLabel(struct _CODEBOOK *pCodeBook, f32 *pVector)
{
    f32 Min;
    f32 Dist;
    f32 diff;
    u8 Label;
    s32 i;
    s32 j;

    Min = FLT_MAX;
    Label = 0;
    {
        f32 *pCdbs;
        f32 *pX;

        for (i = 0; i < pCodeBook->FirstCdbSize; i++) {
            pCdbs = pCodeBook->pFirstCdb + i * pCodeBook->CodeBookDim;
            pX = pVector;
            diff = *pX++ - *pCdbs++;
            Dist = diff * diff;
            for (j = 1; j < pCodeBook->CodeBookDim; j++) {
                diff = *pX++ - *pCdbs++;
                Dist += diff * diff;
                if (Dist > Min) {
                    break;
                }
            }
            if (Dist < Min) {
                Min = Dist;
                Label = i;
            }
        }
    }

    {
        u32 *pIndexInSecCdb;
        s32 i;
        f32 *pSecSearchVector;
        f32 *pX;

        pIndexInSecCdb = pCodeBook->pIndexInSecCdb + pCodeBook->NbrInSecSearch * Label;
        Min = FLT_MAX;
        for (i = 0; i < pCodeBook->NbrInSecSearch; i++) {
            pSecSearchVector = pCodeBook->pSecondCdb + *pIndexInSecCdb++;
            pX = pVector;
            diff = *pX++ - *pSecSearchVector++;
            Dist = diff * diff;
            for (j = 1; j < pCodeBook->CodeBookDim; j++) {
                diff = *pX++ - *pSecSearchVector++;
                Dist += diff * diff;
                if (Dist > Min) {
                    break;
                }
            }
            if (Dist < Min) {
                Min = Dist;
                /* The second-stage vector is followed by its 32-bit label. */
                Label = *(const u32 *) pSecSearchVector;
            }
        }
    }
    return Label;
}

/* Leaves the output queues untouched while the quantizer is idle. */
static void IdleVq1500Process(void)
{
}

/* Scores an incoming vector and queues its label. */
static void Vq1500Process(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    struct _VQ1500 *This = (struct _VQ1500 *) _This;
    u8 *Outvec;
    struct _CODEBOOK *pCodeBook;
    f32 *pIn;
    s32 i;

    pIn = ((void **) pInputs)[0];
    if (pIn == NULL) {
        return;
    }

    Outvec = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    pCodeBook = This->pTheCodeBooks;
    for (i = 0; i < This->NbrOfCodeBooks; i++, pCodeBook++) {
        *Outvec++ = GetLabel(pCodeBook, pIn + pCodeBook->CompStart);
    }
}

/* Loads the active codebooks or releases the block. */
static unsigned char ControlVq1500Process(struct _BLOCK *_This, unsigned char ControlId,
                                          unsigned long CParam1)
{
    struct _VQ1500 *This = (struct _VQ1500 *) _This;

    switch (ControlId) {
    case 0x01:
        if (LoadCodeBooks(This, (struct _LANGUAGE *) CParam1) != 0) {
            /* The idle handler consumes none of the process arguments. */
            This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int)) IdleVq1500Process;
        } else {
            This->BaseBlock.Process = Vq1500Process;
        }
        break;
    case 0xFF:
        FreeVq1500(This);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Releases the loaded codebook data. */
static void FreeCodeBooks(struct _TOS *pTos, struct _VQ1500 *This)
{
    if (This->pTheCodeBooks != NULL) {
        heap_Free(pTos->hHeap, This->pTheCodeBooks);
        This->pTheCodeBooks = NULL;
    }
}

/* Releases the vector quantizer resources. */
static void FreeVq1500(struct _VQ1500 *This)
{
    FreeCodeBooks(This->BaseBlock.Object.pTos, This);
    tosBaseBlockDestruct(&This->BaseBlock);
}

/* Registers the vector quantizer with the scheduler. */
struct _BLOCK *ConstructVq(struct _TOS *pTos, u8 BlockID)
{
    /* The idle handler takes no arguments. The control handler takes one payload word. */
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1,
                                 (void (*)(struct _BLOCK *, void *, int)) IdleVq1500Process,
                                 InitVq1500Process,
                                 (u8 (*)(struct _BLOCK *, u8, u32, u32)) ControlVq1500Process,
                                 sizeof(struct _VQ1500));
}
