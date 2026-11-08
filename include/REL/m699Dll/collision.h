#ifndef M699_COLLISION_H
#define M699_COLLISION_H

#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/object.h"

extern OMOBJ *lbl_1_bss_A68;

/* Actor payload beginning at record offset 0x24. */
typedef struct M699CollisionActor {
    void (*callback)(void *self, void *other);
    Vec *position;
    Vec *velocity;
    Vec savedPosition;
    Vec extent;
    f32 collisionScale;
    s32 ready;
    u8 unknown_02C[4]; /* No code names this field. */
    f32 positiveExtent[3];
    f32 negativeExtent[3];
} M699CollisionActor;

/* The state at +0x10 selects the actor or volume payload at +0x24. */
typedef struct M699CollisionRecord {
    u32 flags;
    u32 mask;
    u32 mode;
    s32 id;
    s32 state;
    void *identity;
    void *excludedIdentity;
    void (*update)(s32 id);
    s32 status;
    union {
        M699CollisionActor actor;
        struct {
            Vec minimum;
            Vec maximum;
        } box;
        struct {
            Vec center;
            f32 radiusX;
            f32 radiusZ;
            s32 direction;
        } ellipse;
        struct {
            Vec *position;
            f32 radius;
            f32 extraRadius;
            s32 collisionState;
        } circle;
    } shape;
} M699CollisionRecord;

s32 fn_1_1BED0(M699CollisionRecord *first, M699CollisionRecord *second);
s32 fn_1_1C280(M699CollisionRecord *actor, M699CollisionRecord *volume);
s32 fn_1_1C4D4(M699CollisionRecord *motion, M699CollisionRecord *settings);
s32 fn_1_1C7C0(M699CollisionRecord *work, M699CollisionRecord *bounds);
s32 fn_1_1CA4C(M699CollisionRecord *work, M699CollisionRecord *bounds);
s32 fn_1_1CB70(s32 axis, M699CollisionRecord *work, const M699CollisionRecord *bounds, f32 fraction);
s32 fn_1_1CD80(M699CollisionRecord *positionRecord, const M699CollisionRecord *boundsRecord);
s32 fn_1_1CE08(M699CollisionRecord *work, M699CollisionRecord *other);
s32 fn_1_1CF50(M699CollisionRecord *first, M699CollisionRecord *second, Vec *separation);
s32 fn_1_1D3E4(M699CollisionRecord *work, M699CollisionRecord *bounds);
int fn_1_1D6BC(void *value);
int fn_1_1D734(void *value);

#endif
