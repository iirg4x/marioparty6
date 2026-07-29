#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

#define HU3D_MOTATTR_LOOP 0x40000001

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

extern OMOBJ *lbl_1_bss_C;
extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_90;
extern const float lbl_1_rodata_B8;

void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);

void fn_1_514C(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_60);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_5C);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_64, lbl_1_rodata_B8, HU3D_MOTATTR_LOOP);
    }
}

void fn_1_51F4(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3],
        lbl_1_rodata_64, lbl_1_rodata_90, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_514C;
}
