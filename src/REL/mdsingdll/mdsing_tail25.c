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
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_178;

void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
void fn_1_595C(OMOBJ *obj);

s32 fn_1_5EF4(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    Vec pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x < lbl_1_rodata_178) {
        return 0;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_595C;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_64, lbl_1_rodata_64, HU3D_MOTATTR_LOOP);
    return 1;
}
