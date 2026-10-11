/* File Select overlay startup, save-slot presentation, and slot selection. */
#include "dolphin.h"
#include "REL/fileseldll.h"
#include "datanum/filesel.h"
#include "game/card.h"
#include "game/process.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/frand.h"
#include "game/pad.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/main.h"
#include "game/saveload_layout.h"
#include "game/wipe.h"
#include "messnum/file_select.h"
#include "msm_se.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

typedef void (*VoidFunc)(void);

enum {
    FILESEL_OUTVIEW_OBJECT_PRIORITY = 32730,
    FILESEL_SLOT_SPRITE_PRIORITY = 95,
    FILESEL_DECIMAL_DIGIT_MASK = 0xF,
    FILESEL_SUBSTICK_STEP_MASK = 0xF8,
};

/* Point coordinates plus an associated 16-bit flag from the File Select point table. */
typedef struct b234_point {
    /* Horizontal coordinate in the point table. */
    s16 x;
    /* Vertical coordinate in the point table. */
    s16 y;
    /* Point attribute flags; the meaning of individual bits is not established here. */
    u16 flag;
} B234Point;
/* Coordinate bounds passed to the keyboard navigation helper. */
typedef struct b234_rect {
    /* Minimum horizontal coordinate. */
    f32 minX;
    /* Minimum vertical coordinate. */
    f32 minY;
    /* Maximum horizontal coordinate. */
    f32 maxX;
    /* Maximum vertical coordinate. */
    f32 maxY;
} B234Rect;

/* Per-model distortion texture matrix, tint, offset, and remaining effect time. */
typedef struct bss18_entry {
    /* First row of the indirect texture matrix. */
    Vec indirectMatrixRow0;
    /* Second row of the indirect texture matrix. */
    Vec indirectMatrixRow1;
    /* Tint applied to the card distortion model. */
    GXColor tint;
    /* Current vertical texture offset. */
    f32 textureOffset;
    /* Texture offset increment applied on each material-hook invocation. */
    f32 textureOffsetStep;
    /* Remaining lifetime in frames. */
    f32 framesRemaining;
} BSS18_ENTRY; /* size = 0x28 */

/* Five particle emission points on the selected slot model. */
typedef struct vec5 {
    /* Five local-space particle emission offsets. */
    Vec v[5];
} VEC5;

/* Sprite asset and placement settings used to build the name keyboard. */
typedef struct data48c {
    /* File Select sprite data number. */
    s32 dataNumber;
    /* Sprite animation bank. */
    u8 bank;
    /* Sprite draw priority. */
    s16 priority;
    /* Horizontal offset in pixels from the keyboard sprite group's origin. */
    s16 x;
    /* Vertical offset in pixels from the keyboard sprite group's origin. */
    s16 y;
} DATA48C; /* size 0xC */

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern CARDFileInfo curFileInfo;

extern OMOBJ *lbl_1_bss_8;
extern void *lbl_1_bss_C;
extern ANIMDATA *lbl_1_bss_10;
extern ANIMDATA *lbl_1_bss_14;
extern BSS18_ENTRY lbl_1_bss_18[];
extern HU3D_PARMANID lbl_1_bss_98[];
extern HU3D_ANIMID lbl_1_bss_A2[];
extern ANIMDATA *lbl_1_bss_C8;
extern ANIMDATA *lbl_1_bss_CC[];
extern HU3D_MODELID lbl_1_bss_2D8[];
extern OMOBJMAN *lbl_1_bss_340;
extern OMOBJ *lbl_1_bss_344;
extern B234Point *lbl_1_bss_348;
extern char lbl_1_bss_34C[];
extern s16 lbl_1_bss_3AC[];
extern u8 lbl_1_bss_3B2[][FILESEL_FILENAME_LENGTH];
extern s16 lbl_1_bss_42A;
extern s16 lbl_1_bss_42E;
extern Vec lbl_1_bss_68[];

extern OM_CAMERA_VIEW lbl_1_data_0;
extern OM_CAMERA_VIEW lbl_1_data_1C;
extern HU3D_PARMAN_PARAM lbl_1_data_E8;
extern char lbl_1_data_138[];
extern char lbl_1_data_144[];
extern char lbl_1_data_1FD[23];
extern DATA48C lbl_1_data_48C[];
extern char lbl_1_data_B8[];
extern char lbl_1_data_E2[];

extern char lbl_1_data_1F3[];

extern Vec lbl_1_bss_2E4[];
extern ANIMDATA *lbl_1_bss_A8[];
extern s32 lbl_1_bss_0[2];

extern char *lbl_1_data_88[];
extern Vec lbl_1_data_154[];
extern char *lbl_1_data_190[];
extern char lbl_1_data_1E6[];
extern char lbl_1_data_1EB[];
extern char lbl_1_data_1F0[];

extern s16 lbl_1_bss_37E;
extern u16 lbl_1_bss_380[];
extern s16 lbl_1_bss_42C;
extern B234Point *lbl_1_data_47C[];
extern int lbl_1_data_98[];
extern char lbl_1_data_1A0[];
extern char lbl_1_data_1AD[];
extern char lbl_1_data_1BB[];
extern char lbl_1_data_1C8[];
extern char lbl_1_data_1DD[];

extern s16 curSlotNo;
extern s16 SLWinId;
extern u8 saveBuf[][SAVE_BUF_SIZE];
extern s64 SLSerialNo[];

s32 SLSaveFlagGet(void);
void SLSaveFlagSet(s32 flag);
void SLSaveDataMake(s32 initializationMode, OSTime *time);
void SLSaveEmptySet(s16 slot, s16 idx);
s32 SLBoxDataOffsetGet(s16 boxNo);
void SLCommonLoad(void);
void SLCommonSet(void);

s32 HuMCProbe(s32 chan);
void HuMCMicSet(s32 flag);
void HuMCInit(s32 chan);
s32 HuMCMount(s32 chan);
void HuMCClose(void);
s32 HuMCMicGet(void);

void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, s32 wrapS, s32 wrapT, s32 filter);

void ObjectSetup(void);
void FileselMain(void);
void fn_1_680(void);
void fn_1_6E8(void);
void fn_1_C60(void);
void fn_1_C8C(void);
void fn_1_179C(void);
void fn_1_2654(s16 slotSelection);
void fn_1_2A40(void);
void fn_1_35AC(void);
void fn_1_3C80(s16 boxNo, Vec *position);
void fn_1_3CC0(s16 boxNo, f32 x, f32 y, f32 z);
void fn_1_3CF4(s16 boxNo, s32 motionMode);
void fn_1_50DC(void (*func)(void), s32 processProperty);
void fn_1_5130(void);
void fn_1_6314(s16 boxNo, s32 active);
void fn_1_63A0(s16 boxNo, s32 active);
s32 FileselEraseConfirm(s16 boxNo);
s32 fn_1_69B0(void);
void FileselWipeDraw(void);
void fn_1_7B64(void);
void fn_1_7A0C(void);
void fn_1_7DEC(void);
void fn_1_7E58(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_82F0(OMOBJ *obj);
void fn_1_8510(s16 effectIndex, Vec *position);
void fn_1_8688(OMOBJ *p);
void fn_1_8A34(void);
void fn_1_8F34(s32 callbackData, s16 selectedKeyIndex);
s16 fn_1_AAEC(s16 winId);
s16 fn_1_ABF8(s16 currentKeyIndex, f32 originX, f32 originY, f32 directionX, f32 directionY);
void fn_1_B234(B234Rect *r);
void NameEnterClose(void);
void fn_1_3D14(void);
s32 FileselCopy(s16 sourceBoxNo);

void FileselInit(void);
void fn_1_15AC(void);
void fn_1_36C4(s16 boxNo, s16 slotNumber);
void fn_1_6E54(s16 selectedBoxNo, s32 animateSelectedSlot);
void NameEnterInit(void);
void fn_1_CB4(s32 showSlots);
s32 FileselSelect(void);

/* ======================================================================== */

/* Overlay entry: build the object manager and hand off to File Select. */
int _prolog(void)
{
    const VoidFunc *ctor;

    for (ctor = _ctors; *ctor != 0; ctor++) {
        (*ctor)();
    }

    ObjectSetup();
    return 0;
}

/* Overlay shutdown: run the registered destructors before unloading. */
void _epilog(void)
{
    const VoidFunc *dtor;

    for (dtor = _dtors; *dtor != 0; dtor++) {
        (*dtor)();
    }
}

/* 0x00A0 */
/* Called by _prolog to create File Select cameras, windows, and main process. */
void ObjectSetup(void)
{
    HU3D_LIGHTID lightId;
    s16 lang;

    OSReport(lbl_1_data_B8);
    lbl_1_bss_340 = omInitObjMan(50, 512);
    omGameSysInit(lbl_1_bss_340);
    lang = GwLanguage;
    GwCommon.languageNo = lang;
    lightId = (int)Hu3DGLightCreate(-1000.0f, 1500.0f, 1000.0f,
                                    1.0f, -1.5f, -1.0f,
                                    255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_344 =
        omAddObjEx(lbl_1_bss_340, FILESEL_OUTVIEW_OBJECT_PRIORITY, 0, 0, -1, omOutViewMulti);
    lbl_1_bss_344->work[0] = 2;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 5.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f,
                          480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(2, 10.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f,
                          480.0f, 0.0f, 1.0f);
    omCameraViewSetMulti(1, &lbl_1_data_0);
    omCameraViewSetMulti(2, &lbl_1_data_1C);
    HuPrcChildCreate(FileselMain, 256, 12288, 0, HuPrcCurrentGet());
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
    HuWinMesLanguageSet(GwLanguage);
    HuWinMesRead();
    HuAudSStreamPlay(MSM_STREAM_TITLE);
    OSReport(lbl_1_data_E2, SAVE_BOX_SIZE);
}

/* 0x0374 */
/* Main File Select process: load card data, handle the chosen slot, then transition overlays. */
void FileselMain(void)
{
    s32 flag;
    s16 i;
    s32 status;

    flag = 1;
    fn_1_6E8();
    FileselInit();
    SLSaveFlagSet(1);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    status = FileBoxInit(-1);
    if (status == -2) {
        goto L_56C;
    }
    if (status == -1) {
        HuAudFXPlay(MSM_SE_FILESEL_17);
        fn_1_CB4(0);
        GwCommon = GwCommonOrig;
        fn_1_6E54(0, 0);
        SLSaveFlagSet(0);
        flag = 0;
        goto L_56C;
    }
    HuAudFXPlay(MSM_SE_FILESEL_17);
    fn_1_CB4(1);
L_454:
    status = FileselSelect();
    if (status == FILESEL_RESULT_CANCEL) {
        goto L_56C;
    }
    if (status == -3) {
        status = FileBoxInit(-1);
        if (status == -2) {
            goto L_56C;
        }
        if (status == -1) {
            GwCommon = GwCommonOrig;
            fn_1_6E54(0, 0);
            SLSaveFlagSet(0);
            goto L_568;
        }
        for (i = 0; i < 3; i++) {
            fn_1_36C4(i, i);
        }
        goto L_454;
    }
    if (status == -1) {
        GwCommon = GwCommonOrig;
        fn_1_6E54(0, 0);
        SLSaveFlagSet(0);
        flag = 0;
        goto L_56C;
    }
    SLSaveFlagSet(1);
    fn_1_6E54(status, 1);
L_568:
    flag = 0;
L_56C:
    HuDataDirClose(DATA_win);
    HuDataDirClose(DATA_effect);
    HuDataDirClose(DATA_filesel);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    HuAudSStreamAllFadeOut(1000);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    if (SLSaveFlagGet() == 0) {
        fn_1_680();
    }
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    for (i = 0; i < 4; i++) {
        HuMemDirectFree(lbl_1_bss_D8[i].cardTextureAnim);
    }
    if (flag != 0) {
        omOvlReturnEx(1, 1);
    } else {
        if (OSGetSoundMode() == 0) {
            msmSysSetOutputMode(0);
        } else if (GwCommon.outputMode == 2) {
            msmSysSetOutputMode(2);
        } else {
            msmSysSetOutputMode(1);
        }
        omOvlCallEx(DLL_mdseldll, 1, 0, 0);
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* 0x0680 */
/* FileselMain calls this when leaving without saving. Disable microphone mode if channel 1
 * detection or mounting fails; initialization status is ignored before closing speech resources. */
void fn_1_680(void)
{
    if (HuMCProbe(1) != 0) {
        HuMCMicSet(0);
    } else {
        HuMCMicSet(1);
        HuMCInit(0);
        if (HuMCMount(1) != 0) {
            HuMCMicSet(0);
        }
        HuMCClose();
    }
    HuMCMicGet(); /* The current setting is read, but this function does not use the returned
                   * value. */
}

/* 0x06E8 */
/* FileselMain calls this at startup to load the scene, slot art, and effects. */
void fn_1_6E8(void)
{
    Vec camPos;
    Vec camTarget;
    Vec camUp;
    s16 i;
    ANIMDATA *parAnim;

    Hu3DShadowCreate(15.0f, 20.0f, 5000.0f);
    camPos.x = 0.0f;
    camPos.y = 1000.0f;
    camPos.z = 2000.0f;
    camUp.y = 1.0f;
    camUp.x = camUp.z = 0.0f;
    camTarget.x = 0.0f;
    camTarget.y = 50.0f;
    camTarget.z = 0.0f;
    Hu3DShadowPosSet(&camPos, &camUp, &camTarget);
    Hu3DShadowTPLvlSet(0.0f);

    lbl_1_bss_2D8[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_BACKGROUND, HU_MEMNUM_OVL, 2));
    Hu3DModelCameraSet(lbl_1_bss_2D8[1], 2);
    Hu3DModelLayerSet(lbl_1_bss_2D8[1], 1);
    Hu3DMotionSpeedSet(lbl_1_bss_2D8[1], 0.05f);
    Hu3DModelPosSet(lbl_1_bss_2D8[1], 0.0f, 0.0f, -9000.0f);
    Hu3DModelAttrSet(lbl_1_bss_2D8[1], HU3D_MOTATTR_LOOP);

    lbl_1_bss_2D8[0] = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_BOOK, HU_MEMNUM_OVL, 2));
    Hu3DModelCameraSet(lbl_1_bss_2D8[0], 2);
    Hu3DModelLayerSet(lbl_1_bss_2D8[0], 1);
    Hu3DModelAttrSet(lbl_1_bss_2D8[0], HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrSet(lbl_1_bss_2D8[0], 0, HU3D_CLUSTER_ATTR_PAUSE);
    Hu3DModelPosSet(lbl_1_bss_2D8[0], 0.0f, 0.0f, -2000.0f);
    Hu3DModelRotSet(lbl_1_bss_2D8[0], -20.0f, -30.0f, 30.0f);
    Hu3DModelShadowMapSet(lbl_1_bss_2D8[0]);

    lbl_1_bss_2D8[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_DATA_SELECT, HU_MEMNUM_OVL, 2));
    Hu3DModelCameraSet(lbl_1_bss_2D8[2], 2);
    Hu3DModelLayerSet(lbl_1_bss_2D8[2], 1);
    Hu3DModelAttrSet(lbl_1_bss_2D8[2], HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(lbl_1_bss_2D8[2]);

    lbl_1_bss_C8 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_DATA_SELECT, HU_MEMNUM_OVL, 2));
    lbl_1_bss_A2[0] = Hu3DAnimCreate(lbl_1_bss_C8, lbl_1_bss_2D8[2], lbl_1_data_138);
    Hu3DAnimBankSet(lbl_1_bss_A2[0], 1);
    lbl_1_bss_A2[1] = Hu3DAnimCreate(lbl_1_bss_C8, lbl_1_bss_2D8[2], lbl_1_data_144);
    Hu3DAnimBankSet(lbl_1_bss_A2[1], 0);

    HuPrcChildCreate(fn_1_15AC, 256, 4096, 0, HuPrcCurrentGet());
    NameEnterInit();
    Hu3DCameraLayerHookSet(1, 0, (HU3D_LAYER_HOOK)fn_1_C60);
    Hu3DCameraLayerHookSet(2, 0, (HU3D_LAYER_HOOK)fn_1_C8C);
    omAddObjEx(lbl_1_bss_340, 4096, 16, 16, -1, fn_1_8688);

    parAnim = HuSprAnimRead(HuDataSelHeapReadNum(DATA_effect, HU_MEMNUM_OVL, 2));
    for (i = 0; i < 5; i++) {
        lbl_1_bss_98[i] = Hu3DParManCreate(parAnim, 100, &lbl_1_data_E8);
        Hu3DParManAttrSet(lbl_1_bss_98[i],
                         HU3D_PARMAN_ATTR_SCALEJITTER |
                             HU3D_PARMAN_ATTR_RANDSCALE70 |
                             HU3D_PARMAN_ATTR_RANDSPEED70);
        Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 1);
        Hu3DParManRotSet(lbl_1_bss_98[i], 90.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 2);
        Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 4);
        Hu3DParManAttrSet(lbl_1_bss_98[i], 1);
    }
}

