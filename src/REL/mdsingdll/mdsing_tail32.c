#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_ANIMID;
typedef struct Process_s HUPROCESS;
typedef HUPROCESS OMOBJMAN;
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

extern OMOBJMAN *lbl_1_bss_0;

void Hu3DAnimKill(HU3D_ANIMID animId);
void Hu3DModelKill(HU3D_MODELID modelId);
void omDelObjEx(OMOBJMAN *objMan, OMOBJ *obj);

void fn_1_8670(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DAnimKill(obj->mtnId[2 * i]);
            Hu3DAnimKill(obj->mtnId[(2 * i) + 1]);
            obj->mtnId[2 * i] = -1;
            obj->mtnId[(2 * i) + 1] = -1;
        }
        for (i = 1; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_8670(OMOBJ *obj);
