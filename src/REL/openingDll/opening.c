#include "dolphin.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

#define OPENING_CHAR_COUNT 10
#define OPENING_WIN_COUNT 4

typedef struct OpeningEventWork {
    void (*callback)(s32, s32);
    s32 arg0;
    s32 arg1;
} OpeningEventWork;

typedef struct OpeningFocusObject {
    u32 unk_00;
    u32 unk_04;
    u32 unk_08;
    u32 unk_0C;
    u32 unk_10;
    u32 unk_14;
    u32 unk_18;
    u32 unk_1C;
    u32 unk_20;
    u32 unk_24;
    u32 unk_28;
    u32 unk_2C;
    u32 unk_30;
    float focus34;
    float focus38;
} OpeningFocusObject;

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern s16 lbl_1_bss_0;
extern s32 lbl_1_bss_4;
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern float lbl_1_bss_10;
extern HuVecF lbl_1_bss_14;
extern float lbl_1_bss_20;
extern HuVec2f lbl_1_bss_24;
extern s16 lbl_1_bss_2C;
extern float lbl_1_bss_38;
extern HuVecF lbl_1_bss_3C;
extern HuVecF lbl_1_bss_48;
extern HuVecF lbl_1_bss_54;
extern HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
extern HU3D_ANIMID lbl_1_bss_60[12];
extern HU3D_ANIMID lbl_1_bss_78;
extern ANIMDATA *lbl_1_bss_7C[79];
extern ANIMDATA *lbl_1_bss_1B8[2];
extern void *lbl_1_bss_1C0;
extern void *lbl_1_bss_1C4[2];
extern s16 lbl_1_bss_1CC;
extern HU3D_MODELID lbl_1_bss_1CE[8];
extern s16 lbl_1_bss_1DE[3];
extern OMOBJMAN *lbl_1_bss_1E4;
extern s16 lbl_1_data_0[OPENING_WIN_COUNT];
extern u32 lbl_1_data_8[80];
extern char *lbl_1_data_1DC[12];
extern char *lbl_1_data_2F0[12];
extern HuVecF lbl_1_data_320;
extern HuVecF lbl_1_data_32C;
extern char lbl_1_data_338[14];
extern char lbl_1_data_346[15];
extern char lbl_1_data_355[20];
extern char lbl_1_data_369[39];

int _prolog(void);
void _epilog(void);

#pragma push
#pragma section code_type ".text.object_setup"
void fn_1_A0(void);
#pragma pop

#pragma push
#pragma section code_type ".text.after_setup"
void fn_1_28C(void);
void fn_1_320(void);
void fn_1_A00(s32 enablePrompt);
void fn_1_9A4(void);
void fn_1_1128(void);
void fn_1_1828(s16 layerNo);
void fn_1_192C(s16 layerNo);
void fn_1_1B7C(void);
void fn_1_1BD8(void);
void fn_1_2104(s32 modelIndex, s32 distance);
void fn_1_2238(u32 frameCount);
void fn_1_2304(u32 guideChar, s32 frameCount);
void fn_1_257C(u32 frameCount);
void fn_1_2708(u32 frameCount);
void fn_1_2B9C(u32 frameCount);
void fn_1_2D74(u32 frameCount);
void fn_1_34EC(u32 frameCount);
void fn_1_37F0(void);
void fn_1_392C(u32 frameCount);
void fn_1_3C28(void);
void fn_1_4068(u32 frameCount);
void fn_1_41B4(void);
void fn_1_420C(void);
void fn_1_4264(u32 frameCount);
void fn_1_4484(u32 frameCount);
void fn_1_45D4(s16 modelIndex, float distance);
void fn_1_46B4(s16 animIndex, s16 frameIndex);
void fn_1_470C(s16 animIndex, s16 frameIndex);
void fn_1_4744(s32 animIndex, s32 frameIndex);
void fn_1_47AC(s16 winIndex, u32 message, s16 frameCount);
void fn_1_48AC(void);
void fn_1_4BD8(void);
void fn_1_4C30(void);
void fn_1_4DB0(void);
void fn_1_4E00(void);
void fn_1_4E34(void (*callback)(void), s32 arg0, s32 arg1);
void fn_1_4ECC(HuVecF *position, HuVecF *target, float roll);
void fn_1_517C(float alpha);
#pragma pop

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