/* 0x0C60 */
/* Camera 1 layer hook: draw the full-screen depth polygon with the RGBA framebuffer format. */
void fn_1_C60(void)
{
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    FileselWipeDraw();
}

/* 0x0C8C */
/* Camera 2 layer hook: restore the normal RGB framebuffer format. */
void fn_1_C8C(void)
{
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
}

/* After card initialization, animate the book and reveal three slots only when showSlots is
 * nonzero. The duration uses the fourth slot model after the setup loop. */
void fn_1_CB4(s32 showSlots)
{
    Mtx objMtx;
    Vec rot;
    Vec pos;
    Vec trans;
    Vec rotV;
    f32 t;
    f32 total;
    f32 rate;
    f32 rate2;
    s16 i;
    FILESEL_BOX *w;

    lbl_1_bss_2E0 = HuWinExCreate(16.0f, 360.0f, 544, 80, -1);
    HuWinAttrSet(lbl_1_bss_2E0, HUWIN_ATTR_ALIGN_CENTER);
    HuWinPadMaskSet(lbl_1_bss_2E0, 1);
    for (i = 0; i < 3; i++) {
        fn_1_36C4(i, i);
        fn_1_3CF4(i, 2);
    }
    total = 10.0f + Hu3DMotionMaxTimeGet(lbl_1_bss_D8[i].slotModel);
    rot.x = -20.0f;
    rot.y = -30.0f;
    rot.z = 30.0f;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = -2000.0f;
    HuPrcSleep(10);
    for (t = 0.0f; t < total; t += 1.0f) {
        HuPrcVSleep();
        if (10.0f == t) {
            Hu3DModelAttrReset(lbl_1_bss_2D8[0], HU3D_MOTATTR_PAUSE);
            Hu3DModelClusterAttrReset(lbl_1_bss_2D8[0], 0, HU3D_CLUSTER_ATTR_PAUSE);
        }
        rate = sin(3.141592653589793 * (90.0 * ((1.0 + t) / total)) / 180.0);
        if (t < 0.7 * total) {
            rate2 = sin(3.141592653589793 * (90.0 * ((1.0 + t) / (0.7 * total))) / 180.0);
        } else {
            rate2 = 1.0f;
        }
        Hu3DModelPosSet(lbl_1_bss_2D8[0],
            pos.x + -pos.x * rate,
            pos.y + (50.0f - pos.y) * rate,
            pos.z + -pos.z * rate);
        Hu3DModelRotSet(lbl_1_bss_2D8[0],
            rot.x + (340.0f - rot.x) * rate,
            rot.y + (360.0f - rot.y) * rate2,
            rot.z + -rot.z * rate2);
        if (showSlots != 0) {
            if (t >= 70.0f) {
                if (70.0f == t) {
                    Hu3DModelAttrReset(lbl_1_bss_2D8[2], HU3D_ATTR_DISPOFF);
                    for (i = 0; i < 3; i++) {
                        Hu3DModelAttrReset(lbl_1_bss_D8[i].slotModel, HU3D_ATTR_DISPOFF);
                    }
                }
                rate = sin(3.141592653589793 * (90.0 * ((t - 70.0f) / (total - 70.0f))) / 180.0);
                Hu3DModelObjMtxGet(lbl_1_bss_2D8[0], lbl_1_data_88[0], objMtx);
                Hu3DMtxTransGet(objMtx, &trans);
                Hu3DMtxRotGet(objMtx, &rotV);
                trans.x = trans.x + -260.0f * rate;
                trans.y = trans.y + -50.0f * rate;
                trans.z = trans.z + 500.0 * sin(3.141592653589793 *
                                                (180.0 * ((t - 70.0f) / (total - 70.0f))) / 180.0);
                Hu3DModelPosSetV(lbl_1_bss_2D8[2], &trans);
                rotV.x = rotV.x * (1.0 - rate);
                rotV.y = rotV.y * (1.0 - rate);
                rotV.z = rotV.z * (1.0 - rate);
                Hu3DModelRotSetV(lbl_1_bss_2D8[2], &rotV);
                Hu3DModelScaleSet(lbl_1_bss_2D8[2], 1.2f, 1.2f, 1.2f);
                for (i = 0; i < 3; i++) {
                    w = &lbl_1_bss_D8[i];
                    Hu3DModelObjMtxGet(lbl_1_bss_2D8[0], lbl_1_data_88[i + 1], objMtx);
                    Hu3DMtxTransGet(objMtx, &trans);
                    Hu3DMtxRotGet(objMtx, &rotV);
                    trans.x = trans.x + rate * (lbl_1_data_154[i].x - trans.x);
                    trans.y = trans.y + rate * (lbl_1_data_154[i].y - trans.y);
                    trans.z = trans.z + rate * (lbl_1_data_154[i].z - trans.z);
                    lbl_1_bss_2E4[i] = trans;
                    fn_1_3C80(i, &trans);
                    rotV.x = rotV.x * (1.0 - rate);
                    rotV.y = rotV.y * (1.0 - rate);
                    rotV.z = rotV.z * (1.0 - rate);
                    Hu3DModelRotSetV(w->slotModel, &rotV);
                    Hu3DModelScaleSet(w->slotModel, 1.5f, 1.5f, 1.5f);
                }
            }
            if (t > total - 30.0f) {
                rate = (t - (total - 30.0f)) / 30.0f;
                Hu3DShadowTPLvlSet(0.5f * rate);
            }
            if (t == total - 10.0f) {
                fn_1_50DC(fn_1_179C, 0);
            }
        }
    }
    if (showSlots != 0) {
        lbl_1_bss_2DE = HuWinCreate(16.0f, 326.0f, 544, 42, 0);
        HuWinAttrSet(lbl_1_bss_2DE, HUWIN_ATTR_ALIGN_CENTER);
        HuWinPriSet(lbl_1_bss_2DE, 100);
        HuWinBGTPLvlSet(lbl_1_bss_2DE, 0.0f);
        HuWinMesSpeedSet(lbl_1_bss_2DE, 0);
    }
}

/* 0x15AC */
/* Child process from scene setup: gently animate the book and data-select model. */
void fn_1_15AC(void)
{
    Mtx mtx;
    f32 verticalPhase;
    f32 rotationPhase;

    verticalPhase = 0.0f;
    rotationPhase = 45.0f;
    while (1) {
        verticalPhase += 2.0f;
        if (verticalPhase > 360.0f) {
            verticalPhase -= 360.0f;
        }
        rotationPhase += 1.1;
        if (rotationPhase > 360.0f) {
            rotationPhase -= 360.0f;
        }
        PSMTXTrans(mtx, 0.0f, (f32)(3.0 * sin(3.141592653589793 * verticalPhase / 180.0)), 0.0f);
        mtxRotCat(mtx, 0.0f, 0.0f, (f32)sin(3.141592653589793 * rotationPhase / 180.0));
        Hu3DModelMtxSet(lbl_1_bss_2D8[0], &mtx);
        PSMTXTrans(mtx, (f32) sin(3.141592653589793 * verticalPhase / 180.0),
                   (f32) (3.0 * sin(3.141592653589793 * rotationPhase / 180.0)), 0.0f);
        Hu3DModelMtxSet(lbl_1_bss_2D8[2], &mtx);
        HuPrcVSleep();
    }
}

