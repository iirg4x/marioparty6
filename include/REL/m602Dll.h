#ifndef M602DLL_H
#define M602DLL_H
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
#include <stddef.h>
/* Retail-consumed M602 layouts. Field names describe consumers, not recovered names. */
typedef struct {
    s16 unknown000[10]; /* fn_1_96EC/9D10: signed halfwords at 0x00 + index*2. */
    s16 unknown014[10]; /* Same consumers at 0x14 + index*2; record stride 0x28. */
} M602Bss250Record;

typedef struct {
    s32 cameraBit;
    f32 fov, nearPlane, farPlane, aspect;
    f32 viewportX, viewportY, viewportW, viewportH, minZ, maxZ;
    Point3d pos, up, target;
} M602CameraParams;

typedef struct {
    s16 model[5];
    s16 anim[5];
    s32 unknown014, unknown018;
    Point3d scale, pos, rot;
    s16 unknown040;
    f32 unknown044;
    s32 unknown048, unknown04C, unknown050;
} M602BssA0Record;

typedef struct { Point3d pos, up, target; } M602CameraPose;
/* arg1 is stored as a full word without extension at target 0x2D40. */

struct _struct_lbl_1_bss_318_0x58 {
    /* 0x00 */ s16 unk0;                            /* inferred */
    /* 0x02 */ s16 unk2;                            /* inferred */
    /* 0x04 */ s16 unk4;                            /* inferred */
    /* 0x06 */ s16 unk6;                            /* inferred */
    /* 0x08 */ s16 unk8;                            /* inferred */
    /* 0x0A */ s16 unkA;                            /* inferred */
    /* 0x0C */ s16 motion[12]; /* Created/read as twelve motion IDs. */
    /* 0x24 */ s16 unk24;                           /* inferred */
    /* 0x26..27: natural alignment before float fields. */
    /* 0x28 */ Point3d pos;
    /* 0x34 */ Point3d rot;
    /* 0x40 */ Point3d scale;
    /* 0x4C */ s16 unk4C;                           /* inferred */
    /* 0x4E */ s16 unk4E;                           /* inferred */
    /* 0x50 */ s16 unk50;                           /* inferred */
    /* 0x52 */ s16 unk52;                           /* inferred */
    /* 0x54 */ s16 unk54;                           /* inferred */
    /* 0x56 */ s16 unk56;                           /* inferred */
};

/* size = 0x58 */

struct _struct_lbl_1_bss_3C_0x8 {
    /* 0x0 */ s16 unk0;                             /* inferred */
    /* 0x2 */ s16 choices[3];
};

/* size = 0x8 */

struct _struct_lbl_1_data_4B4_0xC {
    /* 0x0 */ f32 unk0;                             /* inferred */
    /* 0x4 */ f32 unk4;                             /* inferred */
    /* 0x8 */ u8 unknown8[4]; /* Unconsumed retail zero bytes; original type unknown. */
};

/* size = 0xC */

/* Consumed light layout: short handle, aligned vectors, and RGBA color. */
typedef struct {
    s16 id;
    Point3d pos;
    Point3d dir;
    GXColor color;
} M602Light;

extern MGSEQ_PARAM lbl_1_data_0;

extern M602CameraParams lbl_1_data_28;

extern M602CameraPose lbl_1_data_78;

extern M602CameraPose lbl_1_data_9C[2];

extern s32 lbl_1_data_E8[13];

extern s32 lbl_1_data_11C[2];

extern M602CameraParams lbl_1_data_124[3];

extern M602CameraParams lbl_1_data_214;

extern char lbl_1_data_264[9];

extern char lbl_1_data_26D[11];

extern M602Light lbl_1_data_278;

extern HU3D_PARMAN_PARAM lbl_1_data_298;

extern s32 lbl_1_data_2E8[2];

extern s32 lbl_1_data_2F0[4];

extern char lbl_1_data_300[4];

extern char lbl_1_data_304[4];

extern char lbl_1_data_308[4];

extern char lbl_1_data_30C[4];

