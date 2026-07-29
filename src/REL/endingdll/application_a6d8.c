#include <dolphin/gx/GXStruct.h>
#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_GLIGHTID;

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

typedef struct EndingLightVectors {
    HuVecF vector[2];
} EndingLightVectors;

extern OMOBJ *lbl_1_bss_1C;
extern EndingMotionWork lbl_1_bss_34[96];
extern HU3D_GLIGHTID lbl_1_bss_1A56[2];
extern EndingLightVectors lbl_1_rodata_88;
extern EndingLightVectors lbl_1_rodata_A0;
extern GXColor lbl_1_rodata_B8;
extern HuVecF lbl_1_rodata_288;

void omSetStatBit(OMOBJ *object, u16 bit);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MODELID Hu3DModelLink(HU3D_MODELID modelId);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void HuPrcVSleep(void);
void Hu3DLightAllKill(void);
HU3D_GLIGHTID Hu3DGLightCreateV(
    HuVecF *position, HuVecF *direction, GXColor *color);
void Hu3DGLightInfinitytSet(HU3D_GLIGHTID lightId);
void Hu3DGLightStaticSet(HU3D_GLIGHTID lightId, BOOL enable);
void fn_1_8F80(OMOBJ *object);
void fn_1_F1B8(s16 display, HuVecF *position);
void fn_1_F23C(s16 count);
void fn_1_F068(s16 index, s16 display, HuVecF *position);
void fn_1_F11C(s16 index, s16 count);

static inline void fn_1_93A0(OMOBJ *object)
{
    EndingMotionWork *work;
    s16 model;

    omSetStatBit(object, 0x100);
    for (model = 0, work = lbl_1_bss_34; model < 96; model++, work++) {
        work->state = 0;
    }
    for (model = 0; model < 96; model++) {
        if (model == 0) {
            object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                0x220034, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            object->mdlId[model] = Hu3DModelLink(object->mdlId[0]);
        }
        Hu3DModelAttrSet(object->mdlId[model], HU3D_ATTR_DISPOFF);
    }
    object->objFunc = fn_1_8F80;
}

void fn_1_A6D8(void)
{
    HuVecF position = lbl_1_rodata_288;
    s16 light;

    fn_1_93A0(lbl_1_bss_1C);
    fn_1_F1B8(1, &position);
    HuPrcVSleep();
    fn_1_F23C(1);
    fn_1_F068(1, 1, &position);
    fn_1_F11C(1, 0);
    Hu3DLightAllKill();
    {
        EndingLightVectors lightDir;
        EndingLightVectors lightPos;
        GXColor color;

        lightPos = lbl_1_rodata_88;
        lightDir = lbl_1_rodata_A0;
        color = lbl_1_rodata_B8;

        for (light = 0; light < 2; light++) {
            lbl_1_bss_1A56[light] = Hu3DGLightCreateV(
                &lightPos.vector[light], &lightDir.vector[light], &color);
            Hu3DGLightInfinitytSet(lbl_1_bss_1A56[light]);
            Hu3DGLightStaticSet(lbl_1_bss_1A56[light], 1);
        }
    }
}
