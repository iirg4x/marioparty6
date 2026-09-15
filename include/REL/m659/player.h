#ifndef M659_PLAYER_H
#define M659_PLAYER_H
#include "REL/m659/collision.h"

/* Unobserved ranges preserve storage whose original field types are unknown. */
typedef struct M659Motion {
    int state;
    s16 column;
    s16 direction;
    s16 previousDirection;
    HuVecF start;
    HuVecF unk_18;
    HuVecF unk_24;
    s16 substate;
    int done;
    HuVecF pos;
    HuVecF velocity;
    float phase;
} M659Motion;

typedef struct M659ComRoute {
    HuVecF pos;
    s16 column;
    double distance;
    s16 delay;
} M659ComRoute;

typedef struct M659ComRecord {
    int isCom;
    s16 difficulty;
    int active;
    int state;
    unsigned char unobserved_10[4];
    s16 direction;
    s16 routeIndex;
    M659ComRoute routes[22];
} M659ComRecord;

typedef struct M659PlayerInfo {
    s16 cameraBit;
    s16 playerNo;
    s16 side;
    s16 charNo;
    s16 motionId;
    int unk_0C;
    int state;
    unsigned char unobserved_14[4];
    int result;
    HuVecF unk_1C;
    HuVecF unk_28;
    int unk_34;
} M659PlayerInfo;

typedef struct M659Player {
    M659PlayerInfo info;
    M659Motion motion;
    s16 padNo;
    float stickX;
    float stickY;
    unsigned char unobserved_98[8];
    M659ComRecord com;
    M659Collision collision;
} M659Player;

typedef struct M659ComSlot {
    OMOBJ *obj;
    int isCom;
    int countdown;
} M659ComSlot;

extern OMOBJ *lbl_1_bss_2C[4];
int fn_1_1840(s16 side);
void fn_1_19DC(OMOBJ *obj, s16 motion);
void fn_1_2D0C(OMOBJ *obj);
void fn_1_2FE0(OMOBJ *obj);
void fn_1_5CB0(OMOBJ *obj);
int fn_1_5DB4(M659ComSlot *com);
void fn_1_5E08(M659ComSlot *com);
void fn_1_5E9C(M659ComSlot *com);
void fn_1_64B0(M659ComSlot *com);
void fn_1_6714(M659ComSlot *com, s16 direction);
#endif
