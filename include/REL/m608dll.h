#ifndef M608DLL_H
#define M608DLL_H
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
#ifndef M608_SCENE_CONSUMED_CONTEXT_H
#define M608_SCENE_CONSUMED_CONTEXT_H

/*
 * Target-backed consumed layout of the shared scene allocation.
 *
 * This is a consumed-layout context, not a recovered original declaration.
 * The unknown byte ranges are intentionally retained as unknown storage.
 * Types below use the project SDK declarations; in particular,
 * the object-manager slot is represented by its HUPROCESS pointer type.
 */
/* The target allocates 0x3DE0 bytes; only the first 660 vector slots have observed consumers. */
typedef struct M608PathSamples { Point3d points[660]; u8 unknown_tail[7920]; } M608PathSamples;
typedef struct M608SceneConsumedContext {
    /* 0x000 */ HUPROCESS *objectManager;
    /* 0x004 */ u32 state;
    /* 0x008 */ s32 frame;
    /* 0x00C */ s32 activePlayer;

    /* 0x010 */ HU3D_MOTIONID cameraMotions[10];
    /* 0x024 */ HU3D_MODELID cameraModels[10];
    /* 0x038 */ HU3D_MODELID activeCameraMotion;
    /* 0x03A */ u8 unk_03A[2];
    /* 0x03C */ OMOBJ *cameraObject;
    /* 0x040 */ f32 cameraTime;
    /* 0x044 */ f32 cameraMaxTime;
    /* 0x048 */ u8 unk_048[2];

    /* 0x04A */ HU3D_MODELID sceneModels[5];
    /* 0x054 */ HU3D_MODELID playerModelsA[4];
    /* 0x05C */ HU3D_MODELID playerModelsB[4];
    /* 0x064 */ HU3D_MODELID playerModelsC[4];
    /* 0x06C */ HU3D_MODELID mainModel;
    /* 0x06E */ HU3D_MOTIONID mainMotion;
    /* 0x070 */ Point3d positions_070[4];
    /* 0x0A0 */ u8 unk_0A0[48];
    /* 0x0D0 */ Point3d positions_0D0[4];

    /* 0x100 */ s16 characterNos[4];
    /* 0x108 */ s16 padNos[4];
    /* 0x110 */ HU3D_MODELID characterModels[4];
    /* 0x118 */ HU3D_MOTIONID characterMotions[4][8];
    /* 0x158 */ OMOBJ *playerObjects[4];
    /* 0x168 */ HU3D_MODELID secondaryModels[4];
    /* 0x170 */ HU3D_MODELID models_184_199[16];
    /* 0x190 */ HU3D_MOTIONID motions_200_208[9];
    /* 0x1A2 */ u8 unk_1A2[6];
    /* 0x1A8 */ HU3D_MODELID models_212_213[2];
    /* 0x1AC */ s32 timingSubstate;

    /* 0x1B0 */ f32 angle;
    /* 0x1B4 */ f32 from;
    /* 0x1B8 */ f32 to;
    /* 0x1BC */ s32 frame_1BC;
    /* 0x1C0 */ HU3D_MODELID models[27];
    /* 0x1F6 */ HU3D_MOTIONID motions[27];
    /* 0x22C */ Point3d positions_22C[27];
    /* 0x370 */ OMOBJ *objects[27];

    /* 0x3DC */ s32 count;
    /* 0x3E0 */ s32 scores[4];
    /* 0x3F0 */ u32 record;
    /* 0x3F4 */ s32 winners[4];
    /* 0x404 */ s32 flag;
    /* 0x408 */ s16 difficulty[4];
    /* 0x410 */ s16 base[4];
    /* 0x418 */ s16 interval[4];
    /* 0x420 */ s16 counter[4];
    /* 0x428 */ Point3d vector_428;
    /* 0x434 */ Point3d vector_434;
    /* 0x440 */ Point3d vector_440;
    /* 0x44C */ s32 flag_44C;
    /* 0x450 */ s32 audioHandle_450;
    /* 0x454 */ s32 audioHandle_454;
} M608SceneConsumedContext;

#endif