/* 0x179C */
/* Delayed child process from the reveal animation: open the main message window. */
void fn_1_179C(void)
{
    HuWinExOpen(lbl_1_bss_2E0);
    HuPrcEnd();
}

/* Called by FileselMain for each selection pass; handles slot navigation and actions. */
s32 FileselSelect(void)
{
    s16 sel;
    s16 i;
    s16 j;
    s16 count;
    s16 dir;
    s32 ret;
    s32 fxId;
    FILESEL_BOX *w;
    Vec scale;
    Vec sv[3];
    OSTime time;
    f32 t;

    sel = 0;
    for (i = 0; i < 3; i++) {
        sv[i] = lbl_1_bss_2E4[i];
    }
    Hu3DModelScaleGet(lbl_1_bss_D8[0].slotModel, &scale);
    if (scale.x < 0.01f) {
        for (i = 1; i <= 5; i++) {
            t = (f32)i / 5.0f;
            for (j = 0; j < 3; j++) {
                w = &lbl_1_bss_D8[j];
                Hu3DModelScaleSet(w->slotModel, 1.5f * t, 1.5f * t, 1.5f * t);
            }
            HuPrcVSleep();
        }
    }
L_192C:
    for (i = count = 0; i < 3; i++) {
        if (lbl_1_bss_D8[i].hasSave != 0) {
            count++;
        }
        if (sel == i) {
            fn_1_3CF4(i, 0);
        } else {
            fn_1_3CF4(i, 1);
        }
    }
    Hu3DModelScaleGet(lbl_1_bss_D8[sel].slotModel, &scale);
    if (scale.x < 2.0f) {
        for (i = 1; i <= 10; i++) {
            w = &lbl_1_bss_D8[sel];
            t = (f32)i / 10.0f;
            fn_1_3CC0(sel, sv[sel].x, sv[sel].y, 20.0f * t + sv[sel].z);
            t = (f32) (sin(3.141592653589793 * (130.0f * t) / 180.0) *
                       (1.0 / sin(2.2689280275926285)));
            Hu3DModelScaleSet(w->slotModel,
                (f32)(1.5 + 0.5 * t),
                (f32)(1.5 + 0.5 * t),
                (f32)(1.5 + 0.5 * t));
            if (i == 5) {
                w->visualState = 1;
            }
            HuPrcVSleep();
        }
    }
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_SELECT_FILE);
    HuWinMesWait(lbl_1_bss_2E0);
    HuWinMesSet(lbl_1_bss_2DE, MESSNUM(MESS_SYS_GUIDE, 4));
    HuWinDispOn(lbl_1_bss_2DE);
    for (;;) {
        HuPrcVSleep();
        if (UnMountCnt != 0) {
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
            HuWinMesWait(lbl_1_bss_2E0);
            UnMountCnt = 0;
            goto L_241C;
        }
        dir = 0;
        if (HuPadBtnDown[0] & (PAD_BUTTON_A | PAD_BUTTON_START)) {
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinDispOff(lbl_1_bss_2DE);
            SLCurBoxNoSet(sel);
            if (lbl_1_bss_D8[sel].hasSave != 0) {
                if (count >= 3) {
                    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_PLAY_ERASE);
                } else {
                    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_PLAY_COPY_ERASE);
                }
                HuWinMesWait(lbl_1_bss_2E0);
                ret = HuWinChoiceGet(lbl_1_bss_2E0, -1);
                if (UnMountCnt != 0) {
                    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
                    HuWinMesWait(lbl_1_bss_2E0);
                    UnMountCnt = 0;
                    goto L_241C;
                }
                if (ret == -1) {
                    goto L_192C;
                }
                if (ret == 0) {
                    SLCommonLoad();
                    goto L_2414;
                }
                if ((count >= 3 && ret == 1) || (count < 3 && ret == 2)) {
                    lbl_1_bss_D8[sel].visualState = 2;
                    ret = FileselEraseConfirm(sel);
                    lbl_1_bss_D8[sel].visualState = 1;
                    if (ret == FILESEL_RESULT_CANCEL) {
                        goto L_241C;
                    }
                    if (ret == -4) {
                        goto L_241C;
                    }
                    if (ret == -5) {
                        goto L_241C;
                    }
                    if (ret == -1) {
                        goto L_254C;
                    }
                    goto L_192C;
                } else {
                    lbl_1_bss_D8[sel].visualState = 3;
                    fn_1_63A0(sel, 1);
                    ret = FileselCopy(sel);
                    fn_1_63A0(sel, 0);
                    lbl_1_bss_D8[sel].visualState = 1;
                    if (ret == -4) {
                        goto L_241C;
                    }
                    if (ret == -5) {
                        goto L_241C;
                    }
                    if (ret == -1) {
                        goto L_254C;
                    }
                    goto L_192C;
                }
            } else {
                UnMountCnt = 0;
                fn_1_69B0();
                fn_1_2654(-1);
                ret = NameEnterMain(sel);
                fn_1_2A40();
                if (ret == FILESEL_RESULT_CANCEL) {
                    goto L_192C;
                }
                if (UnMountCnt != 0) {
                    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
                    HuWinMesWait(lbl_1_bss_2E0);
                    UnMountCnt = 0;
                    goto L_241C;
                }
                if (FileTestOpen() == 0) {
                    time = OSGetTime();
                    SLSaveDataMake(1, &time);
                    SLCommonSet();
                    for (i = 0; i < 3; i++) {
                        if (i != sel) {
                            SLSaveEmptySet(curSlotNo, i);
                        }
                    }
                    fxId = HuAudFXPlay(MSM_SE_FILESEL_15);
                    ret = FileClear(-1);
                    HuAudFXStop(fxId);
                    if (ret == FILESEL_RESULT_CANCEL) {
                        goto L_241C;
                    }
                    if (ret == -4) {
                        goto L_241C;
                    }
                    if (ret == -5) {
                        goto L_241C;
                    }
                    if (ret == -1) {
                        goto L_254C;
                    }
                } else {
                    time = OSGetTime();
                    SLSaveDataMake(0, &time);
                    SLCommonSet();
                    fxId = HuAudFXPlay(MSM_SE_FILESEL_15);
                    ret = FileSaveMesOpen(-1, FILE_SELECT_SAVING);
                    HuAudFXStop(fxId);
                    if (ret == FILESEL_RESULT_CANCEL) {
                        goto L_241C;
                    }
                    if (ret == -4) {
                        goto L_241C;
                    }
                    if (ret == -5) {
                        goto L_241C;
                    }
                    if (ret == -1) {
                        goto L_254C;
                    }
                }
                lbl_1_bss_D8[sel].hasSave = 1;
                lbl_1_bss_D8[sel].patternVariant = -1;
                lbl_1_bss_D8[sel].displayNumber = 0;
                OSTicksToCalendarTime(GwCommon.time, &lbl_1_bss_D8[sel].saveTime);
                fn_1_36C4(sel, sel);
                goto L_2414;
            }
        } else {
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                sel = FILESEL_RESULT_CANCEL;
                goto L_2414;
            }
            if (HuPadDStkRep[0] & 0x1) {
                dir = -1;
            }
            if (HuPadDStkRep[0] & 0x2) {
                dir = 1;
            }
            if (dir == 0) {
                continue;
            }
            if (sel + dir < 0) {
                continue;
            }
            if (sel + dir >= 3) {
                continue;
            }
            HuAudFXPlay(MSM_SE_FILESEL_05);
            fn_1_3CF4(sel + dir, 0);
            fn_1_3CF4(sel, 1);
            for (i = 1; i <= 10; i++) {
                t = (f32)i / 10.0f;
                t = (f32) (sin(3.141592653589793 * (130.0f * t) / 180.0) *
                           (1.0 / sin(2.2689280275926285)));
                w = &lbl_1_bss_D8[sel + dir];
                Hu3DModelScaleSet(w->slotModel,
                    (f32)(1.5 + 0.5 * t),
                    (f32)(1.5 + 0.5 * t),
                    (f32)(1.5 + 0.5 * t));
                fn_1_3CC0(sel + dir, sv[sel + dir].x, sv[sel + dir].y, 20.0f * t + sv[sel + dir].z);
                t = (f32)(1.0 - t);
                w = &lbl_1_bss_D8[sel];
                Hu3DModelScaleSet(w->slotModel,
                    (f32)(1.5 + 0.5 * t),
                    (f32)(1.5 + 0.5 * t),
                    (f32)(1.5 + 0.5 * t));
                fn_1_3CC0(sel, sv[sel].x, sv[sel].y, 20.0f * t + sv[sel].z);
                if (i == 5) {
                    lbl_1_bss_D8[sel + dir].visualState = 1;
                    lbl_1_bss_D8[sel].visualState = 0;
                }
                HuPrcVSleep();
            }
            sel += dir;
        }
    }
L_2414:
    return sel;
L_241C:
    HuWinHomeClear(lbl_1_bss_2E0);
    HuWinDispOff(lbl_1_bss_2DE);
    for (i = 1; i <= 10; i++) {
        t = (f32)cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
        for (j = 0; j < 3; j++) {
            w = &lbl_1_bss_D8[j];
            Hu3DModelScaleSet(w->slotModel, t, t, t);
        }
        HuPrcVSleep();
    }
    for (i = 0; i < 3; i++) {
        lbl_1_bss_D8[i].visualState = 0;
    }
    HuPrcSleep(30);
    return -3;
L_254C:
    HuWinHomeClear(lbl_1_bss_2E0);
    for (i = 1; i <= 10; i++) {
        t = (f32)cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
        for (j = 0; j < 3; j++) {
            w = &lbl_1_bss_D8[j];
            Hu3DModelScaleSet(w->slotModel, t, t, t);
        }
        HuPrcVSleep();
    }
    return -1;
}

/* 0x2654 */
/* FileselSelect calls this before name entry to raise the three slots and move the data-select
 * model. */
void fn_1_2654(s16 slotSelection)
{
    f32 scale;
    s16 i;

    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].slotModel, &lbl_1_bss_68[i]);
    }
    Hu3DModelPosGet(lbl_1_bss_2D8[2], &lbl_1_bss_68[i]);
    if (slotSelection == -1) {
        for (i = 1; i <= 10; i++) {
            scale = 1.0 - cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
            fn_1_3CC0(0, lbl_1_bss_68[0].x, lbl_1_bss_68[0].y + 350.0f * scale, lbl_1_bss_68[0].z);
            fn_1_3CC0(1, lbl_1_bss_68[1].x, lbl_1_bss_68[1].y + 350.0f * scale, lbl_1_bss_68[1].z);
            fn_1_3CC0(2, lbl_1_bss_68[2].x, lbl_1_bss_68[2].y + 350.0f * scale, lbl_1_bss_68[2].z);
            Hu3DModelPosSet(lbl_1_bss_2D8[2], lbl_1_bss_68[3].x - 200.0f * scale, lbl_1_bss_68[3].y,
                            lbl_1_bss_68[3].z);
            HuPrcVSleep();
        }
    } else {
        for (i = 1; i <= 10; i++) {
            scale = 1.0 - cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
            switch (slotSelection) {
            case 0:
                fn_1_3CC0(0, lbl_1_bss_68[0].x, lbl_1_bss_68[0].y + 300.0f * scale,
                          lbl_1_bss_68[0].z);
                break;
            case 1:
                fn_1_3CC0(1, lbl_1_bss_68[1].x, lbl_1_bss_68[1].y - 400.0f * scale,
                          lbl_1_bss_68[1].z);
                break;
            case 2:
                fn_1_3CC0(2, lbl_1_bss_68[2].x, lbl_1_bss_68[2].y + 300.0f * scale,
                          lbl_1_bss_68[2].z);
                break;
            }
            HuPrcVSleep();
        }
    }
}

/* 0x2A40 */
/* Called after name entry to return the three slot models and data-select model to their
 * saved positions. */
