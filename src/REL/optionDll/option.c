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

typedef struct OptionAudioEntry {
    s8 type;
    u8 pad[11];
    s32 id;
} OptionAudioEntry;

typedef struct OptionMessageEntry {
    s8 type;
    u8 pad[7];
    s32 message;
    s32 value;
} OptionMessageEntry;

typedef struct OptionModelEntry {
    u32 dataNum;
    s16 jointModel;
    s16 pad;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    u32 attr1;
    u32 attr2;
    s16 cameraBit;
    u16 unk_32;
} OptionModelEntry;

typedef struct OptionAnimPair {
    s16 animId1;
    s16 animId2;
    s16 modelId;
} OptionAnimPair;

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
extern HUPROCESS *lbl_1_bss_838;
extern HUPROCESS *lbl_1_bss_83C;
extern OMOBJ *lbl_1_bss_840;
extern OptionWork lbl_1_bss_8;
extern s16 lbl_1_bss_5D4;
extern OptionAnimPair lbl_1_bss_5D6[17];
extern s16 lbl_1_bss_63C[];
extern s16 lbl_1_bss_66E[];
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_6A0;
extern u32 lbl_1_bss_848[];
extern char lbl_1_bss_9D8[5][4];
extern s16 lbl_1_bss_9EC[7][2];
extern char lbl_1_bss_A08[7][30];
extern s16 lbl_1_bss_AE0[7][2];
extern OptionMessageEntry *lbl_1_bss_AFC;
extern ANIMDATA *lbl_1_bss_BF8[];
extern OptionPrizeEntry lbl_1_data_0[];
extern OM_CAMERA_VIEW lbl_1_data_320;
extern OM_CAMERA_VIEW lbl_1_data_33C;
extern char lbl_1_data_40C[];
extern char lbl_1_data_472[];
extern char lbl_1_data_479[];
extern char lbl_1_data_486[];
extern char lbl_1_data_450[];
extern char lbl_1_data_462[];
extern char lbl_1_data_46A[];
extern char lbl_1_data_1AA8[];
extern OptionModelEntry lbl_1_data_1AB0[];
extern s32 lbl_1_data_2060[60];
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
extern float lbl_1_rodata_54;
extern float lbl_1_rodata_58;
extern float lbl_1_rodata_5C;
extern float lbl_1_rodata_60;
extern float lbl_1_rodata_64;
extern float lbl_1_rodata_68;
extern float lbl_1_rodata_6C;
extern float lbl_1_rodata_70;

void fn_1_A0(void);
void fn_1_804(void);
void fn_1_FDC(void);
void fn_1_18B8(s16 cameraNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_3BD8(OMOBJ *obj);
extern int HuMCMicGet(void);
int sprintf(char *buffer, const char *format, int value);

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
    lbl_1_bss_83C = omInitObjMan(50, 8192);
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
    lbl_1_bss_840 = omAddObjEx(lbl_1_bss_83C, 32730, 0, 0, -1,
        omOutViewMulti);
    lbl_1_bss_840->work[0] = 2;
    omAddObjEx(lbl_1_bss_83C, 0, 32, 32, -1, fn_1_3BD8);
    HuWinInit(0);
    HuPrcChildCreate(fn_1_804, 1000, 12288, 0, lbl_1_bss_83C);
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
    HuPrcChildCreate(fn_1_FDC, 1000, 4096, 0, lbl_1_bss_83C);
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
    lbl_1_bss_838 = HuPrcChildCreate(fn_1_19E0, 1000, 4096, 0,
        lbl_1_bss_83C);
    fn_1_19B8(0);
    HuPrcChildCreate(fn_1_18D8, 1000, 4096, 0, lbl_1_bss_83C);
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
    HuSprExecLayerCameraSet(64, 2, 2);
    lbl_1_bss_5D4 = HuSprGrpCreate(105);

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
    HuSprDrawNoSet(lbl_1_bss_5D4, 4, 64);
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
    for (i = 0; i < 105; i++) {
        HuSprAttrSet(lbl_1_bss_5D4, i, HUSPR_ATTR_DISPOFF);
    }
    fn_1_E6F8();

    lbl_1_bss_6A2[0] = HuWinExCreateFrame(lbl_1_rodata_60,
        lbl_1_rodata_64, 544, 80, -1, 3);
    HuWinAttrSet(lbl_1_bss_6A2[0], 2048);
    HuWinMesSpeedSet(lbl_1_bss_6A2[0], 0);
    HuWinPadMaskSet(lbl_1_bss_6A2[0], 1);
    lbl_1_bss_6A0 = HuWinCreate(lbl_1_rodata_60, lbl_1_rodata_68,
        544, 40, 0);
    HuWinBGTPLvlSet(lbl_1_bss_6A0, lbl_1_rodata_20);
    HuWinAttrSet(lbl_1_bss_6A0, 2048);
    HuWinMesSpeedSet(lbl_1_bss_6A0, 0);
    HuWinDispOff(lbl_1_bss_6A0);
    HuWinPriSet(lbl_1_bss_6A0, 10);
    Hu3DCameraLayerHookSet(2, 4, fn_1_18B8);
}

