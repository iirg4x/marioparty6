#define _MATH_H

#include "dolphin.h"
#include "game/audio.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"

typedef void (*VoidFunc)(void);

typedef struct OptionDecaRecord {
    s16 character;
    s16 finalScore;
    s16 score[10];
} OptionDecaRecord;

typedef struct OptionPrizeEntry {
    s16 flag;
    s16 unk2;
    u32 name;
    u32 description;
    s16 id;
    s16 unkE;
} OptionPrizeEntry;

typedef struct OptionWork {
    s16 micEnabled;
    s16 micStatus;
    s16 soundMode;
    s16 boardRecord[6];
    s16 boardCharacterRecord[6][11];
    OptionDecaRecord decaRecord[10];
    s32 prizeUnlocked[42];
    s16 prizeId[42];
    u32 prizeName[42];
    u32 prizeDescription[42];
    s32 minigameUnlocked[81];
    u32 record[19];
    u8 consecutiveRecordCount;
    u8 consecutiveRecord[100];
} OptionWork;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s16 lbl_1_bss_6;
extern OMOBJMAN *lbl_1_bss_83C;
extern OMOBJ *lbl_1_bss_840;
extern OptionWork lbl_1_bss_8;
extern OptionPrizeEntry lbl_1_data_0[];
extern OM_CAMERA_VIEW lbl_1_data_320;
extern OM_CAMERA_VIEW lbl_1_data_33C;
extern char lbl_1_data_40C[];
extern float lbl_1_rodata_10;
extern float lbl_1_rodata_14;
extern float lbl_1_rodata_18;
extern float lbl_1_rodata_1C;
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_24;
extern float lbl_1_rodata_28;
extern float lbl_1_rodata_2C;
extern float lbl_1_rodata_30;
extern float lbl_1_rodata_34;
extern float lbl_1_rodata_38;
extern float lbl_1_rodata_3C;
extern float lbl_1_rodata_40;
extern float lbl_1_rodata_44;
extern float lbl_1_rodata_48;
extern float lbl_1_rodata_4C;
extern float lbl_1_rodata_50;

void fn_1_A0(void);
void fn_1_804(void);
void fn_1_3BD8(OMOBJ *obj);
extern int HuMCMicGet(void);

int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void fn_1_A0(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    s16 lightId;
    s32 j;
    s32 i;

    OSReport(lbl_1_data_40C);
    lbl_1_bss_83C = omInitObjMan(50, 0x2000);
    omSysPauseEnable(FALSE);
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_10, lbl_1_rodata_14,
        lbl_1_rodata_18, lbl_1_rodata_1C);
    Hu3DCameraViewportSet(1, lbl_1_rodata_20, lbl_1_rodata_20,
        lbl_1_rodata_24, lbl_1_rodata_28, lbl_1_rodata_20,
        lbl_1_rodata_2C);
    Hu3DCameraPerspectiveSet(2, lbl_1_rodata_30, lbl_1_rodata_14,
        lbl_1_rodata_18, lbl_1_rodata_1C);
    Hu3DCameraViewportSet(2, lbl_1_rodata_20, lbl_1_rodata_20,
        lbl_1_rodata_24, lbl_1_rodata_28, lbl_1_rodata_20,
        lbl_1_rodata_2C);
    omCameraViewSetMulti(1, &lbl_1_data_320);
    omCameraViewSetMulti(2, &lbl_1_data_33C);
    lightId = (int)Hu3DGLightCreate(lbl_1_rodata_20, lbl_1_rodata_34,
        lbl_1_rodata_38, lbl_1_rodata_20, lbl_1_rodata_3C,
        lbl_1_rodata_40, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_840 = omAddObjEx(lbl_1_bss_83C, 0x7FDA, 0, 0, -1,
        omOutViewMulti);
    lbl_1_bss_840->work[0] = 2;
    omAddObjEx(lbl_1_bss_83C, 0, 0x20, 0x20, -1, fn_1_3BD8);
    HuWinInit(0);
    HuPrcChildCreate(fn_1_804, 1000, 0x3000, 0, lbl_1_bss_83C);
    Hu3DShadowCreate(lbl_1_rodata_44, lbl_1_rodata_14,
        lbl_1_rodata_48);
    shadowPos.x = lbl_1_rodata_20;
    shadowPos.y = lbl_1_rodata_38;
    shadowPos.z = lbl_1_rodata_4C;
    shadowUp.y = lbl_1_rodata_2C;
    shadowUp.x = shadowUp.z = lbl_1_rodata_20;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = lbl_1_rodata_20;
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(lbl_1_rodata_50);

    lbl_1_bss_8.micEnabled = GwCommon.vibrateF ? TRUE : FALSE;
    lbl_1_bss_8.micStatus = HuMCMicGet();
    if (OSGetSoundMode() == OS_SOUND_MODE_MONO) {
        lbl_1_bss_8.soundMode = GwCommon.outputMode = 0;
    } else if (GwCommon.outputMode != 0) {
        lbl_1_bss_8.soundMode = GwCommon.outputMode;
    } else {
        lbl_1_bss_8.soundMode = GwCommon.outputMode = 1;
    }

    for (j = 0; j < 6; j++) {
        for (i = 0; i < 11; i++) {
            lbl_1_bss_8.boardCharacterRecord[j][i] =
                GwCommon.charPlayNum[j][i];
        }
        lbl_1_bss_8.boardRecord[j] = GwCommon.boardPlayNum[j];
    }
    for (j = 0; j < 10; j++) {
        lbl_1_bss_8.decaRecord[j].character = GwCommon.decaScore[j].charNo;
        for (i = 0; i < 10; i++) {
            lbl_1_bss_8.decaRecord[j].score[i] =
                GwCommon.decaScore[j].mgScore[i];
        }
        lbl_1_bss_8.decaRecord[j].finalScore =
            GwCommon.decaScore[j].finalScore;
    }

    for (i = 2, lbl_1_bss_6 = 0; lbl_1_data_0[i].unk2 != -1; i++) {
        if (lbl_1_data_0[i].flag != -1) {
            lbl_1_bss_8.prizeId[lbl_1_bss_6] = lbl_1_data_0[i].id;
            lbl_1_bss_8.prizeUnlocked[lbl_1_bss_6] =
                GWSinglePrizeSaveFlagGet(lbl_1_data_0[i].flag);
            lbl_1_bss_8.prizeName[lbl_1_bss_6] = lbl_1_data_0[i].name;
            lbl_1_bss_8.prizeDescription[lbl_1_bss_6] =
                lbl_1_data_0[i].description;
            lbl_1_bss_6++;
        }
    }
    for (i = 0; i < 81; i++) {
        lbl_1_bss_8.minigameUnlocked[i] = GWMgUnlockGet(i + 601);
    }
    lbl_1_bss_8.consecutiveRecordCount = GwCommon.renshoMgRecordNum;
    for (i = 0; i < 100; i++) {
        lbl_1_bss_8.consecutiveRecord[i] = GwCommon.renshoMgRecord[i];
    }
    HuAudBGMPlay(7);
}
