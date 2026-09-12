#ifndef GSSDK_GSAPI_H
#define GSSDK_GSAPI_H

#include "types.h"

struct GSContext;
struct GSResult;
typedef s32 (*GSNotifyCallback)(void *context, u32 event, u32 value,
                               u32 auxiliary);
typedef s32 (*GSGetSamplesCallback)(void *source, void *samples,
                                   u32 *sampleCount);

typedef struct GSAPIState {
    void *heap;
    u8 reserved04[0x14];
    GSNotifyCallback resultCallback;
    GSNotifyCallback notify;
    GSGetSamplesCallback getSamples;
    u8 reserved24[0x38];
    s32 resultThreshold;
    u8 reserved60[0x20];
    u32 resultDetails;
    u8 reserved84[0xC];
    u32 sessionDataCount;
} GSAPIState;

typedef struct GSCallbackContext {
    void *engine;
    u32 flags;
    u8 reserved08[0x28];
    struct GSContext *context;
    struct GSResult *results;
    u8 reserved38[0xC];
    u32 value44;
    u8 reserved48[0xC];
    u32 callbackActive;
    u8 reserved58[4];
    s32 stopped;
} GSCallbackContext;

extern GSAPIState gGSAPI;

#endif
