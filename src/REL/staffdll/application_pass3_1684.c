#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
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

extern const f32 lbl_1_rodata_10;

extern f32 lbl_1_bss_124;
extern s16 lbl_1_bss_128;
extern s16 lbl_1_bss_12A;
extern HuVecF lbl_1_bss_12C[32];
extern s16 lbl_1_bss_2AC[44];

void fn_1_1110(OMOBJ *obj);

void fn_1_1684(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 31; i++) {
        lbl_1_bss_2AC[i] = -1;
        lbl_1_bss_12C[i].x = lbl_1_rodata_10;
        lbl_1_bss_12C[i].y = lbl_1_rodata_10;
    }
    lbl_1_bss_12A = 0;
    lbl_1_bss_128 = 0;
    lbl_1_bss_124 = lbl_1_rodata_10;
    obj->objFunc = fn_1_1110;
}