void fn_1_2A40(void)
{
    Vec v14;
    Vec v8;
    Vec v20[4];
    f32 scale;
    s16 i;
    s16 j;

    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].slotModel, &v14);
        PSVECSubtract(&v14, &lbl_1_bss_68[i], &v20[i]);
    }
    Hu3DModelPosGet(lbl_1_bss_2D8[2], &v14);
    PSVECSubtract(&v14, &lbl_1_bss_68[i], &v20[i]);
    for (i = 1; i <= 10; i++) {
        scale = 1.0 - sin(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
        for (j = 0; j < 3; j++) {
            PSVECScale(&v20[j], &v14, scale);
            PSVECAdd(&lbl_1_bss_68[j], &v14, &v8);
            fn_1_3C80(j, &v8);
        }
        PSVECScale(&v20[j], &v14, scale);
        PSVECAdd(&lbl_1_bss_68[j], &v14, &v8);
        Hu3DModelPosSetV(lbl_1_bss_2D8[2], &v8);
        HuPrcVSleep();
    }
}

/* 0x2C6C */
/* Called during scene setup to create the four card panels, their text, and animated slot
 * models. */
void FileselInit(void)
{
    FILESEL_BOX *box;
    s16 j;
    HUSPR_GROUPID spriteGroup;
    s16 i;
    HSF_DATA *modelData;
    HUSPRID sprite;
    HUWINID panelWindow;
    ANIMDATA *digitAnimation;
    HSF_OBJECT *slotObject;
    ANIMDATA *markerAnimation;
    ANIMDATA *letterAnimation;

    HuSprExecLayerCameraSet(64, 2, 1);
    HuSprExecLayerCameraSet(120, 1, 8);
    letterAnimation = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_08, HU_MEMNUM_OVL, 2));
    spriteGroup = HuSprGrpCreate(1);
    sprite = HuSprCreate(letterAnimation, 0, 0);
    HuSprGrpMemberSet(spriteGroup, 0, sprite);
    HuSprGrpDrawNoSet(spriteGroup, 64);
    HuSprGrpPosSet(spriteGroup, 288.0f, 240.0f);
    HuSprGrpScaleSet(spriteGroup, 20.0f, 20.0f);
    digitAnimation = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_07, HU_MEMNUM_OVL, 2));
    markerAnimation = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_06, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[0] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_09, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[1] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_11, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[2] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_10, HU_MEMNUM_OVL, 2));
    for (i = 0; i < 8; i++) {
        lbl_1_bss_A8[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_98[i], HU_MEMNUM_OVL, 2));
    }
    /* This self-assignment has no effect; the slot loop below resets the index. */
    i = i;
    for (i = 0; i < 4; i++) {
        box = &lbl_1_bss_D8[i];
        panelWindow = HuWinCreate((f32)(288.0 * (i & 0x1)), (f32)(168.0 * (i / 2)), 192, 112, 0);
        box->window = panelWindow;
        box->waveOffset.x = box->waveOffset.y = box->waveOffset.z = 0.0f;
        HuWinScaleSet(panelWindow, 1.5f, 1.5f);
        HuWinBGTPLvlSet(panelWindow, 0.0f);
        HuWinAttrSet(panelWindow, HUWIN_ATTR_ALIGN_CENTER | HUWIN_ATTR_OUTLINE);
        box->cardTextureAnim = HuSprAnimMake(320, 168, ANIM_BMP_RGB5A3);
        box->cardTextureBuffer = box->cardTextureAnim->bmp->data = HuMemDirectMallocNum(
            0, GXGetTexBufferSize(320, 168, GX_TF_RGB5A3, 0, 0), HU_MEMNUM_OVL);
        HuWinDrawNoSet(panelWindow, 120);
        HuWinMesSpeedSet(panelWindow, 0);
        HuWinMesSet(panelWindow, MESSNUM(MESS_SAF_TEST, 9));
        spriteGroup = HuSprGrpCreate(17);
        box->spriteGroup = spriteGroup;
        HuSprGrpPosSet(spriteGroup, (f32)(288.0 * (i & 0x1)), (f32)(168.0 * (i / 2)));
        HuSprGrpScaleSet(spriteGroup, 1.5f, 1.5f);
        for (j = 0; j < 10; j++) {
            sprite = HuSprCreate(digitAnimation, FILESEL_SLOT_SPRITE_PRIORITY, 0);
            HuSprGrpMemberSet(spriteGroup, j, sprite);
            HuSprPosSet(spriteGroup, j, (f32)(j * 12 + 42), 70.0f);
            HuSprAttrReset(spriteGroup, j, HUSPR_ATTR_LINEAR);
        }
        sprite = HuSprCreate(markerAnimation, FILESEL_SLOT_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(spriteGroup, 10, sprite);
        HuSprPosSet(spriteGroup, 10, 70.0f, 96.0f);
        HuSprAttrReset(spriteGroup, 10, HUSPR_ATTR_LINEAR);
        for (j = 0; j < 5; j++) {
            sprite = HuSprCreate(digitAnimation, FILESEL_SLOT_SPRITE_PRIORITY, 0);
            HuSprGrpMemberSet(spriteGroup, j + 12, sprite);
            HuSprPosSet(spriteGroup, j + 12, (f32)(j * 12 + 88), 96.0f);
            HuSprAttrReset(spriteGroup, j + 12, HUSPR_ATTR_LINEAR);
        }
        HuSprGrpDrawNoSet(spriteGroup, 120);
        box->slotModel = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(box->slotModel, 2);
        Hu3DModelLayerSet(box->slotModel, 1);
        Hu3DModelAttrSet(box->slotModel, HU3D_MOTATTR_LOOP);
        j = Hu3DMotionMaxTimeGet(box->slotModel);
        Hu3DMotionTimeSet(box->slotModel, frandmod(j));
        box->frameAnimation = Hu3DAnimCreate(lbl_1_bss_CC[0], box->slotModel, lbl_1_data_1A0);
        box->patternAnimation = Hu3DAnimCreate(lbl_1_bss_A8[0], box->slotModel, lbl_1_data_1AD);
        Hu3DModelShadowSet(box->slotModel);
        box->textModel =
            Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_MOJI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(box->textModel, 2);
        Hu3DModelLayerSet(box->textModel, 2);
        Hu3DAnimCreate(box->cardTextureAnim, box->textModel, lbl_1_data_1BB);
        box->motionProcess = HuPrcChildCreate(fn_1_3D14, 16, 12288, 0, HuPrcCurrentGet());
        slotObject = Hu3DModelObjPtrGet(box->slotModel, lbl_1_data_1C8);
        strcpy(slotObject->name, lbl_1_data_190[i]);
        box->stateModel1 =
            Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(box->stateModel1, 2);
        Hu3DModelLayerSet(box->stateModel1, 2);
        Hu3DModelAttrSet(box->stateModel1, HU3D_ATTR_DISPOFF);
        box->stateModel2 =
            Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI2, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(box->stateModel2, 2);
        Hu3DModelLayerSet(box->stateModel2, 2);
        Hu3DModelAttrSet(box->stateModel2, HU3D_ATTR_DISPOFF);
        box->stateModel3 =
            Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI2, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(box->stateModel3, 2);
        Hu3DModelLayerSet(box->stateModel3, 2);
        Hu3DModelAttrSet(box->stateModel3, HU3D_ATTR_DISPOFF);
        modelData = Hu3DData[box->stateModel3].hsf;
        for (j = 0; j < modelData->materialNum; j++) {
            modelData->material[j].litColor[0] = modelData->material[j].color[0] = 241;
            modelData->material[j].litColor[1] = modelData->material[j].color[1] = 123;
            modelData->material[j].litColor[2] = modelData->material[j].color[2] = 18;
        }
        box->visualState = 0;
        Hu3DModelAttrSet(box->slotModel, HU3D_ATTR_DISPOFF);
        box->hasSave = 1;
        strcpy(box->fileName, lbl_1_data_1DD);
        OSTicksToCalendarTime(OSGetTime(), &box->saveTime);
        box->displayNumber = 10;
        fn_1_36C4(i, i % 3);
    }
    Hu3DCameraLayerHookSet(1, 1, (HU3D_LAYER_HOOK)fn_1_35AC);
}

/* 0x35AC */
/* Camera 1 layer callback: copy each card panel into its texture before the scene is drawn. */
void fn_1_35AC(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        FILESEL_BOX *w = &lbl_1_bss_D8[i];
        Hu3DFbCopyExec(1.1111111640930176 * (288.0 * (i & 0x1)),
                       168.0 * (i / 2),
                       320, 168, 5, 0, w->cardTextureBuffer);
    }
}

/* 0x36C4 */
/* Called after loading, creating, copying, or erasing a save to refresh one panel's details. */
void fn_1_36C4(s16 boxNo, s16 slotNumber)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[boxNo];
    s16 sprId = p->spriteGroup;
    OSCalendarTime *q = &p->saveTime;
    u8 i;
    u8 len;
    char buf[12];

    p->slotNumberText[0] = (u8)(slotNumber + '1');
    p->slotNumberText[1] = 0;
    HuWinInsertMesSet(p->window, (u32)p->slotNumberText, 0);
    if (p->hasSave == 0) {
        p->patternVariant = -1;
        p->fileName[0] = 0;
        HuWinInsertMesSet(p->window, FILE_SELECT_NEW, 1);
        HuWinMesSet(p->window, FILE_SELECT_NAME_TEMPLATE);
        for (i = 0; i < 10; i++) {
            HuSprAttrReset(sprId, i, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(sprId, i, 12);
        }
        HuSprBankSet(sprId, 2, 11);
        HuSprBankSet(sprId, 5, 11);
        /* Bank 10 is immediately replaced by bank 14 on this sprite before the next frame. */
        HuSprBankSet(sprId, 12, 10);
        HuSprBankSet(sprId, 12, 14);
        HuSprBankSet(sprId, 13, 0);
        Hu3DAnimAnimSet(p->patternAnimation, lbl_1_bss_A8[0]);
        /* Sprite 13 is reset to bank 0 twice in this empty-slot path. */
        HuSprBankSet(sprId, 13, 0);
        for (i = 0; i < 3; i++) {
            HuSprAttrSet(sprId, i + 14, HUSPR_ATTR_DISPOFF);
        }
        HuSprPosSet(sprId, 10, 84.0f, 96.0f);
        HuSprPosSet(sprId, 12, 102.0f, 96.0f);
        HuSprPosSet(sprId, 13, 114.0f, 96.0f);
    } else {
        for (i = 0; i < 15; i++) {
            HuSprAttrReset(sprId, i, HUSPR_ATTR_DISPOFF);
        }
        HuSprBankSet(sprId, 2, 11);
        HuSprBankSet(sprId, 5, 11);
        sprintf(buf, lbl_1_data_1E6, q->mon + 1);
        for (i = 0; i < 2; i++) {
            HuSprBankSet(sprId, i, buf[i] & FILESEL_DECIMAL_DIGIT_MASK);
        }
        sprintf(buf, lbl_1_data_1E6, q->mday);
        for (i = 0; i < 2; i++) {
            HuSprBankSet(sprId, i + 3, buf[i] & FILESEL_DECIMAL_DIGIT_MASK);
        }
        sprintf(buf, lbl_1_data_1EB, q->year);
        for (i = 0; i < 4; i++) {
            HuSprBankSet(sprId, i + 6, buf[i] & FILESEL_DECIMAL_DIGIT_MASK);
        }
        HuWinInsertMesSet(p->window, (u32)p->fileName, 1);
        HuWinMesSet(p->window, FILE_SELECT_NAME_TEMPLATE);
        HuSprBankSet(sprId, 12, 14);
        /* Hide members 13 through 16; the final call for member 17 is ignored because the group has
        * only 17 members. */
        for (i = 0; i < 5; i++) {
            HuSprAttrSet(sprId, i + 13, HUSPR_ATTR_DISPOFF);
        }
        /* Clamp the stored display count as well as the number shown on the panel. */
        if (p->displayNumber > 9999) {
            p->displayNumber = 9999;
        }
        sprintf(buf, lbl_1_data_1F0, p->displayNumber);
        len = strlen(buf);
        HuSprPosSet(sprId, 10, (f32)(108 - (len * 12 + 36) / 2), 96.0f);
        HuSprPosSet(sprId, 12, (f32)(126 - (len * 12 + 36) / 2), 96.0f);
        for (i = 0; i < 4; i++) {
            if (buf[i] == 0) {
                break;
            }
            HuSprBankSet(sprId, i + 13, buf[i] & FILESEL_DECIMAL_DIGIT_MASK);
            HuSprAttrReset(sprId, i + 13, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(sprId, i + 13,
                        (f32)(126 - (len * 12 + 36) / 2 + (i + 1) * 12),
                        96.0f);
        }
        if (p->patternVariant == -1) {
            Hu3DAnimAnimSet(p->patternAnimation, lbl_1_bss_A8[1]);
        } else {
            Hu3DAnimAnimSet(p->patternAnimation, lbl_1_bss_A8[p->patternVariant + 2]);
        }
    }
}

/* 0x3C80 */
/* Called by slot animations to store a panel's base position in model coordinates. */
void fn_1_3C80(s16 boxNo, Vec *position)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[boxNo];

    p->basePosition = *position;
}

