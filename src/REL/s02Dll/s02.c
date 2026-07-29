#define Hu3DModelLightInfoSet Hu3DModelLightInfoSet_Header

#include "dolphin.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/board/masu.h"

#undef Hu3DModelLightInfoSet

typedef void (*VoidFunc)(void);

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

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s16 lbl_1_bss_0;
extern HuVecF lbl_1_bss_4;
extern S02Work lbl_1_bss_1C;
extern HuVecF lbl_1_data_88;
extern HuVecF lbl_1_data_94;
extern HuVecF lbl_1_data_278[2];
extern HuVecF lbl_1_data_290[2];
extern char lbl_1_data_20[2][16];
extern s32 lbl_1_data_C8[8];
extern HuVecF lbl_1_data_E8[12];
extern HuVecF lbl_1_data_178[12];
extern HuVecF lbl_1_data_208[2];
extern HuVecF lbl_1_data_220[3];
extern char lbl_1_data_244[8];
extern float lbl_1_rodata_1C;
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_24;
extern float lbl_1_rodata_68;
extern float lbl_1_rodata_6C;

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_268(OMOBJ *obj);
void fn_1_26C(OMOBJ *obj);
int fn_1_390(int playerNo, s16 id);
int fn_1_3EC(int playerNo, s16 id);
int fn_1_3F4(int playerNo, s16 id);
void fn_1_450(int playerNo);
void fn_1_454(int playerNo);
void fn_1_474(void);
void fn_1_4B0(void);
void fn_1_4B4(void);
void fn_1_4B8(int playerNo, s16 id);
void fn_1_928(void);
void fn_1_1120(int playerNo, s16 id);
void fn_1_1C0C(void);
void fn_1_1DC0(void);
void fn_1_1DC4(void);
void fn_1_22C8(void);
void fn_1_22CC(void);

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));
int mbObjCreate(int dataNum, const int *motDataNum, BOOL linkF);
void mbObjAttrSet(s16 modelId, u32 attr);
int mbObjModelIDGet(s16 modelId);
void Hu3DModelLightInfoSet(int modelId, BOOL lightInfoF);
void mbObjPosGet(s16 modelId, HuVecF *pos);
void mbObjPosSet(s16 modelId, float x, float y, float z);
void mbObjPosSetV(s16 modelId, const HuVecF *pos);
void mbObjRotSetV(s16 modelId, const HuVecF *rot);
void mbObjScaleSet(s16 modelId, float x, float y, float z);
void mbObjDispSet(s16 modelId, BOOL dispF);
void mbObjMotionTimeSet(s16 modelId, float time);
void mbObjMotionSpeedSet(s16 modelId, float speed);
void mbObjMotionStartEndSet(s16 modelId, s16 start, s16 end);
void mbObjHookSet(s16 modelId, char *objName, s16 hookModelId);
int mbBoardDataNumGet(int dataNum);

int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void fn_1_A0(void)
{
    GwSystem.partyF = FALSE;
    mbObjectSetup(7, fn_1_F4, fn_1_268);
}

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

    if (attr & 16) {
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

    if (attr & 5) {
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

void fn_1_928(void)
{
    HuVecF pos;
    s32 modelId;
    s32 i;

    for (i = 0; i < 3; i++) {
        modelId = (s16)mbObjCreate(13107203 + i, NULL, FALSE);
        lbl_1_bss_1C.modelId[i] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 1073741825);
    }

    modelId = (s16)mbObjCreate(13107211, NULL, FALSE);
    lbl_1_bss_1C.modelIdC = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107212, NULL, FALSE);
    lbl_1_bss_1C.eventModelId = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);

    modelId = (s16)mbObjCreate(13107213, NULL, FALSE);
    lbl_1_bss_1C.modelId14 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107214, NULL, FALSE);
    lbl_1_bss_1C.modelId5C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107215, NULL, FALSE);
    lbl_1_bss_1C.effectModelId = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
    mbObjHookSet((s16)lbl_1_bss_1C.modelId5C, lbl_1_data_244,
        (s16)lbl_1_bss_1C.effectModelId);

    modelId = (s16)mbObjCreate(13107216, NULL, FALSE);
    lbl_1_bss_1C.modelId64 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107217, NULL, FALSE);
    lbl_1_bss_1C.modelId18 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);
    mbObjDispSet(modelId, FALSE);

    for (i = 0; i < 12; i++) {
        modelId = (s16)mbObjCreate(13107218, NULL, TRUE);
        lbl_1_bss_1C.mapObjId[i] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 1073741825);
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

    modelId = (s16)mbObjCreate(13107219, NULL, FALSE);
    lbl_1_bss_1C.modelId4C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107221, NULL, FALSE);
    lbl_1_bss_1C.modelId58 = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);

    for (i = 0; i < 2; i++) {
        modelId = (s16)mbObjCreate(13107222 + i, NULL, FALSE);
        lbl_1_bss_1C.pairObjId[i][0] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
        mbObjPosSetV(modelId, &lbl_1_data_208[i]);
        mbObjRotSetV(modelId, &lbl_1_data_220[i]);

        modelId = (s16)mbObjCreate(lbl_1_data_C8[i], NULL, FALSE);
        lbl_1_bss_1C.pairObjId[i][2] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
        mbObjAttrSet(modelId, 1073741825);

        modelId = (s16)mbObjCreate(13107210, NULL, TRUE);
        lbl_1_bss_1C.pairObjId[i][3] = modelId;
        mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_24);
        mbObjHookSet((s16)lbl_1_bss_1C.pairObjId[i][0], lbl_1_data_20[i],
            (s16)lbl_1_bss_1C.pairObjId[i][3]);
    }

    modelId = (s16)mbObjCreate(mbBoardDataNumGet(327771), NULL, FALSE);
    lbl_1_bss_1C.modelId88 = modelId;
    mbObjPosSet(modelId, lbl_1_rodata_24, lbl_1_rodata_24, lbl_1_rodata_24);
    mbObjDispSet(modelId, FALSE);

    modelId = (s16)mbObjCreate(13107206, NULL, FALSE);
    lbl_1_bss_1C.modelId8C = modelId;
    mbObjMotionTimeSet(modelId, lbl_1_rodata_24);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_68);
    mbObjAttrSet(modelId, 1073741825);
}

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

void fn_1_22C8(void)
{
}

void fn_1_22CC(void)
{
}
