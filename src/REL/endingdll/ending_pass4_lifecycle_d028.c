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

extern OMOBJ *lbl_1_bss_4;
extern s16 lbl_1_bss_1A1C[2];
extern float lbl_1_rodata_C8;

void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
void HuSprGrpPosSet(s16 group, float x, float y);
void fn_1_EAB8(s16 display, HuVecF *position);
void fn_1_F1B8(s16 display, HuVecF *position);
void fn_1_E0EC(s16 group, u32 attr);
void fn_1_1160(OMOBJ *object);

void fn_1_D028(void)
{
    OMOBJ *object = lbl_1_bss_4;

    Hu3DMotionKill(object->mtnId[7]);
    Hu3DModelKill(object->mdlId[7]);
    fn_1_EAB8(0, NULL);
    fn_1_F1B8(0, NULL);
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
        lbl_1_rodata_C8);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    lbl_1_bss_4->work[0] = 3;
    lbl_1_bss_4->objFunc = fn_1_1160;
    lbl_1_bss_4->objFunc = NULL;
}
