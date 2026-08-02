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
#include "game/wipe.h"
#include "messnum/file_select.h"
#include "msm_se.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

typedef void (*VoidFunc)(void);

/* lbl_1_bss_348 element (fn_1_8F34 / fn_1_ABF8 / fn_1_B234), 6 bytes */
typedef struct b234_point { s16 x; s16 y; u16 flag; } B234Point;
/* fn_1_B234 out rect */
typedef struct b234_rect { f32 x0; f32 y0; f32 x1; f32 y1; } B234Rect;

/* lbl_1_bss_18 element (fn_1_8510 / fn_1_8688 / fn_1_82F0), stride 0x28 */
typedef struct bss18_entry {
    /* 0x00 */ Vec unk_0;
    /* 0x0C */ Vec unk_c;
    /* 0x18 */ GXColor unk_18;
    /* 0x1C */ f32 unk_1c;
    /* 0x20 */ f32 unk_20;
    /* 0x24 */ f32 unk_24;
} BSS18_ENTRY; /* size = 0x28 */

/* fn_1_7B64 rodata block */
typedef struct vec5 { Vec v[5]; } VEC5;

/* lbl_1_data_48C[] element (NameEnterInit sprite table), stride 0xC */
typedef struct data48c {
    /* 0x0 */ s32 unk_0;
    /* 0x4 */ u8  unk_4;
    /* 0x6 */ s16 unk_6;
    /* 0x8 */ s16 unk_8;
    /* 0xA */ s16 unk_A;
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
extern u8 lbl_1_bss_3B2[][0x11];
extern s16 lbl_1_bss_42A;
extern s16 lbl_1_bss_42E;
extern Vec lbl_1_bss_68[];

extern OM_CAMERA_VIEW lbl_1_data_0;
extern OM_CAMERA_VIEW lbl_1_data_1C;
extern u32 lbl_1_data_E8[20];
extern char lbl_1_data_138[];
extern char lbl_1_data_144[];
extern char lbl_1_data_1FD[0x17];
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
extern u8 saveBuf[][0xA000];
extern s64 SLSerialNo[];

s32 SLSaveFlagGet(void);
void SLSaveFlagSet(s32 flag);
void SLSaveDataMake(s32 arg, OSTime *time);
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
void fn_1_2654(s16 arg);
void fn_1_2A40(void);
void fn_1_35AC(void);
void fn_1_3C80(s16 id, Vec *pos);
void fn_1_3CC0(s16 id, f32 x, f32 y, f32 z);
void fn_1_3CF4(s16 id, s32 flag);
void fn_1_50DC(void (*func)(void), s32 arg1);
void fn_1_5130(void);
void fn_1_6314(s16 id, s32 flag);
void fn_1_63A0(s16 id, s32 flag);
s32 FileselEraseConfirm(s16 arg0);
s32 fn_1_69B0(void);
void FileselWipeDraw(void);
void fn_1_7B64(void);
void fn_1_7A0C(void);
void fn_1_7DEC(void);
void fn_1_7E58(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_82F0(OMOBJ *obj);
void fn_1_8510(s16 id, Vec *pos);
void fn_1_8688(OMOBJ *p);
void fn_1_8A34(void);
void fn_1_8F34(s32 arg0, s16 idx);
s16 fn_1_AAEC(s16 winId);
s16 fn_1_ABF8(s16 id, f32 refX, f32 refY, f32 dirX, f32 dirY);
void fn_1_B234(B234Rect *r);
void NameEnterClose(void);
void fn_1_3D14(void);
s32 FileselCopy(s16 arg);

void FileselInit(void);
void fn_1_15AC(void);
void fn_1_36C4(s16 a, s16 b);
void fn_1_6E54(s16 a, s32 b);
void NameEnterInit(void);
void fn_1_CB4(s32 arg);
s32 FileselSelect(void);

/* ======================================================================== */

int _prolog(void)
{
    const VoidFunc *ctor;

    for (ctor = _ctors; *ctor != 0; ctor++) {
        (*ctor)();
    }

    ObjectSetup();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor;

    for (dtor = _dtors; *dtor != 0; dtor++) {
        (*dtor)();
    }
}

/* 0x00A0 */
void ObjectSetup(void)
{
    HU3D_LIGHTID lightId;
    s16 lang;

    OSReport(lbl_1_data_B8);
    lbl_1_bss_340 = omInitObjMan(0x32, 0x200);
    omGameSysInit(lbl_1_bss_340);
    lang = GwLanguage;
    GwCommon.languageNo = lang;
    lightId = (int)Hu3DGLightCreate(-1000.0f, 1500.0f, 1000.0f,
                                    1.0f, -1.5f, -1.0f,
                                    0xff, 0xff, 0xff);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_344 = omAddObjEx(lbl_1_bss_340, 0x7fda, 0, 0, -1, omOutViewMulti);
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
    HuPrcChildCreate(FileselMain, 0x100, 0x3000, 0, HuPrcCurrentGet());
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
    HuWinMesLanguageSet(GwLanguage);
    HuWinMesRead();
    HuAudSStreamPlay(2);
    OSReport(lbl_1_data_E2, 0xcb2);
}

/* 0x0374 */
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
    if (status == -0x4d2) {
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
    HuDataDirClose(0xF20000);
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
        omOvlCallEx(0x5d, 1, 0, 0);
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* 0x0680 */
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
    HuMCMicGet();
}

/* 0x06E8 */
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

    lbl_1_bss_2D8[1] = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_BACKGROUND, HU_MEMNUM_OVL, 2));
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

    lbl_1_bss_2D8[2] = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_DATA_SELECT, HU_MEMNUM_OVL, 2));
    Hu3DModelCameraSet(lbl_1_bss_2D8[2], 2);
    Hu3DModelLayerSet(lbl_1_bss_2D8[2], 1);
    Hu3DModelAttrSet(lbl_1_bss_2D8[2], HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(lbl_1_bss_2D8[2]);

    lbl_1_bss_C8 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_DATA_SELECT, HU_MEMNUM_OVL, 2));
    lbl_1_bss_A2[0] = Hu3DAnimCreate(lbl_1_bss_C8, lbl_1_bss_2D8[2], lbl_1_data_138);
    Hu3DAnimBankSet(lbl_1_bss_A2[0], 1);
    lbl_1_bss_A2[1] = Hu3DAnimCreate(lbl_1_bss_C8, lbl_1_bss_2D8[2], lbl_1_data_144);
    Hu3DAnimBankSet(lbl_1_bss_A2[1], 0);

    HuPrcChildCreate(fn_1_15AC, 0x100, 0x1000, 0, HuPrcCurrentGet());
    NameEnterInit();
    Hu3DCameraLayerHookSet(1, 0, (HU3D_LAYER_HOOK)fn_1_C60);
    Hu3DCameraLayerHookSet(2, 0, (HU3D_LAYER_HOOK)fn_1_C8C);
    omAddObjEx(lbl_1_bss_340, 4096, 16, 16, -1, fn_1_8688);

    parAnim = HuSprAnimRead(HuDataSelHeapReadNum(DATA_effect, HU_MEMNUM_OVL, 2));
    for (i = 0; i < 5; i++) {
        lbl_1_bss_98[i] = Hu3DParManCreate(parAnim, 100, (HU3D_PARMAN_PARAM *)lbl_1_data_E8);
        Hu3DParManAttrSet(lbl_1_bss_98[i], 0x64);
        Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 1);
        Hu3DParManRotSet(lbl_1_bss_98[i], 90.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 2);
        Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_98[i]), 4);
        Hu3DParManAttrSet(lbl_1_bss_98[i], 1);
    }
}

