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

typedef struct omObjMan OMOBJMAN;

extern OMOBJMAN *lbl_1_bss_0;

void Hu3DModelHookReset(HU3D_MODELID modelId);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *object);

void fn_1_26B0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}
