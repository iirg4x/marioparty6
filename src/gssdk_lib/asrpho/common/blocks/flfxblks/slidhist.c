/* Tracks recent TriggerLR peak-energy values to estimate a lower quantile. */
#include "gssdk/heap.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>

#include "types.h"

#include <string.h>

#include "gssdk/triggerlr.h"

/* Replaces the oldest bin index and advances the circular cursor. */
static int Fifo_PushPop(TriggerLR *This, int Bin)
{
    int RetVal;

    RetVal = *This->pOldestEntry;
    *This->pOldestEntry = Bin;
    if (++This->pOldestEntry == This->pFifoEnd) {
        This->pOldestEntry = This->pFifoStart;
    }
    return RetVal;
}

/* Removes an expired observation if its bin index is valid. */
static void Histo_Remove(TriggerLR *This, int Bin)
{
    if (Bin >= 0) {
        // Remove the value being overwritten so the histogram covers only recent peaks.
        This->pHisto[Bin]--;
        This->NbInHisto--;
    }
}

// Adds a tracked candidate's peak to the rolling histogram.
// TriggerLR calls this at candidate update intervals and when a candidate ends.
void SlidingHisto_NewItem(TriggerLR *This, f32 LogE)
{
    s32 itemBin;
    s32 expiredBin;
    f32 minimum = This->BinMin;
    f32 range = This->BinRng;
    f32 maximum = minimum + range;

    if (LogE > maximum) {
        itemBin = This->NbBinsm1;
    } else if (LogE < minimum) {
        itemBin = 0;
    } else {
        // Add half a bin width before flooring so values round to the nearest bin.
        f32 offset = LogE - minimum;
        f32 binWidth = This->BinsWidth;
        f32 halfWidth = binWidth / 2.0f;
        f32 centered = halfWidth + offset;
        f32 position = centered / binWidth;
        f32 roundedPosition = floorf(position);
        itemBin = (s32)roundedPosition;
    }

    This->pHisto[itemBin]++;
    This->NbInHisto++;

    expiredBin = Fifo_PushPop(This, itemBin);

    Histo_Remove(This, expiredBin);
}

// Returns the first bin whose accumulated count reaches ceil(0.001 + quantile * item count).
// TriggerLR calls this after histogram updates and after restoring valid saved state.
f32 SlidingHisto_LowerQuantile(TriggerLR *This, f32 Quantile)
{
    u32 *pHisto = This->pHisto;
    u32 CumSum = 0;
    u32 QuantThr;

    // The small bias makes a zero Quantile select the first occupied bin.
    QuantThr = (u32)ceilf((f32)(0.001 + Quantile * This->NbInHisto));
    do {
        CumSum += *pHisto++;
    } while (CumSum < QuantThr);

    return This->BinMin + This->BinsWidth * ((s32)(pHisto - This->pHisto) - 1);
}

// Empties the histogram and marks every history slot unused.
// ControlTriggerLR calls this while resetting or loading saved trigger state.
void SlidingHisto_Clear(TriggerLR *This)
{
    u32 *pEnd = This->pHisto + This->NbBins;
    s32 *pFifo;

    while (pEnd > This->pHisto) {
        *--pEnd = 0;
    }
    This->NbInHisto = 0;

    pFifo = This->pFifoStart;
    while (pFifo < This->pFifoEnd) {
        *pFifo++ = -1;
    }
}

// Reads the histogram settings and allocates its bins and rolling history.
// InitTriggerLR calls this while constructing the speech trigger block.
int SlidingHisto_Init(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    f32 lowerLogE;
    f32 upperLogE;
    int RetVal = 0;

    lowerLogE = _tosGetProfileFloat(&This->BaseBlock.Object, 12, 6.5f);
    upperLogE = _tosGetProfileFloat(&This->BaseBlock.Object, 13, 27.0f);
    This->BinMin = lowerLogE;
    This->BinRng = upperLogE - lowerLogE;

    This->NbBins = _tosGetProfileU32(&This->BaseBlock.Object, 14, 150);
    This->pHisto = heap_Calloc(pTos->hHeap, This->NbBins, sizeof(u32));
    This->NbBinsm1 = This->NbBins - 1;
    This->BinsWidth = This->BinRng / This->NbBinsm1;

    This->FifoLength = _tosGetProfileU32(&This->BaseBlock.Object, 11, 20);
    This->pFifoStart = heap_Calloc(pTos->hHeap, This->FifoLength, sizeof(s32));
    This->pFifoEnd = This->pFifoStart + This->FifoLength;
    This->pOldestEntry = This->pFifoStart;

    if (This->pHisto == NULL || This->pFifoStart == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        RetVal = 1;
    } else {
        SlidingHisto_Clear(This);
    }
    return RetVal;
}

// Releases the histogram allocations when TriggerLR is destructed.
// ControlTriggerLR calls this during block destruction.
void SlidingHisto_Free(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    if (This->pHisto != NULL) {
        heap_Free(pTos->hHeap, This->pHisto);
    }
    if (This->pFifoStart != NULL) {
        heap_Free(pTos->hHeap, This->pFifoStart);
    }
}

// Returns the bytes TriggerLR reserves for the saved rolling history.
u32 SlidingHisto_sizeof_SessionData(TriggerLR *This)
{
    return This->FifoLength * sizeof(s32) + sizeof(struct _SLIDINGHISTOSESSION);
}

// Restores histogram counts and the ring position from TriggerLR session data.
// ControlTriggerLR calls this when loading saved trigger state.
void SlidingHisto_PutSession(
    TriggerLR *This, struct _SLIDINGHISTOSESSION *pSlidingHistoSession)
{
    s32 *pFifo;
    long writeIndex;
    s32 *historyWrite;
    u32 historyLength;
    u32 historyBytes;

    SlidingHisto_Clear(This);

    writeIndex = pSlidingHistoSession->OldestElemIdx;
    historyWrite = This->pFifoStart + writeIndex;
    This->pOldestEntry = historyWrite;
    historyLength = This->FifoLength;
    historyBytes = historyLength * sizeof(s32);
    memcpy(This->pFifoStart, pSlidingHistoSession + 1, historyBytes);

    pFifo = This->pFifoStart;
    while (pFifo < This->pFifoEnd) {
        if (*pFifo >= 0) {
            This->pHisto[*pFifo]++;
            This->NbInHisto++;
        }
        pFifo++;
    }
}

// Copies the rolling history and write position into TriggerLR session data.
// ControlTriggerLR calls this when saving trigger state.
void SlidingHisto_GetSession(
    TriggerLR *This, struct _SLIDINGHISTOSESSION *pSlidingHistoSession)
{
    pSlidingHistoSession->OldestElemIdx = This->pOldestEntry - This->pFifoStart;
    memcpy(pSlidingHistoSession + 1, This->pFifoStart, This->FifoLength * sizeof(s32));
}