void fn_1_28C(void)
{
    fn_1_320();
    fn_1_A00(GwCommon.viewOpening);
    GwCommon.viewOpening = 1;

    Hu3DAnimKill((&lbl_1_bss_78)[0]);
    Hu3DAnimKill((&lbl_1_bss_78)[1]);
    HuSprAnimKill(lbl_1_bss_1B8[0]);
    HuSprAnimKill(lbl_1_bss_1B8[1]);

    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_320(void)
{
    void *temp;
    s16 i;

    lbl_1_bss_1CE[0] = Hu3DModelCreate(HuDataSelHeapReadNum(13959190, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[0], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[0], 0.015f);
    Hu3DModelPosSet(lbl_1_bss_1CE[0], 0.0f, 0.0f, -4500.f);

    lbl_1_bss_1CE[1] = Hu3DModelCreate(HuDataSelHeapReadNum(13959191, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[1], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[1], 0.5f);
    Hu3DMotionClusterSpeedSet(lbl_1_bss_1CE[1], 0, 0.5f);
    Hu3DModelPosSet(lbl_1_bss_1CE[1], 0.0f, 0.0f, -2000.f);
    Hu3DModelRotSet(lbl_1_bss_1CE[1], -20.f, -30.f, 30.f);

    lbl_1_bss_1CE[2] = Hu3DModelCreate(HuDataSelHeapReadNum(13959192, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[2], 0);
    Hu3DModelPosSet(lbl_1_bss_1CE[2], 0.0f, -330.f, 0.0f);

    lbl_1_bss_1CE[3] = Hu3DModelCreate(HuDataSelHeapReadNum(13959193, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[3], 1);

    lbl_1_bss_1CE[4] = Hu3DModelCreate(HuDataSelHeapReadNum(13959194, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[4], 1);

    lbl_1_bss_1CE[5] = Hu3DModelCreate(HuDataSelHeapReadNum(13959196, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[5], 1);

    lbl_1_bss_1CE[6] = Hu3DModelCreate(HuDataSelHeapReadNum(13959197, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[6], 1);
    Hu3DModelAttrSet(lbl_1_bss_1CE[6], 1073741825);

    lbl_1_bss_1CE[7] = Hu3DModelCreate(HuDataSelHeapReadNum(13959195, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[7], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[7], 2.0f);
    Hu3DModelScaleSet(lbl_1_bss_1CE[7], 2.0f, 2.0f, 2.0f);

    temp = HuMemDirectMallocNum(HEAP_DVD, 1048576, HU_MEMNUM_OVL);
    lbl_1_bss_1C4[0] = HuMemDirectMallocNum(
        HEAP_DVD,
        GXGetTexBufferSize(640, 320, GX_TF_RGBA8, FALSE, 0),
        HU_MEMNUM_OVL);
    HuMemDirectFree(temp);
    lbl_1_bss_1C4[1] = HuMemDirectMallocNum(
        HEAP_HEAP,
        GXGetTexBufferSize(640, 320, GX_TF_RGBA8, FALSE, 0),
        HU_MEMNUM_OVL);

    lbl_1_bss_1B8[0] = HuSprAnimMake(640, 320, 0);
    lbl_1_bss_1B8[0]->bmp->data = lbl_1_bss_1C4[0];
    lbl_1_bss_1B8[1] = HuSprAnimMake(640, 320, 0);
    lbl_1_bss_1B8[1]->bmp->data = lbl_1_bss_1C4[1];
    (&lbl_1_bss_78)[0] = Hu3DAnimCreate(lbl_1_bss_1B8[0], lbl_1_bss_1CE[1], lbl_1_data_338);
    (&lbl_1_bss_78)[1] = Hu3DAnimCreate(lbl_1_bss_1B8[1], lbl_1_bss_1CE[1], lbl_1_data_346);

    lbl_1_bss_1C0 = HuMemDirectMallocNum(
        HEAP_HEAP,
        GXGetTexBufferSize(640, 320, GX_TF_I8, FALSE, 0),
        HU_MEMNUM_OVL);

    for (i = 0; i < 79; i++) {
        lbl_1_bss_7C[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_8[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }

    for (i = 0; i < 12; i++) {
        lbl_1_bss_60[i] = Hu3DAnimCreate(lbl_1_bss_7C[0], lbl_1_bss_1CE[2], lbl_1_data_1DC[i]);
    }

    HuPrcChildCreate(fn_1_4C30, 256, 4096, 0, HuPrcCurrentGet());

    lbl_1_bss_48.x = lbl_1_bss_48.y = lbl_1_bss_48.z = 0.0f;
    lbl_1_bss_54 = lbl_1_data_320;
    lbl_1_bss_3C = lbl_1_data_32C;
    lbl_1_bss_38 = 1.0f;

    fn_1_9A4();
}

void fn_1_9A4(void)
{
    s16 i;

    for (i = 0; i < 8; i++) {
        Hu3DModelAttrSet(lbl_1_bss_1CE[i], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_A00(s32 enablePrompt)
{
    HUPROCESS *eventProcess;
    HUWINID messageWinId;
    Mtx matrix;
    u32 frame;
    s16 i;
    s16 j;

    Hu3DModelAttrSet(lbl_1_bss_1CE[3], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1CE[4], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1CE[5], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1CE[6], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1CE[7], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[2], HU3D_ATTR_DISPOFF);

    Hu3DCameraLayerHookSet(1, 0, fn_1_1828);
    Hu3DCameraLayerHookSet(1, 8, fn_1_192C);
    Hu3DModelRotSet(lbl_1_bss_1CE[5], 0.0f, 0.0f, 0.0f);

    lbl_1_bss_0 = 0;
    lbl_1_bss_8 = 0;
    lbl_1_bss_C = 0;
    Hu3DAmbColorSet(1.0f, 1.0f, 1.0f);
    Hu3DShineSet(FALSE);

    lbl_1_bss_48.x = lbl_1_bss_48.y = lbl_1_bss_48.z = 0.0f;
    lbl_1_bss_54 = lbl_1_data_320;
    lbl_1_bss_3C = lbl_1_data_32C;
    lbl_1_bss_38 = 1.0f;

    Hu3DMotionTimeSet(lbl_1_bss_1CE[1], 0.0f);
    Hu3DModelAttrSet(lbl_1_bss_1CE[1], 1073741826);
    Hu3DMotionClusterTimeSet(lbl_1_bss_1CE[1], 0, 0.0f);
    Hu3DModelClusterAttrSet(lbl_1_bss_1CE[1], 0, 3221225474);

    PSMTXTrans(matrix, 150.f, 0.0f, 0.0f);
    Hu3DModelMtxSet(lbl_1_bss_1CE[1], &matrix);

    fn_1_46B4(0, 0);
    fn_1_46B4(1, 7);
    fn_1_46B4(2, 14);
    fn_1_46B4(3, 21);
    fn_1_46B4(4, 28);
    fn_1_46B4(5, 35);
    fn_1_46B4(6, 42);
    fn_1_46B4(7, 49);
    fn_1_46B4(8, 56);
    fn_1_46B4(9, 63);
    fn_1_46B4(10, 70);
    fn_1_46B4(11, 74);

    for (i = 0; i < 4; i++) {
        lbl_1_bss_2E[i] = HuWinExCreateFrame(-10000.f, 364.f, 544, 68, -1, lbl_1_data_0[i]);
        HuWinAttrSet(lbl_1_bss_2E[i], 2048);
    }

    lbl_1_bss_24.x = -225.0f;
    lbl_1_bss_24.y = -135.0f;
    fn_1_45D4(10, lbl_1_bss_24.x);
    fn_1_45D4(11, lbl_1_bss_24.y);

    messageWinId = HuWinCreate(16.f, 326.f, 544, 42, 0);
    HuWinAttrSet(messageWinId, 2048);
    HuWinPriSet(messageWinId, 100);
    HuWinBGTPLvlSet(messageWinId, 0.0f);
    HuWinMesSpeedSet(messageWinId, 0);
    HuWinMesSet(messageWinId, 65541);
    HuWinDispOff(messageWinId);

    Hu3DGLightPosSet(lbl_1_bss_1DE[0], 300.f, 1000.f, 1000.f,
                     -0.3f, -1.0f, -1.0f);
    Hu3DGLightColorSet(lbl_1_bss_1DE[0], 255, 255, 255, 255);
    Hu3DGLightColorSet(lbl_1_bss_1DE[1], 0, 0, 0, 255);

    HuAudSStreamPlay(0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    lbl_1_bss_4 = 0;
    eventProcess = HuPrcChildCreate(fn_1_1128, 256, 12288, 0, HuPrcCurrentGet());

    frame = 0;
    while (lbl_1_bss_4 == 0) {
        if (enablePrompt != 0) {
            if (frame > 250 && omcurovl != 1) {
                HuWinDispOn(messageWinId);
            }

            if (omcurovl == 1 && (HuPadBtnDown[0] & PAD_BUTTON_A)) {
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_START) {
                break;
            }
        }

        frame++;
        HuPrcVSleep();
    }

    HuWinDispOff(messageWinId);
    HuAudSStreamAllFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    HuWinKill(messageWinId);
    HuPrcKill(eventProcess);
    Hu3DCameraLayerHookReset(1, 0);
    Hu3DCameraLayerHookReset(1, 8);

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_2E[i]);
    }

    for (j = 0; j < 8; j++) {
        Hu3DModelAttrSet(lbl_1_bss_1CE[j], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_1128(void)
{
    lbl_1_bss_2C = 0;
    fn_1_4E34(fn_1_1BD8, 0, 0);
    fn_1_4E34((void (*)(void))fn_1_257C, 1050, 0);
    fn_1_4E34((void (*)(void))fn_1_4068, 990, 0);
    fn_1_4E34((void (*)(void))fn_1_2238, 1010, 0);
    fn_1_4E34(fn_1_37F0, 0, 0);
    fn_1_48AC();

    HuPrcSleep(60);
    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 720896, 240);
    fn_1_47AC(0, 720897, 240);
    fn_1_47AC(0, 720898, 240);
    fn_1_47AC(0, 720899, 80);
    fn_1_4E34((void (*)(void))fn_1_2304, 10, 60);
    fn_1_4E34(fn_1_41B4, 0, 0);
    HuPrcSleep(160);

    while (lbl_1_bss_20 > -200.f) {
        HuPrcVSleep();
    }

    lbl_1_bss_2C = 2;
    fn_1_4E34((void (*)(void))fn_1_2304, 32779, 60);
    fn_1_4E34(fn_1_420C, 0, 0);
    fn_1_47AC(0, 720900, 80);
    fn_1_4E34((void (*)(void))fn_1_2304, 32778, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 720901, 70);
    fn_1_4E34((void (*)(void))fn_1_2304, 32779, 40);
    fn_1_4E34(fn_1_420C, 0, 0);

    lbl_1_bss_2C = 4;
    fn_1_47AC(0, 720902, 60);
    fn_1_4E34((void (*)(void))fn_1_2304, 32778, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 720903, 60);
    fn_1_4E34((void (*)(void))fn_1_2304, 32779, 40);
    fn_1_4E34(fn_1_420C, 0, 0);

    lbl_1_bss_2C = 4;
    fn_1_47AC(0, 720902, 50);
    fn_1_4E34((void (*)(void))fn_1_2304, 32778, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 720903, 50);

    lbl_1_bss_2C = 5;
    fn_1_4E34((void (*)(void))fn_1_2304, 32778, 10000);
    fn_1_4E34((void (*)(void))fn_1_2304, 32779, 10000);
    fn_1_4E34((void (*)(void))fn_1_4264, 600, 0);
    fn_1_47AC(0, 720904, 240);
    fn_1_47AC(0, 720905, 240);
    HuPrcSleep(120);

    HuWinExClose(lbl_1_bss_2E[0]);
    lbl_1_bss_C = 1;
    lbl_1_bss_8 = 1;
    fn_1_4E34((void (*)(void))fn_1_2708, 20, 0);
    fn_1_470C(0, 4);
    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 720906, 0);
    HuPrcSleep(20);

    fn_1_470C(1, 11);
    fn_1_470C(2, 18);
    fn_1_470C(3, 25);
    fn_1_470C(4, 32);
    fn_1_470C(5, 39);
    fn_1_470C(6, 46);
    fn_1_470C(7, 53);
    HuPrcSleep(30);
    fn_1_470C(8, 57);
    fn_1_470C(9, 64);
    HuPrcSleep(60);

    fn_1_4E34((void (*)(void))fn_1_2B9C, 20, 0);
    HuPrcSleep(30);
    fn_1_47AC(0, 720907, 10);
    fn_1_4E34((void (*)(void))fn_1_2D74, 30, 0);
    HuPrcSleep(60);

    lbl_1_bss_C = lbl_1_bss_8 = 0;
    fn_1_4E34((void (*)(void))fn_1_2304, 32778, 20);
    fn_1_4E34((void (*)(void))fn_1_2304, 32779, 20);
    HuPrcSleep(170);

    fn_1_47AC(0, 720908, 240);
    fn_1_47AC(0, 720909, 240);
    fn_1_47AC(0, 720910, 40);
    fn_1_4E34((void (*)(void))fn_1_4484, 200, 0);
    HuPrcSleep(200);
    fn_1_4E34((void (*)(void))fn_1_34EC, 20, 0);

    HuWinExClose(lbl_1_bss_2E[0]);
    lbl_1_bss_0 = 1;
    Hu3DModelAttrReset(lbl_1_bss_1CE[1], 1073741826);
    Hu3DModelClusterAttrReset(lbl_1_bss_1CE[1], 0, 3221225474);
    HuPrcSleep(10);
    fn_1_46B4(10, 72);
    fn_1_46B4(11, 76);
    fn_1_4E34((void (*)(void))fn_1_4484, 390, 0);
    HuPrcSleep(140);

    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_4E34((void (*)(void))fn_1_392C, 30, 0);
    fn_1_47AC(0, 720911, 240);
    lbl_1_bss_0 = 2;
    HuWinExClose(lbl_1_bss_2E[0]);
    HuPrcSleep(130);

    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 720912, 300);
    HuWinExClose(lbl_1_bss_2E[0]);
    fn_1_4E34(fn_1_3C28, 0, 0);
    HuPrcSleep(140);

    lbl_1_bss_4 = 1;
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_1828(s16 layerNo)
{
    HuVecF position;
    HuVecF target;

    Hu3DCameraPerspectiveSet(1, 45.0f, 20.f, 8000.f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.f, 480.f,
                         0.0f, 1.0f);
    position.x = position.y = position.z = 0.0f;
    target.x = 0.0f;
    target.y = 0.0f;
    target.z = 0.0f;
    fn_1_4ECC(&position, &target, 450.f);
}

void fn_1_192C(s16 layerNo)
{
    HuVecF position;
    HuVecF target;

    Hu3DCameraPerspectiveSet(1, 30.f, 20.f, 15000.f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.f, 480.f,
                         0.0f, 1.0f);
    position.x = position.y = position.z = 0.0f;
    target.x = target.y = target.z = 0.0f;
    fn_1_4ECC(&position, &target, 500.f);

    switch (lbl_1_bss_0) {
        case 0:
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[0]);
            lbl_1_bss_10 = 0.0f;
            break;
        case 1:
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[1]);
            break;
        case 2:
            lbl_1_bss_10 += 0.01f;
            if (lbl_1_bss_10 > 0.9f) {
                lbl_1_bss_10 = 0.9f;
            }
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_I8, FALSE, lbl_1_bss_1C0);
            fn_1_517C(lbl_1_bss_10);
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[1]);
            break;
    }
    Hu3DZClear();
}

void fn_1_1B7C(void)
{
    Hu3DAnimKill((&lbl_1_bss_78)[0]);
    Hu3DAnimKill((&lbl_1_bss_78)[1]);
    HuSprAnimKill(lbl_1_bss_1B8[0]);
    HuSprAnimKill(lbl_1_bss_1B8[1]);
}

void fn_1_1BD8(void)
{
    HSF_OBJECT *bgObj;
    s16 prevState;
    s32 counter;
    s32 frameNo;
    float temp;
    float savedRotZ;

    prevState = -1;
    lbl_1_bss_2C = 0;
    bgObj = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_355);
    counter = 0;
    while (TRUE) {
        if (prevState != lbl_1_bss_2C) {
            counter = 0;
        }
        prevState = lbl_1_bss_2C;

        switch (lbl_1_bss_2C) {
        case 0:
            counter++;
            if (counter > 1000) {
                break;
            }
            temp = (float)sin(3.141592653589793 *
                              (90.f * (counter / 1000.f)) /
                              180.);
            bgObj->mesh.base.rot.z = -200.f * temp;
            frameNo = (s32)(bgObj->mesh.base.rot.z / 19.0f);
            if (bgObj->mesh.base.rot.z > -120.f) {
                lbl_1_bss_24.x = -45.0f;
            } else {
                lbl_1_bss_24.x = -225.0f;
            }
            fn_1_45D4(10, lbl_1_bss_24.x);
            frameNo = (s32)(bgObj->mesh.base.rot.z / 19.0f);
            lbl_1_bss_24.y = -135.0f;
            fn_1_45D4(11, lbl_1_bss_24.y);
            lbl_1_bss_20 = bgObj->mesh.base.rot.z;
            break;
        case 2:
            counter++;
            if (counter > 30) {
                break;
            }
            temp = (float)(counter / 30.f);
            bgObj->mesh.base.rot.z = (float)(-200. +
                                              20. *
                                                  sin(3.141592653589793 *
                                                      (90.f * temp) /
                                                      180.));
            if (counter == 1) {
                fn_1_4E34((void (*)(void))fn_1_2104, 11, -150);
            }
            if (15.0 == counter) {
                fn_1_4E34((void (*)(void))fn_1_2104, 10, -210);
            }
            break;
        case 3:
            if (counter == 0) {
                savedRotZ = bgObj->mesh.base.rot.z;
            }
            counter++;
            if (counter > 30) {
                break;
            }
            temp = (float)(counter / 30.f);
            bgObj->mesh.base.rot.z = savedRotZ +
                                     (-200.f - savedRotZ) *
                                         sin(3.141592653589793 *
                                             (90.f * temp) /
                                             180.);
            break;
        case 4:
            if (counter == 0) {
                savedRotZ = bgObj->mesh.base.rot.z;
            }
            counter++;
            if (counter > 30) {
                break;
            }
            temp = (float)(counter / 30.f);
            bgObj->mesh.base.rot.z = savedRotZ +
                                     (-160.f - savedRotZ) *
                                         sin(3.141592653589793 *
                                             (90.f * temp) /
                                             180.);
            break;
        case 5:
            if (counter == 0) {
                savedRotZ = bgObj->mesh.base.rot.z;
            }
            counter++;
            if (counter > 30) {
                break;
            }
            temp = (float)(counter / 30.f);
            bgObj->mesh.base.rot.z = savedRotZ +
                                     (-180.f - savedRotZ) *
                                         sin(3.141592653589793 *
                                             (90.f * temp) /
                                             180.);
            break;
        }
        HuPrcVSleep();
    }
}

void fn_1_2104(s32 modelIndex, s32 distance)
{
    s16 i;
    float startDist;
    float wave;

    startDist = (&lbl_1_bss_24.x)[modelIndex - 10];
    for (i = 1; i <= 20; i++) {
        wave = (float)sin(3.141592653589793 *
                          (90.f * (i / 20.f)) /
                          180.);
        fn_1_45D4((s16)modelIndex,
                  startDist + wave * ((float)distance - startDist));
        HuPrcVSleep();
    }
}

void fn_1_2238(u32 frameCount)
{
    s16 phase;
    s16 count;
    s16 i;

    fn_1_46B4(10, 71);
    fn_1_46B4(11, 75);
    phase = 1;
    for (i = 1, count = 0; i <= frameCount; i++) {
        if (count++ > 10) {
            phase ^= 1;
            fn_1_46B4(10, phase + 70);
            fn_1_46B4(11, phase + 74);
            count = 0;
        }
        HuPrcVSleep();
    }
    fn_1_46B4(10, 70);
    fn_1_46B4(11, 74);
}

void fn_1_2304(u32 guideChar, s32 frameCount)
{
    s16 frame;
    s16 toggleCounter;
    s16 phase;
    BOOL alternate;
    float focusValue;
    OpeningFocusObject *object;

    alternate = guideChar & 32768;
    lbl_1_bss_8 = 0;
    guideChar &= 255;
    object = (OpeningFocusObject *)Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[guideChar]);

    if (guideChar == 10) {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 71);
        } else {
            fn_1_46B4((s16)guideChar, 73);
        }
    } else {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 75);
        } else {
            fn_1_46B4((s16)guideChar, 77);
        }
    }

    focusValue = 1.2f;
    phase = 1;
    for (frame = 1, toggleCounter = 0; frame <= frameCount; frame++) {
        object->focus38 = focusValue;
        object->focus34 = focusValue;

        if (toggleCounter++ > 5) {
            phase ^= 1;
            if (lbl_1_bss_C != 0) {
                focusValue = 1.0f;
            } else {
                focusValue = (1.0f == focusValue) ? 1.2f : 1.0f;
            }

            if (guideChar == 10) {
                if (!alternate) {
                    fn_1_46B4((s16)guideChar, (s16)(70 + phase));
                } else {
                    fn_1_46B4((s16)guideChar, (s16)(72 + phase));
                }
            } else {
                if (!alternate) {
                    fn_1_46B4((s16)guideChar, (s16)(74 + phase));
                } else {
                    fn_1_46B4((s16)guideChar, (s16)(76 + phase));
                }
            }
            toggleCounter = 0;
        }

        if (lbl_1_bss_8 != 0) {
            break;
        }
        HuPrcVSleep();
    }

    if (guideChar == 10) {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 71);
        } else {
            fn_1_46B4((s16)guideChar, 73);
        }
    } else {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 75);
        } else {
            fn_1_46B4((s16)guideChar, 77);
        }
    }
    object->focus34 = object->focus38 = 1.0f;
}

