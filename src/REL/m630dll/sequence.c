#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/mg/seqman.h"

void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_3BC(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_3FC(s16 mode, s16 frameNo);
void fn_1_428(s16 mode, s16 frameNo);
void fn_1_448(s16 mode, s16 frameNo);
void fn_1_474(s16 mode, s16 frameNo);
void fn_1_494(s16 mode, s16 frameNo);
void fn_1_4B4(s16 mode, s16 frameNo);
void fn_1_4E8(void);
void fn_1_5D0(void);
void fn_1_AF0(void);
int fn_1_594(void);
void fn_1_B50(void);
int fn_1_1670(void);
void fn_1_1E70(void);
void fn_1_1E90(void);

MGSEQ_PARAM lbl_1_data_0 = {
    30, 2, fn_1_F0, fn_1_3BC, fn_1_3DC, fn_1_3FC, fn_1_428,
    fn_1_448, fn_1_474, fn_1_494, fn_1_4B4
};
HUPROCESS *lbl_1_bss_0;

void fn_1_A0(void)
{
    lbl_1_bss_0 = omInitObjMan(200, 8192);
    omGameSysInit(lbl_1_bss_0);
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_F0(s16 mode, s16 frameNo)
{
    OM_CAMERA_VIEW view;
    HuVecF lightPos = { 0.0f, 1000.0f, 5000.0f };
    HuVecF lightDir = { 0.0f, -1.0f, -1.0f };
    s16 light;
    GXColor color = { 255, 255, 255, 255 };
    HuVecF shadowPos = { 0.0f, 5000.0f, -2500.0f };
    HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
    HuVecF shadowTarget = { 0.0f, 0.0f, -2800.0f };

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 10.0f, 20.0f, 30000.0f, 1.2f);
    omAddObjEx(lbl_1_bss_0, OM_OUTVIEW_PRIO, 0, 0, -1, omOutView);
    view.center = (HuVecF){ 0.0f, 349.7182f, -451.8600f };
    view.rot = (HuVecF){ -22.5770f, 0.0f, 0.0f };
    view.zoom = 8000.0f;
    omCameraViewSet(&view);
    light = Hu3DGLightCreateV(&lightPos, &lightDir, &color);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    Hu3DShadowCreate(-60.0f, 20.0f, 10000.0f);
    Hu3DShadowColSet(0, 0, 0);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    fn_1_4E8();
    MgSeqModeNext();
}

void fn_1_3BC(s16 mode, s16 frameNo)
{
    fn_1_5D0();
}

void fn_1_3DC(s16 mode, s16 frameNo)
{
    fn_1_AF0();
}

void fn_1_3FC(s16 mode, s16 frameNo)
{
    if (fn_1_594() != 0) {
        MgSeqModeNext();
    }
}

void fn_1_428(s16 mode, s16 frameNo)
{
    fn_1_B50();
}

void fn_1_448(s16 mode, s16 frameNo)
{
    if (fn_1_1670() != 0) {
        MgSeqModeNext();
    }
}

void fn_1_474(s16 mode, s16 frameNo)
{
    fn_1_1E70();
}

void fn_1_494(s16 mode, s16 frameNo)
{
    fn_1_1E90();
}

void fn_1_4B4(s16 mode, s16 frameNo)
{
    HuAudAllStop();
    Hu3DModelAllKill();
    Hu3DLightAllKill();
    omOvlReturnEx(1, 1);
}