/* Callers copy a full Point3d before fn_1_5DA4.
 * Gekko passes the value record using a hidden pointer. */
void fn_1_5DA4(Point3d position);
BOOL fn_1_5F64(void);
void fn_1_5F90(s16 modelIndex);
void fn_1_5FDC(s16 modelIndex, s16 motionIndex);
void fn_1_6534(s16 scoreIndex, s16 value);
void fn_1_6958(s32 member);
void fn_1_69F4(void);
u32 MgSeqModeNext(void);



struct _struct_lbl_1_bss_3DF0_0x18 {
    /* 0x00 */ s16 unk0;                            /* inferred */
    /* 0x04 */ MGSCORE *unk4;                       /* inferred */
    /* 0x08 */ ANIMDATA *unk8;                      /* inferred */
    /* 0x0C */ s16 unkC;                            /* inferred */
    /* 0x0E */ s16 unkE;                            /* inferred */
    /* 0x10 */ u8 unknown10[8]; /* Target-backed unconsumed bytes, not padding. */
};

/* size = 0x18 */

/* Explicit bytes at 0x0B..0x0F have unknown semantics, not padding. */
struct _struct_lbl_1_data_44C_0x14 {
    s16 unk0, unk2, unk4, unk6;
    u8 unk8, unk9, unkA, unkB;
    u8 unknown0C[4];
    u8 unk10, unk11, unk12, unk13;
};

