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
extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_18;
extern const float lbl_1_rodata_1C;
extern const float lbl_1_rodata_20;
extern const float lbl_1_rodata_24;
extern const float lbl_1_rodata_28;
extern const float lbl_1_rodata_2C;
extern const float lbl_1_rodata_30;
extern const float lbl_1_rodata_34;
extern const float lbl_1_rodata_38;
extern const float lbl_1_rodata_3C;
extern const float lbl_1_rodata_40;
extern const float lbl_1_rodata_44;
extern const float lbl_1_rodata_48;
extern const float lbl_1_rodata_4C;
extern const float lbl_1_rodata_50;
extern const float lbl_1_rodata_54;
extern const float lbl_1_rodata_58;
extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_68;
extern const float lbl_1_rodata_6C;
extern const float lbl_1_rodata_70;
extern const float lbl_1_rodata_74;
extern const float lbl_1_rodata_78;
extern const float lbl_1_rodata_7C;
extern const float lbl_1_rodata_80;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_88;
extern const float lbl_1_rodata_8C;
extern const float lbl_1_rodata_90;
extern const float lbl_1_rodata_94;
extern const double lbl_1_rodata_98;
extern const double lbl_1_rodata_A8;
extern const double lbl_1_rodata_C8;
extern const float lbl_1_rodata_E8;
extern const float lbl_1_rodata_EC;
extern const double lbl_1_rodata_140;
extern const double lbl_1_rodata_148;
extern const float lbl_1_rodata_150;
extern const float lbl_1_rodata_154;
extern const double lbl_1_rodata_188;
extern const GXColor lbl_1_rodata_1C0;
extern const GXColor lbl_1_rodata_1C4;
extern const float lbl_1_rodata_1C8;
extern const float lbl_1_rodata_1CC;
extern const float lbl_1_rodata_1D0;
extern const float lbl_1_rodata_1D4;
extern const float lbl_1_rodata_1D8;
extern const float lbl_1_rodata_1DC;

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
extern HU3D_ANIMID lbl_1_bss_78[2];
extern ANIMDATA *lbl_1_bss_7C[79];
extern ANIMDATA *lbl_1_bss_1B8[2];
extern void *lbl_1_bss_1C0;
extern void *lbl_1_bss_1C4[2];
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
void fn_1_A0(void);
void fn_1_28C(void);
void fn_1_320(void);
void fn_1_A00(s32 enablePrompt);
void fn_1_9A4(void);
void fn_1_1128(void);
void fn_1_1828(s16 layerNo);
void fn_1_192C(s16 layerNo);
void fn_1_1BD8(void);
void fn_1_1B7C(void);
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
    OSReport(lbl_1_data_369);
    lbl_1_bss_1E4 = omInitObjMan(50, 8192);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_6C, lbl_1_rodata_70, lbl_1_rodata_88,
                             lbl_1_rodata_78);
    Hu3DCameraViewportSet(1, lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_7C,
                          lbl_1_rodata_80, lbl_1_rodata_14, lbl_1_rodata_38);
    HuPrcCreate(fn_1_28C, 100, 12288, 0);

    lbl_1_bss_1DE[0] = Hu3DGLightCreate(lbl_1_rodata_14, lbl_1_rodata_E8,
                                        lbl_1_rodata_5C, lbl_1_rodata_14,
                                        lbl_1_rodata_1D0, lbl_1_rodata_64,
                                        64, 64, 96);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[0]);
    lbl_1_bss_1DE[1] = Hu3DGLightCreate(lbl_1_rodata_14, lbl_1_rodata_1D4,
                                        lbl_1_rodata_1D8, lbl_1_rodata_14,
                                        lbl_1_rodata_64, lbl_1_rodata_1DC,
                                        160, 160, 160);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[1]);
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
}

