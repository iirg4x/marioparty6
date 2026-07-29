#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_ANIMID;
typedef struct AnimData_s ANIMDATA;
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

extern OMOBJ *lbl_1_bss_8;
extern ANIMDATA *lbl_1_bss_12A4[25];

ANIMDATA *Hu3DAnimAnimSet(HU3D_ANIMID animId, ANIMDATA *anim);

void fn_1_7E3C(s16 animNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DAnimAnimSet(
        obj->mtnId[2], lbl_1_bss_12A4[(2 * animNo) + 3]);
    Hu3DAnimAnimSet(
        obj->mtnId[3], lbl_1_bss_12A4[(2 * animNo) + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

inline void fn_1_7E3C(s16 animNo);
