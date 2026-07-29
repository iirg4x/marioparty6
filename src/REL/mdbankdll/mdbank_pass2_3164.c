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

extern OMOBJ *lbl_1_bss_14;
extern HuVecF lbl_1_bss_1710;
extern const float lbl_1_rodata_68;
extern const float lbl_1_rodata_74;
extern const float lbl_1_rodata_15C;
extern const float lbl_1_rodata_160;

void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelRotSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);

static inline void fn_1_3100(HuVecF *pos)
{
    lbl_1_bss_1710.x = pos->x - lbl_1_rodata_15C;
    lbl_1_bss_1710.y = lbl_1_rodata_15C + pos->y;
    lbl_1_bss_1710.z = lbl_1_rodata_15C + pos->z;
}

void fn_1_3164(HuVecF *pos)
{
    OMOBJ *obj = lbl_1_bss_14;

    Hu3DModelPosSetV(obj->mdlId[0], pos);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_160);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_74, lbl_1_rodata_74,
        lbl_1_rodata_74);
    fn_1_3100(pos);
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}
