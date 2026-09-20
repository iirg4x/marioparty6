#ifndef M637DLL_H
#define M637DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

#include "game/esprite.h"

typedef struct M637Record118 {
    s16 pair[8][2];
    f32 value[8];
    f32 field40, field44;
    s32 field48, field4C, field50, field54, field58;
} M637Record118;

typedef struct M637Record1D0 {
    s16 pair[2][2];
    f32 field08[2];
    f32 field10[2];
    s32 field18[2];
    s32 field20[2];
    s32 field28[2];
    f32 field30[2];
    s16 field38, field3A, field3C, field3E, field40;
    s32 field44;
    s16 field48[2];
    s16 field4C[2][2];
    s16 field54[2];
    s16 field58[2];
    s16 field5C, field5E;
} M637Record1D0;

typedef struct M637Record2A0 {
    HU3D_MODELID model;
    HU3D_MOTIONID motion[9];
    HU3D_MOTIONID selectedMotion;
    s32 field18, field1C, count20;
} M637Record2A0;

typedef struct M637RecordB8 {
    s16 scoreBox, sizeX, sizeY;
    f32 field08, field0C, field10;
    s16 espA[3];
    s16 espB[3];
} M637RecordB8;

u32 MgSeqModeNext(void);

typedef struct M637Record08 {
    s32 field00, field04, field08, field0C;
    unsigned char unknown10[4]; /* Unaccessed interval in the observed player record. */
    s32 field14, field18, field1C, field20, field24, field28;
} M637Record08;

/* Consumed record layouts; field names do not claim original declarations. */
extern s32 lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_4;
extern M637Record08 lbl_1_bss_8[4];
extern M637RecordB8 lbl_1_bss_B8[2];
extern f32 lbl_1_bss_F8[2];
extern f32 lbl_1_bss_100;
extern s32 lbl_1_bss_104;
extern s16 lbl_1_bss_108;
extern s16 lbl_1_bss_10A;
extern s16 lbl_1_bss_10C;
extern s16 lbl_1_bss_10E;
extern s32 lbl_1_bss_110;
extern s32 lbl_1_bss_114;
extern M637Record118 lbl_1_bss_118[2];
extern M637Record1D0 lbl_1_bss_1D0[2];
extern s32 lbl_1_bss_290[4];
extern M637Record2A0 lbl_1_bss_2A0[4];
extern OM_CAMERA_VIEW lbl_1_bss_330;
extern MGTIMER *lbl_1_bss_34C;
extern s32 lbl_1_bss_350;
extern s32 lbl_1_bss_354;
extern s32 lbl_1_bss_358;
extern s32 lbl_1_bss_35C;
extern s32 lbl_1_bss_360;
extern s32 lbl_1_bss_364;
extern s32 lbl_1_bss_368;
extern s32 lbl_1_bss_36C;
extern s32 lbl_1_bss_370;
extern s32 lbl_1_bss_374;
extern MGSEQ_PARAM lbl_1_data_0;
extern u32 lbl_1_data_28[9];
extern s32 lbl_1_data_4C[4], lbl_1_data_5C[3];
extern char lbl_1_data_68[13];
void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_334(s16 mode, s16 frameNo);
void fn_1_3B8(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_418(s16 mode, s16 frameNo);
void fn_1_488(s16 mode, s16 frameNo);
void fn_1_50C(s16 mode, s16 frameNo);
void fn_1_510(s16 mode, s16 frameNo);
void fn_1_514(s16 mode, s16 frameNo);
void fn_1_548(void);
s32 fn_1_65C(void);
void fn_1_A54(void);
void fn_1_B94(void);
void fn_1_E4C(void);
s32 fn_1_1110(s32 arg0);
void fn_1_1464(s32 arg0);
s32 fn_1_1814(void);
void fn_1_181C(void);
void fn_1_1CF0(void);
void fn_1_2D7C(void);
void fn_1_3460(void);
s32 fn_1_36E8(s32 arg0);
s32 fn_1_4C24(s32 arg0);
s32 fn_1_5444(s32 var_r29, s32 arg1);
void fn_1_55A4(s32 arg0);
void fn_1_5914(s32 arg0);
void fn_1_5C98(s32 arg0);
s32 fn_1_603C(void);
s32 fn_1_65A0(void);
void fn_1_67C4(void);
void fn_1_6BD4(void);

#endif
