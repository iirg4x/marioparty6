#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

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

typedef struct EndingAudioState {
    s32 channel[32];
} EndingAudioState;

extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern s16 lbl_1_bss_26;
extern EndingMotionWork lbl_1_bss_1A5C[2];
extern EndingAudioState lbl_1_bss_1D5C;
extern float lbl_1_rodata_BC;
extern float lbl_1_rodata_C0;
extern float lbl_1_rodata_C4;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_D0;
extern float lbl_1_rodata_200;
extern float lbl_1_rodata_204;
extern float lbl_1_rodata_208;
extern float lbl_1_rodata_20C;
extern float lbl_1_rodata_210;
extern float lbl_1_rodata_214;
extern float lbl_1_rodata_218;

s32 HuAudFXPlayVolPan(s32 seId, s16 volume, s16 pan);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelRotSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void fn_1_2AEC(OMOBJ *object);

static inline int fn_1_44(int seId, s16 volume, s16 pan)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlayVolPan(seId, volume, pan);
    }
    return -1;
}

void fn_1_31FC(void)
{
    OMOBJ *object = lbl_1_bss_C;
    EndingMotionWork *first = &lbl_1_bss_1A5C[0];
    EndingMotionWork *second = &lbl_1_bss_1A5C[1];

    first->state = 1;
    first->time = lbl_1_rodata_C8;
    first->duration = lbl_1_rodata_200;
    first->unk_0C.x = lbl_1_rodata_204;
    first->unk_0C.y = lbl_1_rodata_C0;
    first->unk_0C.z = lbl_1_rodata_208;
    first->unk_18.x = lbl_1_rodata_20C;
    first->unk_18.y = lbl_1_rodata_D0;
    first->unk_18.z = lbl_1_rodata_20C;
    first->unk_24.x = lbl_1_rodata_BC;
    first->unk_24.y = lbl_1_rodata_C0;
    first->unk_24.z = lbl_1_rodata_C8;

    second->state = 1;
    second->time = lbl_1_rodata_C8;
    second->duration = lbl_1_rodata_200;
    second->unk_0C.x = lbl_1_rodata_C4;
    second->unk_0C.y = lbl_1_rodata_C0;
    second->unk_0C.z = lbl_1_rodata_208;
    second->unk_18.x = lbl_1_rodata_210;
    second->unk_18.y = lbl_1_rodata_D0;
    second->unk_18.z = lbl_1_rodata_20C;
    second->unk_24.x = lbl_1_rodata_D0;
    second->unk_24.y = lbl_1_rodata_C0;
    second->unk_24.z = lbl_1_rodata_C8;

    Hu3DModelPosSet(lbl_1_bss_C->mdlId[0], lbl_1_rodata_204,
        lbl_1_rodata_C0, lbl_1_rodata_208);
    Hu3DModelPosSet(lbl_1_bss_10->mdlId[0], lbl_1_rodata_C4,
        lbl_1_rodata_C0, lbl_1_rodata_208);
    Hu3DModelRotSet(lbl_1_bss_C->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_214, lbl_1_rodata_C8);
    Hu3DModelRotSet(lbl_1_bss_10->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_218, lbl_1_rodata_C8);
    Hu3DModelAttrReset(lbl_1_bss_C->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_10->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[1],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    lbl_1_bss_1D5C.channel[2] = fn_1_44(0x47E, 90, 64);
    lbl_1_bss_1D5C.channel[3] = fn_1_44(0x47C, 90, 64);
    object->objFunc = fn_1_2AEC;
}