/* 0x0C60 */
void fn_1_C60(void)
{
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    FileselWipeDraw();
}

/* 0x0C8C */
void fn_1_C8C(void)
{
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
}

void fn_1_CB4(s32 mode)
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
        if (mode != 0) {
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
                trans.z = trans.z + 500.0 * sin(3.141592653589793 * (180.0 * ((t - 70.0f) / (total - 70.0f))) / 180.0);
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
    if (mode != 0) {
        lbl_1_bss_2DE = HuWinCreate(16.0f, 326.0f, 544, 42, 0);
        HuWinAttrSet(lbl_1_bss_2DE, HUWIN_ATTR_ALIGN_CENTER);
        HuWinPriSet(lbl_1_bss_2DE, 0x64);
        HuWinBGTPLvlSet(lbl_1_bss_2DE, 0.0f);
        HuWinMesSpeedSet(lbl_1_bss_2DE, 0);
    }
}

/* 0x15AC */
void fn_1_15AC(void)
{
    Mtx mtx;
    f32 a;
    f32 b;

    a = 0.0f;
    b = 45.0f;
    while (1) {
        a += 2.0f;
        if (a > 360.0f) {
            a -= 360.0f;
        }
        b += 1.1;
        if (b > 360.0f) {
            b -= 360.0f;
        }
        PSMTXTrans(mtx, 0.0f, (f32)(3.0 * sin(3.141592653589793 * a / 180.0)), 0.0f);
        mtxRotCat(mtx, 0.0f, 0.0f, (f32)sin(3.141592653589793 * b / 180.0));
        Hu3DModelMtxSet(lbl_1_bss_2D8[0], &mtx);
        PSMTXTrans(mtx, (f32)sin(3.141592653589793 * a / 180.0), (f32)(3.0 * sin(3.141592653589793 * b / 180.0)), 0.0f);
        Hu3DModelMtxSet(lbl_1_bss_2D8[2], &mtx);
        HuPrcVSleep();
    }
}

