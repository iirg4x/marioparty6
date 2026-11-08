#ifndef M699_NATURAL_VIEWS_H
#define M699_NATURAL_VIEWS_H

#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/object.h"
#include "REL/m699Dll/m699_work_record_prefix_v3.h"
#include "REL/m699Dll/collision.h"

void M699EffectObjectInit(OMOBJMAN *objectManager);

typedef void (*M699Callback)(void *work, void *arg);



void *fn_1_8BB4(M699CharacterWorkPrefix *work, void *camera, Vec *position, Vec *velocity);
void *fn_1_909C(M699CharacterWorkPrefix *work, void *camera, Vec *position, Vec *velocity);
void *fn_1_96F4(M699CharacterWorkPrefix *work, void *camera, Vec *position, Vec *velocity);

struct M699MotionCallbackView;
struct M699MotionEventView;
s32 fn_1_AA54(struct M699MotionCallbackView *motion, struct M699MotionEventView *event);


typedef struct M699Record {
    u8 target_bytes_000[0x364];
    s32 field_364;
    s32 player;
    s32 score;
    u32 field_370;
    u8 target_bytes_374[0x4];
    s32 field_378;
    s32 targetValue;
    s32 savedTargetValue;
    u8 target_bytes_384[0x14];
    s32 field_398;
} M699Record;


typedef struct M699ScalarRecord {
    s32 field_000;
    Point3d vector_004;
    Point3d vector_010;
    Point3d vector_01C;
} M699ScalarRecord;


typedef struct M699VectorWork {
    s32 status;
    Point3d first;
    Point3d second;
    Point3d result;
} M699VectorWork;


typedef struct M699SequenceWork {
    u8 target_bytes_000[0x10];
    s32 gameTimeRemaining;
} M699SequenceWork;


typedef struct M699IndexWork {
    s32 values[0x28];
} M699IndexWork;

/* Collision actor fields used by the sweep test. */
typedef struct M699SweepActor {
    u32 flags;
    u32 mask;
    u32 state;
    u8 otherFields[0x18];
    void (*callback)(void *actor, void *bounds);
    Vec *position;
    Vec *motion;
    Vec previousPosition;
} M699SweepActor;

s32 fn_1_7864(OMOBJ *object);
void fn_1_83C0(M699CharacterWorkPrefix *work);
void fn_1_191B0(HUPROCESS *process);
void fn_1_19238(void);
void fn_1_1B5A0(s32 player, s32 points);
s32 fn_1_1B650(s32 index);
s32 fn_1_1B690(void);
void fn_1_1B6E4(HUPROCESS *objectManager);
void fn_1_1B76C(void);
void fn_1_5958(void);
void fn_1_597C(void);
void fn_1_59CC(int argument);
Vec *fn_1_19080(const f32 *value);
void fn_1_190C8(f32 firstValue, f32 secondValue);

#endif /* M699_NATURAL_VIEWS_H */
