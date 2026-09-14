#ifndef _REL_M657DLL_H
#define _REL_M657DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "game/mg/score.h"

typedef struct {
    OMOBJMAN *objman;
    OMOBJ *cameraObj[2];
    HU3D_LIGHTID light;
    s32 state;
    s32 music;
    s32 timer;
    s16 winners[2];
    s16 winnerCount;
    s16 unk_22;
} M657Work;

typedef struct {
    /* Unaccessed prefix of the target-allocated 28-byte camera record. */
    u8 unk_00[8];
    s32 cameraMask;
    OMOBJ *target;
    HuVecF pos;
} M657Camera;

/* Common player prefix recovered from the state, motion and camera consumers. */
typedef struct {
    s16 cameraMask;
    s16 playerNo;
    s16 team;
    s16 charNo;
    s16 motionIndex;
    u8 unk_0A[2];
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    BOOL falling;
    u8 unk_34[4];
    s32 state;
    u8 unk_3C[4];
    BOOL winner;
    float progress;
    BOOL ready;
} M657PlayerView;

/* The complete allocation is 152 bytes; unaccessed spans remain unnamed. */
typedef struct {
    M657PlayerView player;
    s32 sound;
    s16 padNo;
    u8 unk_52[2];
    float stickX;
    float stickY;
    u32 buttons;
    u32 prevButtons;
    u8 unk_64[20];
    BOOL exitFinished;
    BOOL computer;
    s16 difficulty;
    u8 unk_82[2];
    BOOL inputActive;
    s32 inputState;
    s32 inputTimer;
    s32 inputDelay;
    s32 inputDuration;
} M657Player;

typedef struct {
    OMOBJ *obj;
    BOOL enabled;
} M657ComPlayer;

typedef struct {
    M657ComPlayer *players[2];
    s16 state;
    u8 unk_0A[2];
    s32 timer;
    s32 unk_10;
    s32 unk_14;
} M657ComWork;

typedef struct {
    float y;
    float x;
    float width;
    float height;
} M657Viewport;

typedef struct {
    s16 sprites[9];
    s16 playerSprites[2];
} M657SpriteWork;

typedef struct {
    HUSPR_GROUPID box;
    MGSCORE *seconds;
    MGSCORE *hundredths;
    HUSPR_GROUPID separator;
    /* Unaccessed bytes within the target's 28-byte team record. */
    u8 unk_0E[6];
    s32 frames;
    BOOL running;
} M657ScoreTeam;

typedef struct {
    s32 state;
    M657ScoreTeam teams[2];
    s32 frames;
} M657ScoreWork;

typedef struct {
    HuVecF pos;
    s32 unk_0C;
} M657ArenaItem;

typedef struct {
    M657ArenaItem item;
    u8 unk_10[12];
    s16 state;
    u8 unk_1E[2];
} M657ArenaWork;

extern M657Work lbl_1_bss_0;
extern M657Work *lbl_1_data_0;
extern M657Viewport lbl_1_data_4[2];
extern s32 lbl_1_data_24[2];
extern MGSEQ_PARAM lbl_1_data_2C;
extern OMOBJ *lbl_1_bss_30[2];
extern OMOBJ *lbl_1_bss_28;
extern OMOBJ *lbl_1_bss_38;
extern OMOBJ *lbl_1_bss_40;
extern OMOBJ *lbl_1_bss_48;

void fn_1_0(void);
void fn_1_6C(void);
void fn_1_70(void);
void fn_1_BC(OMOBJMAN *objman);
void fn_1_258(void);
void fn_1_3EC(void);
void fn_1_3F0(s16 team, OMOBJ *target);
void fn_1_428(OMOBJ *obj);
void fn_1_438(OMOBJ *obj);
void fn_1_5C4(OMOBJ *obj);
void fn_1_5C8(void);
void fn_1_6E0(void);
void fn_1_784(void);
void fn_1_B10(s16 mode, s16 frameNo);
void fn_1_BD4(s16 mode, s16 frameNo);
void fn_1_C74(s16 mode, s16 frameNo);
void fn_1_CA0(s16 mode, s16 frameNo);
void fn_1_D28(s16 mode, s16 frameNo);
void fn_1_1090(s16 mode, s16 frameNo);
void fn_1_1564(s16 mode, s16 frameNo);
void fn_1_15B8(s16 mode, s16 frameNo);
void fn_1_15BC(s16 mode, s16 frameNo);
void fn_1_15C0(OMOBJMAN *objman);
void fn_1_1658(void);
void fn_1_165C(OMOBJ *obj);
void fn_1_1720(void);
void fn_1_1884(OMOBJ *obj);
void fn_1_19B4(OMOBJ *obj);
void fn_1_1AD4(void);
s32 fn_1_1B20(void);
void fn_1_1B84(OMOBJ *obj);
void fn_1_1F6C(OMOBJ *obj);
void fn_1_2018(OMOBJMAN *objman);
void fn_1_21A8(void);
void fn_1_21CC(OMOBJ *obj);
void fn_1_23F4(OMOBJ *obj);
void fn_1_2694(OMOBJ *obj, BOOL visible);
void fn_1_2748(OMOBJ *obj);
void fn_1_2840(OMOBJ *obj, s16 motionIndex);
void fn_1_294C(OMOBJ *obj);
void fn_1_2A50(OMOBJ *obj);
s32 fn_1_2AF0(OMOBJ *obj);
s32 fn_1_2B64(s16 team);
void fn_1_2B98(OMOBJ *obj);
void fn_1_2BE4(s16 team, s32 state);
void fn_1_2C20(s32 state);
s32 fn_1_2E78(s16 team);
s32 fn_1_2EEC(s16 team);
float *fn_1_2F48(s16 team);
void fn_1_2F90(s16 team, s16 cameraMask);
void fn_1_306C(s16 team, HuVecF pos);
void fn_1_30FC(s16 team);
s32 fn_1_3154(s16 team);
s32 fn_1_319C(OMOBJ *obj);
void fn_1_31CC(OMOBJ *obj);
void fn_1_32F4(OMOBJ *obj);
void fn_1_3468(OMOBJ *obj);
void fn_1_3B50(OMOBJ *obj);
void fn_1_3E1C(OMOBJMAN *objman);
void fn_1_3EA4(void);
void fn_1_3EA8(s16 team);
s32 fn_1_3EEC(s16 team);
s32 fn_1_3F2C(void);
void fn_1_3F54(void);
void fn_1_4004(OMOBJ *obj);
void fn_1_42F4(OMOBJ *obj);
void fn_1_4570(OMOBJMAN *objman);
void fn_1_4608(OMOBJ *obj);
void fn_1_463C(OMOBJ *obj);
void fn_1_47A8(OMOBJ *obj);
void fn_1_47D0(OMOBJ *obj);
void fn_1_47EC(OMOBJ *obj);
s32 fn_1_48C0(void);
s32 fn_1_48E8(void);
s32 fn_1_4910(M657ComPlayer *com);
void fn_1_4964(M657ComPlayer *com);
void fn_1_4A48(M657ComPlayer *com);
void fn_1_4A80(M657ComPlayer *com);
void fn_1_4B10(M657ComPlayer *com);
void fn_1_4BA4(M657ComPlayer *com);
void fn_1_4C78(M657ComPlayer *com);
void fn_1_4D4C(M657ComPlayer *com);
void fn_1_4E58(OMOBJMAN *objman);
void fn_1_4EE4(s16 team);
void fn_1_5020(void);
void fn_1_5094(void);
void fn_1_5248(OMOBJ *obj);
void fn_1_5414(OMOBJ *obj);

#endif