/* 0x3CC0 */
/* Called by slot animations to store a panel's base position in model coordinates. */
void fn_1_3CC0(s16 boxNo, f32 x, f32 y, f32 z)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[boxNo];

    p->basePosition.x = x;
    p->basePosition.y = y;
    p->basePosition.z = z;
}

/* 0x3CF4 */
/* FileselSelect and the reveal process switch a slot between its motion modes here. */
void fn_1_3CF4(s16 boxNo, s32 motionMode)
{
    lbl_1_bss_D8[boxNo].motionState = (u8)motionMode;
}

/* 0x3D14 */
/* Child process created for each slot in FileselInit; animate its idle motion and state models. */
void fn_1_3D14(void)
{
    s16 prevState;
    HUPROCESS *proc;
    FILESEL_BOX *p;
    HU3D_MODEL *dst;
    HU3D_MODEL *src;
    Mtx mtx;
    s16 i;
    s16 idx;
    f32 angleX;
    f32 angleY;
    f32 wavePhase;
    f32 delta;
    f32 amp;

    proc = HuPrcCurrentGet();
    angleX = 0.0f;
    angleY = 0.0f;
    wavePhase = 0.0f;
    delta = 0.0f;
    amp = 0.0f;
    for (i = 0; i < 4; i++) {
        p = &lbl_1_bss_D8[i];
        if (p->motionProcess == proc) {
            break;
        }
    }
    idx = i;
    angleX = frandmod(90);
    angleY = frandmod(180);
    delta = 0.01f * frandmod(100);
    while (1) {
        switch (p->motionState) {
        case 0:
            amp -= 0.05f;
            if (amp < 0.0f) {
                amp = 0.0f;
            }
            break;
        case 1:
            amp += 0.01f;
            if (amp > 1.0f) {
                amp = 1.0f;
            }
            break;
        }
        angleX += 1.0f + delta;
        if (angleX > 360.0f) {
            angleX -= 360.0f;
        }
        angleY += 2.0f + delta;
        if (angleY > 1440.0f) {
            angleY -= 1440.0f;
        }
        if (p->motionState != 2) {
            Hu3DModelRotSet(p->slotModel, 0.0f, 0.0f,
                amp * (5.0 * sin(3.141592653589793 * angleX / 180.0)));
            p->waveOffset.x = amp * (2.0 * sin(3.141592653589793 * angleY / 180.0));
            p->waveOffset.y = amp * (2.0 * sin(3.141592653589793 * (angleY / 2.0f) / 180.0));
        }
        Hu3DModelPosSet(p->slotModel, p->basePosition.x + p->waveOffset.x,
                        p->basePosition.y + p->waveOffset.y, p->basePosition.z + p->waveOffset.z);
        if ((Hu3DModelAttrGet(p->slotModel) & HU3D_ATTR_DISPOFF) == 0) {
            Hu3DModelAttrReset(p->textModel, HU3D_ATTR_DISPOFF);
            Hu3DModelObjMtxGet(p->slotModel, lbl_1_data_190[i], mtx);
            Hu3DModelMtxSet(p->textModel, &mtx);
            src = &Hu3DData[p->slotModel];
            dst = &Hu3DData[p->stateModel1];
            /* The state model's matrix is copied back into the slot model; position, scale,
             * and rotation are then copied from the slot into the state model. The next two
             * state models do the same. */
            PSMTXCopy(dst->mtx, src->mtx);
            dst->pos = src->pos;
            dst->scale = src->scale;
            dst->rot = src->rot;
            dst = &Hu3DData[p->stateModel2];
            PSMTXCopy(dst->mtx, src->mtx);
            dst->pos = src->pos;
            dst->scale = src->scale;
            dst->rot = src->rot;
            dst = &Hu3DData[p->stateModel3];
            PSMTXCopy(dst->mtx, src->mtx);
            dst->pos = src->pos;
            dst->scale = src->scale;
            dst->rot = src->rot;
        } else {
            Hu3DModelAttrSet(p->textModel, HU3D_ATTR_DISPOFF);
        }
        switch (p->visualState) {
        case 0:
            Hu3DModelAttrSet(p->stateModel1, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel2, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel3, HU3D_ATTR_DISPOFF);
            break;
        case 1:
            Hu3DModelAttrReset(p->stateModel1, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel2, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel3, HU3D_ATTR_DISPOFF);
            break;
        case 2:
            Hu3DModelAttrSet(p->stateModel1, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(p->stateModel2, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel3, HU3D_ATTR_DISPOFF);
            break;
        case 3:
            Hu3DModelAttrSet(p->stateModel1, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(p->stateModel2, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(p->stateModel3, HU3D_ATTR_DISPOFF);
            break;
        }
        /* The first update compares an unset previous state; later updates compare the
         * preceding frame's visual state. */
        if (prevState != p->visualState) {
            wavePhase = 0.0f;
        }
        src = &Hu3DData[p->stateModel1];
        PSVECScale(&src->scale, &src->scale,
            1.0 + 0.20000000298023224 * sin(3.141592653589793 * wavePhase / 180.0));
        Hu3DModelTPLvlSet(p->stateModel1,
            sin(3.141592653589793 * (2.0f * wavePhase) / 180.0));
        src = &Hu3DData[p->stateModel2];
        PSVECScale(&src->scale, &src->scale,
            1.0 + 0.20000000298023224 * sin(3.141592653589793 * wavePhase / 180.0));
        Hu3DModelTPLvlSet(p->stateModel2,
            1.0 - sin(3.141592653589793 * wavePhase / 180.0));
        src = &Hu3DData[p->stateModel3];
        PSVECScale(&src->scale, &src->scale,
            1.0 + 0.20000000298023224 * sin(3.141592653589793 * wavePhase / 180.0));
        Hu3DModelTPLvlSet(p->stateModel3,
            1.0 - sin(3.141592653589793 * wavePhase / 180.0));
        if (p->visualState == 2) {
            wavePhase += 3.0f;
        } else {
            wavePhase += 2.0f;
        }
        if (wavePhase > 90.0f) {
            wavePhase -= 90.0f;
        }
        prevState = p->visualState;
        HuPrcVSleep();
    }
}

/* Camera control helper: when lbl_1_bss_0[0] is nonzero, apply stick and trigger input to the
 * File Select scene camera. */
void fn_1_457C(OMOBJ *object)
{
    HuVecF eye;
    HuVecF delta;
    HuVecF up;
    HuVecF roll;
    s8 subStick;
    float rotZ;

    if (!lbl_1_bss_0[0]) {
        return;
    }

    CRotM[1].y += 0.1f * (float)HuPadStkX[0];
    CRotM[1].x += 0.1f * (float)HuPadStkY[0];
    CZoomM[1] += (float)(HuPadTrigL[0] / 2);
    CZoomM[1] -= (float)(HuPadTrigR[0] / 2);
    if (CZoomM[1] < 100.0f) {
        CZoomM[1] = 100.0f;
    }

    eye.x = CenterM[1].x + (CZoomM[1] *
        (sin(3.141592653589793 * CRotM[1].y / 180.0) *
         cos(3.141592653589793 * CRotM[1].x / 180.0)));
    eye.y = CenterM[1].y + (CZoomM[1] *
        -sin(3.141592653589793 * CRotM[1].x / 180.0));
    eye.z = CenterM[1].z + (CZoomM[1] *
        (cos(3.141592653589793 * CRotM[1].y / 180.0) *
         cos(3.141592653589793 * CRotM[1].x / 180.0)));

    delta.x = CenterM[1].x - eye.x;
    delta.y = CenterM[1].y - eye.y;
    delta.z = CenterM[1].z - eye.z;

    up.x = sin(3.141592653589793 * CRotM[1].y / 180.0) *
           sin(3.141592653589793 * CRotM[1].x / 180.0);
    up.y = cos(3.141592653589793 * CRotM[1].x / 180.0);
    up.z = cos(3.141592653589793 * CRotM[1].y / 180.0) *
           sin(3.141592653589793 * CRotM[1].x / 180.0);

    rotZ = CRotM[1].z;

    /* Compute the rolled pan direction with the unnormalized eye-to-center vector.
     * Normalize the resulting direction only when preparing vertical C-stick panning. */
    roll.x = (up.x * ((delta.x * delta.x) +
               ((1.0f - (delta.x * delta.x)) *
                cos(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.z * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.y * sin(3.141592653589793 * rotZ / 180.0))));

    roll.y = (up.x * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.z * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * ((delta.y * delta.y) +
               ((1.0f - (delta.y * delta.y)) *
                cos(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.x * sin(3.141592653589793 * rotZ / 180.0))));

    roll.z = (up.x * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.y * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.x * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * ((delta.z * delta.z) +
               ((1.0f - (delta.z * delta.z)) *
                cos(3.141592653589793 * rotZ / 180.0))));

    PSVECCrossProduct(&up, &delta, &delta);
    PSVECNormalize(&delta, &delta);

    subStick = HuPadSubStkX[0] & FILESEL_SUBSTICK_STEP_MASK;
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }

    PSVECNormalize(&roll, &delta);

    subStick = -(HuPadSubStkY[0] & FILESEL_SUBSTICK_STEP_MASK);
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }
}

/* 0x50DC */
/* The reveal flow calls this to start a child process with an integer property. */
void fn_1_50DC(void (*func)(void), s32 processProperty)
{
    HUPROCESS *proc = HuPrcChildCreate(func, 16, 12288, 0, HuPrcCurrentGet());
    proc->property = (void *)processProperty;
}

/* 0x5130 */
/* Child process used by copy and erase flows to flip the data-select model between banks. */
void fn_1_5130(void)
{
    HUPROCESS *proc;
    s32 bank;
    s16 i;
    f32 angle;

    proc = HuPrcCurrentGet();
    bank = (s32)proc->property;
    for (i = 1; i <= 10; i++) {
        angle = (float)i / 10.0f;
        if (i == 5) {
            Hu3DAnimBankSet(lbl_1_bss_A2[0], (u16)bank);
            Hu3DAnimBankSet(lbl_1_bss_A2[1], (u16)bank);
        }
        Hu3DModelRotSet(lbl_1_bss_2D8[2], 0.0f, 180.0f * angle, 0.0f);
        HuPrcVSleep();
    }
    HuPrcEnd();
}

/* FileselSelect calls this after the player chooses Copy; wait for a destination and copy the
 * save. */
s32 FileselCopy(s16 sourceBoxNo)
{
    HUPROCESS *proc;
    HUPROCESS *procB;
    Vec pos[3];
    FILESEL_BOX *wpa;
    FILESEL_BOX *wpb;
    FILESEL_BOX *wpc;
    FILESEL_BOX *wpd;
    FILESEL_BOX *wpe;
    FILESEL_BOX *wpf;
    s16 cur;
    s16 prev;
    s16 i;
    s16 dir;
    s32 ret;
    s16 emptyCnt;
    s32 sndH;
    f32 scale;
    f32 sx;
    f32 sy;
    f32 sz;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 px;
    f32 py;
    f32 pz;
    f32 rx;
    f32 ry;
    f32 rz;
    f32 ex;
    f32 ey;
    f32 ez;
    HUPROCESS *procC;
    f32 qx;
    f32 qy;
    f32 qz;
    HUPROCESS *procD;

    cur = 0;
    proc = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
    proc->property = (void *)2;
    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].slotModel, &pos[i]);
    }
    HuWinInsertMesSet(lbl_1_bss_2E0, (u32)lbl_1_bss_D8[sourceBoxNo].fileName, 0);
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_COPY_DESTINATION);
    HuWinMesWait(lbl_1_bss_2E0);
    HuWinMesSet(lbl_1_bss_2DE, MESSNUM(MESS_SYS_GUIDE, 2));
    HuWinDispOn(lbl_1_bss_2DE);
    for (cur = 0; cur < 3; cur++) {
        if (lbl_1_bss_D8[cur].hasSave == 0) {
            break;
        }
    }
    lbl_1_bss_D8[cur].motionState = 0;
    for (emptyCnt = 0, i = emptyCnt; i < 3; i++) {
        if (lbl_1_bss_D8[i].hasSave == 0) {
            emptyCnt++;
        }
    }
    for (i = 1; i <= 5; i++) {
        scale = (f32)i / 5.0f;
        Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, (f32) (1.5 + 0.5 * scale),
                          (f32) (1.5 + 0.5 * scale), (f32) (1.5 + 0.5 * scale));
        sz = pos[cur].z + 20.0f * scale;
        sy = pos[cur].y;
        sx = pos[cur].x;
        wpa = &lbl_1_bss_D8[cur];
        wpa->basePosition.x = sx;
        wpa->basePosition.y = sy;
        wpa->basePosition.z = sz;
        HuPrcVSleep();
    }
    lbl_1_bss_D8[cur].visualState = 1;
    prev = cur;
    for (;;) {
        if (emptyCnt > 1) {
            dir = 0;
            if (HuPadDStkRep[0] & 0x1) {
                dir = -1;
            } else if (HuPadDStkRep[0] & 0x2) {
                dir = 1;
            }
            if (dir != 0) {
                HuAudFXPlay(MSM_SE_FILESEL_05);
                prev = cur;
                cur += dir;
                if (cur < 0) {
                    cur = 2;
                } else if (cur >= 3) {
                    cur = 0;
                }
                while (lbl_1_bss_D8[cur].hasSave != 0) {
                    cur += dir;
                    if (cur < 0) {
                        cur = 2;
                    } else if (cur >= 3) {
                        cur = 0;
                    }
                }
                for (i = 1; i <= 5; i++) {
                    scale = sin(3.141592653589793 * (130.0 * (i / 5.0)) / 180.0) *
                            (1.0 / sin(2.2689280275926285));
                    scale = 1.5 + 0.5 * scale;
                    Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, scale, scale, scale);
                    scale = i / 5.0;
                    nz = pos[cur].z + 20.0f * scale;
                    ny = pos[cur].y;
                    nx = pos[cur].x;
                    wpb = &lbl_1_bss_D8[cur];
                    wpb->basePosition.x = nx;
                    wpb->basePosition.y = ny;
                    wpb->basePosition.z = nz;
                    scale = (f32)(1.5 + 0.5 * (1.0 - i / 5.0));
                    Hu3DModelScaleSet(lbl_1_bss_D8[prev].slotModel, scale, scale, scale);
                    scale = 1.0 - i / 5.0;
                    pz = pos[prev].z + 20.0f * scale;
                    py = pos[prev].y;
                    px = pos[prev].x;
                    wpc = &lbl_1_bss_D8[prev];
                    wpc->basePosition.x = px;
                    wpc->basePosition.y = py;
                    wpc->basePosition.z = pz;
                    HuPrcVSleep();
                }
                lbl_1_bss_D8[cur].visualState = 1;
                lbl_1_bss_D8[prev].visualState = 0;
                lbl_1_bss_D8[cur].motionState = 0;
                lbl_1_bss_D8[prev].motionState = 1;
            }
        }
        if (UnMountCnt != 0) {
            UnMountCnt = 0;
            lbl_1_bss_D8[cur].visualState = 0;
            HuWinHomeClear(lbl_1_bss_2DE);
            procB = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
            procB->property = (void *)1;
            return -4;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_02);
            goto confirm;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            goto cancel;
        }
        HuPrcVSleep();
        continue;
    confirm:
        HuWinHomeClear(lbl_1_bss_2DE);
        HuWinHomeClear(lbl_1_bss_2E0);
        for (i = 1; i <= 20; i++) {
            scale = (f32)(1.5 * (1.0 - sin(3.141592653589793 * (90.0 * (i / 20.0)) / 180.0)));
            Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, 1.5f * scale, 1.5f * scale,
                              1.5f * scale);
            HuPrcVSleep();
        }
        lbl_1_bss_D8[cur].visualState = 0;
        /* Copy into the save buffer before writing the card; a failed save does not undo this
         * copy. */
        memcpy(&saveBuf[curSlotNo][SLBoxDataOffsetGet(cur)],
               &saveBuf[curSlotNo][SLBoxDataOffsetGet(sourceBoxNo)], SAVE_BOX_SIZE);
        sndH = HuAudFXPlay(MSM_SE_FILESEL_06);
        ret = FileSaveMesOpen(-1, FILE_SELECT_COPYING);
        HuAudFXStop(sndH);
        HuAudFXPlay(MSM_SE_FILESEL_07);
        /* Treat a canceled save as card removal so the selection flow reloads the card. */
        if (ret == FILESEL_RESULT_CANCEL) {
            ret = -4;
        }
        if (ret == 0) {
            lbl_1_bss_D8[cur].hasSave = 1;
            lbl_1_bss_D8[cur].patternVariant = lbl_1_bss_D8[sourceBoxNo].patternVariant;
            lbl_1_bss_D8[cur].saveTime = lbl_1_bss_D8[sourceBoxNo].saveTime;
            lbl_1_bss_D8[cur].displayNumber = lbl_1_bss_D8[sourceBoxNo].displayNumber;
            memcpy(lbl_1_bss_D8[cur].fileName, lbl_1_bss_D8[sourceBoxNo].fileName,
                   FILESEL_FILENAME_LENGTH);
            fn_1_36C4(cur, cur);
            HuAudFXPlay(MSM_SE_FILESEL_10);
            rz = pos[cur].z;
            ry = pos[cur].y;
            rx = pos[cur].x;
            wpd = &lbl_1_bss_D8[cur];
            wpd->basePosition.x = rx;
            wpd->basePosition.y = ry;
            wpd->basePosition.z = rz;
            for (i = 1; i <= 10; i++) {
                scale = (f32)(1.5 * (i / 10.0));
                Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, scale, scale, scale);
                HuPrcVSleep();
            }
            lbl_1_bss_D8[cur].motionState = 1;
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_COPY_COMPLETE);
            HuWinMesWait(lbl_1_bss_2E0);
        } else {
            ez = pos[cur].z;
            ey = pos[cur].y;
            ex = pos[cur].x;
            wpe = &lbl_1_bss_D8[cur];
            wpe->basePosition.x = ex;
            wpe->basePosition.y = ey;
            wpe->basePosition.z = ez;
            for (i = 1; i <= 5; i++) {
                scale = (f32)(1.5 * (i / 5.0));
                Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, scale, scale, scale);
                HuPrcVSleep();
            }
            lbl_1_bss_D8[cur].motionState = 1;
        }
        procC = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
        procC->property = (void *)1;
        return ret;
    cancel:
        HuWinHomeClear(lbl_1_bss_2DE);
        lbl_1_bss_D8[cur].visualState = 0;
        for (i = 1; i <= 5; i++) {
            scale = (f32)(1.5 * (1.0 + 0.5 * (1.0 - i / 5.0)));
            Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, scale, scale, scale);
            scale = 1.0 - i / 5.0;
            qz = pos[cur].z + 20.0f * scale;
            qy = pos[cur].y;
            qx = pos[cur].x;
            wpf = &lbl_1_bss_D8[cur];
            wpf->basePosition.x = qx;
            wpf->basePosition.y = qy;
            wpf->basePosition.z = qz;
            HuPrcVSleep();
        }
        lbl_1_bss_D8[cur].motionState = 1;
        procD = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
        procD->property = (void *)1;
        return FILESEL_RESULT_CANCEL;
    }
}