extern char lbl_1_data_310[4];

extern char lbl_1_data_314[4];

extern char lbl_1_data_318[4];

extern char lbl_1_data_31C[4];

extern char lbl_1_data_320[4];

extern char lbl_1_data_324[5];

extern char lbl_1_data_329[5];

extern char lbl_1_data_32E[5];

extern char lbl_1_data_333[5];

extern char lbl_1_data_338[5];

extern char lbl_1_data_33D[5];

extern char lbl_1_data_342[5];

extern char lbl_1_data_347[5];

extern char lbl_1_data_34C[5];

extern char lbl_1_data_351[5];

extern char lbl_1_data_356[5];

extern char lbl_1_data_35B[5];

extern char lbl_1_data_360[5];

extern char lbl_1_data_365[5];

extern char lbl_1_data_36A[6];

extern char *lbl_1_data_370[24];

extern u32 lbl_1_data_3D0[12];

extern s32 lbl_1_data_400;

extern char lbl_1_data_404[16];

extern char lbl_1_data_414[16];

extern char lbl_1_data_424[16];

extern char lbl_1_data_434[16];

extern char lbl_1_data_444[10];

extern char lbl_1_data_44E[14];

extern u8 lbl_1_data_4B0[3];

extern struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4B4[4];

extern struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4E4[2];

extern char lbl_1_data_4FC[16];

extern char lbl_1_data_50C[16];

extern char lbl_1_data_51C[16];

extern char lbl_1_data_52C[16];

extern s16 lbl_1_bss_0;

extern s16 lbl_1_bss_2;

extern s16 lbl_1_bss_4;

extern HUPROCESS *lbl_1_bss_8;

extern s32 lbl_1_bss_C;

extern s16 lbl_1_bss_10;

extern s16 lbl_1_bss_18;

extern s16 lbl_1_bss_20;

extern ANIMDATA *lbl_1_bss_24;

extern u8 lbl_1_bss_28;

extern s16 lbl_1_bss_2A;

extern s16 lbl_1_bss_2C;

extern s16 lbl_1_bss_2E;

extern s16 lbl_1_bss_30;

extern s16 lbl_1_bss_32;

extern s16 lbl_1_bss_34[2];

extern s16 lbl_1_bss_38;

extern s16 lbl_1_bss_3A;

extern struct _struct_lbl_1_bss_3C_0x8 lbl_1_bss_3C[10];

extern ANIMDATA *lbl_1_bss_8C[5];

extern M602BssA0Record lbl_1_bss_A0[3];

extern s16 lbl_1_bss_1A0;

extern s16 lbl_1_bss_1A2[24][2];

extern s16 lbl_1_bss_202[24];

extern s16 lbl_1_bss_232;

extern s16 lbl_1_bss_234;

extern s16 lbl_1_bss_236[3];

extern s16 lbl_1_bss_23C[3];

extern u8 lbl_1_bss_248;

extern s16 lbl_1_bss_24A;

extern s16 lbl_1_bss_24C;

extern s16 lbl_1_bss_24E;

extern M602Bss250Record lbl_1_bss_250[4];

extern s16 lbl_1_bss_2F0;

extern s16 lbl_1_bss_2F2;

extern s16 lbl_1_bss_2F4[4];

extern Point3d lbl_1_bss_2FC;

extern s16 lbl_1_bss_308;

extern s16 lbl_1_bss_30A;

extern s16 lbl_1_bss_30C;

extern s32 lbl_1_bss_310;

extern u8 lbl_1_bss_314;

extern struct _struct_lbl_1_bss_318_0x58 lbl_1_bss_318[4];

extern u8 lbl_1_bss_478;

extern MGTIMER *lbl_1_bss_47C;

extern s16 lbl_1_bss_480;

extern s16 lbl_1_bss_482[4];

extern s16 lbl_1_bss_48A[4];

extern s16 lbl_1_bss_492[4];

extern s16 lbl_1_bss_49A[4];

void fn_1_A0(void);

void fn_1_10C(s16 mode, s16 frameNo);

