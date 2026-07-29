#include <dolphin/mtx/GeoTypes.h>

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

extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;

void fn_1_884(OMOBJ *obj);

void fn_1_2B94(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[1] = 1;
    lbl_1_bss_C->objFunc = fn_1_884;
}