void fn_1_28C(void)
{
    fn_1_320();
    fn_1_A00(GwCommon.viewOpening);
    GwCommon.viewOpening = 1;

    Hu3DAnimKill(lbl_1_bss_78[0]);
    Hu3DAnimKill(lbl_1_bss_78[1]);
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
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[0], lbl_1_rodata_10);
    Hu3DModelPosSet(lbl_1_bss_1CE[0], lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_18);

    lbl_1_bss_1CE[1] = Hu3DModelCreate(HuDataSelHeapReadNum(13959191, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[1], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[1], lbl_1_rodata_1C);
    Hu3DMotionClusterSpeedSet(lbl_1_bss_1CE[1], 0, lbl_1_rodata_1C);
    Hu3DModelPosSet(lbl_1_bss_1CE[1], lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_20);
    Hu3DModelRotSet(lbl_1_bss_1CE[1], lbl_1_rodata_24, lbl_1_rodata_28, lbl_1_rodata_2C);

    lbl_1_bss_1CE[2] = Hu3DModelCreate(HuDataSelHeapReadNum(13959192, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[2], 0);
    Hu3DModelPosSet(lbl_1_bss_1CE[2], lbl_1_rodata_14, lbl_1_rodata_30, lbl_1_rodata_14);

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
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[7], lbl_1_rodata_34);
    Hu3DModelScaleSet(lbl_1_bss_1CE[7], lbl_1_rodata_34, lbl_1_rodata_34, lbl_1_rodata_34);

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
    lbl_1_bss_78[0] = Hu3DAnimCreate(lbl_1_bss_1B8[0], lbl_1_bss_1CE[1], lbl_1_data_338);
    lbl_1_bss_78[1] = Hu3DAnimCreate(lbl_1_bss_1B8[1], lbl_1_bss_1CE[1], lbl_1_data_346);

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

    lbl_1_bss_48.x = lbl_1_bss_48.y = lbl_1_bss_48.z = lbl_1_rodata_14;
    lbl_1_bss_54 = lbl_1_data_320;
    lbl_1_bss_3C = lbl_1_data_32C;
    lbl_1_bss_38 = lbl_1_rodata_38;

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
    Hu3DModelRotSet(lbl_1_bss_1CE[5], lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_14);

    lbl_1_bss_0 = 0;
    lbl_1_bss_8 = 0;
    lbl_1_bss_C = 0;
    Hu3DAmbColorSet(lbl_1_rodata_38, lbl_1_rodata_38, lbl_1_rodata_38);
    Hu3DShineSet(FALSE);

    lbl_1_bss_48.x = lbl_1_bss_48.y = lbl_1_bss_48.z = lbl_1_rodata_14;
    lbl_1_bss_54 = lbl_1_data_320;
    lbl_1_bss_3C = lbl_1_data_32C;
    lbl_1_bss_38 = lbl_1_rodata_38;

    Hu3DMotionTimeSet(lbl_1_bss_1CE[1], lbl_1_rodata_14);
    Hu3DModelAttrSet(lbl_1_bss_1CE[1], 1073741826);
    Hu3DMotionClusterTimeSet(lbl_1_bss_1CE[1], 0, lbl_1_rodata_14);
    Hu3DModelClusterAttrSet(lbl_1_bss_1CE[1], 0, 3221225474);

    PSMTXTrans(matrix, lbl_1_rodata_3C, lbl_1_rodata_14, lbl_1_rodata_14);
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
        lbl_1_bss_2E[i] = HuWinExCreateFrame(lbl_1_rodata_40, lbl_1_rodata_44, 544, 68, -1, lbl_1_data_0[i]);
        HuWinAttrSet(lbl_1_bss_2E[i], 2048);
    }

    lbl_1_bss_24.x = lbl_1_rodata_48;
    lbl_1_bss_24.y = lbl_1_rodata_4C;
    fn_1_45D4(10, lbl_1_bss_24.x);
    fn_1_45D4(11, lbl_1_bss_24.y);

    messageWinId = HuWinCreate(lbl_1_rodata_50, lbl_1_rodata_54, 544, 42, 0);
    HuWinAttrSet(messageWinId, 2048);
    HuWinPriSet(messageWinId, 100);
    HuWinBGTPLvlSet(messageWinId, lbl_1_rodata_14);
    HuWinMesSpeedSet(messageWinId, 0);
    HuWinMesSet(messageWinId, 65541);
    HuWinDispOff(messageWinId);

    Hu3DGLightPosSet(lbl_1_bss_1DE[0], lbl_1_rodata_58, lbl_1_rodata_5C, lbl_1_rodata_5C,
                     lbl_1_rodata_60, lbl_1_rodata_64, lbl_1_rodata_64);
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

    while (lbl_1_bss_20 > lbl_1_rodata_68) {
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

    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_6C, lbl_1_rodata_70, lbl_1_rodata_74, lbl_1_rodata_78);
    Hu3DCameraViewportSet(1, lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_7C, lbl_1_rodata_80,
                         lbl_1_rodata_14, lbl_1_rodata_38);
    position.x = position.y = position.z = lbl_1_rodata_14;
    target.x = lbl_1_rodata_14;
    target.y = lbl_1_rodata_14;
    target.z = lbl_1_rodata_14;
    fn_1_4ECC(&position, &target, lbl_1_rodata_84);
}

