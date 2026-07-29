#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef struct omObj_s OMOBJ;

typedef struct S02Work {
    s32 modelId[3];
    s32 modelIdC;
    s32 eventModelId;
    s32 modelId14;
    s32 modelId18;
    s32 mapObjId[12];
    s32 modelId4C;
    s32 unk_50;
    s32 unk_54;
    s32 modelId58;
    s32 modelId5C;
    s32 effectModelId;
    s32 modelId64;
    s32 pairObjId[2][4];
    s32 modelId88;
    s32 modelId8C;
} S02Work;

extern s16 lbl_1_bss_0;
extern HuVecF lbl_1_bss_4;
extern S02Work lbl_1_bss_1C;
extern HuVecF lbl_1_data_88;
extern HuVecF lbl_1_data_94;
extern float lbl_1_rodata_1C;
extern float lbl_1_rodata_20;

u32 mbMasuMAttrGet(s16 id);
int mbObjModelIDGet(s16 modelId);
void mbObjPosGet(s16 modelId, HuVecF *pos);
void mbObjPosSetV(s16 modelId, const HuVecF *pos);
void Hu3DModelLightInfoSet(int modelId, BOOL lightInfoF);

void fn_1_4B8(int playerNo, s16 id);
void fn_1_1120(int playerNo, s16 id);
void fn_1_1C0C(void);

void fn_1_268(OMOBJ *obj)
{
}

void fn_1_26C(OMOBJ *obj)
{
    HuVecF pos;
    s32 i;

    for (i = 0; i < 12; i++) {
        mbObjPosGet((s16)lbl_1_bss_1C.mapObjId[i], &pos);
        pos.x += lbl_1_bss_4.x;
        pos.z += lbl_1_bss_4.z;
        if (pos.x >= lbl_1_rodata_1C + lbl_1_data_88.x) {
            pos = lbl_1_data_94;
            pos.x += lbl_1_rodata_1C;
            pos.z += lbl_1_rodata_20;
        }
        mbObjPosSetV((s16)lbl_1_bss_1C.mapObjId[i], &pos);
    }
}

int fn_1_390(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 0x10) {
        fn_1_4B8(playerNo, id);
    }
    return 0;
}

int fn_1_3EC(int playerNo, s16 id)
{
    return 0;
}

int fn_1_3F4(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 0x5) {
        fn_1_1120(playerNo, id);
    }
    return 0;
}

void fn_1_450(int playerNo)
{
}

void fn_1_454(int playerNo)
{
    fn_1_1C0C();
}

void fn_1_474(void)
{
    s16 *modelId = &lbl_1_bss_0;

    Hu3DModelLightInfoSet(mbObjModelIDGet(modelId[0]), TRUE);
}

void fn_1_4B0(void)
{
}

void fn_1_4B4(void)
{
}
