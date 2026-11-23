#include "gssdk/gsapi_internal.h"
#include "gssdk/heap.h"

struct _SID {
    int fIsRecording; /* Whether sampling is active. */
};

/* Allocates the external audio session and leaves sampling stopped. */
long ExtAudioInit(void *hHeap, unsigned int *phSid)
{
    struct _SID *pSid = heap_Alloc(hHeap, sizeof(struct _SID));

    pSid->fIsRecording = FALSE;
    *phSid = (unsigned int)pSid;
    return 0;
}

/* Frees the external audio session from its heap. */
long ExtAudioClose(void *hHeap, unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    heap_Free(hHeap, pSid);
    return 0;
}

/* Marks external sampling as active. */
long ExtAudioStart(unsigned int hSid)
{
    struct _SID *pSid = (struct _SID *)hSid;

    pSid->fIsRecording = TRUE;
    return 0;
}

/* Marks external sampling as stopped. */
long ExtAudioStop(unsigned int hSid)
{
    ((struct _SID *)hSid)->fIsRecording = FALSE;
    return 0;
}

/* Requests samples from the client and returns the count it supplied. */
long ExtAudioGetSamples(void *hEngine, short **Buffer, unsigned long nSamples)
{
    gGSAPI.cbGetWaveData(hEngine, Buffer, &nSamples);
    return nSamples;
}

/* Reports zero pending samples for the external source. */
long ExtAudioGetSamplesLeft(void *hEngine)
{
    return 0;
}
