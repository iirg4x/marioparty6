/* Applies a liftered discrete cosine transform to each input frame. */
#include "dolphin/math.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/cmath>
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include "gssdk/tos.h"

/* Transform coefficients and input and output frame lengths. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 nCep; /* Output cepstral coefficient count. */
    u32 nLogPower; /* Input log-power band count. */
    u32 FirstBand; /* First input band used by the transform. */
    f32 *pTransformer; /* Cosine transform weights with the lifter applied. */
    f32 Scale; /* Multiplier applied to each output coefficient. */
} DCTLift;

/* Transforms one input frame into the output queue. */
static void ProcessDCTLift(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    DCTLift *This = (DCTLift *)_This;
    f32 *pLogPowerIn = ((void **)pInputs)[0];
    f32 *pLogPowerEnd;
    f32 *pLogPower;
    f32 *pCep;
    f32 *pCepEnd;
    f32 *t;
    f32 Scale;

    if (pLogPowerIn == NULL) {
        return;
    }

    pCep = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    t = This->pTransformer;
    Scale = This->Scale;
    *pCep++ = *pLogPowerIn++;
    pCepEnd = pCep + This->nCep;
    pLogPowerEnd = pLogPowerIn + This->nLogPower;
    pLogPowerIn += This->FirstBand;

    while (pCep < pCepEnd) {
        pLogPower = pLogPowerIn;
        *pCep = 0.0f;
        while (pLogPower < pLogPowerEnd) {
            *pCep += *t++ * *pLogPower++;
        }
        *pCep++ *= Scale;
    }

    pCep = pCepEnd - This->nCep;
    *pCep *= 0.5f;
}

/* Releases the transform table and block on shutdown. */
static u8 ControlDCTLift(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    DCTLift *This = (DCTLift *)_This;

    switch (ControlId) {
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pTransformer);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Builds the coefficient table and sizes the queues from the profile. */
static unsigned char InitDCTLift(struct _BLOCK *_This)
{
    DCTLift *This = (DCTLift *)_This;
    u32 nCep;
    s32 HalfCycle;
    f32 *p;
    u32 j;
    u32 i;
    u32 nLogPowerUsed;
    f32 LiftCoef;

    nCep = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 2, 13);
    This->nCep = nCep;
    HalfCycle = (s16)_tosGetProfileU32(&This->BaseBlock.Object, 3, 16);
    This->nLogPower = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 1, 21);
    This->FirstBand = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 4, 0);

    nLogPowerUsed = This->nLogPower - This->FirstBand;
    This->BaseBlock.InputQueues->Size =
        (This->nLogPower + 1) * sizeof(f32);
    This->BaseBlock.OutputQueues->Size =
        (nCep + 1) * sizeof(f32);

    This->Scale = 2.0 / (f32)nLogPowerUsed;
    This->pTransformer = heap_Alloc(
        This->BaseBlock.Object.pTos->hHeap,
        nCep * nLogPowerUsed * sizeof(f32));

    p = This->pTransformer;
    for (i = 0; i < nCep; i++) {
        LiftCoef = 1.0f + 6.0f * sinf(M_PI * (f32)i / (f32)HalfCycle);
        for (j = 0; j < nLogPowerUsed; j++) {
            *p++ =
                LiftCoef * cosf(M_PI * ((f32)j + 0.5) * (f32)i / (f32)nLogPowerUsed);
        }
    }
    return 0;
}

/* Allocates a transform block with one input and output. */
struct _BLOCK *ConstructDCTLift(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 1, 1, ProcessDCTLift, InitDCTLift,
        ControlDCTLift, sizeof(DCTLift));
}
