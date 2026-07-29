#include "dolphin/mtx/GeoTypes.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include <string.h>

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_ANIMID;
typedef struct AnimData_s ANIMDATA;
typedef struct Process_s HUPROCESS;
typedef HUPROCESS OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

#define HU3D_ATTR_DISPOFF (1 << 0)

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

typedef struct Lbl1BssE74Entry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 field_A;
    Vec pos;
    Vec rot;
    Vec scale;
    s16 field_30;
    u8 field_32[6];
} LBL_1_BSS_E74_ENTRY;

typedef struct BitmapNameTable {
    char *name[4];
} BITMAP_NAME_TABLE;

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_24;
extern LBL_1_BSS_E74_ENTRY lbl_1_bss_E74[16];
extern ANIMDATA *lbl_1_bss_11F4[25];
extern s32 lbl_1_data_744[25];
extern char lbl_1_data_AD4[];
extern char lbl_1_data_ADC[];
extern char lbl_1_data_AE4[];
extern char lbl_1_data_AEC[];

const BITMAP_NAME_TABLE lbl_1_rodata_270 = {
    {
        lbl_1_data_AD4,
        lbl_1_data_ADC,
        lbl_1_data_AE4,
        lbl_1_data_AEC,
    },
};

void HuPrcVSleep(void);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MODELID Hu3DModelLink(HU3D_MODELID linkMdlId);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layerNo);
HU3D_ANIMID Hu3DAnimCreate(
    void *dataP, HU3D_MODELID modelId, char *bmpName);
void Hu3DAnimKill(HU3D_ANIMID animId);
OMOBJ *omAddObjEx(OMOBJMAN *objMan, s16 prio, u16 mdlcnt,
    u16 mtncnt, s16 grpNo, OMOBJ_FUNC objFunc);

void fn_1_B898(s16 arg0)
{
    lbl_1_bss_24->mtnId[0] = arg0;
}

s16 fn_1_B8B0(void)
{
    return lbl_1_bss_24->mtnId[0];
}

s16 fn_1_B8C8(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

void fn_1_B900(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

void fn_1_B998(void)
{
    BITMAP_NAME_TABLE bitmapName = lbl_1_rodata_270;
    s16 i;
    s16 j;

    for (i = 0; i < 25; i++) {
        lbl_1_bss_11F4[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_744[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 16; i++) {
        memset(&lbl_1_bss_E74[i], 0, sizeof(LBL_1_BSS_E74_ENTRY));
        if (i == 0) {
            lbl_1_bss_E74[i].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdsing, 0x15), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            lbl_1_bss_E74[i].modelId =
                Hu3DModelLink(lbl_1_bss_E74[0].modelId);
        }
        for (j = 0; j < 4; j++) {
            lbl_1_bss_E74[i].animId[j] = Hu3DAnimCreate(
                lbl_1_bss_11F4[0], lbl_1_bss_E74[i].modelId,
                bitmapName.name[j]);
        }
        Hu3DModelLayerSet(lbl_1_bss_E74[i].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_E74[i].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, NULL);
}

void fn_1_BBC8(void)
{
    s16 i;
    s16 j;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 15; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_E74[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_E74[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}
