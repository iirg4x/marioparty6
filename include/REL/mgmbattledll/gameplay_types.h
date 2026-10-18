/* Shared battle-mode state and gameplay routine declarations. */
#ifndef REL_MGMBATTLEDLL_GAMEPLAY_TYPES_H
#define REL_MGMBATTLEDLL_GAMEPLAY_TYPES_H

#include "REL/mgmbattledll/include/game/sprite.h"
#include "REL/mgmbattledll/module_types.h"

extern u8 MgModeWork[256];
extern int MgModeWorkInt[16];
extern f32 MgModeWorkFloat[16];
extern s16 MgGameNo;
extern s16 MgModeSubMode;
extern s16 MgModeScore[4];
extern u8 MgModePlayNum;
extern u8 MgBattleOrder[4];
s16 _CharFXPlay(s16 charNo, s16 seNo, u8 voiceFlag);

typedef struct MgBattleChoices_s {
    /* Current availability or menu choice for each of the four players. */
    s16 value[4];
} MgBattleChoices;

typedef struct MgBattleAwardFlags_s {
    /* Nonzero for players who receive the tied-score award effect. */
    s16 player[4];
} MGBATTLE_AWARD_FLAGS;

extern u32 lbl_1_data_3F8[12];
extern Point3d lbl_1_data_428[3][4];
extern Point3d lbl_1_data_4B8[4];
extern f32 lbl_1_data_4E8[2];
extern s16 lbl_1_data_4F0[4];
extern s16 lbl_1_data_628[4];
extern s16 lbl_1_data_630[4];
extern s16 lbl_1_data_638[4];
extern s32 lbl_1_data_640[16];
extern s32 lbl_1_data_680[3][3];
extern s32 lbl_1_data_6A4;
extern Point3d lbl_1_data_6A8;
extern Point3d lbl_1_data_6B4;
extern Point3d lbl_1_data_6C0;
extern Point3d lbl_1_data_6CC;
extern Point3d lbl_1_data_6D8;
extern Point3d lbl_1_data_6E4[3][4];
extern Point3d lbl_1_data_774[2][4];
extern Point3d lbl_1_data_7D4[2][4];
extern Point3d lbl_1_data_834[2];
extern Point3d lbl_1_data_84C[2][4];
extern MgBattleMotion lbl_1_data_4F8[14];
extern s16 lbl_1_bss_3E8;
extern s16 lbl_1_bss_3EA[32];
extern s16 lbl_1_bss_42A[4];
extern s16 lbl_1_bss_432[4];
extern s16 lbl_1_bss_43A;
extern s16 lbl_1_bss_43C[4];
extern s16 lbl_1_bss_444;
extern s16 lbl_1_bss_446;
extern s16 lbl_1_bss_448[4][4];
extern s16 lbl_1_bss_468[4];
extern s16 lbl_1_bss_470[3];
extern s16 lbl_1_bss_476;
extern s16 lbl_1_bss_478[6];
extern s16 lbl_1_bss_484;
extern s16 lbl_1_bss_486[6];
extern s16 lbl_1_bss_492;
extern ANIMDATA * lbl_1_bss_494[16];
extern MgBattleRecord lbl_1_bss_4D4[4];

s32 fn_1_1AF5C(Point3d *vector, Point3d *out);
void fn_1_1B0AC(f32 rotX, f32 rotY, f32 rotZ, Point3d *position, Point3d *out);
f64 fn_1_1B16C(Point3d *, Point3d *);
void fn_1_3840(void);
s32 fn_1_394C(void);
void fn_1_4548(u16 enabled);
void fn_1_72E4(s16, Point3d *, f32 *);
void fn_1_1408(s16 windowCount);
void fn_1_1528(s16 windowCount);
s16 fn_1_1648(s16 messageId);
void fn_1_35CC(void);
s16 fn_1_0(s16, s16, Vec *);
s16 fn_1_1B324(void);
s32 fn_1_1B358(void);
void fn_1_3974(void);
s32 fn_1_3A80(void);
s16 fn_1_5DA8(s16 player);
s16 fn_1_5E24(void);
void fn_1_5808(s16 player, s16 model);
void fn_1_71AC(s16 playerIndex, Point3d *targetPosition);
void fn_1_734C(s16 playerIndex);
void fn_1_7370(s16 playerIndex);
void fn_1_3BB4(void);
void fn_1_7AF0(void);
void fn_1_8428(void);
void fn_1_1B380(void);
void fn_1_2F10(s16, s16);
void fn_1_2FA0(s16, Point3d *);
void fn_1_3018(void);
s16 fn_1_7394(s16);
void fn_1_3B78(Point3d *position);
void fn_1_2A4(s16 count, u8 *playerOrder);
s16 fn_1_45D4(void);
void fn_1_43C(void);
void fn_1_4F8(void);
void fn_1_5E0(s16 modeNo, s16 timeNo);
void fn_1_6F4(void);

void fn_1_AB04(s16 playerIndex, s16 motionEntry);
f32 fn_1_AD8C(s16 playerIndex);
void fn_1_ADA8(void);
void fn_1_B414(void);
f32 fn_1_B85C(void);
f32 fn_1_B8E0(void);
s16 fn_1_B964(void);
s16 fn_1_B9E8(void);
s16 fn_1_BA94(s16 playerIndex);
s16 fn_1_BAEC(s16 playerIndex);
s16 fn_1_BB40(s16 playerIndex);
s16 fn_1_BB60(s16 playerIndex);
void fn_1_BB7C(void);
s16 fn_1_BC74(s16 player);
s16 fn_1_CA1C(s16 playerIndex, s16 choice);
s16 fn_1_E770(s16 player, s16 result);
s16 fn_1_EE48(s16 player);
void fn_1_F464(void);
s16 fn_1_FA3C(s16 player);
s16 fn_1_107F0(void);
s16 fn_1_11020(s16 player);
s16 fn_1_11898(s16 playerIndex);
void fn_1_124FC(void);
void fn_1_12544(s16 groupId, s16 memberIndex, f32 *positionX, f32 *positionY);
void fn_1_125A0(void);
void fn_1_139BC(void);
void fn_1_13AAC(void);
void fn_1_148BC(void);
s16 fn_1_156CC(void);
void fn_1_169FC(void);
void fn_1_174FC(void);
void fn_1_175A0(void);
void fn_1_17740(void);
s32 fn_1_17A74(void);
void fn_1_17E40(void);
s32 fn_1_18434(void);
void fn_1_18508(void);
void fn_1_1899C(void);
void fn_1_19080(void);

#endif