/* 0x179C */
void fn_1_179C(void)
{
    HuWinExOpen(lbl_1_bss_2E0);
    HuPrcEnd();
}

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
        for (i = 1; i <= 0xa; i++) {
            w = &lbl_1_bss_D8[sel];
            t = (f32)i / 10.0f;
            fn_1_3CC0(sel, sv[sel].x, sv[sel].y, 20.0f * t + sv[sel].z);
            t = (f32)(sin(3.141592653589793 * (130.0f * t) / 180.0) * (1.0 / sin(2.2689280275926285)));
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
    HuWinMesSet(lbl_1_bss_2DE, 0x10004);
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
                    if (ret == -0x4d2) {
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
                if (ret == -0x4d2) {
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
                    if (ret == -0x4d2) {
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
                    if (ret == -0x4d2) {
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
            if (HuPadDStkRep[0] & 1) {
                dir = -1;
            }
            if (HuPadDStkRep[0] & 2) {
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
            for (i = 1; i <= 0xa; i++) {
                t = (f32)i / 10.0f;
                t = (f32)(sin(3.141592653589793 * (130.0f * t) / 180.0) * (1.0 / sin(2.2689280275926285)));
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
    for (i = 1; i <= 0xa; i++) {
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
    for (i = 1; i <= 0xa; i++) {
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
void fn_1_2654(s16 arg)
{
    f32 scale;
    s16 i;

    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].slotModel, &lbl_1_bss_68[i]);
    }
    Hu3DModelPosGet(*(s16 *)((u8 *)lbl_1_bss_2D8 + 4), &lbl_1_bss_68[i]);
    if (arg == -1) {
        for (i = 1; i <= 10; i++) {
            scale = 1.0 - cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
            fn_1_3CC0(0, lbl_1_bss_68[0].x, lbl_1_bss_68[0].y + 350.0f * scale, lbl_1_bss_68[0].z);
            fn_1_3CC0(1, lbl_1_bss_68[1].x, lbl_1_bss_68[1].y + 350.0f * scale, lbl_1_bss_68[1].z);
            fn_1_3CC0(2, lbl_1_bss_68[2].x, lbl_1_bss_68[2].y + 350.0f * scale, lbl_1_bss_68[2].z);
            Hu3DModelPosSet(*(s16 *)((u8 *)lbl_1_bss_2D8 + 4), lbl_1_bss_68[3].x - 200.0f * scale, lbl_1_bss_68[3].y, lbl_1_bss_68[3].z);
            HuPrcVSleep();
        }
    } else {
        for (i = 1; i <= 10; i++) {
            scale = 1.0 - cos(3.141592653589793 * (90.0 * ((double)i / 10.0)) / 180.0);
            switch (arg) {
            case 0:
                fn_1_3CC0(0, lbl_1_bss_68[0].x, lbl_1_bss_68[0].y + 300.0f * scale, lbl_1_bss_68[0].z);
                break;
            case 1:
                fn_1_3CC0(1, lbl_1_bss_68[1].x, lbl_1_bss_68[1].y - 400.0f * scale, lbl_1_bss_68[1].z);
                break;
            case 2:
                fn_1_3CC0(2, lbl_1_bss_68[2].x, lbl_1_bss_68[2].y + 300.0f * scale, lbl_1_bss_68[2].z);
                break;
            }
            HuPrcVSleep();
        }
    }
}

/* 0x2A40 */
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
    Hu3DModelPosGet(*(s16 *)((u8 *)lbl_1_bss_2D8 + 4), &v14);
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
        Hu3DModelPosSetV(*(s16 *)((u8 *)lbl_1_bss_2D8 + 4), &v8);
        HuPrcVSleep();
    }
}

/* 0x2C6C */
void FileselInit(void)
{
    FILESEL_BOX *p;
    s16 j;
    HUSPR_GROUPID grp;
    s16 i;
    HSF_DATA *hsf;
    HUSPRID spr;
    HUWINID win;
    ANIMDATA *animB;
    HSF_OBJECT *obj;
    ANIMDATA *animC;
    ANIMDATA *animA;

    HuSprExecLayerCameraSet(0x40, 2, 1);
    HuSprExecLayerCameraSet(120, 1, 8);
    animA = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_08, HU_MEMNUM_OVL, 2));
    grp = HuSprGrpCreate(1);
    spr = HuSprCreate(animA, 0, 0);
    HuSprGrpMemberSet(grp, 0, spr);
    HuSprGrpDrawNoSet(grp, 0x40);
    HuSprGrpPosSet(grp, 288.0f, 240.0f);
    HuSprGrpScaleSet(grp, 20.0f, 20.0f);
    animB = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_07, HU_MEMNUM_OVL, 2));
    animC = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_06, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[0] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_09, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[1] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_11, HU_MEMNUM_OVL, 2));
    lbl_1_bss_CC[2] = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_10, HU_MEMNUM_OVL, 2));
    for (i = 0; i < 8; i++) {
        lbl_1_bss_A8[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_98[i], HU_MEMNUM_OVL, 2));
    }
    i = i;
    for (i = 0; i < 4; i++) {
        p = &lbl_1_bss_D8[i];
        win = HuWinCreate((f32)(288.0 * (i & 1)), (f32)(168.0 * (i / 2)), 0xc0, 0x70, 0);
        p->window = win;
        p->waveOffset.x = p->waveOffset.y = p->waveOffset.z = 0.0f;
        HuWinScaleSet(win, 1.5f, 1.5f);
        HuWinBGTPLvlSet(win, 0.0f);
        HuWinAttrSet(win, HUWIN_ATTR_ALIGN_CENTER | HUWIN_ATTR_OUTLINE);
        p->cardTextureAnim = HuSprAnimMake(320, 168, ANIM_BMP_RGB5A3);
        p->cardTextureBuffer = p->cardTextureAnim->bmp->data = HuMemDirectMallocNum(0, GXGetTexBufferSize(320, 168, GX_TF_RGB5A3, 0, 0), HU_MEMNUM_OVL);
        HuWinDrawNoSet(win, 0x78);
        HuWinMesSpeedSet(win, 0);
        HuWinMesSet(win, 0x4a0009);
        grp = HuSprGrpCreate(0x11);
        p->spriteGroup = grp;
        HuSprGrpPosSet(grp, (f32)(288.0 * (i & 1)), (f32)(168.0 * (i / 2)));
        HuSprGrpScaleSet(grp, 1.5f, 1.5f);
        for (j = 0; j < 10; j++) {
        spr = HuSprCreate(animB, 95, 0);
            HuSprGrpMemberSet(grp, j, spr);
            HuSprPosSet(grp, j, (f32)(j * 12 + 42), 70.0f);
            HuSprAttrReset(grp, j, HUSPR_ATTR_LINEAR);
        }
        spr = HuSprCreate(animC, 0x5f, 0);
        HuSprGrpMemberSet(grp, 0xa, spr);
        HuSprPosSet(grp, 10, 70.0f, 96.0f);
        HuSprAttrReset(grp, 0xa, HUSPR_ATTR_LINEAR);
        for (j = 0; j < 5; j++) {
            spr = HuSprCreate(animB, 0x5f, 0);
            HuSprGrpMemberSet(grp, j + 0xc, spr);
            HuSprPosSet(grp, j + 12, (f32)(j * 12 + 88), 96.0f);
            HuSprAttrReset(grp, j + 0xc, HUSPR_ATTR_LINEAR);
        }
        HuSprGrpDrawNoSet(grp, 120);
        p->slotModel = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(p->slotModel, 2);
        Hu3DModelLayerSet(p->slotModel, 1);
        Hu3DModelAttrSet(p->slotModel, HU3D_MOTATTR_LOOP);
        j = Hu3DMotionMaxTimeGet(p->slotModel);
        Hu3DMotionTimeSet(p->slotModel, (f32)(s32)frandmod((s16)j));
        p->frameAnimation = Hu3DAnimCreate(lbl_1_bss_CC[0], p->slotModel, lbl_1_data_1A0);
        p->patternAnimation = Hu3DAnimCreate(lbl_1_bss_A8[0], p->slotModel, lbl_1_data_1AD);
        Hu3DModelShadowSet(p->slotModel);
        p->textModel = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_MOJI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(p->textModel, 2);
        Hu3DModelLayerSet(p->textModel, 2);
        Hu3DAnimCreate(p->cardTextureAnim, p->textModel, lbl_1_data_1BB);
        p->motionProcess = HuPrcChildCreate(fn_1_3D14, 0x10, 0x3000, 0, HuPrcCurrentGet());
        obj = Hu3DModelObjPtrGet(p->slotModel, lbl_1_data_1C8);
        strcpy(obj->name, lbl_1_data_190[i]);
        p->stateModel1 = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(p->stateModel1, 2);
        Hu3DModelLayerSet(p->stateModel1, 2);
        Hu3DModelAttrSet(p->stateModel1, HU3D_ATTR_DISPOFF);
        p->stateModel2 = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI2, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(p->stateModel2, 2);
        Hu3DModelLayerSet(p->stateModel2, 2);
        Hu3DModelAttrSet(p->stateModel2, HU3D_ATTR_DISPOFF);
        p->stateModel3 = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_SIORI_HIKARI2, HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(p->stateModel3, 2);
        Hu3DModelLayerSet(p->stateModel3, 2);
        Hu3DModelAttrSet(p->stateModel3, HU3D_ATTR_DISPOFF);
        hsf = Hu3DData[p->stateModel3].hsf;
        for (j = 0; j < hsf->materialNum; j++) {
            hsf->material[j].litColor[0] = hsf->material[j].color[0] = 0xf1;
            hsf->material[j].litColor[1] = hsf->material[j].color[1] = 0x7b;
            hsf->material[j].litColor[2] = hsf->material[j].color[2] = 0x12;
        }
        p->visualState = 0;
        Hu3DModelAttrSet(p->slotModel, HU3D_ATTR_DISPOFF);
        p->hasSave = 1;
        strcpy(p->fileName, lbl_1_data_1DD);
        OSTicksToCalendarTime(OSGetTime(), &p->saveTime);
        p->displayNumber = 0xa;
        fn_1_36C4(i, i % 3);
    }
    Hu3DCameraLayerHookSet(1, 1, (HU3D_LAYER_HOOK)fn_1_35AC);
}

