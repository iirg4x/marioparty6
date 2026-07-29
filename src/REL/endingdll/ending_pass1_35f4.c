#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;

typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);

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
    HuVecF trans;
    HuVecF rot;
    HuVecF scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern float lbl_1_rodata_C8;

void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);

void fn_1_35F4(s16 index, s16 motion, float blend, u32 attr)
{
    if (index == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
            lbl_1_bss_C->mtnId[motion], lbl_1_rodata_C8, blend, attr);
    } else {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0],
            lbl_1_bss_10->mtnId[motion], lbl_1_rodata_C8, blend, attr);
    }
}
