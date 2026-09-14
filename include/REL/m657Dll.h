#ifndef _REL_M657DLL_H
#define _REL_M657DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"

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

/* Accessed prefix only; this does not describe the full player allocation. */
typedef struct {
    s16 cameraMask;
    s16 playerNo;
    s16 team;
    s16 charNo;
    u8 unk_08[4];
    HuVecF pos;
} M657PlayerView;

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

extern M657Work lbl_1_bss_0;
extern M657Work *lbl_1_data_0;
extern M657Viewport lbl_1_data_4[2];
extern s32 lbl_1_data_24[2];
extern MGSEQ_PARAM lbl_1_data_2C;
extern OMOBJ *lbl_1_bss_30[2];
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
void fn_1_2018(OMOBJMAN *objman);
s32 fn_1_2B64(s16 team);
void fn_1_2BE4(s16 team, s32 state);
void fn_1_2C20(s32 state);
s32 fn_1_2EEC(s16 team);
float *fn_1_2F48(s16 team);
void fn_1_2F90(s16 team, s16 cameraMask);
void fn_1_306C(s16 team, HuVecF pos);
void fn_1_30FC(s16 team);
s32 fn_1_3154(s16 team);
void fn_1_3E1C(OMOBJMAN *objman);
s32 fn_1_3EEC(s16 team);
s32 fn_1_3F2C(void);
void fn_1_3F54(void);
void fn_1_4570(OMOBJMAN *objman);
void fn_1_4E58(OMOBJMAN *objman);
void fn_1_4EE4(s16 team);
void fn_1_5020(void);
void fn_1_5094(void);
void fn_1_5248(OMOBJ *obj);
void fn_1_5414(OMOBJ *obj);

#endif
