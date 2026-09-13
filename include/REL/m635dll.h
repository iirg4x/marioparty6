#ifndef _REL_M635DLL_H
#define _REL_M635DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"

typedef struct M635Model {
    s16 model;
    s16 time;
} M635Model;

typedef struct M635Team {
    u16 buttonIndex[2];
    s16 pressCount[2];
    s16 unk_08;
    s16 unk_0A;
    s16 unk_0C;
    s16 unk_0E;
    M635Model unk_10;
    M635Model unk_14;
    M635Model unk_18;
    M635Model unk_1C;
    s16 unk_20;
    s16 unk_22;
    s16 unk_24;
} M635Team;

typedef struct M635Work {
    M635Team team[2];
    s16 light;
    s16 model_4E;
    s16 model_50;
    s16 winner;
    s16 nightF;
    s16 state;
    s32 music;
} M635Work;

typedef struct M635Player {
    s16 model;
    s16 motions[6];
    s16 playerNo;
    s16 charNo;
    s16 padNo;
    s16 teamNo;
    s16 memberNo;
    s16 comF;
    s16 difficulty;
    float z;
    s16 timer;
    s16 state;
} M635Player;

typedef struct M635MovingModel {
    s16 model;
    s16 timer;
    float x;
    s16 direction;
} M635MovingModel;

typedef struct M635PositionStep {
    float z[2];
    s16 frames;
} M635PositionStep;

typedef struct M635Motion {
    u32 dataNum;
    u32 attr;
} M635Motion;

typedef struct M635Sprite {
    s16 group;
    s16 state;
    s16 timer;
    s16 member;
    float scale;
    /* Unaccessed storage in the target's four 16-byte records; type unknown. */
    u8 unk_0C[4];
} M635Sprite;

extern M635Work lbl_1_bss_4;
extern OMOBJMAN *lbl_1_bss_64;
extern M635MovingModel lbl_1_bss_68;
extern M635Sprite lbl_1_bss_74[4];
extern s16 lbl_1_bss_B4[2][2];
extern M635Player lbl_1_bss_BC[4];
extern MGSEQ_PARAM lbl_1_data_78;
extern M635PositionStep lbl_1_data_2E8[21];
extern HuVecF lbl_1_data_F8[4];
extern M635Motion lbl_1_data_158[6];

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_134(s16 mode, s16 frameNo);
void fn_1_164(s16 mode, s16 frameNo);
void fn_1_1AC(s16 mode, s16 frameNo);
void fn_1_224(s16 mode, s16 frameNo);
void fn_1_2D0(s16 mode, s16 frameNo);
void fn_1_3D8(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_3E0(s16 mode, s16 frameNo);
void fn_1_408(s16 frameNo);
void fn_1_638(void);
void fn_1_8B0(void);
void fn_1_954(s16 frameNo);
void fn_1_9B8(s16 team);
s16 fn_1_D84(void);
u16 fn_1_EC0(u16 previous, s16 phase);
void fn_1_F44(s16 team);
void fn_1_10A0(void);
u16 fn_1_1168(s16 team, s16 member);
s16 fn_1_1308(s16 difficulty);
s16 fn_1_13F8(s16 difficulty);
void fn_1_1774(OMOBJMAN *objman);
void fn_1_195C(void);
void fn_1_2014(void);
void fn_1_2018(void);
void fn_1_2268(s16 team, s16 player);
void fn_1_2330(void);
void fn_1_25EC(s16 team, s16 player, s16 state);
s16 fn_1_27E4(s16 team, s16 player);
void fn_1_280C(s16 team, s16 player, s16 member);
void fn_1_28A8(s16 team, s16 player);
void fn_1_2904(void);
void fn_1_2954(void);
void fn_1_2CE8(void);
void fn_1_2F40(s16 player);
void fn_1_30C8(s16 player, s16 charNo);
void fn_1_31BC(s16 charNo);
void fn_1_3218(s16 player, s16 motion, float time);
s16 fn_1_32E0(s16 team, s16 member);
void fn_1_3340(void);
void fn_1_33F8(void);
void fn_1_33FC(s16 team);
void fn_1_3738(s16 team, s16 value);
void fn_1_3C34(void);
void fn_1_4114(void);
void fn_1_42A0(void);
void fn_1_448C(void);

#endif