/* Retail-backed consumed view; other fields in these 80-byte records are unknown. */
typedef struct M608Unknown80 {
    u32 unknown00[2];
    f32 field08;
    u32 unknown0C[17];
} M608Unknown80;
s32 fn_1_A0(s32 arg0, s32 arg1);
void fn_1_104(s32 arg0);
void fn_1_140(s16 mode, s16 frameNo);
void fn_1_190(s16 arg1, s16 frameNo);
void fn_1_57C(s16 mode, s16 frameNo);
void fn_1_580(s16 arg1, s16 frameNo);
void fn_1_1A2C(s16 arg1, s16 frameNo);
void fn_1_1DD8(s16 arg1, s16 frameNo);
void fn_1_2374(s16 arg1, s16 frameNo);
void fn_1_2538(s16 mode, s16 frameNo);
void fn_1_253C(s16 mode, s16 frameNo);
void fn_1_2540(OMOBJ *arg0);
void fn_1_2FD8(s16 modelId, s16 motId, BOOL lagF);
void fn_1_33A4(OMOBJ *arg0);
void fn_1_34D0(OMOBJ *arg0);
void fn_1_3718(OMOBJ *arg0);
void fn_1_388C(OMOBJ *arg0);
void fn_1_38E8(HU3D_MODEL *modelP, f32 (*mtx)[3][4]);
void fn_1_38EC(Point3d position);
void fn_1_38F0(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_39B4(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3A78(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3B3C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3C00(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3C84(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3D7C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3E00(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_3F18(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_400C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_40F0(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_41D4(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4]);
void fn_1_42B8(void);
void fn_1_437C(s16 layerNo);
void fn_1_43C8(void);
void fn_1_5DA4(Point3d position);
void fn_1_5E6C(s32 arg0);
BOOL fn_1_5F64(void);
void fn_1_5F90(s16 modelIndex);
void fn_1_5FDC(s16 modelIndex, s16 motionIndex);
void fn_1_605C(s16 arg0);
void fn_1_60CC(void);
void fn_1_6148(void);
void fn_1_614C(void);
void fn_1_6150(void);
void fn_1_6534(s16 scoreIndex, s16 value);
void fn_1_6578(void);
void fn_1_6604(void);
void fn_1_66C0(OMOBJ *arg0);
void fn_1_6828(void);
void fn_1_6958(s32 member);
void fn_1_69F4(void);
u32 fn_1_6B90(s32 player);
extern /* button mask */
MGSEQ_PARAM lbl_1_data_0;
extern f32 lbl_1_data_28[14];
extern f32 lbl_1_data_60[14];
extern f32 lbl_1_data_98[14];
extern char lbl_1_data_D0[9];
extern char lbl_1_data_D9[16];
extern char lbl_1_data_E9[16];
extern char lbl_1_data_F9[16];
extern char lbl_1_data_109[16];
extern char lbl_1_data_119[14];
extern char lbl_1_data_127[5];
extern char lbl_1_data_12C[22];
extern char lbl_1_data_142[5];
extern char lbl_1_data_147[26];
extern char lbl_1_data_161[24];
extern char lbl_1_data_179[16];
extern char lbl_1_data_189[7];
extern char lbl_1_data_1B4[29];
extern char lbl_1_data_1D1[5];
extern char lbl_1_data_1D6[65];
extern char lbl_1_data_217[16];
extern char lbl_1_data_227[14];
extern char lbl_1_data_235[13];
extern char lbl_1_data_242[12];
extern char lbl_1_data_24E[18];
extern char lbl_1_data_295[11];
extern char lbl_1_data_2A0[16];
extern char lbl_1_data_2B0[5];
extern char lbl_1_data_2B5[5];
extern char lbl_1_data_2BA[11];
extern char lbl_1_data_2C5[11];
extern char lbl_1_data_2D0[11];
extern char lbl_1_data_2DB[29];
extern char lbl_1_data_2F8[40];
extern M608Unknown80 lbl_1_data_320;
extern M608Unknown80 lbl_1_data_370;
extern M608Unknown80 lbl_1_data_3C0;
extern s32 lbl_1_data_410[15];
extern struct _struct_lbl_1_data_44C_0x14 lbl_1_data_44C[5];
extern s32 lbl_1_data_4B0[6];
extern char lbl_1_data_4C8[40];
extern s32 lbl_1_data_4F0[10];
extern s32 lbl_1_data_518[4];
extern s32 lbl_1_data_528[4];
extern s32 lbl_1_data_538[4];
extern unsigned int lbl_1_data_548[9];
extern u32 lbl_1_data_56C[8];
extern char lbl_1_data_58C[17];
extern char lbl_1_data_59D[17];
extern char lbl_1_data_5AE[17];
extern char lbl_1_data_5BF[17];
extern char *lbl_1_data_5D0[4];
extern char lbl_1_data_5E0[17];
extern char lbl_1_data_5F1[17];
extern char lbl_1_data_602[17];
extern char lbl_1_data_613[17];
extern char *lbl_1_data_624[4];
extern char lbl_1_data_634[6];
extern char lbl_1_data_63A[6];
extern char lbl_1_data_640[6];
extern char lbl_1_data_646[6];
extern char *lbl_1_data_64C[4];
extern char lbl_1_data_65C[10];
extern char lbl_1_data_666[10];
extern char lbl_1_data_670[10];
extern char lbl_1_data_67A[10];
extern char *lbl_1_data_684[4];
extern s32 lbl_1_data_694[27];
extern struct _struct_lbl_1_bss_3DF0_0x18 lbl_1_bss_3DF0[5];
typedef struct M608SpriteGroupWork { s16 currentMember; HUSPR_GROUPID group; ANIMDATA *anim[5]; } M608SpriteGroupWork;
extern M608SpriteGroupWork lbl_1_bss_3E68;
extern M608SceneConsumedContext lbl_1_bss_3E80;
extern const Point3d lbl_1_rodata_88;
extern const Point3d lbl_1_rodata_94;
extern const Point3d lbl_1_rodata_A0;
extern const GXColor lbl_1_rodata_AC;
extern const Point3d lbl_1_rodata_B0;
extern const Point3d lbl_1_rodata_BC;
extern const Point3d lbl_1_rodata_C8;
extern const GXColor lbl_1_rodata_D4;

typedef char m608_scene_extent[(sizeof(M608SceneConsumedContext)==1112)?1:-1];
typedef char m608_path_extent[(sizeof(M608PathSamples)==15840)?1:-1];
typedef char m608_sprite_extent[(sizeof(M608SpriteGroupWork)==24)?1:-1];
typedef char m608_score_extent[(sizeof(struct _struct_lbl_1_bss_3DF0_0x18)==24)?1:-1];
#endif
