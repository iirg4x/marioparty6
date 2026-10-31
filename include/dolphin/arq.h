#ifndef _DOLPHIN_ARQ
#define _DOLPHIN_ARQ

#include <dolphin/ar.h>

#ifdef __cplusplus
extern "C" {
#endif
void ARQInit(void);
void ARQPostRequest(ARQRequest* request, u32 owner, u32 type, u32 priority, u32 source, u32 dest,
    u32 length, ARQCallback callback);
#ifdef __cplusplus
}
#endif

#define ARQ_CHUNK_SIZE_DEFAULT 4096

#endif // _DOLPHIN_ARQ
