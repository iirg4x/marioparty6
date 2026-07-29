#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

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

extern S02Work lbl_1_bss_1C;
extern s32 lbl_1_data_C8[8];
extern HuVecF lbl_1_data_E8[12];
extern HuVecF lbl_1_data_178[12];
extern HuVecF lbl_1_data_208[2];
extern HuVecF lbl_1_data_220[3];
extern char lbl_1_data_20[2][0x10];
extern char lbl_1_data_244[8];
extern HuVecF lbl_1_bss_4;
extern float lbl_1_rodata_1C;
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_24;
extern float lbl_1_rodata_68;
extern float lbl_1_rodata_6C;

int mbObjCreate(int dataNum, const int *motDataNum, BOOL linkF);
void mbObjAttrSet(s16 modelId, u32 attr);
void mbObjPosSet(s16 modelId, float x, float y, float z);
void mbObjPosSetV(s16 modelId, const HuVecF *pos);
void mbObjRotSetV(s16 modelId, const HuVecF *rot);
void mbObjDispSet(s16 modelId, BOOL dispF);
void mbObjMotionTimeSet(s16 modelId, float time);
void mbObjMotionSpeedSet(s16 modelId, float speed);
void mbObjHookSet(s16 modelId, char *objName, s16 hookModelId);
int mbBoardDataNumGet(int dataNum);

void fn_1_928(void)
{
    HuVecF pos;
    s32 modelId;
    s32 i;

    for (i = 0; i < 3; i++) {
        modelId = (s16)mbObjCreate(0xC80003 + i, NULL, FALSE);
        lbl_1_bss_1C.modelId[i] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 0x40000001);
    }

    modelId = (s16)mbObjCreate(0xC8000B, NULL, FALSE);
    lbl_1_bss_1C.modelIdC = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);

    modelId = (s16)mbObjCreate(0xC8000C, NULL, FALSE);
    lbl_1_bss_1C.eventModelId = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);

    modelId = (s16)mbObjCreate(0xC8000D, NULL, FALSE);
    lbl_1_bss_1C.modelId14 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);

    modelId = (s16)mbObjCreate(0xC8000E, NULL, FALSE);
    lbl_1_bss_1C.modelId5C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);

    modelId = (s16)mbObjCreate(0xC8000F, NULL, FALSE);
    lbl_1_bss_1C.effectModelId = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
    mbObjHookSet((s16)lbl_1_bss_1C.modelId5C, lbl_1_data_244,
        (s16)lbl_1_bss_1C.effectModelId);

    modelId = (s16)mbObjCreate(0xC80010, NULL, FALSE);
    lbl_1_bss_1C.modelId64 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
    mbObjAttrSet(modelId, 0x40000001);

    modelId = (s16)mbObjCreate(0xC80011, NULL, FALSE);
    lbl_1_bss_1C.modelId18 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);
    mbObjDispSet(modelId, FALSE);

    for (i = 0; i < 12; i++) {
        modelId = (s16)mbObjCreate(0xC80012, NULL, TRUE);
        lbl_1_bss_1C.mapObjId[i] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 0x40000001);
        pos = lbl_1_data_E8[i];
        pos.x += lbl_1_rodata_1C;
        pos.z += lbl_1_rodata_20;
        mbObjPosSetV(modelId, &pos);
        mbObjRotSetV(modelId, &lbl_1_data_178[i]);
    }

    lbl_1_bss_4.x = (lbl_1_rodata_1C
        + ((lbl_1_rodata_1C + lbl_1_data_E8[0].x) - lbl_1_data_E8[11].x))
        / lbl_1_rodata_6C;
    lbl_1_bss_4.z = (lbl_1_rodata_20
        + ((lbl_1_rodata_20 + lbl_1_data_E8[0].z) - lbl_1_data_E8[11].z))
        / lbl_1_rodata_6C;

    modelId = (s16)mbObjCreate(0xC80013, NULL, FALSE);
    lbl_1_bss_1C.modelId4C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);

    modelId = (s16)mbObjCreate(0xC80015, NULL, FALSE);
    lbl_1_bss_1C.modelId58 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);

    for (i = 0; i < 2; i++) {
        modelId = (s16)mbObjCreate(0xC80016 + i, NULL, FALSE);
        lbl_1_bss_1C.pairObjId[i][0] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
        mbObjPosSetV(modelId, &lbl_1_data_208[i]);
        mbObjRotSetV(modelId, &lbl_1_data_220[i]);

        modelId = (s16)mbObjCreate(lbl_1_data_C8[i], NULL, FALSE);
        lbl_1_bss_1C.pairObjId[i][2] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 0x40000001);

        modelId = (s16)mbObjCreate(0xC8000A, NULL, TRUE);
        lbl_1_bss_1C.pairObjId[i][3] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
        mbObjHookSet((s16)lbl_1_bss_1C.pairObjId[i][0], lbl_1_data_20[i],
            (s16)lbl_1_bss_1C.pairObjId[i][3]);
    }

    modelId = (s16)mbObjCreate(mbBoardDataNumGet(0x5005B), NULL, FALSE);
    lbl_1_bss_1C.modelId88 = modelId;
    mbObjPosSet(modelId, lbl_1_rodata_24, lbl_1_rodata_24, lbl_1_rodata_24);
    mbObjDispSet(modelId, FALSE);

    modelId = (s16)mbObjCreate(0xC80006, NULL, FALSE);
    lbl_1_bss_1C.modelId8C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 0x40000001);
}