/* 0x6314 */
/* Switch a slot's frame between its first highlight animation and its normal animation. */
void fn_1_6314(s16 boxNo, s32 active)
{
    if (active != 0) {
        Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[1]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
    }
}

/* 0x63A0 */
/* Called during copy prompts to show or restore the alternate slot frame animation. */
void fn_1_63A0(s16 boxNo, s32 active)
{
    if (active != 0) {
        Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[2]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
    }
}

/* FileselSelect calls this after Erase; confirm it, clear the card save, and refresh the slot
* panel. Declining or canceling the confirmation returns zero without erasing. */
s32 FileselEraseConfirm(s16 boxNo)
{
    HUWIN *w;
    s32 ret;
    int fxId;
    int i;
    HUPROCESS *proc1;
    HUPROCESS *proc2;
    HUPROCESS *proc3;
    HUPROCESS *proc4;
    f32 s;

    w = &winData[lbl_1_bss_2E0];
    proc1 = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
    proc1->property = (void *)3;
    Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[1]);
    HuWinInsertMesSet(lbl_1_bss_2E0, (u32)&lbl_1_bss_D8[boxNo].fileName, 0);
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_ERASE_CONFIRMATION);
    HuWinMesWait(lbl_1_bss_2E0);
    HuWinChoiceSet(lbl_1_bss_2E0, -1);
    while (w->stat != 0) {
        if (UnMountCnt != 0) {
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
            HuWinMesWait(lbl_1_bss_2E0);
            UnMountCnt = 0;
            proc2 = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
            proc2->property = (void *)1;
            Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
            return -4;
        }
        HuPrcVSleep();
    }
    ret = w->choice;
    HuWinHomeClear(lbl_1_bss_2E0);
    if (UnMountCnt != 0) {
        HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
        HuWinMesWait(lbl_1_bss_2E0);
        UnMountCnt = 0;
        proc3 = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
        proc3->property = (void *)1;
        Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
        return -4;
    }
    if (ret == 0) {
        fxId = HuAudFXPlay(MSM_SE_FILESEL_08);
        /* Mark both the box and its backup empty before saving; failure does not restore their
         * tags. */
        SLSaveEmptySet(curSlotNo, boxNo);
        ret = FileSaveMesOpen(-1, FILE_SELECT_ERASING);
        HuAudFXStop(fxId);
        fxId = HuAudFXPlay(MSM_SE_FILESEL_09);
        if (ret == 0) {
            HuAudFXPlay(MSM_SE_FILESEL_11);
        for (i = 1; i <= 20; i++) {
                s = 1.0 - sin(3.141592653589793 * (90.0 * ((double)i / 20.0)) / 180.0);
                Hu3DModelScaleSet(lbl_1_bss_D8[boxNo].slotModel, 1.5f * s, 1.5f * s, 1.5f * s);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            /* The same empty-state value is assigned twice in this path. */
            lbl_1_bss_D8[boxNo].hasSave = 0;
            lbl_1_bss_D8[boxNo].hasSave = 0;
            fn_1_36C4(boxNo, boxNo);
            lbl_1_bss_D8[boxNo].visualState = 1;
            Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
        for (i = 1; i <= 10; i++) {
                s = (double)i / 10.0;
                Hu3DModelScaleSet(lbl_1_bss_D8[boxNo].slotModel, 1.5f * s, 1.5f * s, 1.5f * s);
                HuPrcVSleep();
            }
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_ERASE_COMPLETE);
            HuWinMesWait(lbl_1_bss_2E0);
        }
    } else {
        ret = 0;
    }
    proc4 = HuPrcChildCreate(fn_1_5130, 16, 12288, 0, HuPrcCurrentGet());
    proc4->property = (void *)1;
    Hu3DAnimAnimSet(lbl_1_bss_D8[boxNo].frameAnimation, lbl_1_bss_CC[0]);
    return ret;
}