void fn_1_257C(u32 frameCount)
{
    HSF_OBJECT *background;
    HSF_OBJECT *object;
    float phase;
    float amplitude;
    s16 i;

    phase = 0.0f;
    amplitude = 10.f;
    background = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_355);

    for (i = 1; ; i++) {
        object = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[10]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude *
                                   sin(3.141592653589793 * phase / 180.);

        object = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[11]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude *
                                   cos(3.141592653589793 * phase / 180.);

        phase += 5.0f;
        if (i > frameCount - 10) {
            amplitude -= 1.0f;
            if (amplitude < 0.0f) {
                amplitude = 0.0f;
            }
        }
        HuPrcVSleep();
    }
}

void fn_1_2708(u32 frameCount)
{
    HuVecF pos;
    s16 i;
    s16 endFrame;
    s16 startFrame;
    float scaleX;
    float scaleY;

    Hu3DModelAttrReset(lbl_1_bss_1CE[3], HU3D_ATTR_DISPOFF);
    Hu3DModelPosGet(lbl_1_bss_1CE[1], &pos);
    pos.x += 30.f;
    pos.y -= 45.0f;
    pos.z += 20.f;
    Hu3DModelPosSetV(lbl_1_bss_1CE[3], &pos);
    Hu3DModelScaleSet(lbl_1_bss_1CE[3], 0.0f, 0.0f,
                      0.0f);
    HuAudFXPlay(1200);

    for (i = 1; i <= frameCount; i++) {
        if (i <= 0.7 * frameCount) {
            endFrame = (s16)(0.7 * frameCount);
            scaleY = (float)(0.8999999761581421 *
                             (sin(3.141592653589793 *
                                  (130.f * ((float)i / endFrame)) /
                                  180.) *
                              (1.0 / sin(2.2689280275926285))));
        }
        if (i > 0.3 * frameCount) {
            startFrame = (s16)((float)i - 0.3f * frameCount);
            endFrame = (s16)(0.7f * frameCount);
            scaleX = (float)(0.8999999761581421 *
                             (sin(3.141592653589793 *
                                  (130.f * ((float)startFrame / endFrame)) /
                                  180.) *
                              (1.0 / sin(2.2689280275926285))));
        }
        Hu3DModelScaleSet(lbl_1_bss_1CE[3], scaleX, scaleY, 1.0f);
        HuPrcVSleep();
    }
}

