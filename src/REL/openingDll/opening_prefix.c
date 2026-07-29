/* This owner does not carry math.h's weak sqrtf constant pool. */
#define _MATH_H

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
extern char lbl_1_data_338[0xE];
extern char lbl_1_data_346[0xF];
extern char lbl_1_data_355[0x14];
extern char lbl_1_data_369[0x27];

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

void fn_1_A0(void)
{
    OSReport(lbl_1_data_369);
    lbl_1_bss_1E4 = omInitObjMan(0x32, 0x2000);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_6C, lbl_1_rodata_70, lbl_1_rodata_88,
                             lbl_1_rodata_78);
    Hu3DCameraViewportSet(1, lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_7C,
                          lbl_1_rodata_80, lbl_1_rodata_14, lbl_1_rodata_38);
    HuPrcCreate(fn_1_28C, 0x64, 0x3000, 0);

    lbl_1_bss_1DE[0] = Hu3DGLightCreate(lbl_1_rodata_14, lbl_1_rodata_E8,
                                        lbl_1_rodata_5C, lbl_1_rodata_14,
                                        lbl_1_rodata_1D0, lbl_1_rodata_64,
                                        0x40, 0x40, 0x60);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[0]);
    lbl_1_bss_1DE[1] = Hu3DGLightCreate(lbl_1_rodata_14, lbl_1_rodata_1D4,
                                        lbl_1_rodata_1D8, lbl_1_rodata_14,
                                        lbl_1_rodata_64, lbl_1_rodata_1DC,
                                        0xA0, 0xA0, 0xA0);
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

    lbl_1_bss_1CE[0] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D50016, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[0], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[0], lbl_1_rodata_10);
    Hu3DModelPosSet(lbl_1_bss_1CE[0], lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_18);

    lbl_1_bss_1CE[1] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D50017, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[1], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[1], lbl_1_rodata_1C);
    Hu3DMotionClusterSpeedSet(lbl_1_bss_1CE[1], 0, lbl_1_rodata_1C);
    Hu3DModelPosSet(lbl_1_bss_1CE[1], lbl_1_rodata_14, lbl_1_rodata_14, lbl_1_rodata_20);
    Hu3DModelRotSet(lbl_1_bss_1CE[1], lbl_1_rodata_24, lbl_1_rodata_28, lbl_1_rodata_2C);

    lbl_1_bss_1CE[2] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D50018, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[2], 0);
    Hu3DModelPosSet(lbl_1_bss_1CE[2], lbl_1_rodata_14, lbl_1_rodata_30, lbl_1_rodata_14);

    lbl_1_bss_1CE[3] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D50019, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[3], 1);

    lbl_1_bss_1CE[4] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D5001A, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[4], 1);

    lbl_1_bss_1CE[5] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D5001C, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[5], 1);

    lbl_1_bss_1CE[6] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D5001D, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[6], 1);
    Hu3DModelAttrSet(lbl_1_bss_1CE[6], 0x40000001);

    lbl_1_bss_1CE[7] = Hu3DModelCreate(HuDataSelHeapReadNum(0x00D5001B, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_1CE[7], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_1CE[7], lbl_1_rodata_34);
    Hu3DModelScaleSet(lbl_1_bss_1CE[7], lbl_1_rodata_34, lbl_1_rodata_34, lbl_1_rodata_34);

    temp = HuMemDirectMallocNum(HEAP_DVD, 0x100000, HU_MEMNUM_OVL);
    lbl_1_bss_1C4[0] = HuMemDirectMallocNum(
        HEAP_DVD,
        GXGetTexBufferSize(0x280, 0x140, GX_TF_RGBA8, FALSE, 0),
        HU_MEMNUM_OVL);
    HuMemDirectFree(temp);
    lbl_1_bss_1C4[1] = HuMemDirectMallocNum(
        HEAP_HEAP,
        GXGetTexBufferSize(0x280, 0x140, GX_TF_RGBA8, FALSE, 0),
        HU_MEMNUM_OVL);

    lbl_1_bss_1B8[0] = HuSprAnimMake(0x280, 0x140, 0);
    lbl_1_bss_1B8[0]->bmp->data = lbl_1_bss_1C4[0];
    lbl_1_bss_1B8[1] = HuSprAnimMake(0x280, 0x140, 0);
    lbl_1_bss_1B8[1]->bmp->data = lbl_1_bss_1C4[1];
    lbl_1_bss_78[0] = Hu3DAnimCreate(lbl_1_bss_1B8[0], lbl_1_bss_1CE[1], lbl_1_data_338);
    lbl_1_bss_78[1] = Hu3DAnimCreate(lbl_1_bss_1B8[1], lbl_1_bss_1CE[1], lbl_1_data_346);

    lbl_1_bss_1C0 = HuMemDirectMallocNum(
        HEAP_HEAP,
        GXGetTexBufferSize(0x280, 0x140, GX_TF_I8, FALSE, 0),
        HU_MEMNUM_OVL);

    for (i = 0; i < 79; i++) {
        lbl_1_bss_7C[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_8[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }

    for (i = 0; i < 12; i++) {
        lbl_1_bss_60[i] = Hu3DAnimCreate(lbl_1_bss_7C[0], lbl_1_bss_1CE[2], lbl_1_data_1DC[i]);
    }

    HuPrcChildCreate(fn_1_4C30, 0x100, 0x1000, 0, HuPrcCurrentGet());

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
    Hu3DModelAttrSet(lbl_1_bss_1CE[1], 0x40000002);
    Hu3DMotionClusterTimeSet(lbl_1_bss_1CE[1], 0, lbl_1_rodata_14);
    Hu3DModelClusterAttrSet(lbl_1_bss_1CE[1], 0, 0xC0000002);

    PSMTXTrans(matrix, lbl_1_rodata_3C, lbl_1_rodata_14, lbl_1_rodata_14);
    Hu3DModelMtxSet(lbl_1_bss_1CE[1], &matrix);

    fn_1_46B4(0, 0);
    fn_1_46B4(1, 7);
    fn_1_46B4(2, 0x0E);
    fn_1_46B4(3, 0x15);
    fn_1_46B4(4, 0x1C);
    fn_1_46B4(5, 0x23);
    fn_1_46B4(6, 0x2A);
    fn_1_46B4(7, 0x31);
    fn_1_46B4(8, 0x38);
    fn_1_46B4(9, 0x3F);
    fn_1_46B4(10, 0x46);
    fn_1_46B4(11, 0x4A);

    for (i = 0; i < 4; i++) {
        lbl_1_bss_2E[i] = HuWinExCreateFrame(lbl_1_rodata_40, lbl_1_rodata_44, 0x220, 0x44, -1, lbl_1_data_0[i]);
        HuWinAttrSet(lbl_1_bss_2E[i], 0x800);
    }

    lbl_1_bss_24.x = lbl_1_rodata_48;
    lbl_1_bss_24.y = lbl_1_rodata_4C;
    fn_1_45D4(10, lbl_1_bss_24.x);
    fn_1_45D4(11, lbl_1_bss_24.y);

    messageWinId = HuWinCreate(lbl_1_rodata_50, lbl_1_rodata_54, 0x220, 0x2A, 0);
    HuWinAttrSet(messageWinId, 0x800);
    HuWinPriSet(messageWinId, 100);
    HuWinBGTPLvlSet(messageWinId, lbl_1_rodata_14);
    HuWinMesSpeedSet(messageWinId, 0);
    HuWinMesSet(messageWinId, 0x10005);
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
    eventProcess = HuPrcChildCreate(fn_1_1128, 0x100, 0x3000, 0, HuPrcCurrentGet());

    frame = 0;
    while (lbl_1_bss_4 == 0) {
        if (enablePrompt != 0) {
            if (frame > 0xFA && omcurovl != 1) {
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
    fn_1_4E34((void (*)(void))fn_1_257C, 0x41A, 0);
    fn_1_4E34((void (*)(void))fn_1_4068, 0x3DE, 0);
    fn_1_4E34((void (*)(void))fn_1_2238, 0x3F2, 0);
    fn_1_4E34(fn_1_37F0, 0, 0);
    fn_1_48AC();

    HuPrcSleep(60);
    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 0x000B0000, 0xF0);
    fn_1_47AC(0, 0x000B0001, 0xF0);
    fn_1_47AC(0, 0x000B0002, 0xF0);
    fn_1_47AC(0, 0x000B0003, 0x50);
    fn_1_4E34((void (*)(void))fn_1_2304, 10, 60);
    fn_1_4E34(fn_1_41B4, 0, 0);
    HuPrcSleep(0xA0);

    while (lbl_1_bss_20 > lbl_1_rodata_68) {
        HuPrcVSleep();
    }

    lbl_1_bss_2C = 2;
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800B, 60);
    fn_1_4E34(fn_1_420C, 0, 0);
    fn_1_47AC(0, 0x000B0004, 0x50);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800A, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 0x000B0005, 70);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800B, 40);
    fn_1_4E34(fn_1_420C, 0, 0);

    lbl_1_bss_2C = 4;
    fn_1_47AC(0, 0x000B0006, 60);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800A, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 0x000B0007, 60);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800B, 40);
    fn_1_4E34(fn_1_420C, 0, 0);

    lbl_1_bss_2C = 4;
    fn_1_47AC(0, 0x000B0006, 50);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800A, 40);
    fn_1_4E34(fn_1_41B4, 0, 0);

    lbl_1_bss_2C = 3;
    fn_1_47AC(0, 0x000B0007, 50);

    lbl_1_bss_2C = 5;
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800A, 10000);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800B, 10000);
    fn_1_4E34((void (*)(void))fn_1_4264, 600, 0);
    fn_1_47AC(0, 0x000B0008, 0xF0);
    fn_1_47AC(0, 0x000B0009, 0xF0);
    HuPrcSleep(120);

    HuWinExClose(lbl_1_bss_2E[0]);
    lbl_1_bss_C = 1;
    lbl_1_bss_8 = 1;
    fn_1_4E34((void (*)(void))fn_1_2708, 20, 0);
    fn_1_470C(0, 4);
    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 0x000B000A, 0);
    HuPrcSleep(20);

    fn_1_470C(1, 0x0B);
    fn_1_470C(2, 0x12);
    fn_1_470C(3, 0x19);
    fn_1_470C(4, 0x20);
    fn_1_470C(5, 0x27);
    fn_1_470C(6, 0x2E);
    fn_1_470C(7, 0x35);
    HuPrcSleep(30);
    fn_1_470C(8, 0x39);
    fn_1_470C(9, 0x40);
    HuPrcSleep(60);

    fn_1_4E34((void (*)(void))fn_1_2B9C, 20, 0);
    HuPrcSleep(30);
    fn_1_47AC(0, 0x000B000B, 10);
    fn_1_4E34((void (*)(void))fn_1_2D74, 30, 0);
    HuPrcSleep(60);

    lbl_1_bss_C = lbl_1_bss_8 = 0;
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800A, 20);
    fn_1_4E34((void (*)(void))fn_1_2304, 0x800B, 20);
    HuPrcSleep(0xAA);

    fn_1_47AC(0, 0x000B000C, 0xF0);
    fn_1_47AC(0, 0x000B000D, 0xF0);
    fn_1_47AC(0, 0x000B000E, 40);
    fn_1_4E34((void (*)(void))fn_1_4484, 200, 0);
    HuPrcSleep(200);
    fn_1_4E34((void (*)(void))fn_1_34EC, 20, 0);

    HuWinExClose(lbl_1_bss_2E[0]);
    lbl_1_bss_0 = 1;
    Hu3DModelAttrReset(lbl_1_bss_1CE[1], 0x40000002);
    Hu3DModelClusterAttrReset(lbl_1_bss_1CE[1], 0, 0xC0000002);
    HuPrcSleep(10);
    fn_1_46B4(10, 0x48);
    fn_1_46B4(11, 0x4C);
    fn_1_4E34((void (*)(void))fn_1_4484, 0x186, 0);
    HuPrcSleep(0x8C);

    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_4E34((void (*)(void))fn_1_392C, 30, 0);
    fn_1_47AC(0, 0x000B000F, 0xF0);
    lbl_1_bss_0 = 2;
    HuWinExClose(lbl_1_bss_2E[0]);
    HuPrcSleep(0x82);

    HuWinExOpen(lbl_1_bss_2E[0]);
    fn_1_47AC(0, 0x000B0010, 0x12C);
    HuWinExClose(lbl_1_bss_2E[0]);
    fn_1_4E34(fn_1_3C28, 0, 0);
    HuPrcSleep(0x8C);

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
            Hu3DFbCopyExec(0, 0x50, 0x280, 0x140, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[0]);
            lbl_1_bss_10 = lbl_1_rodata_14;
            break;
        case 1:
            Hu3DFbCopyExec(0, 0x50, 0x280, 0x140, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[1]);
            break;
        case 2:
            lbl_1_bss_10 += lbl_1_rodata_90;
            if (lbl_1_bss_10 > lbl_1_rodata_94) {
                lbl_1_bss_10 = lbl_1_rodata_94;
            }
            Hu3DFbCopyExec(0, 0x50, 0x280, 0x140, GX_TF_I8, FALSE, lbl_1_bss_1C0);
            fn_1_517C(lbl_1_bss_10);
            Hu3DFbCopyExec(0, 0x50, 0x280, 0x140, GX_TF_RGBA8, FALSE, lbl_1_bss_1C4[1]);
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