/* FileselSelect calls this before naming a new save. Restore common defaults and force mono or
 * stereo from the system sound mode. Detect and mount the channel-1 microphone, ignoring
 * initialization status, then close speech resources and return the resulting microphone mode. */
/* 0x69B0 */
s32 fn_1_69B0(void)
{
    GwCommon = GwCommonOrig;
    if (OSGetSoundMode() == 0) {
        GwCommon.outputMode = 0;
    } else {
        GwCommon.outputMode = 1;
    }
    if (HuMCProbe(1) != 0) {
        HuMCMicSet(0);
    } else {
        HuMCMicSet(1);
        HuMCInit(0);
        if (HuMCMount(1) != 0) {
            HuMCMicSet(0);
        }
        HuMCClose();
    }
    return HuMCMicGet();
}

/* 0x6A8C */
/* Camera-layer helper: draw a full-screen polygon at the far depth with color writes disabled. */
void FileselWipeDraw(void)
{
    HU3D_CAMERA *cam = &Hu3DCamera[Hu3DCameraNo];
    Mtx44 proj;
    Mtx view;
    Vec camPos;
    Vec target;
    Vec up;
    f32 farDepth;

    C_MTXPerspective(proj, cam->fov, cam->aspect, cam->near, cam->far);
    GXSetProjection(proj, 0);
    GXSetViewport(cam->viewportX, cam->viewportY, cam->viewportW, cam->viewportH, cam->viewportNear,
                  cam->viewportFar);
    GXSetScissor(cam->scissorX, cam->scissorY, cam->scissorW, cam->scissorH);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetBlendMode(1, 4, 4, 5);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 7);
    GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 1, 7, 0);
    GXSetZMode(1, 7, 1);
    GXSetColorUpdate(0);

    camPos.x = camPos.y = camPos.z = 0.0f;
    target.x = target.y = 0.0f;
    target.z = -100.0f;
    up.x = up.z = 0.0f;
    up.y = 1.0f;
    C_MTXLookAt(view, &camPos, &up, &target);
    GXLoadPosMtxImm(view, 0);

    farDepth = cam->far - 1.0f;
    camPos.x = camPos.y = farDepth * tan(3.141592653589793 * (cam->fov / 2.0f) / 180.0);
    camPos.x = camPos.x * cam->aspect;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-camPos.x, -camPos.y, -farDepth);
    GXPosition3f32(camPos.x, -camPos.y, -farDepth);
    GXPosition3f32(camPos.x, camPos.y, -farDepth);
    GXPosition3f32(-camPos.x, camPos.y, -farDepth);

    GXSetColorUpdate(1);
}

/* 0x6E54 */
/* After selection, center the book and start the camera zoom. When the selected-slot
 * animation is enabled, move the chosen slot toward the book as the other slots shrink. */
void fn_1_6E54(s16 selectedBoxNo, s32 animateSelectedSlot)
{
    HUPROCESS *proc1;
    HUPROCESS *proc2;
    FILESEL_BOX *wp;
    HUPROCESS *proc3;
    FILESEL_BOX *bp;
    Vec vpos;
    static const Vec l_19c = {0.0f, 0.0f, 15.0f};
    Vec transformVector;
    Vec pos24;
    Vec pos18;
    Vec rot0c;
    s16 i;
    s16 j;
    s32 dir;
    f32 phase;
    f32 sv;
    f32 dirf;

    vpos = l_19c;
    Hu3DModelPosGet(lbl_1_bss_2D8[0], &pos18);
    Hu3DModelRotGet(lbl_1_bss_2D8[0], &rot0c);
    rot0c.x = (f32)((s32)rot0c.x % 360);
    if (rot0c.x > 180.0f) {
        rot0c.x -= 360.0f;
    }
    rot0c.y = (f32)((s32)rot0c.y % 360);
    if (rot0c.y > 180.0f) {
        rot0c.y -= 360.0f;
    }
    rot0c.z = (f32)((s32)rot0c.z % 360);
    if (rot0c.z > 180.0f) {
        rot0c.z -= 360.0f;
    }
    if (animateSelectedSlot == 0) {
        HuWinDispOff(lbl_1_bss_2DE);
        HuWinExClose(lbl_1_bss_2E0);
        Hu3DModelScaleGet(lbl_1_bss_2D8[2], &transformVector);
        if (transformVector.x > 0.1f) {
            for (i = 1; i <= 10; i++) {
                phase = (f32)i / 10.0f;
                Hu3DModelRotSet(lbl_1_bss_2D8[2], 0.0f, 520.0f * phase, 0.0f);
                sv = 1.0 - phase;
                Hu3DModelScaleSet(lbl_1_bss_2D8[2], sv, sv, sv);
                HuPrcVSleep();
            }
        }
        for (i = 1; i <= 20; i++) {
            phase = (f32)sin(3.141592653589793 * (90.0 * ((f32)i / 20.0f)) / 180.0);
            transformVector.x = (1.0 - phase) * pos18.x;
            transformVector.y = (1.0 - phase) * pos18.y;
            transformVector.z = (1.0 - phase) * pos18.z;
            Hu3DModelPosSetV(lbl_1_bss_2D8[0], &transformVector);
            transformVector.x = (1.0 - phase) * rot0c.x;
            transformVector.y = (1.0 - phase) * rot0c.y;
            transformVector.z = (1.0 - phase) * rot0c.z;
            Hu3DModelRotSetV(lbl_1_bss_2D8[0], &transformVector);
            HuPrcVSleep();
        }
        proc1 = HuPrcChildCreate(fn_1_7A0C, 16, 12288, 0, HuPrcCurrentGet());
        proc1->property = (void *)0;
        HuPrcSleep(40);
        HuAudFXPlay(MSM_SE_FILESEL_14);
        fn_1_8510(0, &vpos);
    } else {
        /* Both loops set the selected slot's state; the loop index is not used here. */
        for (i = 0; i < 3; i++) {
            lbl_1_bss_D8[selectedBoxNo].motionState = 0;
        }
        HuWinDispOff(lbl_1_bss_2DE);
        HuWinExClose(lbl_1_bss_2E0);
        for (i = 0; i < 3; i++) {
            lbl_1_bss_D8[selectedBoxNo].motionState = 2;
        }
        for (i = 1; i <= 10; i++) {
            phase = (f32)i / 10.0f;
            for (j = 0; j < 3; j++) {
                if (j != selectedBoxNo) {
                    bp = &lbl_1_bss_D8[j];
                    Hu3DModelRotSet(bp->slotModel, 0.0f, 520.0f * phase, 0.0f);
                    sv = 1.5 * (1.0 - phase);
                    Hu3DModelScaleSet(bp->slotModel, sv, sv, sv);
                }
            }
            Hu3DModelRotSet(lbl_1_bss_2D8[2], 0.0f, 520.0f * phase, 0.0f);
            sv = 1.0 - phase;
            Hu3DModelScaleSet(lbl_1_bss_2D8[2], sv, sv, sv);
            HuPrcVSleep();
        }
        bp = &lbl_1_bss_D8[selectedBoxNo];
        Hu3DModelPosGet(bp->slotModel, &pos24);
        OSReport(lbl_1_data_1F3, pos24.x, pos24.y, pos24.z);
        if (frandmod(2) == 0) {
            dir = 1;
        } else {
            dir = -1;
        }
        dirf = (f32)dir;
        for (i = 1; i <= 80; i++) {
            if (i <= 20) {
                phase = (f32)sin(3.141592653589793 * (90.0 * ((f32)i / 20.0f)) / 180.0);
                transformVector.x = (1.0 - phase) * pos18.x;
                transformVector.y = (1.0 - phase) * pos18.y;
                transformVector.z = (1.0 - phase) * pos18.z;
                Hu3DModelPosSetV(lbl_1_bss_2D8[0], &transformVector);
                transformVector.x = (1.0 - phase) * rot0c.x;
                transformVector.y = (1.0 - phase) * rot0c.y;
                transformVector.z = (1.0 - phase) * rot0c.z;
                Hu3DModelRotSetV(lbl_1_bss_2D8[0], &transformVector);
                Hu3DShadowTPLvlSet(0.5 * (1.0 - phase));
            }
            if (i == 10) {
                proc2 = HuPrcChildCreate(fn_1_7B64, 16, 12288, 0, HuPrcCurrentGet());
                proc2->property = (void *)selectedBoxNo;
                HuAudFXPlay(MSM_SE_FILESEL_13);
            }
            if (i > 10) {
                phase = ((f32)i - 10.0f) / 70.0f;
                transformVector.x = (1.0 - phase) * pos24.x;
                transformVector.y = (1.0 - phase) * pos24.y +
                            100.0 * sin(3.141592653589793 * (180.0f * phase) / 180.0);
                transformVector.z = pos24.z +
                                    300.0 * sin(3.141592653589793 * (180.0f * phase) / 180.0) -
                                    150.0f * phase;
                wp = &lbl_1_bss_D8[selectedBoxNo];
                wp->basePosition = transformVector;
                Hu3DModelRotSet(bp->slotModel, 0.0f, 480.0f * phase, 45.0f * phase * dirf);
                sv = 1.125 + 1.125 * (1.0 - phase);
                Hu3DModelScaleSet(bp->slotModel, sv, sv, sv);
            }
            if (i == 40) {
                proc3 = HuPrcChildCreate(fn_1_7A0C, 16, 12288, 0, HuPrcCurrentGet());
                proc3->property = (void *)0;
            }
            if (i == 75) {
                HuAudFXPlay(MSM_SE_FILESEL_14);
                fn_1_8510(0, &vpos);
            }
            HuPrcVSleep();
        }
    }
}

/* 0x7A0C */
/* Child process launched by fn_1_6E54 to ease the File Select camera zoom. */
void fn_1_7A0C(void)
{
    HU3D_CAMERA *cam = &Hu3DCamera[1];
    f32 zoom0 = CZoomM[1];
    s16 i;
    f32 t;

    for (i = 0; i < 100.0; i++) {
        t = i / 100.0;
        t = 1.0 - cos(3.14159265358979323846 * (90.0 * t) / 180.0);
        CZoomM[1] = zoom0 + (200.0f - zoom0) * t;
        HuPrcVSleep();
    }
    HuPrcEnd();
}

/* 0x7B64 */
/* Child process launched for a slot animation; emit particles from its moving model. */
void fn_1_7B64(void)
{
    HUPROCESS *proc;
    s32 idx;
    s32 i;
    Mtx objMtx;
    Vec dir;
    Vec trans;
    Mtx tmpMtx;
    VEC5 arr;
    static const VEC5 l_1d0 = {{
        {0.0f, 0.0f, 0.0f},
        {40.0f, 10.0f, 0.0f},
        {-40.0f, 10.0f, 0.0f},
        {20.0f, -40.0f, 0.0f},
        {-20.0f, -40.0f, 0.0f},
    }};

    proc = HuPrcCurrentGet();
    idx = (s32)proc->property;
    arr = l_1d0;

    for (i = 0; i < 5; i++) {
        Hu3DParManAttrReset(lbl_1_bss_98[i], 1);
    }

    while (1) {
        Hu3DModelObjMtxGet(lbl_1_bss_D8[idx].slotModel, lbl_1_data_1FD, objMtx);
        Hu3DMtxTransGet(objMtx, &trans);
        if (trans.z > 0.0f) {
            for (i = 0; i < 5; i++) {
                PSMTXTrans(tmpMtx, arr.v[i].x, arr.v[i].y, arr.v[i].z);
                PSMTXConcat(objMtx, tmpMtx, tmpMtx);
                Hu3DMtxTransGet(tmpMtx, &dir);
                Hu3DParManPosSet(lbl_1_bss_98[i], dir.x, dir.y, dir.z);
                PSVECSubtract(&dir, &trans, &dir);
                if (dir.z * dir.z + (dir.x * dir.x + dir.y * dir.y) <= 0.0) {
                    dir.x = dir.y = dir.z = 0.0f;
                } else {
                    PSVECNormalize(&dir, &dir);
                }
                /* Retain X and Z from the three-dimensional direction, normalized only when
                 * nonzero, and force the emission Y component to -0.5. */
                Hu3DParManVecSet(lbl_1_bss_98[i], dir.x, -0.5f, dir.z);
            }
        } else {
            /* Stop emission when the attachment reaches z <= 0; this loop never clears the stop
            * flag again, even if the attachment returns to positive z. */
            for (i = 0; i < 5; i++) {
                Hu3DParManAttrSet(lbl_1_bss_98[i], 1);
            }
        }
        HuPrcVSleep();
    }
}