void fn_1_18B8(s16 cameraNo)
{
    Hu3DZClear();
}

void fn_1_18D8(void)
{
    float rot;

    Hu3DAnimCreate(HuDataSelHeapReadNum(12451874, HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_472);
    Hu3DAnimCreate(HuDataSelHeapReadNum(12451875, HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_479);
    rot = lbl_1_rodata_20;
    while (TRUE) {
        Hu3DModelRotSet(lbl_1_bss_66E[1], lbl_1_rodata_20,
            lbl_1_rodata_20, rot);
        rot -= lbl_1_rodata_6C;
        if (rot < lbl_1_rodata_20) {
            rot += lbl_1_rodata_70;
        }
        HuPrcVSleep();
    }
}

void fn_1_19B8(s32 value)
{
    void **property = &lbl_1_bss_838->property;

    *property = (void *)value;
}

void fn_1_E688(OptionAudioEntry *entry)
{
    HuAudSStreamAllFadeOut(150);
    HuAudStreamFadeOut(10);
    if (entry->type == 0) {
        HuPrcSleep(30);
        HuAudSStreamPlay((s16)entry->id);
    } else {
        HuAudFXPlay(entry->id);
        HuPrcSleep(20);
    }
}

void fn_1_E538(s16 page)
{
    OptionMessageEntry *entry = lbl_1_bss_AFC;
    s16 entryCount;
    s16 pageCount;
    s16 row;
    s16 column;

    for (entryCount = 0; entry->type != -1; entryCount++, entry++) {
    }
    pageCount = (entryCount + 1) / 2;
    entry = &lbl_1_bss_AFC[page * 2];
    for (row = 0; row < 7; row++) {
        for (column = 0; column < 2; column++, entry++) {
            if (column + (row * 2) + (page * 2) >= entryCount) {
                HuWinMesSet(lbl_1_bss_AE0[row][column],
                    (s32)lbl_1_data_1AA8);
            } else {
                HuWinMesSet(lbl_1_bss_AE0[row][column], entry->message);
            }
        }
    }
}

void fn_1_6504(s16 prizeNo)
{
    if (lbl_1_bss_8.prizeUnlocked[prizeNo] == FALSE) {
        HuWinMesSet(lbl_1_bss_6A2[0], 1638501);
    } else {
        HuWinMesSet(lbl_1_bss_6A2[0],
            lbl_1_bss_8.prizeDescription[prizeNo]);
    }
}

void fn_1_6590(s16 firstPrize)
{
    s16 i;

    for (i = 0; i < 7; i++) {
        if (lbl_1_bss_8.prizeUnlocked[firstPrize + i] == FALSE) {
            HuWinMesSet(lbl_1_bss_9EC[i][0], 1638450);
            lbl_1_bss_A08[i][0] = 14;
            lbl_1_bss_A08[i][1] = 19;
            lbl_1_bss_A08[i][2] = 16;
            lbl_1_bss_A08[i][3] = 195;
            lbl_1_bss_A08[i][4] = 195;
            lbl_1_bss_A08[i][5] = 195;
            lbl_1_bss_A08[i][6] = 0;
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)lbl_1_bss_A08[i]);
        } else {
            HuWinMesSet(lbl_1_bss_9EC[i][0],
                lbl_1_bss_8.prizeName[firstPrize + i]);
            lbl_1_bss_A08[i][0] = 14;
            lbl_1_bss_A08[i][1] = 19;
            lbl_1_bss_A08[i][2] = 16;
            sprintf(&lbl_1_bss_A08[i][3], lbl_1_data_486,
                lbl_1_bss_8.prizeId[firstPrize + i]);
            if (lbl_1_bss_A08[i][2] == '0') {
                lbl_1_bss_A08[i][2] = ' ';
                if (lbl_1_bss_A08[i][3] == '0') {
                    lbl_1_bss_A08[i][3] = ' ';
                }
            }
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)lbl_1_bss_A08[i]);
        }
    }
}