/* 0x35AC */
void fn_1_35AC(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        FILESEL_BOX *w = &lbl_1_bss_D8[i];
        Hu3DFbCopyExec(1.1111111640930176 * (288.0 * (i & 1)),
                       168.0 * (i / 2),
                       0x140, 0xa8, 5, 0, w->cardTextureBuffer);
    }
}

/* 0x36C4 */
void fn_1_36C4(s16 a, s16 b)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[a];
    s16 sprId = p->spriteGroup;
    OSCalendarTime *q = &p->saveTime;
    u8 i;
    u8 len;
    char buf[12];

    p->slotNumberText[0] = (u8)(b + 0x31);
    p->slotNumberText[1] = 0;
    HuWinInsertMesSet(p->window, (u32)p->slotNumberText, 0);
    if (p->hasSave == 0) {
        p->patternVariant = -1;
        p->fileName[0] = 0;
        HuWinInsertMesSet(p->window, FILE_SELECT_NEW, 1);
        HuWinMesSet(p->window, FILE_SELECT_NAME_TEMPLATE);
        for (i = 0; i < 10; i++) {
            HuSprAttrReset(sprId, i, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(sprId, i, 0xc);
        }
        HuSprBankSet(sprId, 2, 0xb);
        HuSprBankSet(sprId, 5, 0xb);
        HuSprBankSet(sprId, 0xc, 0xa);
        HuSprBankSet(sprId, 12, 14);
        HuSprBankSet(sprId, 0xd, 0);
        Hu3DAnimAnimSet(p->patternAnimation, lbl_1_bss_A8[0]);
        HuSprBankSet(sprId, 0xd, 0);
        for (i = 0; i < 3; i++) {
            HuSprAttrSet(sprId, i + 0xe, HUSPR_ATTR_DISPOFF);
        }
        HuSprPosSet(sprId, 0xa, 84.0f, 96.0f);
        HuSprPosSet(sprId, 0xc, 102.0f, 96.0f);
        HuSprPosSet(sprId, 0xd, 114.0f, 96.0f);
    } else {
        for (i = 0; i < 15; i++) {
            HuSprAttrReset(sprId, i, HUSPR_ATTR_DISPOFF);
        }
        HuSprBankSet(sprId, 2, 0xb);
        HuSprBankSet(sprId, 5, 0xb);
        sprintf(buf, lbl_1_data_1E6, q->mon + 1);
        for (i = 0; i < 2; i++) {
            HuSprBankSet(sprId, i, buf[i] & 0xf);
        }
        sprintf(buf, lbl_1_data_1E6, q->mday);
        for (i = 0; i < 2; i++) {
            HuSprBankSet(sprId, i + 3, buf[i] & 0xf);
        }
        sprintf(buf, lbl_1_data_1EB, q->year);
        for (i = 0; i < 4; i++) {
            HuSprBankSet(sprId, i + 6, buf[i] & 0xf);
        }
        HuWinInsertMesSet(p->window, (u32)p->fileName, 1);
        HuWinMesSet(p->window, FILE_SELECT_NAME_TEMPLATE);
        HuSprBankSet(sprId, 12, 14);
        for (i = 0; i < 5; i++) {
            HuSprAttrSet(sprId, i + 0xd, HUSPR_ATTR_DISPOFF);
        }
        if (p->displayNumber > 0x270f) {
            p->displayNumber = 0x270f;
        }
        sprintf(buf, lbl_1_data_1F0, p->displayNumber);
        len = strlen(buf);
        HuSprPosSet(sprId, 0xa, (f32)(0x6c - (len * 0xc + 0x24) / 2), 96.0f);
        HuSprPosSet(sprId, 0xc, (f32)(0x7e - (len * 0xc + 0x24) / 2), 96.0f);
        for (i = 0; i < 4; i++) {
            if (buf[i] == 0) {
                break;
            }
            HuSprBankSet(sprId, i + 13, buf[i] & 15);
            HuSprAttrReset(sprId, i + 0xd, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(sprId, i + 0xd,
                        (f32)(0x7e - (len * 0xc + 0x24) / 2 + (i + 1) * 0xc),
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
void fn_1_3C80(s16 id, Vec *pos)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[id];

    p->basePosition = *pos;
}

/* 0x3CC0 */
void fn_1_3CC0(s16 id, f32 x, f32 y, f32 z)
{
    FILESEL_BOX *p = &lbl_1_bss_D8[id];

    p->basePosition.x = x;
    p->basePosition.y = y;
    p->basePosition.z = z;
}

/* 0x3CF4 */
void fn_1_3CF4(s16 id, s32 flag)
{
    lbl_1_bss_D8[id].motionState = (u8)flag;
}

/* 0x3D14 */
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
    angleX = (f32)(s32)frandmod(0x5a);
    angleY = (f32)(s32)frandmod(0xb4);
    delta = 0.01f * (f32)(s32)frandmod(0x64);
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
        Hu3DModelPosSet(p->slotModel, p->basePosition.x + p->waveOffset.x, p->basePosition.y + p->waveOffset.y,
            p->basePosition.z + p->waveOffset.z);
        if ((Hu3DModelAttrGet(p->slotModel) & HU3D_ATTR_DISPOFF) == 0) {
            Hu3DModelAttrReset(p->textModel, HU3D_ATTR_DISPOFF);
            Hu3DModelObjMtxGet(p->slotModel, lbl_1_data_190[i], mtx);
            Hu3DModelMtxSet(p->textModel, &mtx);
            src = &Hu3DData[p->slotModel];
            dst = &Hu3DData[p->stateModel1];
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

/* 0x457C (clone of bootDll CameraMove retargeted to camera index [1]) */
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

    subStick = HuPadSubStkX[0] & 0xF8;
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }

    PSVECNormalize(&roll, &delta);

    subStick = -(HuPadSubStkY[0] & 0xF8);
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }
}

/* 0x50DC */
void fn_1_50DC(void (*func)(void), s32 arg1)
{
    HUPROCESS *proc = HuPrcChildCreate(func, 0x10, 0x3000, 0, HuPrcCurrentGet());
    proc->property = (void *)arg1;
}

/* 0x5130 */
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

/* 0x5244 */
s32 FileselCopy(s16 arg)
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
    proc = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
    proc->property = (void *)2;
    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].slotModel, &pos[i]);
    }
    HuWinInsertMesSet(lbl_1_bss_2E0, (u32)lbl_1_bss_D8[arg].fileName, 0);
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_COPY_DESTINATION);
    HuWinMesWait(lbl_1_bss_2E0);
    HuWinMesSet(lbl_1_bss_2DE, 0x10002);
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
        Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, (f32)(1.5 + 0.5 * scale), (f32)(1.5 + 0.5 * scale), (f32)(1.5 + 0.5 * scale));
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
            if (HuPadDStkRep[0] & 1) {
                dir = -1;
            } else if (HuPadDStkRep[0] & 2) {
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
                    scale = sin(3.141592653589793 * (130.0 * (i / 5.0)) / 180.0) * (1.0 / sin(2.2689280275926285));
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
            procB = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
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
            Hu3DModelScaleSet(lbl_1_bss_D8[cur].slotModel, 1.5f * scale, 1.5f * scale, 1.5f * scale);
            HuPrcVSleep();
        }
        lbl_1_bss_D8[cur].visualState = 0;
        memcpy(&saveBuf[curSlotNo][SLBoxDataOffsetGet(cur)], &saveBuf[curSlotNo][SLBoxDataOffsetGet(arg)], 0xcb2);
        sndH = HuAudFXPlay(MSM_SE_FILESEL_06);
        ret = FileSaveMesOpen(-1, FILE_SELECT_COPYING);
        HuAudFXStop(sndH);
        HuAudFXPlay(MSM_SE_FILESEL_07);
        if (ret == -0x4d2) {
            ret = -4;
        }
        if (ret == 0) {
            lbl_1_bss_D8[cur].hasSave = 1;
            lbl_1_bss_D8[cur].patternVariant = lbl_1_bss_D8[arg].patternVariant;
            lbl_1_bss_D8[cur].saveTime = lbl_1_bss_D8[arg].saveTime;
            lbl_1_bss_D8[cur].displayNumber = lbl_1_bss_D8[arg].displayNumber;
            memcpy(lbl_1_bss_D8[cur].fileName, lbl_1_bss_D8[arg].fileName, FILESEL_FILENAME_LENGTH);
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
        procC = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
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
        procD = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
        procD->property = (void *)1;
        return -0x4d2;
    }
}