void fn_1_2B9C(u32 frameCount)
{
    s16 frame;
    float scale;
    float scaleY;

    for (frame = 1; frame <= frameCount; frame++) {
        scale = (float)(0.8999999761581421 *
                        (1.0 -
                         sin(3.141592653589793 *
                             (90.f * ((float)frame / frameCount)) /
                             180.)));
        scaleY = (float)(0.8999999761581421 *
                         (1.0 -
                          sin(3.141592653589793 *
                              (90.f * ((float)frame / frameCount)) /
                              180.)));
        Hu3DModelScaleSet(lbl_1_bss_1CE[3], scale, scaleY, 1.0f);
        HuPrcVSleep();
    }
}

void fn_1_2D74(u32 frameCount)
{
    HuVecF pos;
    s16 i;
    s16 halfFrames;
    s16 endFrame;
    s16 startFrame;
    float scaleX;
    float scaleY;

    Hu3DModelAttrReset(lbl_1_bss_1CE[4], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[5], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[6], HU3D_ATTR_DISPOFF);
    Hu3DModelPosGet(lbl_1_bss_1CE[1], &pos);
    pos.x += 30.f;
    pos.y -= 45.0f;
    pos.z += 50.f;
    Hu3DModelPosSetV(lbl_1_bss_1CE[4], &pos);
    Hu3DModelScaleSet(lbl_1_bss_1CE[4], 0.0f, 0.0f,
                      0.0f);
    pos.x = 0.0f;
    pos.y += 90.f;
    pos.z += 50.f;
    lbl_1_bss_14 = pos;
    Hu3DModelScaleSet(lbl_1_bss_1CE[5], 0.0f, 0.0f,
                      0.0f);
    Hu3DModelScaleSet(lbl_1_bss_1CE[6], 0.0f, 0.0f,
                      0.0f);

    halfFrames = frameCount >> 1;
    for (i = 1; i <= halfFrames; i++) {
        if (i <= 0.7 * halfFrames) {
            endFrame = (s16)(0.7 * halfFrames);
            scaleY = (float)(0.8999999761581421 *
                             (sin(3.141592653589793 *
                                  (130.f * ((float)i / endFrame)) /
                                  180.) *
                              (1.0 / sin(2.2689280275926285))));
        }
        if (i > 0.3 * halfFrames) {
            startFrame = (s16)((float)i - 0.3f * halfFrames);
            endFrame = (s16)(0.7f * halfFrames);
            scaleX = (float)(0.8999999761581421 *
                             (sin(3.141592653589793 *
                                  (130.f * ((float)startFrame / endFrame)) /
                                  180.) *
                              (1.0 / sin(2.2689280275926285))));
        }
        Hu3DModelScaleSet(lbl_1_bss_1CE[4], scaleX, scaleY, scaleX);
        HuPrcVSleep();
    }

    HuAudFXPlay(1201);
    for (i = 1; i <= halfFrames; i++) {
        scaleX = (float)(0.5 *
                         (sin(3.141592653589793 *
                             (130.f * ((float)i / halfFrames)) /
                             180.) *
                          (1.0 / sin(2.2689280275926285))));
        Hu3DModelScaleSet(lbl_1_bss_1CE[5], scaleX, scaleX, 0.5f);
        scaleX = (float)(0.5 *
                         sin(3.141592653589793 *
                             (90.f * ((float)i / halfFrames)) /
                             180.));
        Hu3DModelScaleSet(lbl_1_bss_1CE[6], scaleX, scaleX, 0.5f);
        HuPrcVSleep();
    }
}

