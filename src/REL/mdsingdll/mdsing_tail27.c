#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
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

typedef struct MdsingMoveWork {
    s16 state;
    s16 pad;
    Vec pos;
    Vec unk_10;
    Vec unk_1C;
    float time;
    float duration;
} MDSING_MOVE_WORK;

#define HU3D_MOTATTR_LOOP 0x40000001

extern OMOBJ *lbl_1_bss_10;
extern MDSING_MOVE_WORK lbl_1_bss_CF4[2];
extern MDSING_CHARACTER_DESC lbl_1_bss_1308[2];

extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_70;
extern const float lbl_1_rodata_90;
extern const float lbl_1_rodata_144;
extern const float lbl_1_rodata_180;
extern const float lbl_1_rodata_184;
extern const float lbl_1_rodata_188;
extern const float lbl_1_rodata_E0;

void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
s32 HuAudFXPlay(s32 soundId);
void fn_1_633C(OMOBJ *obj);

void fn_1_68C0(void)
{
    MDSING_CHARACTER_DESC *desc;
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_MOVE_WORK *work;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        work = &lbl_1_bss_CF4[i];
        work->state = 1;
        work->time = lbl_1_rodata_64;
        work->duration = lbl_1_rodata_70;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->pos);
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_10);
        work->unk_10.x += lbl_1_rodata_180 * work->unk_10.x;
        work->unk_10.y = lbl_1_rodata_184;
        work->unk_10.z = lbl_1_rodata_144;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_1C);
        work->unk_1C.y = lbl_1_rodata_188;
        work->unk_1C.z = lbl_1_rodata_E0;
        Hu3DMotionShiftSet(obj->mdlId[desc->chrSel],
            obj->mtnId[(2 * desc->chrSel) + 1], lbl_1_rodata_64,
            lbl_1_rodata_90, HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(0x4A2);
    obj->objFunc = fn_1_633C;
}
