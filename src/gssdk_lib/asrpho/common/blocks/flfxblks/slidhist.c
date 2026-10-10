// Tracks recent TriggerLR peak-energy values to estimate a lower quantile.

#include "types.h"

#include <string.h>

#include "gssdk/triggerlr.h"

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 floorf(f32 value);
extern f32 ceilf(f32 value);

// Adds a tracked candidate's peak to the rolling histogram.
// TriggerLR calls this at candidate update intervals and when a candidate ends.
void SlidingHisto_NewItem(TriggerLR *block, f32 item)
{
    s32 itemBin;
    s32 expiredBin;
    f32 minimum = block->histogramMinimum;
    f32 range = block->histogramRange;
    f32 maximum = minimum + range;

    if (item > maximum) {
        itemBin = block->histogramLastBin;
    } else if (item < minimum) {
        itemBin = 0;
    } else {
        // Add half a bin width before flooring so values round to the nearest bin.
        f32 offset = item - minimum;
        f32 binWidth = block->histogramBinWidth;
        f32 halfWidth = binWidth / 2.0f;
        f32 centered = halfWidth + offset;
        f32 position = centered / binWidth;
        f32 roundedPosition = floorf(position);
        itemBin = (s32)roundedPosition;
    }

    block->histogramBins[itemBin]++;
    block->histogramItemCount++;

    expiredBin = *block->histogramHistoryWrite;
    *block->histogramHistoryWrite = itemBin;
    if (++block->histogramHistoryWrite == block->histogramHistoryEnd) {
        block->histogramHistoryWrite = block->histogramHistory;
    }

    if (expiredBin >= 0) {
        // Remove the value being overwritten so the histogram covers only recent peaks.
        block->histogramBins[expiredBin]--;
        block->histogramItemCount--;
    }
}

// Returns the first bin whose accumulated count reaches ceil(0.001 + quantile * item count).
// TriggerLR calls this after histogram updates and after restoring valid saved state.
f32 SlidingHisto_LowerQuantile(TriggerLR *block, f32 quantile)
{
    u32 *currentBin = block->histogramBins;
    u32 accumulatedItems = 0;
    u32 targetItemCount;

    // The small bias makes a zero quantile select the first occupied bin.
    targetItemCount = (u32)ceilf(
        (f32)(0.001 + quantile * block->histogramItemCount));
    do {
        accumulatedItems += *currentBin++;
    } while (accumulatedItems < targetItemCount);

    return block->histogramMinimum +
           block->histogramBinWidth *
               ((s32)(currentBin - block->histogramBins) - 1);
}

// Empties the histogram and marks every history slot unused.
// ControlTriggerLR calls this while resetting or loading saved trigger state.
void SlidingHisto_Clear(TriggerLR *block)
{
    u32 *currentBin = block->histogramBins + block->histogramBinCount;
    s32 *historyEntry;

    while (currentBin > block->histogramBins) {
        *--currentBin = 0;
    }
    block->histogramItemCount = 0;

    historyEntry = block->histogramHistory;
    while (historyEntry < block->histogramHistoryEnd) {
        *historyEntry++ = -1;
    }
}

// Reads the histogram settings and allocates its bins and rolling history.
// InitTriggerLR calls this while constructing the speech trigger block.
u32 SlidingHisto_Init(TriggerLR *block)
{
    TosContext *context = block->base.context;
    f32 minimum;
    f32 maximum;
    f32 range;
    u32 *currentBin;
    s32 *historyEntry;
    u32 result = 0;

    minimum = _tosGetProfileFloat(block, 12, 6.5f);
    maximum = _tosGetProfileFloat(block, 13, 27.0f);
    range = maximum - minimum;
    block->histogramMinimum = minimum;
    block->histogramRange = range;

    block->histogramBinCount = _tosGetProfileU32(block, 14, 150);
    block->histogramBins = heap_Calloc(
        context->heap, block->histogramBinCount, sizeof(u32));
    block->histogramLastBin = block->histogramBinCount - 1;
    block->histogramBinWidth =
        block->histogramRange / block->histogramLastBin;

    block->histogramHistoryLength = _tosGetProfileU32(block, 11, 20);
    block->histogramHistory = heap_Calloc(
        context->heap, block->histogramHistoryLength, sizeof(s32));
    block->histogramHistoryEnd =
        block->histogramHistory + block->histogramHistoryLength;
    block->histogramHistoryWrite = block->histogramHistory;

    if (block->histogramBins == NULL || block->histogramHistory == NULL) {
        _tosErrorLog(block, 2);
        result = 1;
    } else {
        // Start with no recorded peaks in the histogram.
        currentBin = block->histogramBins + block->histogramBinCount;
        while (currentBin > block->histogramBins) {
            *--currentBin = 0;
        }
        block->histogramItemCount = 0;

        historyEntry = block->histogramHistory;
        while (historyEntry < block->histogramHistoryEnd) {
            *historyEntry++ = -1;
        }
    }
    return result;
}

// Releases the histogram allocations when TriggerLR is destructed.
// ControlTriggerLR calls this during block destruction.
void SlidingHisto_Free(TriggerLR *block)
{
    TosContext *context = block->base.context;

    if (block->histogramBins != NULL) {
        heap_Free(context->heap, block->histogramBins);
    }
    if (block->histogramHistory != NULL) {
        heap_Free(context->heap, block->histogramHistory);
    }
}

// Returns the bytes TriggerLR reserves for the saved rolling history.
u32 SlidingHisto_sizeof_SessionData(TriggerLR *block)
{
    return block->histogramHistoryLength * sizeof(s32) + sizeof(u32);
}

// Restores histogram counts and the ring position from TriggerLR session data.
// ControlTriggerLR calls this when loading saved trigger state.
void SlidingHisto_PutSession(
    TriggerLR *block, const SlidingHistoSessionData *session)
{
    s32 historyBin;
    s32 *historyEntry;
    s32 *clearHistoryEntry;
    u32 *currentBin;
    u32 writeIndex;
    s32 *historyWrite;
    u32 historyLength;
    u32 historyBytes;

    currentBin = block->histogramBins;
    currentBin += block->histogramBinCount;

    while (currentBin > block->histogramBins) {
        *--currentBin = 0;
    }
    block->histogramItemCount = 0;

    // Mark every history slot unused, although the saved history overwrites all of them below.
    clearHistoryEntry = block->histogramHistory;
    while (clearHistoryEntry < block->histogramHistoryEnd) {
        *clearHistoryEntry++ = -1;
    }

    writeIndex = session->historyWriteIndex;
    historyWrite = block->histogramHistory + writeIndex;
    block->histogramHistoryWrite = historyWrite;
    historyLength = block->histogramHistoryLength;
    historyBytes = historyLength * sizeof(s32);
    memcpy(
        block->histogramHistory, session->history, historyBytes);

    historyEntry = block->histogramHistory;
    while (historyEntry < block->histogramHistoryEnd) {
        historyBin = *historyEntry;
        if (historyBin >= 0) {
            block->histogramBins[historyBin]++;
            block->histogramItemCount++;
        }
        historyEntry++;
    }
}

// Copies the rolling history and write position into TriggerLR session data.
// ControlTriggerLR calls this when saving trigger state.
void SlidingHisto_GetSession(
    TriggerLR *block, SlidingHistoSessionData *session)
{
    session->historyWriteIndex =
        block->histogramHistoryWrite - block->histogramHistory;
    memcpy(
        session->history, block->histogramHistory,
        block->histogramHistoryLength * sizeof(s32));
}
