#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef struct Process_s OMOBJMAN;
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

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern s16 lbl_1_data_10E;

void HuSprGrpKill(s16 group);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *object);
void fn_1_111B0(s16 display);
void fn_1_E1EC(s16 display, HuVecF *position);
void fn_1_EAB8(s16 display, HuVecF *position);

static inline void fn_1_9498(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_111B0(0);
        for (model = 95; model >= 0; model--) {
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

static inline void fn_1_4A0C(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_E1EC(0, NULL);
        fn_1_EAB8(0, NULL);
        for (model = 0; model < 16; model++) {
            Hu3DMotionKill(object->mtnId[model]);
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_B0A4(void)
{
    OMOBJ *object;

    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
    object = lbl_1_bss_4;
    Hu3DMotionKill(object->mtnId[0]);
    Hu3DModelKill(object->mdlId[0]);

    fn_1_9498(lbl_1_bss_1C);
    lbl_1_bss_1C->objFunc = NULL;
    lbl_1_bss_1C = NULL;

    fn_1_4A0C(lbl_1_bss_18);
    lbl_1_bss_18->objFunc = NULL;
    lbl_1_bss_18 = NULL;
}