void fn_1_34EC(u32 frameCount)
{
    s16 frame;
    s16 halfFrames;
    float scale;
    float scaleY;

    halfFrames = frameCount >> 1;
    for (frame = 1; frame <= halfFrames; frame++) {
        scale = (float)(0.5 *
                        (1.0 -
                         sin(3.141592653589793 *
                             (90.f * ((float)frame / halfFrames)) /
                             180.)));
        Hu3DModelScaleSet(lbl_1_bss_1CE[5], scale, scale, 0.5f);
        Hu3DModelScaleSet(lbl_1_bss_1CE[6], scale, scale, 0.5f);
        HuPrcVSleep();
    }

    for (frame = 1; frame <= halfFrames; frame++) {
        scale = (float)(0.8999999761581421 *
                        (1.0 -
                         sin(3.141592653589793 *
                             (90.f * ((float)frame / halfFrames)) /
                             180.)));
        scaleY = (float)(0.8999999761581421 *
                         (1.0 -
                          sin(3.141592653589793 *
                              (90.f * ((float)frame / halfFrames)) /
                              180.)));
        Hu3DModelScaleSet(lbl_1_bss_1CE[4], scale, scaleY, scale);
        HuPrcVSleep();
    }
}

void fn_1_37F0(void)
{
    HuVecF position;
    float phase;

    phase = 0.0f;
    while (TRUE) {
        position = lbl_1_bss_14;
        position.x += 3.0 *
                      sin(3.141592653589793 * (phase / 2.f) / 180.);
        position.y += 5.0 *
                      sin(3.141592653589793 * phase / 180.);
        Hu3DModelPosSetV(lbl_1_bss_1CE[5], &position);
        Hu3DModelPosSetV(lbl_1_bss_1CE[6], &position);
        phase += 1.0f;
        if (phase > 720.f) {
            phase -= 720.f;
        }
        HuPrcVSleep();
    }
}

void fn_1_392C(u32 frameCount)
{
    HuVecF pos;
    s16 i;
    float scale;

    Hu3DModelAttrReset(lbl_1_bss_1CE[5], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_1CE[6], HU3D_ATTR_DISPOFF);
    Hu3DModelPosGet(lbl_1_bss_1CE[1], &pos);
    pos.y = 80.f;
    pos.z += 150.f;
    Hu3DModelPosSetV(lbl_1_bss_1CE[5], &pos);
    Hu3DModelScaleSet(lbl_1_bss_1CE[5], 0.0f, 0.0f,
                      0.0f);
    Hu3DModelPosSetV(lbl_1_bss_1CE[6], &pos);
    Hu3DModelScaleSet(lbl_1_bss_1CE[6], 0.0f, 0.0f,
                      0.0f);
    HuAudFXPlay(1202);

    for (i = 1; i <= frameCount; i++) {
        scale = (float)(0.5 *
                        (sin(3.141592653589793 *
                             (130.f * ((float)i / frameCount)) /
                             180.) *
                         (1.0 / sin(2.2689280275926285))));
        Hu3DModelScaleSet(lbl_1_bss_1CE[5], scale, scale, 0.5f);
        scale = (float)(0.5 *
                        sin(3.141592653589793 *
                            (90.f * ((float)i / frameCount)) /
                            180.));
        Hu3DModelScaleSet(lbl_1_bss_1CE[6], scale, scale, 0.5f);
        HuPrcVSleep();
    }
}

