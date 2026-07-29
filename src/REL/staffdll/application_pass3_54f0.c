#include <dolphin/gx/GXStruct.h>
#include <dolphin/mtx/GeoTypes.h>
#include "game/gamework.h"

#define HU_MEMNUM_OVL 0x10000000
#define HUSPR_ATTR_DISPOFF 0x4
#define OM_GRP_NONE -1

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LIGHTID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
typedef s16 HUWINID;

typedef enum HeapID_s {
    HEAP_HEAP,
    HEAP_SOUND,
    HEAP_MODEL,
    HEAP_DVD,
    HEAP_SPACE,
    HEAP_MAX
} HEAPID;

typedef struct AnimData_s ANIMDATA;
typedef struct Process HUPROCESS;
typedef HUPROCESS OMOBJMAN;
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

typedef struct StaffLightVectors {
    HuVecF entries[2];
} STAFF_LIGHT_VECTORS;

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_40;
extern const f32 lbl_1_rodata_64;
extern const f32 lbl_1_rodata_80;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_B0;
extern const f32 lbl_1_rodata_B4;
extern const f32 lbl_1_rodata_B8;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C0;
extern const f32 lbl_1_rodata_C4;
extern const STAFF_LIGHT_VECTORS lbl_1_rodata_C8;
extern const STAFF_LIGHT_VECTORS lbl_1_rodata_E0;
extern const GXColor lbl_1_rodata_F8;
extern const f32 lbl_1_rodata_1C4;
extern const f32 lbl_1_rodata_1C8;
extern const f32 lbl_1_rodata_1CC;

extern char lbl_1_data_9C1[];

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern ANIMDATA *lbl_1_bss_20;
extern f32 lbl_1_bss_2C;
extern f32 lbl_1_bss_30;
extern s16 lbl_1_bss_304[];
extern HUSPRID lbl_1_bss_828;
extern HUSPR_GROUPID lbl_1_bss_82A;
extern HU3D_LIGHTID lbl_1_bss_82C[2];

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_LIGHTID Hu3DGLightCreateV(
    HuVecF *position, HuVecF *direction, GXColor *color);
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId);
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL isStatic);
void Hu3DCameraCreate(u32 cameraBit);
void Hu3DCameraPerspectiveSet(u32 cameraBit, float fov, float near,
    float far, float aspect);
void Hu3DCameraViewportSet(u32 cameraBit, float x, float y, float width,
    float height, float near, float far);
void Hu3DCameraPosSet(u32 cameraBit, float posX, float posY, float posZ,
    float upX, float upY, float upZ, float targetX, float targetY,
    float targetZ);
OMOBJMAN *omInitObjMan(s16 objMax, s32 objManPrio);
OMOBJ *omAddObjEx(OMOBJMAN *objMan, s16 prio, u16 mdlcnt, u16 mtncnt,
    s16 grpNo, OMOBJ_FUNC objFunc);
ANIMDATA *HuSprAnimRead(void *data);
HUSPRID HuSprCreate(ANIMDATA *anim, s16 prio, s16 bank);
HUSPR_GROUPID HuSprGrpCreate(s16 sprNum);
void HuSprGrpMemberSet(
    HUSPR_GROUPID grpId, s16 memberNo, HUSPRID sprId);
void HuSprPosSet(
    HUSPR_GROUPID grpId, s16 memberNo, float posX, float posY);
void HuSprTPLvlSet(HUSPR_GROUPID grpId, s16 memberNo, float tpLvl);
void HuSprAttrSet(HUSPR_GROUPID grpId, s16 memberNo, s32 attr);
void HuWinInit(s32 heapMode);
HUWINID HuWinExCreateFrame(float x, float y, s16 w, s16 h,
    s16 speakerNo, s16 frame);
void HuWinDispOff(HUWINID winId);
void HuWinBGTPLvlSet(HUWINID winId, float tpLvl);
HUPROCESS *HuPrcChildCreate(void (*func)(void), u16 prio, u32 stackSize,
    s32 extraSize, HUPROCESS *parent);

void fn_1_1684(OMOBJ *obj);
void fn_1_389C(OMOBJ *obj);
void fn_1_3A98(OMOBJ *obj);
void fn_1_3C38(OMOBJ *obj);
void fn_1_4AF4(OMOBJ *obj);
void fn_1_5148(OMOBJ *obj);
void fn_1_528C(void);
void fn_1_96A4(void);

static inline void fn_1_E20(void)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_B0, lbl_1_rodata_B4,
        lbl_1_rodata_B8, lbl_1_rodata_BC);
    Hu3DCameraViewportSet(1, lbl_1_rodata_10, lbl_1_rodata_10,
        lbl_1_rodata_C0, lbl_1_rodata_C4, lbl_1_rodata_10, lbl_1_rodata_40);
    Hu3DCameraPosSet(1, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_80, lbl_1_rodata_10, lbl_1_rodata_40,
        lbl_1_rodata_10, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_98);
}

static inline void fn_1_F68(void)
{
    STAFF_LIGHT_VECTORS positions = lbl_1_rodata_C8;
    STAFF_LIGHT_VECTORS directions = lbl_1_rodata_E0;
    GXColor color = lbl_1_rodata_F8;
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_82C[i] = Hu3DGLightCreateV(
            &positions.entries[i], &directions.entries[i], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_82C[i]);
        Hu3DGLightStaticSet(lbl_1_bss_82C[i], TRUE);
    }
}

void fn_1_54F0(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 0x2000);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 2, OM_GRP_NONE, fn_1_389C);
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(0xCD002B, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, lbl_1_rodata_1C4, lbl_1_rodata_1C8);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 0x1000,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(lbl_1_rodata_1CC,
            lbl_1_rodata_1C8, 0x220, 0x2A, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], lbl_1_rodata_10);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 0x1000, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 0x3000, 0x3000, 0, lbl_1_bss_0);
}

void fn_1_5A7C(void)
{
    OSReport(lbl_1_data_9C1);
    lbl_1_bss_0 = omInitObjMan(11, 0x2000);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 2, OM_GRP_NONE, fn_1_389C);
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 0x1000, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 0x1000, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(0xCD002B, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, lbl_1_rodata_1C4, lbl_1_rodata_1C8);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 0x1000,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(lbl_1_rodata_1CC,
            lbl_1_rodata_1C8, 0x220, 0x2A, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], lbl_1_rodata_10);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 0x1000, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 0x3000, 0x3000, 0, lbl_1_bss_0);
}
