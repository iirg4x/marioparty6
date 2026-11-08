#ifndef M699_EFFECT_H
#define M699_EFFECT_H
#include "game/object.h"
/* Forty-byte points and twelve-byte links in the 0x1A70 ring allocation. */
typedef struct M699MotionPointRecord {
    s32 inactive;
    Vec position;
    Vec previousPosition;
    Vec velocity;
} M699MotionPointRecord;
typedef struct M699RingContact {
    M699MotionPointRecord *first;
    M699MotionPointRecord *second;
    f32 distance;
} M699RingContact;
typedef struct M699RingWork_s {
    M699MotionPointRecord particles[9][12];
    M699RingContact contacts[204];
} M699RingWork;
typedef struct M699TransitionState {
    s32 active;
    s32 mode;
    Vec start;
    Vec destination;
} M699TransitionState;
typedef struct M699RingDisplay {
    s32 active;
    u32 unknown_004;
    f32 outerRadius;
    f32 innerRadius;
    f32 opacity;
    Vec position;
    u32 color;
} M699RingDisplay;
typedef struct M699CaptureBuffer {
    s32 state;
    s32 capturePending;
    f32 positionX;
    f32 positionY;
    f32 scaleX;
    f32 scaleY;
    void *texture;
} M699CaptureBuffer;
typedef struct M699TransitionWork {
    M699RingDisplay rings[4];
    M699TransitionState transition;
} M699TransitionWork;
extern OMOBJ *lbl_1_bss_A58;
#endif