void fn_1_3C28(void)
{
    HuVecF vec;
    HuVecF pos;
    HuVecF target = { 0.0f, -40.0f, 500.0f };
    float rot;
    s16 i;

    for (i = 1; i <= 60; i++) {
        rot = 720.f * ((float)i / 60.f);
        Hu3DModelRotSet(lbl_1_bss_1CE[5], 0.0f, 0.0f,
                        (float)(30. *
                                sin(3.141592653589793 * rot / 180.)));
        HuPrcVSleep();
    }

    for (i = 1; i <= 30; i++) {
        if (i == 5) {
            HuAudFXPlay(1203);
        }
        rot = (float)(720. *
                      (1.0 -
                       cos(3.141592653589793 *
                           (90.f * ((float)i / 30.f)) /
                           180.)));
        Hu3DModelRotSet(lbl_1_bss_1CE[5], 0.0f, rot,
                        0.0f);
        HuPrcVSleep();
    }

    HuAudFXPlay(1201);
    Hu3DModelAttrReset(lbl_1_bss_1CE[7], HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(lbl_1_bss_1CE[7], 0.0f);
    Hu3DModelPosGet(lbl_1_bss_1CE[5], &pos);
    Hu3DModelPosSetV(lbl_1_bss_1CE[7], &pos);
    HuPrcSleep(20);
    Hu3DModelPosGet(lbl_1_bss_1CE[5], &pos);

    for (i = 1; i <= 60; i++) {
        if (i == 5) {
            HuAudFXPlay(1204);
        }
        rot = (float)i / 60.f;
        PSVECSubtract(&target, &pos, &vec);
        PSVECScale(&vec, &vec,
                   (float)(1.0 -
                           cos(3.141592653589793 *
                               (90.f * rot) /
                               180.)));
        PSVECAdd(&pos, &vec, &vec);
        vec.y -= 100. *
                 sin(3.141592653589793 *
                     (180.f * ((float)i / 50.f)) /
                     180.);
        lbl_1_bss_14 = vec;
        Hu3DModelRotSet(lbl_1_bss_1CE[5], 0.0f, 0.0f,
                        20.f * rot);
        HuPrcVSleep();
    }
}

void fn_1_4068(u32 frameCount)
{
    s16 i;
    s16 j;
    s16 timer[OPENING_CHAR_COUNT];
    s16 count[OPENING_CHAR_COUNT];

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        timer[i] = (s16)frandmod(30);
        count[i] = 1;
    }

    for (j = 0; j < frameCount - 10; j++) {
        for (i = 0; i < OPENING_CHAR_COUNT; i++) {
            if (timer[i] == 0) {
                fn_1_470C((s16)i, (s16)((i * 7) + (count[i] & 1)));
                count[i]++;
                timer[i] = (s16)(frandmod(20) + 30);
            }
            timer[i]--;
        }
        HuPrcVSleep();
    }

    HuPrcSleep(11);
}

void fn_1_41B4(void)
{
    s16 i;

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        fn_1_470C((s16)i, (s16)((i * 7) + 5));
    }
    HuPrcSleep(10);
}

void fn_1_420C(void)
{
    s16 i;

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        fn_1_470C((s16)i, (s16)((i * 7) + 6));
    }
    HuPrcSleep(10);
}

void fn_1_4264(u32 frameCount)
{
    s16 charIndex;
    s16 elapsed;
    s32 focusStarted;
    s32 rearmLimit;
    float remainingFraction;
    s16 countdowns[OPENING_CHAR_COUNT];
    s16 phases[OPENING_CHAR_COUNT];

    focusStarted = 0;
    for (charIndex = 0; charIndex < OPENING_CHAR_COUNT; charIndex++) {
        countdowns[charIndex] = (s16)frandmod(30);
        phases[charIndex] = 1;
    }

    for (elapsed = 0; elapsed < frameCount - 10; elapsed++) {
        remainingFraction = 1.0f - ((float)elapsed / frameCount);
        for (charIndex = 0; charIndex < OPENING_CHAR_COUNT; charIndex++) {
            if (charIndex != 0 || focusStarted == 0) {
                if (countdowns[charIndex] == 0) {
                    if (focusStarted == 0 && charIndex == 0 && elapsed > 240) {
                        fn_1_46B4(0, 78);
                        focusStarted = 1;
                        continue;
                    }
                    fn_1_470C((s16)charIndex,
                              (s16)((charIndex * 7) + 5 +
                                    (phases[charIndex] & 1)));
                    phases[charIndex]++;
                    rearmLimit = (s32)(40.f * remainingFraction);
                    countdowns[charIndex] =
                        (s16)(frandmod((s16)rearmLimit) + 10);
                }
                countdowns[charIndex]--;
            }
        }
        HuPrcVSleep();
    }

    HuPrcSleep(11);
}

void fn_1_4484(u32 frameCount)
{
    s16 frame;
    s16 i;
    s16 countdowns[OPENING_CHAR_COUNT];
    s16 phases[OPENING_CHAR_COUNT];

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        countdowns[i] = (s16)frandmod(30);
        phases[i] = 1;
    }

    for (frame = 0; frame < frameCount - 10; frame++) {
        for (i = 0; i < OPENING_CHAR_COUNT; i++) {
            if (countdowns[i] == 0) {
                fn_1_470C((s16)i, (s16)((i * 7) + 2 + (phases[i] & 1)));
                phases[i]++;
                countdowns[i] = (s16)(frandmod(30) + 20);
            }
            countdowns[i]--;
        }
        HuPrcVSleep();
    }

    HuPrcSleep(11);
}

void fn_1_45D4(s16 modelIndex, float distance)
{
    HSF_OBJECT *object;

    object = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[modelIndex]);
    object->mesh.base.pos.x = 450. * sin(3.141592653589793 * distance / 180.);
    object->mesh.base.pos.y = 450. * cos(3.141592653589793 * distance / 180.);
}

void fn_1_46B4(s16 animIndex, s16 frameIndex)
{
    Hu3DAnimAnimSet(lbl_1_bss_60[animIndex], lbl_1_bss_7C[frameIndex]);
}

void fn_1_470C(s16 animIndex, s16 frameIndex)
{
    fn_1_4E34((void (*)(void))fn_1_4744, animIndex, frameIndex);
}

void fn_1_4744(s32 animIndex, s32 frameIndex)
{
    s16 scratch = 0;

    HuPrcSleep(5);
    Hu3DAnimAnimSet(lbl_1_bss_60[animIndex], lbl_1_bss_7C[frameIndex]);
}

void fn_1_47AC(s16 winIndex, u32 message, s16 frameCount)
{
    s16 i;

    for (i = 0; i < OPENING_WIN_COUNT; i++) {
        HuWinDispOff(lbl_1_bss_2E[i]);
    }

    HuWinDispOn(lbl_1_bss_2E[winIndex]);
    if (message != 0) {
        HuWinMesSet(lbl_1_bss_2E[winIndex], message);
    } else {
        HuWinHomeClear(lbl_1_bss_2E[winIndex]);
    }

    if (frameCount >= 0 && frameCount != 0) {
        HuPrcSleep(frameCount);
    }
}

