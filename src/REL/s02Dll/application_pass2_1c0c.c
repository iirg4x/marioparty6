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
extern HuVecF lbl_1_data_278[2];
extern HuVecF lbl_1_data_290[2];
extern float lbl_1_rodata_24;
extern float lbl_1_rodata_68;

void mbObjDispSet(s16 modelId, BOOL dispF);
void mbObjPosSetV(s16 modelId, const HuVecF *pos);
void mbObjRotSetV(s16 modelId, const HuVecF *rot);
void mbObjScaleSet(s16 modelId, float x, float y, float z);
void mbObjMotionTimeSet(s16 modelId, float time);
void mbObjMotionSpeedSet(s16 modelId, float speed);
void mbObjMotionStartEndSet(s16 modelId, s16 start, s16 end);

void fn_1_1C0C(void)
{
    s32 i;

    for (i = 0; i < 2; i++) {
        mbObjDispSet((s16)lbl_1_bss_1C.pairObjId[i][0], TRUE);
        mbObjMotionTimeSet((s16)lbl_1_bss_1C.pairObjId[i][0], lbl_1_rodata_24);
        mbObjMotionSpeedSet((s16)lbl_1_bss_1C.pairObjId[i][0], lbl_1_rodata_24);
        mbObjPosSetV((s16)lbl_1_bss_1C.pairObjId[i][0], &lbl_1_data_278[i]);
        mbObjRotSetV((s16)lbl_1_bss_1C.pairObjId[i][0], &lbl_1_data_290[i]);
        mbObjScaleSet((s16)lbl_1_bss_1C.pairObjId[i][0], lbl_1_rodata_68,
            lbl_1_rodata_68, lbl_1_rodata_68);
        mbObjMotionTimeSet((s16)lbl_1_bss_1C.pairObjId[i][3], lbl_1_rodata_24);
        mbObjMotionSpeedSet((s16)lbl_1_bss_1C.pairObjId[i][3], lbl_1_rodata_24);
        mbObjMotionStartEndSet((s16)lbl_1_bss_1C.pairObjId[i][3], 0, 150);
    }
}

void fn_1_1DC0(void)
{
}

void fn_1_1DC4(void)
{
}
