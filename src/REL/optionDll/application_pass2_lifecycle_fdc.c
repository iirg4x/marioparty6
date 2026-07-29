#define _MATH_H

#include "dolphin.h"
#include "game/audio.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"

typedef struct OptionAnimPair {
    s16 animId1;
    s16 animId2;
    s16 modelId;
} OptionAnimPair;

extern HUPROCESS *lbl_1_bss_838;
extern OMOBJMAN *lbl_1_bss_83C;
extern s16 lbl_1_bss_5D4;
extern OptionAnimPair lbl_1_bss_5D6[17];
extern s16 lbl_1_bss_63C[];
extern s16 lbl_1_bss_66E[];
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_6A0;
extern ANIMDATA *lbl_1_bss_BF8[];
extern char lbl_1_data_450[];
extern char lbl_1_data_462[];
extern char lbl_1_data_46A[];
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_2C;
extern float lbl_1_rodata_54;
extern float lbl_1_rodata_58;
extern float lbl_1_rodata_5C;
extern float lbl_1_rodata_60;
extern float lbl_1_rodata_64;
extern float lbl_1_rodata_68;

void fn_1_FDC(void);
void fn_1_18B8(s16 cameraNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);

void fn_1_FDC(void)
{
    HuAudSStreamAllFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 40);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    SLSaveModeExec(FALSE);
    omOvlReturnEx(1, TRUE);
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_1044(void)
{
    HuPrcChildCreate(fn_1_FDC, 1000, 0x1000, 0, lbl_1_bss_83C);
}

void fn_1_1084(void)
{
    s16 spriteId;
    s16 i;

    fn_1_F1D8();
    Hu3DModelShadowMapSet(lbl_1_bss_66E[0]);
    Hu3DModelShadowSet(lbl_1_bss_66E[2]);
    Hu3DModelShadowSet(lbl_1_bss_66E[6]);
    Hu3DModelHookSet(lbl_1_bss_66E[2], lbl_1_data_450,
        lbl_1_bss_66E[6]);
    Hu3DMotionSet(lbl_1_bss_66E[2], lbl_1_bss_63C[3]);
    Hu3DModelAttrSet(lbl_1_bss_66E[2], HU3D_MOTATTR_LOOP);
    lbl_1_bss_838 = HuPrcChildCreate(fn_1_19E0, 1000, 0x1000, 0,
        lbl_1_bss_83C);
    fn_1_19B8(0);
    HuPrcChildCreate(fn_1_18D8, 1000, 0x1000, 0, lbl_1_bss_83C);
    fn_1_F394();
    for (i = 0; i < 17; i++) {
        Hu3DModelScaleSet(lbl_1_bss_66E[i + 7], lbl_1_rodata_20,
            lbl_1_rodata_20, lbl_1_rodata_20);
        lbl_1_bss_5D6[i].animId1 = Hu3DAnimCreate(lbl_1_bss_BF8[0],
            lbl_1_bss_66E[i + 7], lbl_1_data_462);
        lbl_1_bss_5D6[i].animId2 = Hu3DAnimCreate(lbl_1_bss_BF8[0],
            lbl_1_bss_66E[i + 7], lbl_1_data_46A);
        lbl_1_bss_5D6[i].modelId = lbl_1_bss_66E[i + 7];
        if (i >= 7) {
            Hu3DModelLayerSet(lbl_1_bss_66E[i + 7], 3);
        }
    }
    for (i = 14; i <= 23; i++) {
        Hu3DModelLayerSet(lbl_1_bss_66E[i], 4);
    }
    Hu3DModelLayerSet(lbl_1_bss_66E[24], 4);
    HuSprExecLayerCameraSet(0x40, 2, 2);
    lbl_1_bss_5D4 = HuSprGrpCreate(0x69);

    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 0, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 2);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 1, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 5);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 2, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 7);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 3, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[16], 110, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 4, spriteId);
    HuSprDrawNoSet(lbl_1_bss_5D4, 4, 0x40);
    spriteId = HuSprCreate(lbl_1_bss_BF8[21], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 5, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[22], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 6, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[24], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 7, spriteId);
    HuSprScaleSet(lbl_1_bss_5D4, 7, lbl_1_rodata_54,
        lbl_1_rodata_2C);

    for (i = 0; i < 11; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[15], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 8, spriteId);
    }
    for (i = 0; i < 50; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[19], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 19, spriteId);
    }
    for (i = 0; i < 10; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[18], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 69, spriteId);
    }
    for (i = 0; i < 10; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[17], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 79, spriteId);
    }
    for (i = 0; i < 5; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[20], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 89, spriteId);
    }
    for (i = 0; i < 11; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[i + 37], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, i + 94, spriteId);
    }
    HuSprGrpPosSet(lbl_1_bss_5D4, lbl_1_rodata_58, lbl_1_rodata_5C);
    for (i = 0; i < 0x69; i++) {
        HuSprAttrSet(lbl_1_bss_5D4, i, HUSPR_ATTR_DISPOFF);
    }
    fn_1_E6F8();

    lbl_1_bss_6A2[0] = HuWinExCreateFrame(lbl_1_rodata_60,
        lbl_1_rodata_64, 0x220, 0x50, -1, 3);
    HuWinAttrSet(lbl_1_bss_6A2[0], 0x800);
    HuWinMesSpeedSet(lbl_1_bss_6A2[0], 0);
    HuWinPadMaskSet(lbl_1_bss_6A2[0], 1);
    lbl_1_bss_6A0 = HuWinCreate(lbl_1_rodata_60, lbl_1_rodata_68,
        0x220, 0x28, 0);
    HuWinBGTPLvlSet(lbl_1_bss_6A0, lbl_1_rodata_20);
    HuWinAttrSet(lbl_1_bss_6A0, 0x800);
    HuWinMesSpeedSet(lbl_1_bss_6A0, 0);
    HuWinDispOff(lbl_1_bss_6A0);
    HuWinPriSet(lbl_1_bss_6A0, 10);
    Hu3DCameraLayerHookSet(2, 4, fn_1_18B8);
}
