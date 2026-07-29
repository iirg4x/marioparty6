#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_ANIMID;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

struct omObj_s {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    Vec trans;
    Vec rot;
    Vec scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

typedef struct MdsingCharacterDesc {
    s16 unk_0;
    s16 unk_2;
    s16 unk_4;
    s16 unk_6;
    s16 chrSel;
    s16 unk_A;
    s16 unk_C;
} MDSING_CHARACTER_DESC;

typedef struct MdsingModelEntry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 unk_A;
    Vec pos;
    Vec rot;
    Vec scale;
    s16 unk_30;
    u8 unk_32;
    u8 unk_33;
    u8 unk_34;
    u8 unk_35;
    u8 unk_36;
    u8 unk_37;
} MDSING_MODEL_ENTRY;

#define HU3D_ATTR_DISPOFF (1 << 0)

extern OMOBJ *lbl_1_bss_10;
extern MDSING_MODEL_ENTRY lbl_1_bss_E74[16];
extern MDSING_CHARACTER_DESC lbl_1_bss_1308[2];

extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_18C;

void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelPosSetV(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void HuPrcSleep(s32 time);
s32 HuAudFXPlay(s32 soundId);
void fn_1_6A7C(OMOBJ *obj);
void fn_1_344B4(s16 playerNo, Vec *pos, s16 arg2, s16 arg3);

void fn_1_6C48(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_CHARACTER_DESC *desc;
    Vec pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_344B4(i, &pos, 4, 0);
        } else {
            fn_1_344B4(i, &pos, desc->unk_A, 0);
        }
    }
    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        pos.y -= lbl_1_rodata_18C;
        Hu3DModelPosSetV(obj->mdlId[desc->chrSel], &pos);
        Hu3DModelScaleSet(obj->mdlId[desc->chrSel], lbl_1_rodata_64,
            lbl_1_rodata_64, lbl_1_rodata_64);
        Hu3DModelAttrReset(obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_6A7C;
    HuPrcSleep(15);
    HuAudFXPlay(0x4A1);
    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_344B4(i, &pos, 4, 1);
        } else {
            fn_1_344B4(i, &pos, desc->unk_A, 1);
        }
    }
}
