#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HUSPR_GROUPID;
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
extern HUSPR_GROUPID lbl_1_bss_1292[9];
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_12C;
extern const float lbl_1_rodata_1C8;

void HuSprGrpPosSet(HUSPR_GROUPID groupId, float x, float y);
void HuSprGrpTPLvlSet(HUSPR_GROUPID groupId, float level);

void fn_1_7B64(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

inline void fn_1_7B64(void);

void fn_1_7B94(void)
{
    HuSprGrpPosSet(
        lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
}

inline void fn_1_7B94(void);