/* 0x6314 */
void fn_1_6314(s16 id, s32 flag)
{
    if (flag != 0) {
        Hu3DAnimAnimSet(lbl_1_bss_D8[id].frameAnimation, lbl_1_bss_CC[1]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_D8[id].frameAnimation, lbl_1_bss_CC[0]);
    }
}

/* 0x63A0 */
void fn_1_63A0(s16 id, s32 flag)
{
    if (flag != 0) {
        Hu3DAnimAnimSet(lbl_1_bss_D8[id].frameAnimation, lbl_1_bss_CC[2]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_D8[id].frameAnimation, lbl_1_bss_CC[0]);
    }
}

/* 0x642C */
s32 FileselEraseConfirm(s16 arg0)
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
    proc1 = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
    proc1->property = (void *)3;
    Hu3DAnimAnimSet(lbl_1_bss_D8[arg0].frameAnimation, lbl_1_bss_CC[1]);
    HuWinInsertMesSet(lbl_1_bss_2E0, (u32)&lbl_1_bss_D8[arg0].fileName, 0);
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_ERASE_CONFIRMATION);
    HuWinMesWait(lbl_1_bss_2E0);
    HuWinChoiceSet(lbl_1_bss_2E0, -1);
    while (w->stat != 0) {
        if (UnMountCnt != 0) {
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_CARD_REMOVED);
            HuWinMesWait(lbl_1_bss_2E0);
            UnMountCnt = 0;
            proc2 = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
            proc2->property = (void *)1;
            Hu3DAnimAnimSet(lbl_1_bss_D8[arg0].frameAnimation, lbl_1_bss_CC[0]);
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
        proc3 = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
        proc3->property = (void *)1;
        Hu3DAnimAnimSet(lbl_1_bss_D8[arg0].frameAnimation, lbl_1_bss_CC[0]);
        return -4;
    }
    if (ret == 0) {
        fxId = HuAudFXPlay(MSM_SE_FILESEL_08);
        SLSaveEmptySet(curSlotNo, arg0);
        ret = FileSaveMesOpen(-1, FILE_SELECT_ERASING);
        HuAudFXStop(fxId);
        fxId = HuAudFXPlay(MSM_SE_FILESEL_09);
        if (ret == 0) {
            HuAudFXPlay(MSM_SE_FILESEL_11);
            for (i = 1; i <= 0x14; i++) {
                s = 1.0 - sin(3.141592653589793 * (90.0 * ((double)i / 20.0)) / 180.0);
                Hu3DModelScaleSet(lbl_1_bss_D8[arg0].slotModel, 1.5f * s, 1.5f * s, 1.5f * s);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            lbl_1_bss_D8[arg0].hasSave = 0;
            lbl_1_bss_D8[arg0].hasSave = 0;
            fn_1_36C4(arg0, arg0);
            lbl_1_bss_D8[arg0].visualState = 1;
            Hu3DAnimAnimSet(lbl_1_bss_D8[arg0].frameAnimation, lbl_1_bss_CC[0]);
            for (i = 1; i <= 0xa; i++) {
                s = (double)i / 10.0;
                Hu3DModelScaleSet(lbl_1_bss_D8[arg0].slotModel, 1.5f * s, 1.5f * s, 1.5f * s);
                HuPrcVSleep();
            }
            HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_ERASE_COMPLETE);
            HuWinMesWait(lbl_1_bss_2E0);
        }
    } else {
        ret = 0;
    }
    proc4 = HuPrcChildCreate(fn_1_5130, 0x10, 0x3000, 0, HuPrcCurrentGet());
    proc4->property = (void *)1;
    Hu3DAnimAnimSet(lbl_1_bss_D8[arg0].frameAnimation, lbl_1_bss_CC[0]);
    return ret;
}

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
void FileselWipeDraw(void)
{
    HU3D_CAMERA *cam = &Hu3DCamera[Hu3DCameraNo];
    Mtx44 proj;
    Mtx view;
    Vec camPos;
    Vec target;
    Vec up;
    f32 nearz;

    C_MTXPerspective(proj, cam->fov, cam->aspect, cam->near, cam->far);
    GXSetProjection(proj, 0);
    GXSetViewport(cam->viewportX, cam->viewportY, cam->viewportW, cam->viewportH, cam->viewportNear, cam->viewportFar);
    GXSetScissor(cam->scissorX, cam->scissorY, cam->scissorW, cam->scissorH);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetBlendMode(1, 4, 4, 5);
    GXSetTevOrder(0, 0xff, 0xff, 0);
    GXSetTevColorIn(0, 0xf, 0xf, 0xf, 0xc);
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

    nearz = cam->far - 1.0f;
    camPos.x = camPos.y = nearz * tan(3.141592653589793 * (cam->fov / 2.0f) / 180.0);
    camPos.x = camPos.x * cam->aspect;

    GXBegin(0x80, 0, 4);
    GXPosition3f32(-camPos.x, -camPos.y, -nearz);
    GXPosition3f32(camPos.x, -camPos.y, -nearz);
    GXPosition3f32(camPos.x, camPos.y, -nearz);
    GXPosition3f32(-camPos.x, camPos.y, -nearz);

    GXSetColorUpdate(1);
}

