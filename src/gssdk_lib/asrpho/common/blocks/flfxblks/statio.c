/* Stationarity detection from changes in frame energy. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include "gssdk/tos.h"

/* Energy history and delayed activity values for the stationarity check. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 Flushing; /* Set when flushing the delayed activity values. */
    u32 Delay; /* Frames buffered toward the delay. */
    u32 MaxDelay; /* Delay in frames. */
    f32 StationarityThreshold; /* Log-energy range above which the frame is nonstationary. */
    /* Log-energy ring bounds and the next sample slot. */
    f32 *pBeginStationarityBuf;
    f32 *pEndStationarityBuf;
    f32 *pWriteStationarityBuf;
    /* Activity delay ring bounds and the next sample slot. */
    f32 *pBeginVadDelayBuf;
    f32 *pEndVadDelayBuf;
    f32 *pWriteVadDelayBuf;
} Stationarity;

/* Returns the difference between the maximum and minimum frame log energy. */
static f32 EnergyVariation(Stationarity *This)
{
    f32 *pBeg = This->pBeginStationarityBuf;
    f32 *pEnd = This->pEndStationarityBuf;
    f32 MaxVal;
    f32 MinVal = *pBeg;

    MaxVal = MinVal;
    while (++pBeg < pEnd) {
        f32 value = *pBeg;

        if (value < MinVal) {
            MinVal = value;
        }
        if (value > MaxVal) {
            MaxVal = value;
        }
    }
    return MaxVal - MinVal;
}

/* Attenuates delayed activity when the energy range crosses the stationarity threshold. */
static void ProcessStationarity(struct _BLOCK *_This, void *pInputs)
{
    Stationarity *This = (Stationarity *)_This;
    f32 LogE;
    f32 Vad = 0.0f;
    f32 VadOut;
    f32 *pBuf;
    f32 *pFrameWeight;

    if (((void **)pInputs)[0] != NULL) {
        LogE = *(f32 *)((void **)pInputs)[0];
        Vad = *(f32 *)((void **)pInputs)[1];
        if (This->Delay == 0) {
            pBuf = This->pBeginStationarityBuf;
            while (pBuf < This->pEndStationarityBuf) {
                *pBuf = LogE;
                pBuf++;
            }
            This->pWriteStationarityBuf = This->pBeginStationarityBuf;
        } else {
            *This->pWriteStationarityBuf++ = LogE;
            if (This->pWriteStationarityBuf == This->pEndStationarityBuf) {
                This->pWriteStationarityBuf = This->pBeginStationarityBuf;
            }
        }
        This->Delay++;
    } else if (This->Flushing == 0) {
        return;
    }

    VadOut = *This->pWriteVadDelayBuf;
    *This->pWriteVadDelayBuf++ = Vad;
    if (This->pWriteVadDelayBuf == This->pEndVadDelayBuf) {
        This->pWriteVadDelayBuf = This->pBeginVadDelayBuf;
    }
    if (This->Flushing != 0 && This->Delay == 0) {
        This->Flushing = 0;
    }
    if (This->Delay <= This->MaxDelay && This->Flushing == 0) {
        return;
    }

    pFrameWeight = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    This->Delay--;
    if (VadOut > 0.0f) {
        if (!(EnergyVariation(This) < This->StationarityThreshold)) {
            VadOut *= 0.1f;
        }
    }
    *pFrameWeight = VadOut;
}

/* Starts draining buffered activity on flush and releases the buffers on shutdown. */
static u8 ControlStationarity(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    Stationarity *This = (Stationarity *)_This;
    struct _TOS *pTos = _This->Object.pTos;

    switch (ControlId) {
    case 0x01:
        This->Flushing = 1;
        break;
    case 0xFF:
        heap_Free(pTos->hHeap, This->pBeginStationarityBuf);
        heap_Free(pTos->hHeap, This->pBeginVadDelayBuf);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Sizes the ports and initializes the energy and activity history buffers. */
static u8 InitStationarity(struct _BLOCK *_This)
{
    Stationarity *This = (Stationarity *)_This;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    u32 i;

    This->BaseBlock.InputQueues[0].Size = 6 * sizeof(f32); /* VAD levels. Uses frame log energy. */
    /* VAD activity score, delayed by the stationarity block. */
    This->BaseBlock.InputQueues[1].Size = sizeof(f32);
    This->BaseBlock.OutputQueues->Size = sizeof(f32);
    This->MaxDelay = (u8)_tosGetProfileU32(&This->BaseBlock.Object, 1, 2);
    This->StationarityThreshold = _tosGetProfileFloat(&This->BaseBlock.Object, 2, 2.0f);
    This->pBeginStationarityBuf = heap_Calloc(pTos->hHeap, This->MaxDelay * 2 + 1, sizeof(f32));
    This->pBeginVadDelayBuf = heap_Calloc(pTos->hHeap, This->MaxDelay, sizeof(f32));
    if (This->pBeginStationarityBuf == NULL || This->pBeginVadDelayBuf == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 2);
    }
    for (i = 0; i < This->MaxDelay * 2 + 1; i++) {
        This->pBeginStationarityBuf[i] = 0.0f;
    }
    This->pEndStationarityBuf = This->pBeginStationarityBuf + This->MaxDelay * 2 + 1;
    This->pWriteStationarityBuf = This->pBeginStationarityBuf;
    This->pEndVadDelayBuf = This->pBeginVadDelayBuf + This->MaxDelay;
    This->pWriteVadDelayBuf = This->pBeginVadDelayBuf;
    return 0;
}

/* Constructs a stationarity block with energy and activity inputs. */
struct _BLOCK *ConstructStationarity(struct _TOS *pTos, u8 BlockID)
{
    /* ProcessStationarity reads the two input pointers without using their count. */
    return tosBaseBlockConstruct(
        pTos, BlockID, 2, 1, (void (*)(struct _BLOCK *, void *, int))ProcessStationarity,
        InitStationarity,
        ControlStationarity, sizeof(Stationarity));
}
