#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

#define HUSPR_ATTR_DISPOFF 0x4

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef struct AnimData_s ANIMDATA;
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

typedef struct EndingAudioState {
    s32 channel[32];
} EndingAudioState;

typedef struct EndingScenePositions {
    HuVecF position[3];
} EndingScenePositions;

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

typedef enum EndingWipeType {
    WIPE_TYPE_NORMAL,
} EndingWipeType;

typedef enum EndingWipeMode {
    WIPE_MODE_DUMMY,
    WIPE_MODE_IN,
    WIPE_MODE_OUT,
} EndingWipeMode;

extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_14;
extern s16 lbl_1_bss_24;
extern s16 lbl_1_bss_26;
extern float lbl_1_bss_28;
extern s16 lbl_1_bss_2C;
extern s16 lbl_1_bss_1A08[10];
extern s16 lbl_1_bss_1A1C[2];
extern ANIMDATA *lbl_1_bss_1A20[10];
extern EndingAudioState lbl_1_bss_1D5C;

extern float lbl_1_rodata_C8;
extern EndingSpritePositions lbl_1_rodata_11C;
extern float lbl_1_rodata_118;
extern float lbl_1_rodata_1A0;
extern float lbl_1_rodata_200;
extern float lbl_1_rodata_210;
extern float lbl_1_rodata_2C8;
extern EndingScenePositions lbl_1_rodata_2CC;
extern float lbl_1_rodata_2F0;

s32 HuAudFXPlay(s32 soundId);
void HuAudFXStop(s32 channel);
void WipeCreate(s16 mode, s16 type, s16 time);
u8 WipeCheck(void);
void HuPrcSleep(s32 time);
void HuPrcVSleep(void);
void *HuDataSelHeapReadNum(s32 dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
s16 HuSprGrpCreate(s16 members);
s16 HuSprCreate(ANIMDATA *animation, s16 priority, s16 bank);
void HuSprGrpMemberSet(s16 group, s16 member, s16 sprite);
void HuSprPosSet(s16 group, s16 member, float x, float y);
void HuSprGrpPosSet(s16 group, float x, float y);
void HuSprExecLayerSet(s16 drawNo, s16 layer);
void HuSprGrpDrawNoSet(s16 group, s16 drawNo);
void HuSprAttrReset(s16 group, s16 member, u32 attr);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DCameraPosGet(s16 camera, Vec *position, Vec *up, Vec *target);
void Hu3DCameraPosSetV(s16 camera, Vec *position, Vec *up, Vec *target);
void fn_1_1160(OMOBJ *object);
void fn_1_E0EC(s16 groupId, u32 attr);
void fn_1_EAB8(s16 display, HuVecF *position);
void fn_1_EB54(s16 time);
void fn_1_F068(s16 index, s16 display, HuVecF *position);
void fn_1_F1B8(s16 display, HuVecF *position);
void fn_1_F23C(s16 count);

static inline int fn_1_0(int soundId)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlay(soundId);
    }
    return -1;
}

static inline void fn_1_12A8(s16 state)
{
    if (state == 2) {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    } else {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    }
    fn_1_E0EC(lbl_1_bss_1A1C[0], HUSPR_ATTR_DISPOFF);
    lbl_1_bss_4->work[0] = state;
    lbl_1_bss_4->objFunc = fn_1_1160;

    switch (state) {
        case 0:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 4, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 6, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 3, HUSPR_ATTR_DISPOFF);
            break;
        case 1:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 1, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 5, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 8, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 9, HUSPR_ATTR_DISPOFF);
            break;
        case 2:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 0, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 2, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 7, HUSPR_ATTR_DISPOFF);
            break;
        case 3:
            lbl_1_bss_4->objFunc = NULL;
            break;
    }
}

static inline void fn_1_14F4(void)
{
    EndingSpritePositions positions = lbl_1_rodata_11C;
    s16 index;

    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A20[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            0x220014 + index, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A1C[0] = HuSprGrpCreate(10);
    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A08[index] = HuSprCreate(lbl_1_bss_1A20[index],
            positions.position[index].z, 0);
        HuSprGrpMemberSet(lbl_1_bss_1A1C[0], index,
            lbl_1_bss_1A08[index]);
        HuSprPosSet(lbl_1_bss_1A1C[0], index,
            positions.position[index].x, positions.position[index].y);
    }
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
        lbl_1_rodata_C8);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    HuSprExecLayerSet(0x40, 2);
    HuSprGrpDrawNoSet(lbl_1_bss_1A1C[0], 0x40);
}

static inline void fn_1_C40C(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 3]);
        Hu3DModelKill(object->mdlId[model + 3]);
    }
    fn_1_F1B8(0, 0);
    fn_1_F068(1, 0, NULL);
    object = lbl_1_bss_14;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 2]);
    }
    Hu3DModelKill(object->mdlId[2]);
}

static inline void fn_1_C4F0(void)
{
    OMOBJ *object;
    Vec position;
    Vec up;
    Vec target;

    fn_1_14F4();
    object = lbl_1_bss_4;
    object->mdlId[7] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x220007, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[7] = Hu3DMotionIDGet(object->mdlId[7]);
    Hu3DMotionShiftSet(object->mdlId[7], object->mtnId[7],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0x40000001);

    Hu3DCameraPosGet(1, &position, &up, &target);
    position.x = lbl_1_rodata_C8;
    position.y = lbl_1_rodata_210;
    position.z = lbl_1_rodata_2C8;
    target.x = lbl_1_rodata_C8;
    target.y = lbl_1_rodata_210;
    target.z = lbl_1_rodata_C8;
    Hu3DCameraPosSetV(1, &position, &up, &target);
    fn_1_EB54(1);
}

void fn_1_C7DC(void)
{
    float particleY = lbl_1_rodata_2F0;
    float phaseDuration;
    EndingScenePositions positions = lbl_1_rodata_2CC;

    HuAudFXStop(lbl_1_bss_1D5C.channel[6]);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_C40C();
    fn_1_C4F0();

    fn_1_12A8(0);
    fn_1_EAB8(1, &positions.position[0]);
    positions.position[0].y = particleY;
    fn_1_F1B8(1, &positions.position[0]);
    fn_1_F23C(1);
    HuPrcSleep(60);
    lbl_1_bss_1D5C.channel[10] = fn_1_0(0x57A);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    phaseDuration = lbl_1_rodata_1A0;
    HuPrcSleep(60);

    lbl_1_bss_1D5C.channel[11] = fn_1_0(0x57B);
    fn_1_12A8(1);
    fn_1_EAB8(1, &positions.position[1]);
    positions.position[1].y = particleY;
    fn_1_F1B8(1, &positions.position[1]);
    phaseDuration = lbl_1_rodata_1A0;
    HuPrcSleep(48);

    lbl_1_bss_1D5C.channel[7] = fn_1_0(0x57C);
    fn_1_12A8(2);
    fn_1_EAB8(1, &positions.position[2]);
    positions.position[2].y = particleY;
    fn_1_F1B8(1, &positions.position[2]);
    phaseDuration = lbl_1_rodata_200;
    HuPrcSleep(132);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}