void fn_1_B02C(s16 first, s16 count)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (first + i >= count) {
            HuWinHomeClear(lbl_1_bss_9EC[i][0]);
            HuWinHomeClear(lbl_1_bss_9EC[i][1]);
        } else {
            sprintf(lbl_1_bss_9D8[i], lbl_1_data_486, first + i + 1);
            HuWinInsertMesSet(lbl_1_bss_9EC[i][0],
                (u32)lbl_1_bss_9D8[i], 0);
            HuWinMesSet(lbl_1_bss_9EC[i][0], 4390973);
            HuWinMesSet(lbl_1_bss_9EC[i][1],
                MgDataTbl[lbl_1_bss_8.consecutiveRecord[first + i]].nameMes);
        }
    }
}

s16 fn_1_C324(s16 flag, s16 type)
{
    MGDATA *data = MgDataTbl;
    s16 count = 0;
    s16 index = count;

    while (data->ovl != 65535) {
        if ((data->flag & flag) && data->type == type) {
            if (lbl_1_bss_8.minigameUnlocked[index] == FALSE) {
                lbl_1_bss_848[count] = 4390943;
            } else {
                lbl_1_bss_848[count] = data->nameMes;
            }
            count++;
        }
        index++;
        data++;
    }
    return count;
}

void fn_1_F1D8(void)
{
    OptionModelEntry *entry = lbl_1_data_1AB0;
    s16 i;

    for (i = 0; entry->dataNum != 4294967295; entry++, i++) {
        if (entry->jointModel == -1) {
            lbl_1_bss_66E[i] = Hu3DModelCreateData(entry->dataNum);
            Hu3DModelPosSetV(lbl_1_bss_66E[i], &entry->pos);
            Hu3DModelRotSetV(lbl_1_bss_66E[i], &entry->rot);
            Hu3DModelScaleSetV(lbl_1_bss_66E[i], &entry->scale);
            Hu3DModelAttrSet(lbl_1_bss_66E[i], entry->attr1);
            Hu3DModelAttrSet(lbl_1_bss_66E[i], entry->attr2);
            Hu3DModelCameraSet(lbl_1_bss_66E[i], entry->cameraBit);
            Hu3DModelLayerSet(lbl_1_bss_66E[i], 1);
        } else {
            lbl_1_bss_63C[i] = Hu3DJointMotionData(
                lbl_1_bss_66E[entry->jointModel], entry->dataNum);
        }
    }
}

void fn_1_F394(void)
{
    s16 i;

    for (i = 0; i < 60; i++) {
        lbl_1_bss_BF8[i] = HuSprAnimDataRead(lbl_1_data_2060[i]);
    }
}

void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2)
{
    s16 i;

    for (i = 0; i < 17; i++) {
        if (modelId == lbl_1_bss_5D6[i].modelId) {
            break;
        }
    }
    if (i != 17) {
        Hu3DAnimAnimSet(lbl_1_bss_5D6[i].animId1, lbl_1_bss_BF8[animNo1]);
        Hu3DAnimBankSet(lbl_1_bss_5D6[i].animId1, bank1);
        Hu3DAnimAnimSet(lbl_1_bss_5D6[i].animId2, lbl_1_bss_BF8[animNo2]);
        Hu3DAnimBankSet(lbl_1_bss_5D6[i].animId2, bank2);
    }
}

BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2)
{
    HU3D_TEXANIM *anim;
    s16 i;

    for (i = 0; i < 17; i++) {
        if (modelId == lbl_1_bss_5D6[i].modelId) {
            break;
        }
    }
    if (i == 17) {
        return FALSE;
    }
    anim = &Hu3DTexAnimData[lbl_1_bss_5D6[i].animId1];
    if (anim->anim != lbl_1_bss_BF8[animNo1] || anim->bank != bank1) {
        return FALSE;
    }
    anim = &Hu3DTexAnimData[lbl_1_bss_5D6[i].animId2];
    if (anim->anim != lbl_1_bss_BF8[animNo2] || anim->bank != bank2) {
        return FALSE;
    }
    return TRUE;
}

float fn_1_F684(s16 groupId, s16 memberNo)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[groupId].sprId[memberNo]];

    return sprite->scale.x;
}
