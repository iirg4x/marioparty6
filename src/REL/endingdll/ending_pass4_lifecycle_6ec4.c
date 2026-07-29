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

typedef struct EndingModelObjectNames {
    char *name[10];
} EndingModelObjectNames;

extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_18;
extern float lbl_1_rodata_19C;
extern float lbl_1_rodata_1E0;
extern EndingModelObjectNames lbl_1_rodata_1B4;

void fn_1_1C0C(s16 index, s16 motion, float time);
void Hu3DModelObjMtxGet(HU3D_MODELID modelId, char *name, Mtx matrix);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
void HuPrcVSleep(void);

void fn_1_6EC4(void)
{
    EndingModelObjectNames names = lbl_1_rodata_1B4;
    Mtx matrix;
    s16 model;
    s16 index;
    s16 motion;

    for (model = 0; model < 10; model++) {
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], names.name[model], matrix);
        Hu3DModelPosSet(lbl_1_bss_8->mdlId[model], matrix[0][3],
            matrix[1][3], matrix[2][3]);
    }
    motion = lbl_1_bss_18->mdlId[0];
    for (index = 0; index < 10; index++) {
        fn_1_1C0C(index, motion, lbl_1_rodata_19C);
    }
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[0], lbl_1_rodata_1E0);
}
