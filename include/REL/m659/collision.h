#ifndef M659_COLLISION_H
#define M659_COLLISION_H
#include "game/object.h"

typedef struct M659Collision M659Collision;
struct M659Collision {
    u32 flags;
    u32 mask;
    u32 unk_08;
    u32 unk_0C;
    s32 unk_10;
    void *userData;
    void (*update)(void *);
    int (*collide)(M659Collision *, M659Collision *);
    HuVecF *pos;
    HuVecF *velocity;
    HuVecF previous;
    float radius;
    float factor;
};

typedef struct M659CollisionTable {
    M659Collision *entries[200];
    void *entries2[130];
} M659CollisionTable;
extern OMOBJ *lbl_1_bss_54;
extern int lbl_1_bss_50;
int fn_1_450C(M659Collision *a, M659Collision *b);
#endif
