#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

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

typedef struct MDSingMotionWork {
    s16 state;
    s16 pad;
    Vec pos;
    Vec control;
    Vec end;
    float time;
    float duration;
} MDSING_MOTION_WORK;

extern OMOBJ *lbl_1_bss_C;
extern MDSING_MOTION_WORK lbl_1_bss_DB4[2];
extern s32 lbl_1_bss_13A4[5];
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_134;
extern const float lbl_1_rodata_13C;
extern const float lbl_1_rodata_140;
extern const float lbl_1_rodata_144;
extern const float lbl_1_rodata_148;
extern const float lbl_1_rodata_14C;
extern const float lbl_1_rodata_150;

void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
int HuAudFXPlay(int soundId);
void fn_1_5268(OMOBJ *obj);

void fn_1_57E4(void)
{
    MDSING_MOTION_WORK *work = &lbl_1_bss_DB4[0];
    OMOBJ *obj = lbl_1_bss_C;

    work->state = 0;
    work->time = lbl_1_rodata_64;
    work->duration = lbl_1_rodata_13C;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = lbl_1_rodata_140;
    work->pos.y = lbl_1_rodata_64;
    work->pos.z = lbl_1_rodata_144;
    Hu3DModelPosGet(obj->mdlId[0], &work->control);
    work->control.x = lbl_1_rodata_64;
    work->control.y = lbl_1_rodata_134;
    work->control.z = lbl_1_rodata_148;
    Hu3DModelPosGet(obj->mdlId[0], &work->end);
    work->end.x = lbl_1_rodata_64;
    work->end.y = lbl_1_rodata_14C;
    work->end.z = lbl_1_rodata_150;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[2],
        lbl_1_rodata_64, lbl_1_rodata_64, 0);
    lbl_1_bss_13A4[0] = HuAudFXPlay(0x47E);
    obj->objFunc = fn_1_5268;
}
