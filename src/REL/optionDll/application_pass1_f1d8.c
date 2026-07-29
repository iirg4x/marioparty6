#define _MATH_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/sprite.h"

typedef struct OptionModelEntry {
    u32 dataNum;
    s16 jointModel;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    u32 attr1;
    u32 attr2;
    s16 cameraBit;
} OptionModelEntry;

typedef struct OptionAnimPair {
    s16 animId1;
    s16 animId2;
    s16 modelId;
} OptionAnimPair;

extern OptionAnimPair lbl_1_bss_5D6[17];
extern s16 lbl_1_bss_63C[];
extern s16 lbl_1_bss_66E[];
extern ANIMDATA *lbl_1_bss_BF8[];
extern OptionModelEntry lbl_1_data_1AB0[];
extern s32 lbl_1_data_2060[60];

void fn_1_F1D8(void)
{
    OptionModelEntry *entry = lbl_1_data_1AB0;
    s16 i;

    for (i = 0; entry->dataNum != 0xFFFFFFFF; entry++, i++) {
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
