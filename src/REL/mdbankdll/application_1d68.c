#include <dolphin/mtx/GeoTypes.h>

typedef struct MdbankObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    void (*objFunc)(struct MdbankObject *obj);
    Vec trans;
    Vec rot;
    Vec scale;
    u16 modelCount;
    s16 *mdlId;
    u16 motionCount;
    s16 *mtnId;
    u32 work[4];
} MDBANK_OBJECT;

typedef MDBANK_OBJECT OMOBJ;

extern MDBANK_OBJECT *lbl_1_bss_10;
extern const float lbl_1_rodata_68;
extern const float lbl_1_rodata_F8;

void Hu3DMotionShiftSet(s16 modelId, s16 motionId, float start, float end,
    u32 attr);
void fn_1_1CD8(MDBANK_OBJECT *obj);

void fn_1_1D68(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1CD8;
}
