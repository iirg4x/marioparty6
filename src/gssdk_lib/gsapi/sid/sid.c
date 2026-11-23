#include "gssdk/gsapi_internal.h"
#include "gssdk/heap.h"

#include "dolphin/m2s.h"

/* Hardware audio input state. */
struct _SID {
    unsigned short fIsRecording; /* Whether sampling is active. */
};

long SidInit(void *hHeap, long s32DevNum, unsigned int *phSid)
{
    struct _SID *pSid = heap_Alloc(hHeap, sizeof *pSid);

    M2SOpen();
    pSid->fIsRecording = FALSE;
    *phSid = (unsigned int)pSid;
    return 0;
}

int SidIsRecording(unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    return pSid->fIsRecording;
}

long SidClose(void *hHeap, unsigned int hSid)
{
    M2SClose();
    heap_Free(hHeap, (struct _SID *)hSid);
    return 0;
}

long SidStart(unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    M2SStart();
    pSid->fIsRecording = TRUE;
    return 0;
}

long SidStop(unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    M2SStop();
    pSid->fIsRecording = FALSE;
    return 0;
}

long SidGetSamples(unsigned int hSid, short **pWaveBuffer, long nSamples)
{
    struct _SID *pSid = (struct _SID *)hSid;

    if (!pSid->fIsRecording) {
        return 0;
    }
    return M2SGetSamples(pWaveBuffer, nSamples);
}

long SidGetSamplesLeft(unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    if (pSid == NULL) {
        return 0;
    }
    if (!pSid->fIsRecording) {
        return 0;
    }
    return M2SGetSamplesLeft();
}

long SidAdvanceBuffer(unsigned int hSid, long nSamples)
{
    struct _SID *pSid = (struct _SID *)hSid;

    if (pSid->fIsRecording) {
        M2SAdvanceBuffer(nSamples);
    }
    return 0;
}