void fn_1_210(s16 arg1, s16 frameNo);

void fn_1_278(s16 mode, s16 frameNo);

void fn_1_2AC(s16 arg1, s16 frameNo);

void fn_1_7B8(s16 arg1, s16 frameNo);

void fn_1_8F0(s16 arg1, s16 frameNo);

void fn_1_A5C(s16 mode, s16 frameNo);

void fn_1_AB8(s16 mode, s16 frameNo);

void fn_1_AD8(void);

u8 fn_1_D54(s16 arg0);

void fn_1_1418(s16 arg0);

void fn_1_1428(void);

void fn_1_1770(void);

void fn_1_1774(void);

void fn_1_1778(s16 unused);

void fn_1_177C(void);

void fn_1_1780(void);

void fn_1_1784(void);

void fn_1_1788(void);

s16 fn_1_178C(void);

void fn_1_179C(void);

void fn_1_2250(s16 layerNo);

void fn_1_22A0(void);

void fn_1_22DC(void);

void fn_1_2998(void);

s16 fn_1_2B28(void);

void fn_1_2BB0(s16 arg0, s16 arg1);

void fn_1_2C00(s16 arg0, s16 arg1, s16 arg2);

void fn_1_2CBC(s16 arg0, s32 arg1);

s16 fn_1_2D5C(void);

s16 fn_1_2E3C(s16 arg0);

void fn_1_2FDC(s16 arg0);

s32 fn_1_3000(s32 arg0);

s16 fn_1_34A4(s16 arg0);

void fn_1_3908(void);

void fn_1_39D0(void);

void fn_1_3FD0(s16 arg0, s16 arg1, s16 arg2);

void fn_1_411C(void);

void fn_1_48B4(void);

void fn_1_4998(void);

s16 fn_1_4A80(void);

void fn_1_4A98(void);

void fn_1_4AAC(void);

void fn_1_4C84(void);

void fn_1_4CD0(s16 unused);

void fn_1_4CD4(void);

void fn_1_4D68(void);

void fn_1_4E48(void);

void fn_1_4EEC(void);

s16 fn_1_4F90(void);

s16 fn_1_4FE8(s16 arg0);

s16 fn_1_502C(s16 arg0);

s16 fn_1_5048(void);

s16 fn_1_50D4(s16 arg0);

void fn_1_50F0(s16 arg0);

void fn_1_5150(s16 arg0);

s16 fn_1_5170(void);

s16 fn_1_51D4(void);

s16 fn_1_5238(s16 arg0);

void fn_1_5254(void);

s8 fn_1_5C7C(s16 arg0);

void fn_1_6078(void);

void fn_1_61E0(void);

void fn_1_647C(void);

void fn_1_6C5C(void);

void fn_1_6DB8(void);

void fn_1_6F00(void);

s16 fn_1_6F14(void);

void fn_1_8D90(s16 arg0, s32 arg1);

s16 fn_1_8E64(s16 arg0);

void fn_1_90B4(void);

void fn_1_9378(void);

void fn_1_9480(s32 arg0);

void fn_1_96EC(void);

void fn_1_9D10(void);

void fn_1_9ED8(void);

s32 fn_1_9EEC(void);

void fn_1_9F34(void);

s32 fn_1_9FD8(void);

void fn_1_A034(void);

void fn_1_A074(void);

void fn_1_A648(s16 arg0);

void fn_1_A78C(void);

void fn_1_A81C(void);

void fn_1_A914(s16 arg0, s16 arg1);

s16 fn_1_AB40(s16 arg0);

s32 fn_1_AB5C(void);

void fn_1_AB64(void);

void fn_1_AB68(s32 arg0);
typedef char m602_player_size[(sizeof(struct _struct_lbl_1_bss_318_0x58) == 88) ? 1 : -1];
typedef char m602_stage_size[(sizeof(M602BssA0Record) == 84) ? 1 : -1];
typedef char m602_light_size[(sizeof(M602Light) == 32) ? 1 : -1];
#endif