/* 0x7DEC */
/* Layer hook installed by fn_1_8510: copy the full framebuffer into the card effect's texture. */
void fn_1_7DEC(void)
{
    if (lbl_1_bss_C != 0) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(640, 480, GX_TF_RGB565, 0);
        GXCopyTex(lbl_1_bss_C, 0);
    }
}

/* 0x7E58 */
/* Material hook installed on the card effect models to draw their animated texture. */
void fn_1_7E58(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    HU3D_CAMERA *cam = &Hu3DCamera[Hu3DCameraNo];
    BSS18_ENTRY *q;
    Mtx tmpMtx;
    Mtx scaleMtx;
    Mtx texMtx;
    Mtx lightMtx;
    Mtx invMtx;
    GXTexObj texObj;
    f32 transS;

    q = drawObj->model->hookData;
    q->textureOffset = q->textureOffset + q->textureOffsetStep;

    GXInitTexObj(&texObj, lbl_1_bss_C, 640, 480, GX_TF_RGB565, 0, 0, 0);
    GXInitTexObjLOD(&texObj, 1, 1, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    GXLoadTexObj(&texObj, 1);
    HuSprTexLoad(lbl_1_bss_10, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_14, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);

    transS = ((f32)cam->scissorX + (f32)cam->scissorW / 2.0f) / 640.0f;
    C_MTXLightPerspective(lightMtx, cam->fov, 1.2f, 0.5f, -0.5f, transS, 0.5f);
    PSMTXInverse(Hu3DCameraMtx, invMtx);
    PSMTXConcat(invMtx, drawObj->matrix, tmpMtx);
    PSMTXConcat(lightMtx, Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    PSMTXTrans(tmpMtx, 0.0f, q->textureOffset, 0.0f);
    PSMTXScale(scaleMtx, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(scaleMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetTevColor(1, q->tint);
    GXSetTevOrder(0, 0, 1, 0);
    GXSetTevColorIn(0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    GXSetTevOrder(1, 1, 0, 0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_CPREV);
    GXSetTevColorOp(1, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(1, 7, 7, 7, 6);
    GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(0, 1, 2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetTevIndWarp(0, 0, 1, 0, 1);
    GXSetIndTexMtx(1, (f32 (*)[3])q, -1);
}

/* 0x82F0 */
/* Object callback installed by fn_1_8510; update effect lifetime and tint each frame. */
void fn_1_82F0(OMOBJ *obj)
{
    BSS18_ENTRY *e;
    s16 i;
    f32 t;

    for (i = 0, e = lbl_1_bss_18; i < 2; i++, e++) {
        if (e->framesRemaining <= 0.0f) {
            continue;
        }
        e->indirectMatrixRow0.x = 0.0f;
        e->indirectMatrixRow0.y = 0.0f;
        e->indirectMatrixRow0.z = 0.09f * e->framesRemaining / 120.0f;
        e->indirectMatrixRow1.x = 0.0f;
        e->indirectMatrixRow1.y = 0.0f;
        e->indirectMatrixRow1.z = 0.09f * e->framesRemaining / 120.0f;
        e->textureOffsetStep = -0.02f * e->framesRemaining / 120.0f;
        t = (e->framesRemaining - 80.0f) / 40.0f;
        if (t >= 0.0f) {
            e->tint.r = 0;
            e->tint.g = 16.0f * t;
            e->tint.b = 64.0f * t;
        }
        e->framesRemaining = e->framesRemaining - 1.0f;
        /* Expiring either effect disables the shared capture hook and the object's update
         * callback, even if the other effect still has time remaining. */
        if (e->framesRemaining <= 0.0f) {
            Hu3DLayerHookReset(10);
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            obj->objFunc = 0;
        }
    }
}

/* 0x8510 */
/* Called when a slot effect starts; place its model and install its update and layer hooks. */
void fn_1_8510(s16 effectIndex, Vec *position)
{
    OMOBJ *effectObject = lbl_1_bss_8;

    if (effectObject == 0) {
        return;
    }
    if (effectIndex >= 2 || effectIndex < 0) {
        return;
    }
    lbl_1_bss_18[effectIndex].textureOffset = 0.0f;
    lbl_1_bss_18[effectIndex].framesRemaining = 120.0f;
    Hu3DModelDispOn(effectObject->mdlId[effectIndex]);
    Hu3DModelPosSet(effectObject->mdlId[effectIndex], position->x, position->y, position->z);
    Hu3DModelRotSet(effectObject->mdlId[effectIndex], 90.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(effectObject->mdlId[effectIndex], 1.0f, 1.0f, 1.0f);
    Hu3DLayerHookSet(10, (HU3D_LAYER_HOOK)fn_1_7DEC);
    effectObject->objFunc = fn_1_82F0;
}

/* 0x8688 */
const GXColor lbl_1_rodata_238[2] = {{0, 16, 64, 16}, {0, 0, 0, 0}};

/* Object-manager callback from scene setup: create the card effect models and textures. */
void fn_1_8688(OMOBJ *p)
{
    GXColor color;
    GXColor *cp;
    s16 i;
    HU3D_MODEL *mdl = NULL;

    omSetStatBit(p, OM_STAT_MODELPAUSE);
    lbl_1_bss_10 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_25, HU_MEMNUM_OVL, 2));
    lbl_1_bss_14 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_25, HU_MEMNUM_OVL, 2));
    lbl_1_bss_C =
        HuMemDirectMallocNum(2, GXGetTexBufferSize(640, 480, GX_TF_RGB565, 0, 0), HU_MEMNUM_OVL);
    for (i = 0; i < 2; i++) {
        p->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_OBJ1, HU_MEMNUM_OVL, 2));
        Hu3DModelPosSet(p->mdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(p->mdlId[i], 90.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(p->mdlId[i], 0.1f, 0.1f, 0.1f);
        Hu3DModelCameraSet(p->mdlId[i], 2);
        Hu3DModelLayerSet(p->mdlId[i], 3);
        Hu3DModelAttrSet(p->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelMatHookSet(p->mdlId[i], fn_1_7E58);
        lbl_1_bss_18[i].indirectMatrixRow0.x = lbl_1_bss_18[i].indirectMatrixRow0.y =
            lbl_1_bss_18[i].indirectMatrixRow0.z = lbl_1_bss_18[i].indirectMatrixRow1.x =
                lbl_1_bss_18[i].indirectMatrixRow1.y = lbl_1_bss_18[i].indirectMatrixRow1.z = 0.0f;
        color = lbl_1_rodata_238[0];
        cp = &color;
        lbl_1_bss_18[i].tint = *cp;
        lbl_1_bss_18[i].textureOffset = 0.0f;
        lbl_1_bss_18[i].textureOffsetStep = 0.0f;
        mdl = &Hu3DData[p->mdlId[i]];
        mdl->hookData = &lbl_1_bss_18[i];
    }
    p->objFunc = NULL;
    lbl_1_bss_8 = p;
}

/* 0x8A34 */
/* Effect cleanup helper: release models, animations, and texture memory. */
void fn_1_8A34(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    HU3D_MODEL *m = 0;
    s16 i;

    if (obj != 0) {
        Hu3DLayerHookReset(10);
        for (i = 0; i < 2; i++) {
            m = &Hu3DData[obj->mdlId[i]];
            m->hookData = 0;
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_340, obj);
    }
    /* Only the local pointer is cleared; lbl_1_bss_8 retains the deleted object's address. */
    obj = 0;
    if (lbl_1_bss_10 != 0) {
        HuSprAnimKill(lbl_1_bss_10);
    }
    lbl_1_bss_10 = 0;
    if (lbl_1_bss_14 != 0) {
        HuSprAnimKill(lbl_1_bss_14);
    }
    lbl_1_bss_14 = 0;
    if (lbl_1_bss_C != 0) {
        HuMemDirectFree(lbl_1_bss_C);
    }
    lbl_1_bss_C = 0;
}

/* ==================== module .data definitions (head, 0x0..0x214) ==================== */
OM_CAMERA_VIEW lbl_1_data_0 = {{0.0f, 250.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 7000.0f};
OM_CAMERA_VIEW lbl_1_data_1C = {{0.0f, -4.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 2800.0f};
char lbl_1_data_38[20] = "book_012-itemhook01";
char lbl_1_data_4C[20] = "book_013-itemhook02";
char lbl_1_data_60[20] = "book_014-itemhook03";
char lbl_1_data_74[20] = "book_015-itemhook04";
char *lbl_1_data_88[4] = {lbl_1_data_38, lbl_1_data_4C, lbl_1_data_60, lbl_1_data_74};
int lbl_1_data_98[8] = {
    FILESEL_ANM_12, FILESEL_ANM_13, FILESEL_ANM_14, FILESEL_ANM_15,
    FILESEL_ANM_16, FILESEL_ANM_17, FILESEL_ANM_18, FILESEL_ANM_19,
};
char lbl_1_data_B8[42] = "******* FileSelect ObjectSetup *********\n";
char lbl_1_data_E2[4] = "%d\n";
HU3D_PARMAN_PARAM lbl_1_data_E8 = {
    50,
    0,
    1.0f,
    10.0f,
    0.0f,
    {0.0f, -0.02f, 0.0f},
    2.0f,
    1.0f,
    20.0f,
    0.98f,
    2,
    {{240, 224, 32, 255}, {255, 255, 64, 255}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{192, 128, 16, 0}, {192, 64, 16, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
};
char lbl_1_data_138[12] = "data_select";
char lbl_1_data_144[16] = "data_select_ura";
Vec lbl_1_data_154[3] = {{-130.0f, 120.0f, 30.0f}, {20.0f, 10.0f, 60.0f}, {170.0f, 90.0f, 30.0f}};
char lbl_1_data_178[6] = "card1";
char lbl_1_data_17E[6] = "card2";
char lbl_1_data_184[6] = "card3";
char lbl_1_data_18A[6] = "card4";
char *lbl_1_data_190[4] = {lbl_1_data_178, lbl_1_data_17E, lbl_1_data_184, lbl_1_data_18A};
char lbl_1_data_1A0[13] = "siori_waku01";
char lbl_1_data_1AD[14] = "siori_moyou01";
char lbl_1_data_1BB[13] = "siori_moji01";
char lbl_1_data_1C8[21] = "siori_fix-itemhook_a";
char lbl_1_data_1DD[9] = "ABCDEFGH";
char lbl_1_data_1E6[5] = "%02d";
char lbl_1_data_1EB[5] = "%04d";
char lbl_1_data_1F0[3] = "%d";
char lbl_1_data_1F3[10] = "%f,%f,%f\n";
char lbl_1_data_1FD[23] = "siori_fix02-siori_waku";

OMOBJ *lbl_1_bss_344;
OMOBJMAN *lbl_1_bss_340;
static f32 wipeTime;
static u16 charHideF;
Vec lbl_1_bss_2E4[7];
s16 lbl_1_bss_2E0;
s16 lbl_1_bss_2DE;
HU3D_MODELID lbl_1_bss_2D8[3];
FILESEL_BOX lbl_1_bss_D8[4];
ANIMDATA *lbl_1_bss_CC[3];
ANIMDATA *lbl_1_bss_C8;
ANIMDATA *lbl_1_bss_A8[8];
HU3D_ANIMID lbl_1_bss_A2[3];
HU3D_PARMANID lbl_1_bss_98[5];
Vec lbl_1_bss_68[4];
BSS18_ENTRY lbl_1_bss_18[2];
ANIMDATA *lbl_1_bss_14;
ANIMDATA *lbl_1_bss_10;
void *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
s32 lbl_1_bss_0[2];