void fn_1_48AC(void)
{
    Mtx matrix;
    HuVecF ofs;
    HuVecF endPos = { 0.0f, 40.0f, -300.0f };
    HuVecF endRot = { -10.0f, 360.0f, 360.0f };
    float rate;
    s16 i;

    Hu3DMotionTimeSet(lbl_1_bss_1CE[1], 0.0f);
    Hu3DMotionClusterTimeSet(lbl_1_bss_1CE[1], 0, 0.0f);
    Hu3DModelAttrReset(lbl_1_bss_1CE[1], HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrReset(lbl_1_bss_1CE[1], 0, HU3D_CLUSTER_ATTR_PAUSE);
    fn_1_4E34(fn_1_4BD8, 0, 0);

    for (i = 1; i <= 180; i++) {
        rate = i / 180.f;

        PSVECSubtract(&endPos, &lbl_1_data_320, &ofs);
        PSVECScale(&ofs, &ofs,
                   (float)sin(3.141592653589793 *
                              (90.f * rate) /
                              180.));
        PSVECAdd(&lbl_1_data_320, &ofs, &lbl_1_bss_54);

        PSVECSubtract(&endRot, &lbl_1_data_32C, &ofs);
        PSVECScale(&ofs, &ofs,
                   (float)sin(3.141592653589793 *
                              (90.f * rate) /
                              180.));
        PSVECAdd(&lbl_1_data_32C, &ofs, &lbl_1_bss_3C);

        PSMTXTrans(matrix,
                   (float)(150. *
                           (1.0 -
                            sin(3.141592653589793 *
                                (90.f * rate) /
                                180.))),
                   0.0f,
                   0.0f);
        Hu3DModelMtxSet(lbl_1_bss_1CE[1], &matrix);

        lbl_1_bss_38 = (float)(0.05000000074505806 +
                               0.949999988079071 *
                                   (1.0 -
                                    sin(3.141592653589793 *
                                        (90.f * rate) /
                                        180.)));
        HuPrcVSleep();
    }
}

void fn_1_4BD8(void)
{
    HuPrcSleep(120);
    Hu3DModelAttrSet(lbl_1_bss_1CE[1], HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrSet(lbl_1_bss_1CE[1], 0, HU3D_CLUSTER_ATTR_PAUSE);
}

void fn_1_4C30(void)
{
    HU3D_MODEL *model;
    float phase;

    model = &Hu3DData[lbl_1_bss_1CE[1]];
    phase = 0.0f;
    do {
        do {
            HuPrcVSleep();
        } while ((model->attr & HU3D_ATTR_DISPOFF) != 0);

        PSVECAdd(&lbl_1_bss_54, &lbl_1_bss_48, &model->pos);
        lbl_1_bss_48.x = lbl_1_bss_38 *
                         (5.0 * sin(3.141592653589793 * phase / 180.));
        lbl_1_bss_48.y = lbl_1_bss_38 *
                         (20. *
                          cos(3.141592653589793 * (2.0f * phase) / 180.));
        model->rot = lbl_1_bss_3C;
        phase += 1.0f;
        if (phase > 720.f) {
            phase -= 720.f;
        }
    } while (TRUE);
}

void fn_1_4DB0(void)
{
    HUPROCESS *process;
    OpeningEventWork *work;

    process = HuPrcCurrentGet();
    work = process->property;
    work->callback(work->arg0, work->arg1);
    HuPrcEnd();
}

void fn_1_4E00(void)
{
    HUPROCESS *process;

    process = HuPrcCurrentGet();
    HuMemDirectFree(process->property);
}

void fn_1_4E34(void (*callback)(void), s32 arg0, s32 arg1)
{
    HUPROCESS *process;
    OpeningEventWork *work;

    process = HuPrcChildCreate(fn_1_4DB0, 256, 12288, 0, HuPrcCurrentGet());
    work = HuMemDirectMalloc(HEAP_HEAP, sizeof(OpeningEventWork));
    process->property = work;
    HuPrcDestructorSet2(process, fn_1_4E00);
    work->callback = (void (*)(s32, s32))callback;
    work->arg0 = arg0;
    work->arg1 = arg1;
}

void fn_1_4ECC(HuVecF *rotation, HuVecF *target, float distance)
{
    HuVecF position;
    HuVecF cameraTarget;
    HuVecF up;
    float x;
    float y;
    float z;

    x = rotation->x;
    y = rotation->y;
    z = rotation->z;

    position.x = target->x +
                 (distance *
                  (sin(3.141592653589793 * y / 180.) *
                   cos(3.141592653589793 * x / 180.)));
    position.y = target->y +
                 (distance * -sin(3.141592653589793 * x / 180.));
    position.z = target->z +
                 (distance *
                  (cos(3.141592653589793 * y / 180.) *
                   cos(3.141592653589793 * x / 180.)));
    cameraTarget.x = target->x;
    cameraTarget.y = target->y;
    cameraTarget.z = target->z;
    up.x = sin(3.141592653589793 * y / 180.) *
           sin(3.141592653589793 * x / 180.);
    up.y = cos(3.141592653589793 * x / 180.);
    up.z = cos(3.141592653589793 * y / 180.) *
           sin(3.141592653589793 * x / 180.);

    Hu3DCameraPosSet(1,
                     position.x,
                     position.y,
                     position.z,
                     up.x,
                     up.y,
                     up.z,
                     cameraTarget.x,
                     cameraTarget.y,
                     cameraTarget.z);
}

void fn_1_517C(float alpha)
{
    Mtx44 projection;
    Mtx modelview;
    GXTexObj texture;
    GXColor color1 = { 154, 112, 86, 255 };
    GXColor color2 = { 166, 166, 166, 255 };

    C_MTXOrtho(projection, 0.0f, 480.f, 0.0f,
               640.f, 0.0f, 8000.f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    color1.a = 255.0f * alpha;
    GXSetTevColor(GX_TEVREG0, color1);
    GXSetTevColor(GX_TEVREG1, color2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_CPREV, GX_CC_C1, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(2);
    GXInitTexObj(&texture, lbl_1_bss_1C0, 640, 320, GX_TF_I8, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, 0.0f, 0.0f,
                    0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(0.0f, 80.f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition2f32(640.f, 80.f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition2f32(640.f, 400.f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition2f32(0.0f, 400.f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXEnd();
}

void fn_1_A0(void)
{
    OSReport(lbl_1_data_369);
    lbl_1_bss_1E4 = omInitObjMan(50, 8192);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.f, 15000.f,
                             1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.f,
                          480.f, 0.0f, 1.0f);
    HuPrcCreate(fn_1_28C, 100, 12288, 0);

    lbl_1_bss_1DE[0] = Hu3DGLightCreate(0.0f, 10.f,
                                        1000.f, 0.0f,
                                        -0.01f, -1.0f,
                                        64, 64, 96);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[0]);
    lbl_1_bss_1DE[1] = Hu3DGLightCreate(0.0f, 2000.f,
                                        100.f, 0.0f,
                                        -1.0f, -0.05f,
                                        160, 160, 160);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[1]);
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
}

s16 lbl_1_data_0[OPENING_WIN_COUNT] = { 0, 4, 3, 5 };

u32 lbl_1_data_8[80] = {
    DATANUM(DATA_title, 30), DATANUM(DATA_title, 32),
    DATANUM(DATA_title, 34), DATANUM(DATA_title, 33),
    DATANUM(DATA_title, 34), DATANUM(DATA_title, 35),
    DATANUM(DATA_title, 36), DATANUM(DATA_title, 38),
    DATANUM(DATA_title, 39), DATANUM(DATA_title, 40),
    DATANUM(DATA_title, 41), DATANUM(DATA_title, 42),
    DATANUM(DATA_title, 43), DATANUM(DATA_title, 44),
    DATANUM(DATA_title, 45), DATANUM(DATA_title, 46),
    DATANUM(DATA_title, 47), DATANUM(DATA_title, 48),
    DATANUM(DATA_title, 49), DATANUM(DATA_title, 50),
    DATANUM(DATA_title, 51), DATANUM(DATA_title, 52),
    DATANUM(DATA_title, 53), DATANUM(DATA_title, 54),
    DATANUM(DATA_title, 55), DATANUM(DATA_title, 56),
    DATANUM(DATA_title, 57), DATANUM(DATA_title, 58),
    DATANUM(DATA_title, 59), DATANUM(DATA_title, 60),
    DATANUM(DATA_title, 61), DATANUM(DATA_title, 62),
    DATANUM(DATA_title, 63), DATANUM(DATA_title, 64),
    DATANUM(DATA_title, 65), DATANUM(DATA_title, 66),
    DATANUM(DATA_title, 67), DATANUM(DATA_title, 68),
    DATANUM(DATA_title, 69), DATANUM(DATA_title, 70),
    DATANUM(DATA_title, 71), DATANUM(DATA_title, 72),
    DATANUM(DATA_title, 73), DATANUM(DATA_title, 74),
    DATANUM(DATA_title, 75), DATANUM(DATA_title, 76),
    DATANUM(DATA_title, 77), DATANUM(DATA_title, 78),
    DATANUM(DATA_title, 79), DATANUM(DATA_title, 80),
    DATANUM(DATA_title, 81), DATANUM(DATA_title, 82),
    DATANUM(DATA_title, 83), DATANUM(DATA_title, 84),
    DATANUM(DATA_title, 85), DATANUM(DATA_title, 86),
    DATANUM(DATA_title, 87), DATANUM(DATA_title, 88),
    DATANUM(DATA_title, 89), DATANUM(DATA_title, 90),
    DATANUM(DATA_title, 91), DATANUM(DATA_title, 92),
    DATANUM(DATA_title, 93), DATANUM(DATA_title, 94),
    DATANUM(DATA_title, 95), DATANUM(DATA_title, 96),
    DATANUM(DATA_title, 97), DATANUM(DATA_title, 98),
    DATANUM(DATA_title, 99), DATANUM(DATA_title, 100),
    DATANUM(DATA_title, 105), DATANUM(DATA_title, 106),
    DATANUM(DATA_title, 107), DATANUM(DATA_title, 108),
    DATANUM(DATA_title, 101), DATANUM(DATA_title, 102),
    DATANUM(DATA_title, 103), DATANUM(DATA_title, 104),
    DATANUM(DATA_title, 37), HU_DATANUM_NONE,
};

char lbl_1_data_148[12] = "Dummy_mario";
char lbl_1_data_154[12] = "Dummy_Luigi";
char lbl_1_data_160[12] = "Dummy_peach";
char lbl_1_data_16C[12] = "Dummy_yoshi";
char lbl_1_data_178[12] = "Dummy_wario";
char lbl_1_data_184[12] = "Dummy_daisy";
char lbl_1_data_190[14] = "Dummy_waluigi";
char lbl_1_data_19E[14] = "Dummy_kinopio";
char lbl_1_data_1AC[13] = "Dummy_teresa";
char lbl_1_data_1B9[16] = "Dummy_minikoopa";
char lbl_1_data_1C9[9] = "guide001";
char lbl_1_data_1D2[10] = "guide002";

char *lbl_1_data_1DC[12] = {
    lbl_1_data_148, lbl_1_data_154, lbl_1_data_160, lbl_1_data_16C,
    lbl_1_data_178, lbl_1_data_184, lbl_1_data_190, lbl_1_data_19E,
    lbl_1_data_1AC, lbl_1_data_1B9, lbl_1_data_1C9, lbl_1_data_1D2,
};

char lbl_1_data_20C[18] = "op_stage011-mario";
char lbl_1_data_21E[18] = "op_stage011-luigi";
char lbl_1_data_230[18] = "op_stage011-peach";
char lbl_1_data_242[18] = "op_stage011-yoshi";
char lbl_1_data_254[18] = "op_stage011-wario";
char lbl_1_data_266[18] = "op_stage011-daisy";
char lbl_1_data_278[20] = "op_stage011-waluigi";
char lbl_1_data_28C[20] = "op_stage011-kinopio";
char lbl_1_data_2A0[19] = "op_stage011-teresa";
char lbl_1_data_2B3[22] = "op_stage011-minikoopa";
char lbl_1_data_2C9[19] = "op_stage011-soruru";
char lbl_1_data_2DC[20] = "op_stage011-luluna";

char *lbl_1_data_2F0[12] = {
    lbl_1_data_20C, lbl_1_data_21E, lbl_1_data_230, lbl_1_data_242,
    lbl_1_data_254, lbl_1_data_266, lbl_1_data_278, lbl_1_data_28C,
    lbl_1_data_2A0, lbl_1_data_2B3, lbl_1_data_2C9, lbl_1_data_2DC,
};

HuVecF lbl_1_data_320 = { 0.0f, 0.0f, -2000.0f };
HuVecF lbl_1_data_32C = { -20.0f, -30.0f, 30.0f };
char lbl_1_data_338[14] = "op_book_dummy";
char lbl_1_data_346[15] = "op_book_dummy2";
char lbl_1_data_355[20] = "op_stage004-bg_root";
char lbl_1_data_369[39] = "******* Opening ObjectSetup *********\n";

OMOBJMAN *lbl_1_bss_1E4;
s16 lbl_1_bss_1DE[3];
HU3D_MODELID lbl_1_bss_1CE[8];
s16 lbl_1_bss_1CC;
void *lbl_1_bss_1C4[2];
void *lbl_1_bss_1C0;
ANIMDATA *lbl_1_bss_1B8[2];
ANIMDATA *lbl_1_bss_7C[79];
HU3D_ANIMID lbl_1_bss_78;
HU3D_ANIMID lbl_1_bss_60[12];
HuVecF lbl_1_bss_54;
HuVecF lbl_1_bss_48;
HuVecF lbl_1_bss_3C;
float lbl_1_bss_38;
HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
s16 lbl_1_bss_2C;
HuVec2f lbl_1_bss_24;
float lbl_1_bss_20;
HuVecF lbl_1_bss_14;
float lbl_1_bss_10;
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
s32 lbl_1_bss_4;
s16 lbl_1_bss_0;