void fn_1_192C(s16 layerNo)
{
    HuVecF position;
    HuVecF target;

    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_2C, lbl_1_rodata_70, lbl_1_rodata_88, lbl_1_rodata_78);
    Hu3DCameraViewportSet(1, lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_7C, lbl_1_rodata_80,
                         lbl_1_rodata_14, lbl_1_rodata_38);
    position.x = position.y = position.z = lbl_1_rodata_14;
    target.x = target.y = target.z = lbl_1_rodata_14;
    fn_1_4ECC(&position, &target, lbl_1_rodata_8C);

    switch (lbl_1_bss_0) {
        case 0:
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[0]);
            lbl_1_bss_10 = lbl_1_rodata_14;
            break;
        case 1:
            Hu3DFbCopyExec(0, 80, 640, 320, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[1]);
            break;
        case 2:
            lbl_1_bss_10 += lbl_1_rodata_90;
            if (lbl_1_bss_10 > lbl_1_rodata_94) {
                lbl_1_bss_10 = lbl_1_rodata_94;
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
    Hu3DAnimKill(lbl_1_bss_78[0]);
    Hu3DAnimKill(lbl_1_bss_78[1]);
    HuSprAnimKill(lbl_1_bss_1B8[0]);
    HuSprAnimKill(lbl_1_bss_1B8[1]);
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

    focusValue = lbl_1_rodata_78;
    phase = 1;
    for (frame = 1, toggleCounter = 0; frame <= frameCount; frame++) {
        object->focus38 = focusValue;
        object->focus34 = focusValue;

        if (toggleCounter++ > 5) {
            phase ^= 1;
            if (lbl_1_bss_C != 0) {
                focusValue = lbl_1_rodata_38;
            } else {
                focusValue = (lbl_1_rodata_38 == focusValue) ? lbl_1_rodata_78 : lbl_1_rodata_38;
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
    object->focus34 = object->focus38 = lbl_1_rodata_38;
}

void fn_1_257C(u32 frameCount)
{
    HSF_OBJECT *background;
    HSF_OBJECT *object;
    float phase;
    float amplitude;
    s16 i;

    phase = lbl_1_rodata_14;
    amplitude = lbl_1_rodata_E8;
    background = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_355);

    for (i = 1; ; i++) {
        object = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[10]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude *
                                   sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);

        object = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_2F0[11]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude *
                                   cos(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);

        phase += lbl_1_rodata_EC;
        if (i > frameCount - 10) {
            amplitude -= lbl_1_rodata_38;
            if (amplitude < lbl_1_rodata_14) {
                amplitude = lbl_1_rodata_14;
            }
        }
        HuPrcVSleep();
    }
}

void fn_1_37F0(void)
{
    HuVecF position;
    float phase;

    phase = lbl_1_rodata_14;
    while (TRUE) {
        position = lbl_1_bss_14;
        position.x += lbl_1_rodata_140 *
                      sin(lbl_1_rodata_98 * (phase * lbl_1_rodata_1C) / lbl_1_rodata_A8);
        position.y += lbl_1_rodata_148 *
                      sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);
        Hu3DModelPosSetV(lbl_1_bss_1CE[5], &position);
        Hu3DModelPosSetV(lbl_1_bss_1CE[6], &position);
        phase += lbl_1_rodata_38;
        if (phase > lbl_1_rodata_150) {
            phase -= lbl_1_rodata_150;
        }
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
    object->mesh.base.pos.x = lbl_1_rodata_188 * sin(lbl_1_rodata_98 * distance / lbl_1_rodata_A8);
    object->mesh.base.pos.y = lbl_1_rodata_188 * cos(lbl_1_rodata_98 * distance / lbl_1_rodata_A8);
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
    phase = lbl_1_rodata_14;
    do {
        do {
            HuPrcVSleep();
        } while ((model->attr & HU3D_ATTR_DISPOFF) != 0);

        PSVECAdd(&lbl_1_bss_54, &lbl_1_bss_48, &model->pos);
        lbl_1_bss_48.x = lbl_1_bss_38 *
                         (lbl_1_rodata_148 * sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8));
        lbl_1_bss_48.y = lbl_1_bss_38 *
                         (lbl_1_rodata_C8 *
                          cos(lbl_1_rodata_98 * (lbl_1_rodata_34 * phase) / lbl_1_rodata_A8));
        model->rot = lbl_1_bss_3C;
        phase += lbl_1_rodata_38;
        if (phase > lbl_1_rodata_150) {
            phase -= lbl_1_rodata_150;
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
                  (sin(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
                   cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8)));
    position.y = target->y +
                 (distance * -sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8));
    position.z = target->z +
                 (distance *
                  (cos(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
                   cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8)));
    cameraTarget.x = target->x;
    cameraTarget.y = target->y;
    cameraTarget.z = target->z;
    up.x = sin(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
           sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8);
    up.y = cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8);
    up.z = cos(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
           sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8);

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
    GXColor color1 = lbl_1_rodata_1C0;
    GXColor color2 = lbl_1_rodata_1C4;

    C_MTXOrtho(projection, lbl_1_rodata_14, lbl_1_rodata_80, lbl_1_rodata_14,
               lbl_1_rodata_7C, lbl_1_rodata_14, lbl_1_rodata_74);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    color1.a = lbl_1_rodata_1C8 * alpha;
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
    GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, lbl_1_rodata_14, lbl_1_rodata_14,
                    lbl_1_rodata_14, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(lbl_1_rodata_14, lbl_1_rodata_154);
    GXTexCoord2f32(lbl_1_rodata_14, lbl_1_rodata_14);
    GXPosition2f32(lbl_1_rodata_7C, lbl_1_rodata_154);
    GXTexCoord2f32(lbl_1_rodata_38, lbl_1_rodata_14);
    GXPosition2f32(lbl_1_rodata_7C, lbl_1_rodata_1CC);
    GXTexCoord2f32(lbl_1_rodata_38, lbl_1_rodata_38);
    GXPosition2f32(lbl_1_rodata_14, lbl_1_rodata_1CC);
    GXTexCoord2f32(lbl_1_rodata_14, lbl_1_rodata_38);
    GXEnd();
}
