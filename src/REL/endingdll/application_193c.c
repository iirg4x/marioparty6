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

typedef struct EndingMotionWork {
    s16 state;
    float time;
    float duration;
    HuVecF unk_0C;
    HuVecF unk_18;
    HuVecF unk_24;
    float start;
    float end;
    float unk_38;
    float unk_3C;
} EndingMotionWork;

extern OMOBJ *lbl_1_bss_8;
extern EndingMotionWork lbl_1_bss_1ADC[10];
extern float lbl_1_rodata_78;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_19C;
extern float lbl_1_rodata_1B0;

void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DModelRotGet(HU3D_MODELID modelId, HuVecF *rotation);
void Hu3DModelRotSetV(HU3D_MODELID modelId, HuVecF *rotation);
float fn_1_DDF8(float start, float end, float time, float duration);

void fn_1_193C(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;
    HuVecF rotation;

    for (model = 0; model < 10; model++, work++) {
        if (lbl_1_rodata_C8 == work->time) {
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                lbl_1_bss_8->mtnId[3 + (model * 6)],
                lbl_1_rodata_C8, lbl_1_rodata_78, 0x40000001);
        }
        Hu3DModelRotGet(object->mdlId[model], &rotation);
        rotation.y = fn_1_DDF8(work->start, work->end, work->time,
            work->duration);
        Hu3DModelRotSetV(object->mdlId[model], &rotation);
        if ((work->time += lbl_1_rodata_19C) > work->duration) {
            work->time = lbl_1_rodata_19C + work->duration;
            if (lbl_1_rodata_C8 == work->unk_38) {
                work->unk_38 = lbl_1_rodata_19C;
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[model * 6], lbl_1_rodata_C8,
                    lbl_1_rodata_1B0, 0x40000001);
            }
        }
    }
}
