#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

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

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

extern OMOBJ *lbl_1_bss_4;
extern s16 lbl_1_bss_1A08[10];
extern s16 lbl_1_bss_1A1C[2];
extern ANIMDATA *lbl_1_bss_1A20[10];
extern float lbl_1_rodata_C8;
extern EndingSpritePositions lbl_1_rodata_11C;
extern float lbl_1_rodata_210;
extern float lbl_1_rodata_2C8;

void *HuDataSelHeapReadNum(s32 dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
s16 HuSprGrpCreate(s16 members);
s16 HuSprCreate(ANIMDATA *animation, s16 priority, s16 bank);
void HuSprGrpMemberSet(s16 group, s16 member, s16 sprite);
void HuSprPosSet(s16 group, s16 member, float x, float y);
void HuSprGrpPosSet(s16 group, float x, float y);
void HuSprExecLayerSet(s16 drawNo, s16 layer);
void HuSprGrpDrawNoSet(s16 group, s16 drawNo);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DCameraPosGet(s16 camera, Vec *position, Vec *up, Vec *target);
void Hu3DCameraPosSetV(s16 camera, Vec *position, Vec *up, Vec *target);
void fn_1_E0EC(s16 groupId, u32 attr);
void fn_1_EB54(s16 time);

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

void fn_1_C4F0(void)
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
