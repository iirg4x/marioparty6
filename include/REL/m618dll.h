#ifndef M618DLL_H
#define M618DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"

/* Target-backed consumed views; names do not claim original declarations. */
typedef struct M618Player {
    MGPLAYER *player;
    int unk04;
    int unk08;
    int unk0C;
    int unk10;
    int unk14;
    HU3D_MOTIONID motion[4];
    HU3D_MODELID model;
    HuVecF pos24;
    float unk30;
    int unk34;
    HuVecF pos38;
    int unk44;
    int unk48;
    unsigned char unknown4C[4]; /* Observed next vector begins at +0x50. */
    HuVecF pos50;
    int unk5C;
    int unk60;
    float unk64;
} M618Player;

typedef struct M618Input {
    int unk00, unk04, unk08, unk0C;
    int stickX, stickY, button, unk1C;
    int unk20;
} M618Input;

typedef struct M618Model {
    HU3D_MODELID model;
    HU3D_MODELID colModel;
    float unk04;
    unsigned char unknown08[4]; /* Twelve-byte array stride; purpose unresolved. */
} M618Model;

extern int lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_4;
extern HuVecF lbl_1_bss_8[4];
extern int lbl_1_bss_38;
extern int lbl_1_bss_40;
extern int lbl_1_bss_44[4][4];
extern M618Input lbl_1_bss_84[4];
extern M618Player lbl_1_bss_114[4];
extern HU3D_MODELID lbl_1_bss_2B4[2][2];
extern HU3D_MODELID lbl_1_bss_2BC[4][2];
extern HU3D_MODELID lbl_1_bss_2CC[4][5];
extern HU3D_MODELID lbl_1_bss_2F4[4][3];
extern M618Model lbl_1_bss_30C[37];
extern float lbl_1_bss_4C8;
extern OM_CAMERA_VIEW lbl_1_bss_4CC[4];
extern int lbl_1_bss_53C;
extern HU3D_MODELID lbl_1_bss_540[2];
extern MGTIMER *lbl_1_bss_544;
extern int lbl_1_bss_548;
extern int lbl_1_bss_54C;
extern int lbl_1_bss_550;
extern int lbl_1_bss_554;
extern int lbl_1_bss_558;
extern MGSEQ_PARAM lbl_1_data_0;
extern int lbl_1_data_28[4];
extern unsigned int lbl_1_data_38[4];
extern HuVec2f lbl_1_data_48[4];

void fn_1_A0(void);
void fn_1_DC(s16 mode, s16 frameNo);
void fn_1_1EC(s16 mode, s16 frameNo);
void fn_1_294(s16 mode, s16 frameNo);
void fn_1_2B4(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_310(s16 mode, s16 frameNo);
void fn_1_33C(s16 mode, s16 frameNo);
void fn_1_340(s16 mode, s16 frameNo);
void fn_1_344(s16 mode, s16 frameNo);
void fn_1_378(void);
int fn_1_570(void);
void fn_1_644(void);
void fn_1_84C(void);
void fn_1_910(void);
int fn_1_DA8(void);
void fn_1_1B40(int);
int fn_1_1CD8(void);
void fn_1_1CE0(void);
void fn_1_214C(void);
void fn_1_3294(int);
void fn_1_44CC(int);
void fn_1_4BE0(void);
void fn_1_52D0(void);
void fn_1_6298(void);
void fn_1_6704(int);
void fn_1_6C0C(void);
void fn_1_796C(int);
void fn_1_8180(int);
void fn_1_C450(M618Input *, int, int, int, int);

/* Complete existing no-argument SDK declarations for translator inference. */
u32 MgSeqModeNext(void);
u16 MgSeqModeChangeOff(void);
u16 MgSeqModeChangeOn(void);

#endif
