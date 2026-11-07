#ifndef REL_M607_GAME_H
#define REL_M607_GAME_H

#include "game/object.h"
#include "game/mg/actman.h"
#include "game/mg/score.h"

typedef struct M607ItemWork M607ItemWork;
typedef struct M607PlayerWork {
    MGPLAYER *player;
    s16 motionIds[9];
    s16 playerIndex;
    s16 positionIndex;
    s16 characterId;
    s32 motionChangePending;
    s32 initialized;
    HUSPR_GROUPID scoreBox;
    MGSCORE *score;
    s32 timer;
    s32 collisionState;
    M607ItemWork *selectedEntry;
    s32 scoreValue;
    s32 overlayActive;
    f32 selectionAngle;
    f32 selectionSpeed;
    Vec push;
} M607PlayerWork;

struct M607ItemWork {
    OMOBJ *model;
    MGACTOR *actor;
    s32 mode;
    s32 value;
    s32 frame;
    f32 horizontalSpeed;
    f32 rotation;
    f32 verticalSpeed;
    Vec position;
    M607PlayerWork *playerContext;
    s32 flagged;
    void *resource;
    s16 emitterId;
};

typedef struct M607RootWork {
    OMOBJ *model;
    MGACTOR *actor;
    s32 motionIndex;
    s32 frames;
    s32 turnDirection;
    u8 unk_14[24]; /* No code names this field. */
    f32 currentAngle;
    Vec direction;
    s32 phase;
    M607PlayerWork *playerData;
    OMOBJ *characterModel;
    M607ItemWork *activeEntry;
} M607RootWork;

extern M607PlayerWork *lbl_1_bss_4C[4];
extern M607ItemWork *lbl_1_bss_24[10];
extern M607RootWork *lbl_1_bss_20;
extern s32 lbl_1_bss_10[4];
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern OMOBJMAN *lbl_1_bss_4;
extern s32 lbl_1_data_28[2][4];
void fn_1_A0(void);
void *fn_1_2BC(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void fn_1_35C(OMOBJ *object, OMOBJ_FUNC callback);
void fn_1_B2C(OMOBJ *obj);
void fn_1_238(void);

#endif