/* 0x6E54 */
void fn_1_6E54(s16 a, s32 b)
{
    HUPROCESS *proc1;
    HUPROCESS *proc2;
    FILESEL_BOX *wp;
    HUPROCESS *proc3;
    FILESEL_BOX *bp;
    Vec vpos;
    static const Vec l_19c = {0.0f, 0.0f, 15.0f};
    Vec scale30;
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
    if (b == 0) {
        HuWinDispOff(lbl_1_bss_2DE);
        HuWinExClose(lbl_1_bss_2E0);
        Hu3DModelScaleGet(lbl_1_bss_2D8[2], &scale30);
        if (scale30.x > 0.1f) {
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
            scale30.x = (1.0 - phase) * pos18.x;
            scale30.y = (1.0 - phase) * pos18.y;
            scale30.z = (1.0 - phase) * pos18.z;
            Hu3DModelPosSetV(lbl_1_bss_2D8[0], &scale30);
            scale30.x = (1.0 - phase) * rot0c.x;
            scale30.y = (1.0 - phase) * rot0c.y;
            scale30.z = (1.0 - phase) * rot0c.z;
            Hu3DModelRotSetV(lbl_1_bss_2D8[0], &scale30);
            HuPrcVSleep();
        }
        proc1 = HuPrcChildCreate(fn_1_7A0C, 0x10, 0x3000, 0, HuPrcCurrentGet());
        proc1->property = (void *)0;
        HuPrcSleep(40);
        HuAudFXPlay(MSM_SE_FILESEL_14);
        fn_1_8510(0, &vpos);
    } else {
        for (i = 0; i < 3; i++) {
            lbl_1_bss_D8[a].motionState = 0;
        }
        HuWinDispOff(lbl_1_bss_2DE);
        HuWinExClose(lbl_1_bss_2E0);
        for (i = 0; i < 3; i++) {
            lbl_1_bss_D8[a].motionState = 2;
        }
        for (i = 1; i <= 10; i++) {
            phase = (f32)i / 10.0f;
            for (j = 0; j < 3; j++) {
                if (j != a) {
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
        bp = &lbl_1_bss_D8[a];
        Hu3DModelPosGet(bp->slotModel, &pos24);
        OSReport(lbl_1_data_1F3, pos24.x, pos24.y, pos24.z);
        if ((s32)frandmod(2) == 0) {
            dir = 1;
        } else {
            dir = -1;
        }
        dirf = (f32)dir;
        for (i = 1; i <= 80; i++) {
            if (i <= 20) {
                phase = (f32)sin(3.141592653589793 * (90.0 * ((f32)i / 20.0f)) / 180.0);
                scale30.x = (1.0 - phase) * pos18.x;
                scale30.y = (1.0 - phase) * pos18.y;
                scale30.z = (1.0 - phase) * pos18.z;
                Hu3DModelPosSetV(lbl_1_bss_2D8[0], &scale30);
                scale30.x = (1.0 - phase) * rot0c.x;
                scale30.y = (1.0 - phase) * rot0c.y;
                scale30.z = (1.0 - phase) * rot0c.z;
                Hu3DModelRotSetV(lbl_1_bss_2D8[0], &scale30);
                Hu3DShadowTPLvlSet(0.5 * (1.0 - phase));
            }
            if (i == 10) {
                proc2 = HuPrcChildCreate(fn_1_7B64, 0x10, 0x3000, 0, HuPrcCurrentGet());
                proc2->property = (void *)a;
                HuAudFXPlay(MSM_SE_FILESEL_13);
            }
            if (i > 10) {
                phase = ((f32)i - 10.0f) / 70.0f;
                scale30.x = (1.0 - phase) * pos24.x;
                scale30.y = (1.0 - phase) * pos24.y + 100.0 * sin(3.141592653589793 * (180.0f * phase) / 180.0);
                scale30.z = pos24.z + 300.0 * sin(3.141592653589793 * (180.0f * phase) / 180.0) - 150.0f * phase;
                wp = &lbl_1_bss_D8[a];
                wp->basePosition = scale30;
                Hu3DModelRotSet(bp->slotModel, 0.0f, 480.0f * phase, 45.0f * phase * dirf);
                sv = 1.125 + 1.125 * (1.0 - phase);
                Hu3DModelScaleSet(bp->slotModel, sv, sv, sv);
            }
            if (i == 40) {
                proc3 = HuPrcChildCreate(fn_1_7A0C, 0x10, 0x3000, 0, HuPrcCurrentGet());
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
                Hu3DParManVecSet(lbl_1_bss_98[i], dir.x, -0.5f, dir.z);
            }
        } else {
            for (i = 0; i < 5; i++) {
                Hu3DParManAttrSet(lbl_1_bss_98[i], 1);
            }
        }
        HuPrcVSleep();
    }
}

/* 0x7DEC */
void fn_1_7DEC(void)
{
    if (lbl_1_bss_C != 0) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(640, 480, GX_TF_RGB565, 0);
        GXCopyTex(lbl_1_bss_C, 0);
    }
}

/* 0x7E58 */
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
    q->unk_1c = q->unk_1c + q->unk_20;

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
    GXLoadTexMtxImm(texMtx, 0x21, 0);
    GXSetTexCoordGen2(0, 0, 0, 0x21, 0, 0x7d);
    PSMTXTrans(tmpMtx, 0.0f, q->unk_1c, 0.0f);
    PSMTXScale(scaleMtx, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(scaleMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, 0x1e, 1);
    GXSetTexCoordGen2(1, 1, 4, 0x1e, 0, 0x7d);
    GXSetTexCoordGen2(2, 1, 4, 0x3c, 0, 0x7d);

    GXSetTevColor(1, q->unk_18);
    GXSetTevOrder(0, 0, 1, 0);
    GXSetTevColorIn(0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    GXSetTevOrder(1, 1, 0, 0);
    GXSetTevColorIn(1, 0xf, 8, 2, 0);
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
void fn_1_82F0(OMOBJ *obj)
{
    BSS18_ENTRY *e;
    s16 i;
    f32 t;

    for (i = 0, e = lbl_1_bss_18; i < 2; i++, e++) {
        if (e->unk_24 <= 0.0f) {
            continue;
        }
        e->unk_0.x = 0.0f;
        e->unk_0.y = 0.0f;
        e->unk_0.z = 0.09f * e->unk_24 / 120.0f;
        e->unk_c.x = 0.0f;
        e->unk_c.y = 0.0f;
        e->unk_c.z = 0.09f * e->unk_24 / 120.0f;
        e->unk_20 = -0.02f * e->unk_24 / 120.0f;
        t = (e->unk_24 - 80.0f) / 40.0f;
        if (t >= 0.0f) {
            e->unk_18.r = 0;
            e->unk_18.g = 16.0f * t;
            e->unk_18.b = 64.0f * t;
        }
        e->unk_24 = e->unk_24 - 1.0f;
        if (e->unk_24 <= 0.0f) {
            Hu3DLayerHookReset(10);
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            obj->objFunc = 0;
        }
    }
}

/* 0x8510 */
void fn_1_8510(s16 id, Vec *pos)
{
    OMOBJ *p = lbl_1_bss_8;

    if (p == 0) {
        return;
    }
    if (id >= 2 || id < 0) {
        return;
    }
    lbl_1_bss_18[id].unk_1c = 0.0f;
    lbl_1_bss_18[id].unk_24 = 120.0f;
    Hu3DModelDispOn(p->mdlId[id]);
    Hu3DModelPosSet(p->mdlId[id], pos->x, pos->y, pos->z);
    Hu3DModelRotSet(p->mdlId[id], 90.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(p->mdlId[id], 1.0f, 1.0f, 1.0f);
    Hu3DLayerHookSet(0xa, (HU3D_LAYER_HOOK)fn_1_7DEC);
    p->objFunc = fn_1_82F0;
}

/* 0x8688 */
const GXColor lbl_1_rodata_238[2] = {{0, 16, 64, 16}, {0, 0, 0, 0}};

void fn_1_8688(OMOBJ *p)
{
    GXColor color;
    GXColor *cp;
    s16 i;
    HU3D_MODEL *mdl = NULL;

    omSetStatBit(p, OM_STAT_MODELPAUSE);
    lbl_1_bss_10 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_25, HU_MEMNUM_OVL, 2));
    lbl_1_bss_14 = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_ANM_25, HU_MEMNUM_OVL, 2));
    lbl_1_bss_C = HuMemDirectMallocNum(2, GXGetTexBufferSize(640, 480, GX_TF_RGB565, 0, 0), HU_MEMNUM_OVL);
    for (i = 0; i < 2; i++) {
        p->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(FILESEL_HSF_OBJ1, HU_MEMNUM_OVL, 2));
        Hu3DModelPosSet(p->mdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(p->mdlId[i], 90.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(p->mdlId[i], 0.1f, 0.1f, 0.1f);
        Hu3DModelCameraSet(p->mdlId[i], 2);
        Hu3DModelLayerSet(p->mdlId[i], 3);
        Hu3DModelAttrSet(p->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelMatHookSet(p->mdlId[i], fn_1_7E58);
        lbl_1_bss_18[i].unk_0.x = lbl_1_bss_18[i].unk_0.y = lbl_1_bss_18[i].unk_0.z =
            lbl_1_bss_18[i].unk_c.x = lbl_1_bss_18[i].unk_c.y = lbl_1_bss_18[i].unk_c.z =
                0.0f;
        color = lbl_1_rodata_238[0];
        cp = &color;
        lbl_1_bss_18[i].unk_18 = *cp;
        lbl_1_bss_18[i].unk_1c = 0.0f;
        lbl_1_bss_18[i].unk_20 = 0.0f;
        mdl = &Hu3DData[p->mdlId[i]];
        mdl->hookData = &lbl_1_bss_18[i];
    }
    p->objFunc = NULL;
    lbl_1_bss_8 = p;
}

/* 0x8A34 */
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
char lbl_1_data_38[0x14] = "book_012-itemhook01";
char lbl_1_data_4C[0x14] = "book_013-itemhook02";
char lbl_1_data_60[0x14] = "book_014-itemhook03";
char lbl_1_data_74[0x14] = "book_015-itemhook04";
char *lbl_1_data_88[4] = {lbl_1_data_38, lbl_1_data_4C, lbl_1_data_60, lbl_1_data_74};
int lbl_1_data_98[8] = {0x0023000C, 0x0023000D, 0x0023000E, 0x0023000F, 0x00230010, 0x00230011, 0x00230012, 0x00230013};
char lbl_1_data_B8[0x2A] = "******* FileSelect ObjectSetup *********\n";
char lbl_1_data_E2[0x4] = "%d\n";
u32 lbl_1_data_E8[20] = {0x00320000, 0x3F800000, 0x41200000, 0x00000000, 0x00000000, 0xBCA3D70A, 0x00000000, 0x40000000, 0x3F800000, 0x41A00000, 0x3F7AE148, 0x0002F0E0, 0x20FFFFFF, 0x40FF0000, 0x00000000, 0x0000C080, 0x1000C040, 0x10000000, 0x00000000, 0x00000000};
char lbl_1_data_138[0xC] = "data_select";
char lbl_1_data_144[0x10] = "data_select_ura";
Vec lbl_1_data_154[3] = {{-130.0f, 120.0f, 30.0f}, {20.0f, 10.0f, 60.0f}, {170.0f, 90.0f, 30.0f}};
char lbl_1_data_178[0x6] = "card1";
char lbl_1_data_17E[0x6] = "card2";
char lbl_1_data_184[0x6] = "card3";
char lbl_1_data_18A[0x6] = "card4";
char *lbl_1_data_190[4] = {lbl_1_data_178, lbl_1_data_17E, lbl_1_data_184, lbl_1_data_18A};
char lbl_1_data_1A0[0xD] = "siori_waku01";
char lbl_1_data_1AD[0xE] = "siori_moyou01";
char lbl_1_data_1BB[0xD] = "siori_moji01";
char lbl_1_data_1C8[0x15] = "siori_fix-itemhook_a";
char lbl_1_data_1DD[0x9] = "ABCDEFGH";
char lbl_1_data_1E6[0x5] = "%02d";
char lbl_1_data_1EB[0x5] = "%04d";
char lbl_1_data_1F0[0x3] = "%d";
char lbl_1_data_1F3[0xA] = "%f,%f,%f\n";
char lbl_1_data_1FD[0x17] = "siori_fix02-siori_waku";

OMOBJ *lbl_1_bss_344;
OMOBJMAN *lbl_1_bss_340;
static u8 filesel_bss_pad_338[8];
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
