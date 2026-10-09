/* Sets up the motion-check overlay and renders its model, camera, and surface tests. */
#include "math.h"
#include "dolphin/gx.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hsfex.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "humath.h"
#include "string.h"
#include "REL/safdll/saf.h"

/* Resources and message used by the motion-check scene. */
#define SAF_TEST_HSF_ID DATANUM(DATA_saf, 0)
#define SAF_CAR_HSF_ID DATANUM(DATA_saf, 1)
#define SAF_SURFACE_HSF_ID DATANUM(DATA_saf, 3)
#define SAF_REFLECTION_ANIM_ID DATANUM(DATA_saf, 4)
#define SAF_WATER_BUMP_ANIM_ID DATANUM(DATA_effect, 10)
#define SAF_MICQUIZ_MESSAGE_BANK 28
#define SAF_TEST_MESSAGE_INDEX 66
#define SAF_TEST_MESSAGE_ID ((u32)((SAF_MICQUIZ_MESSAGE_BANK << 16) | SAF_TEST_MESSAGE_INDEX))
#define SAF_MARIO_DATA_DIR_ID DATA_mario
#define SAF_INSET_BASE_INTENSITY 24
#define SAF_INSET_MIN_ALPHA 224U
#define SAF_DEPTH_RAMP_FIRST 240
#define SAF_DEPTH_PALETTE_ENTRIES 256

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* The overlay loader calls this entry point to run static setup and start the scene. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* The overlay loader calls this entry point when unloading the module. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_3AC(void);
void fn_1_B30(OMOBJ *cameraObject);
extern s16 lbl_1_bss_1B4;
extern HUPROCESS *lbl_1_bss_1B8;
extern OMOBJ *lbl_1_bss_4BC;
extern OMOBJ *lbl_1_bss_4C0;
extern char lbl_1_data_30;
extern char lbl_1_data_4;

/* Starts the motion-check scene when the overlay loader enters this module. */
void fn_1_A0(void) {
    f32 reportValueLeft;
    f32 reportValueRight;
    f64 sum;

    OSReport(&lbl_1_data_4);
    lbl_1_bss_1B8 = omInitObjMan(50, 8192);
    CRot.x = (-20.0f);
    CRot.y = (0.0f);
    CRot.z = (0.0f);
    Center.x = (0.0f);
    Center.y = (100.0f);
    Center.z = (0.0f);
    CZoom = (400.0f);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, (45.0f), (20.0f), (1500.0f), (1.2f));
    Hu3DCameraViewportSet(1, (0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
    lbl_1_bss_4BC = omAddObjEx(lbl_1_bss_1B8, OM_OUTVIEW_PRIO, 0U, 0U, -1, omOutView);
    lbl_1_bss_4C0 = omAddObjEx(lbl_1_bss_1B8, 0, 64U, 0U, -1, fn_1_B30);
    HuPrcChildCreate(fn_1_3AC, 100U, 12288U, 0, lbl_1_bss_1B8);
    lbl_1_bss_1B4 = Hu3DGLightCreate((200.0f), (1000.0f), (1000.0f), (-0.2f), (-1.0f), (-1.0f),
                                     255U, 255U, 255U);
    Hu3DAmbColorSet((1.5f), (1.5f), (1.5f));
    Hu3DGLightInfinitytSet(lbl_1_bss_1B4);
    HuWinInit(0);
    reportValueLeft = (128.0f);
    reportValueRight = (1024.0f);
    sum = (f64)reportValueLeft + (f64)reportValueRight;
    OSReport(&lbl_1_data_30, sum);
}

extern void fn_1_6E0(void);

typedef struct { char text[4]; /* NUL-terminated three-digit window insertion. */ } SafText4;
typedef struct { char text[2]; /* NUL-terminated one-digit window insertion. */ } SafText2;
const SafText4 lbl_1_rodata_58 = { "100" };
const SafText2 lbl_1_rodata_5C = { "1" };

/* Runs as fn_1_A0's child process, showing the overlay's wipe and controller demonstrations. */
void fn_1_3AC(void) {
    f32 unused = (0.0f);
    HUWINID window;
    HuLoadProcStart(fn_1_6E0);
    HuPrcVSleep();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    WipeWait();
    {
        SafText4 hundredText = lbl_1_rodata_58;
        SafText2 oneText = lbl_1_rodata_5C;
        window = HuWinCreate((20.0f), (100.0f), 530, 100, 0);
        HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
        HuWinInsertMesSet(window, (u32)hundredText.text, 0);
        HuWinInsertMesSet(window, (u32)oneText.text, 1);
    }
    HuWinMesSet(window, SAF_TEST_MESSAGE_ID);
    HuWinMesWait(window);
    while ((HuPadBtnDown[0] & PAD_BUTTON_START) == 0) {
        if ((HuPadBtnDown[0] & PAD_BUTTON_LEFT) != 0) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_DISSOLVE_IN_BLUR, 60); WipeWait();
            HuPrcSleep(30); WipeCreate(WIPE_MODE_IN, WIPE_TYPE_DISSOLVE_IN_BLUR, 60); WipeWait();
        } else if ((HuPadBtnDown[0] & PAD_BUTTON_RIGHT) != 0) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_DISSOLVE_OUT_BLUR, 60); WipeWait();
            HuPrcSleep(30); WipeCreate(WIPE_MODE_IN, WIPE_TYPE_DISSOLVE_OUT_BLUR, 60); WipeWait();
        } else if ((HuPadBtnDown[0] & PAD_BUTTON_UP) != 0) {
            WipeCreate(WIPE_MODE_OUT, (WIPE_TYPE_VIEW_SHIFT | WIPE_TYPE_FBKEEP), 60); WipeWait();
            HuPrcSleep(30);
            WipeCreate(WIPE_MODE_IN, (WIPE_TYPE_VIEW_SHIFT | WIPE_TYPE_FBKEEP), 60);
            WipeWait();
        } else if ((HuPadBtnDown[0] & PAD_BUTTON_DOWN) != 0) {
            WipeCreate(WIPE_MODE_OUT, (WIPE_TYPE_WAVE | WIPE_TYPE_FBKEEP), 60); WipeWait();
            HuPrcSleep(30);
            WipeCreate(WIPE_MODE_IN, (WIPE_TYPE_WAVE | WIPE_TYPE_FBKEEP), 60);
            WipeWait();
        }
        HuPrcVSleep();
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20); WipeWait();
    omOvlReturnEx(1, 1); HuPrcEnd(); while (1) HuPrcVSleep();
}

extern void *lbl_1_bss_1AC;

extern void fn_1_1754(void);
extern void fn_1_22C4(void);
extern void fn_1_33FC(void);
extern void fn_1_3A24(void);

/* HuLoadProcStart runs this setup callback on the OS idle thread to load test models and
 * buffers. */
void fn_1_6E0(void) {
    Point3d shadowPos;
    Point3d shadowTarget;
    Point3d shadowUp;
    HU3D_MODELID model;
    Hu3DShadowCreate((30.0f), (20.0f), (2000.0f));
    shadowPos.x = (200.0f);
    shadowPos.y = (1000.0f);
    shadowPos.z = (200.0f);
    shadowUp.y = (1.0f);
    shadowUp.x = shadowUp.z = (0.0f);
    shadowTarget.x = shadowTarget.y = shadowTarget.z = (0.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet((0.6f));
    model = Hu3DModelCreate(HuDataSelHeapReadNum(SAF_TEST_HSF_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelShadowMapSet(model);
    model = Hu3DModelCreate(HuDataSelHeapReadNum(SAF_CAR_HSF_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelScaleSet(model, (0.2f), (0.2f), (0.2f));
    Hu3DModelShadowSet(model);
    model = Hu3DModelCreate(HuDataSelHeapReadNum(SAF_CAR_HSF_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (-150.0f));
    Hu3DModelScaleSet(model, (0.2f), (0.2f), (0.2f));
    Hu3DModelShadowSet(model);
    model = Hu3DModelCreate(HuDataSelHeapReadNum(SAF_CAR_HSF_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (150.0f));
    Hu3DModelScaleSet(model, (0.2f), (0.2f), (0.2f));
    Hu3DModelShadowSet(model);
    fn_1_3A24();
    HuMemHeapDump(HuMemHeapPtrGet(HEAP_DVD), -1);
    HuDataDirCloseAll();
    HuMemHeapDump(HuMemHeapPtrGet(HEAP_DVD), -1);
    HuDataDirRead(SAF_MARIO_DATA_DIR_ID);
    lbl_1_bss_1AC = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(640U, 480U, GX_TF_Z24X8, 0U, 0U), HU_MEMNUM_OVL);
    fn_1_1754();
    fn_1_22C4();
    fn_1_33FC();
    Hu3DModelDebug();
}

extern f32 lbl_1_bss_0;

/* Builds the supplied model transform with extra X rotation and a vertical oscillation
 * whenever this transform helper is invoked. */
void fn_1_9EC(HSF_OBJECT *object, HSF_TRANSFORM *transform, Mtx *previousMatrix,
              Mtx *currentMatrix) {
    Mtx work;

    PSMTXScale(work, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(work, (3.0f) + transform->rot.x, transform->rot.y, transform->rot.z);
    mtxTransCat(work, transform->pos.x,
        (f32)((f64)transform->pos.y +
              ((3.0) * sin(((3.141592653589793) * (f64)lbl_1_bss_0) /
                                    (180.0)))),
        transform->pos.z);
    PSMTXConcat(*previousMatrix, work, *currentMatrix);
    lbl_1_bss_0 += (3.0f);
    if (lbl_1_bss_0 > (360.0f)) {
        lbl_1_bss_0 -= (360.0f);
    }
}

#include "game/printfunc.h"

#include "datanum/saf.h"
#include "datanum/mario.h"

enum { MOTCHK_SUBSTICK_AXIS_MASK = 0xF8 };
extern u32 lbl_1_bss_49C[8];

#undef M_PI
#define M_PI (3.141592653589793)
#undef HuSin
#undef HuCos
#define HuSin(x) sin(M_PI * (x) / (180.0))
#define HuCos(x) cos(M_PI * (x) / (180.0))

/* Updates the motion-check camera from controller input on each object-manager tick. */
void fn_1_B30(OMOBJ *cameraObject)
{
    HuVecF pos;
    HuVecF offset;
    HuVecF dir;
    HuVecF yOfs;
    float rotZ;
    s8 stickPos;

    /* The exit path treats the eight-word storage block itself as a process pointer. */
    if(HuPadBtnDown[0] & PAD_BUTTON_START) {
        if(lbl_1_bss_49C) {
            HuPrcKill((HUPROCESS *)lbl_1_bss_49C);
        }
        omOvlReturn(1);
        return;
    }
    CRot.x += HuPadStkY[0]/20;
    CRot.y += HuPadStkX[0]/20;
    CZoom += HuPadTrigL[0]/2;
    CZoom -= HuPadTrigR[0]/2;
    pos.x = Center.x + (CZoom * (HuSin(CRot.y) * HuCos(CRot.x)));
    pos.y = (Center.y + (CZoom * -HuSin(CRot.x)));
    pos.z = (Center.z + (CZoom * (HuCos(CRot.y) * HuCos(CRot.x))));
    offset.x = Center.x - pos.x;
    offset.y = Center.y - pos.y;
    offset.z = Center.z - pos.z;
    dir.x = (HuSin(CRot.y) * HuSin(CRot.x));
    dir.y = HuCos(CRot.x);
    dir.z = (HuCos(CRot.y) * HuSin(CRot.x));
    rotZ = CRot.z;
    /* Compute the camera-relative vertical pan direction, including CRot.z roll. */
    yOfs.x = dir.x * (offset.x * offset.x + ((1.0f) - offset.x * offset.x) * HuCos(rotZ))
        + dir.y * (offset.x * offset.y * ((1.0) - HuCos(rotZ)) - offset.z * HuSin(rotZ))
        + dir.z * (offset.x * offset.z * ((1.0) - HuCos(rotZ)) + offset.y * HuSin(rotZ));

    yOfs.y = dir.y * (offset.y * offset.y + ((1.0f) - offset.y * offset.y) * HuCos(rotZ))
        + dir.x * (offset.x * offset.y * ((1.0) - HuCos(rotZ)) + offset.z * HuSin(rotZ))
        + dir.z * (offset.y * offset.z * ((1.0) - HuCos(rotZ)) - offset.x * HuSin(rotZ));

    yOfs.z = dir.z * (offset.z * offset.z + ((1.0f) - offset.z * offset.z) * HuCos(rotZ))
        + (dir.x * (offset.x * offset.z * ((1.0) - HuCos(rotZ)) - offset.y * HuSin(rotZ))
        + dir.y * (offset.y * offset.z * ((1.0) - HuCos(rotZ)) + offset.x * HuSin(rotZ)));
    VECCrossProduct(&dir, &offset, &offset);
    VECNormalize(&offset, &offset);
    stickPos = (HuPadSubStkX[0] & MOTCHK_SUBSTICK_AXIS_MASK);
    if (stickPos != 0) {
        Center.x += (0.05f) * (offset.x * stickPos);
        Center.y += (0.05f) * (offset.y * stickPos);
        Center.z += (0.05f) * (offset.z * stickPos);
    }
    VECNormalize(&yOfs, &offset);
    stickPos = -(HuPadSubStkY[0] & MOTCHK_SUBSTICK_AXIS_MASK);
    if (stickPos != 0) {
        Center.x += (0.05f) * (offset.x * stickPos);
        Center.y += (0.05f) * (offset.y * stickPos);
        Center.z += (0.05f) * (offset.z * stickPos);
    }
}

extern void fn_1_17C4(f32, f32, f32);
extern void fn_1_2380(void);
extern void fn_1_301C(void);

/* When invoked for feedback rendering, draws three planes unless B is held.
 * Holding A also enables the softened inset and full-screen intensity passes. */
void fn_1_16C0(void) {
    if ((HuPadBtn[0] & PAD_BUTTON_B) == 0) {
        if ((HuPadBtn[0] & PAD_BUTTON_A) != 0) {
            fn_1_2380();
        }
        fn_1_17C4((200.0f), (300.0f), (400.0f));
        if ((HuPadBtn[0] & PAD_BUTTON_A) != 0) {
            fn_1_301C();
        }
    }
}

#include "dolphin/gx/GXTexture.h"
#include "dolphin/os/OSCache.h"

extern void *lbl_1_bss_1A4;

/* Called by fn_1_6E0 to allocate and invalidate the RGB565 buffer for feedback planes. */
void fn_1_1754(void) {
    s32 textureBufferSizeBytes;

    textureBufferSizeBytes = GXGetTexBufferSize(640U, 480U, GX_TF_RGB565, 1U, 1U);
    lbl_1_bss_1A4 = HuMemDirectMallocNum(HEAP_MODEL, textureBufferSizeBytes, HU_MEMNUM_OVL);
    DCInvalidateRange(lbl_1_bss_1A4, (u32) textureBufferSizeBytes);
}

extern void *lbl_1_bss_1A4;

/* For each enabled plane, recaptures the current framebuffer and draws it at that depth, far to
 * near. */
void fn_1_17C4(float nearDepth, float middleDepth, float farDepth)
{
    Mtx44 projection;
    Mtx modelview;
    GXTexObj texture;
    Vec eye, target, up;
    HU3D_CAMERA *camera;
    float distance;
    camera = &Hu3DCamera[Hu3DCameraNo];
    C_MTXPerspective(projection, camera->fov, camera->aspect, camera->near, camera->far);
    GXSetProjection(projection, GX_PERSPECTIVE);
    GXSetViewport(camera->viewportX, camera->viewportY, camera->viewportW, camera->viewportH,
                  camera->viewportNear, camera->viewportFar);
    GXSetScissor((u32) camera->scissorX, (u32) camera->scissorY, (u32) camera->scissorW,
                 (u32) camera->scissorH);
    eye.x = eye.y = eye.z = (0.0f);
    target.x = target.y = (0.0f);
    target.z = (-100.0f);
    up.x = up.z = (0.0f);
    up.y = (1.0f);
    C_MTXLookAt(modelview, &eye, &up, &target);
    GXLoadPosMtxImm(modelview, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1U);
    GXSetNumTevStages(1U);
    GXSetZMode(1U, GX_LEQUAL, 1U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((-1.0f) != farDepth) {
        Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_RGB565, 1, lbl_1_bss_1A4);
        GXInitTexObj(&texture, lbl_1_bss_1A4, 320, 240, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXPixModeSync();
        GXLoadTexObj(&texture, GX_TEXMAP0);
        distance = farDepth;
        eye.x = eye.y = distance * tan((3.141592653589793) * (camera->fov / 2.0f) / (180.0));
        eye.x *= (1.2f);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-eye.x, eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (1.0f);
        GXPosition3f32(-eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
    }
    if ((-1.0f) != middleDepth) {
        Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_RGB565, 1, lbl_1_bss_1A4);
        GXInitTexObj(&texture, lbl_1_bss_1A4, 320, 240, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXPixModeSync();
        GXLoadTexObj(&texture, GX_TEXMAP0);
        distance = middleDepth;
        eye.x = eye.y = distance * tan((3.141592653589793) * (camera->fov / 2.0f) / (180.0));
        eye.x *= (1.2f);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-eye.x, eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (1.0f);
        GXPosition3f32(-eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
    }
    if ((-1.0f) != nearDepth) {
        Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_RGB565, 1, lbl_1_bss_1A4);
        GXInitTexObj(&texture, lbl_1_bss_1A4, 320, 240, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXPixModeSync();
        GXLoadTexObj(&texture, GX_TEXMAP0);
        distance = nearDepth;
        eye.x = eye.y = distance * tan((3.141592653589793) * (camera->fov / 2.0f) / (180.0));
        eye.x *= (1.2f);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-eye.x, eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (0.0f);
        GXPosition3f32(eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (1.0f);
        GXPosition3f32(-eye.x, -eye.y, -distance);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
    }
}

extern void *lbl_1_bss_1A0;
extern void *lbl_1_bss_1A4;
extern void *lbl_1_bss_1B0;

/* Called by fn_1_6E0 to allocate I8 intensity and RGB565 inset buffers.
 * Both invalidations address the existing feedback buffer instead of these new buffers. */
void fn_1_22C4(void) {
    s32 bufferSizeBytes;

    bufferSizeBytes = GXGetTexBufferSize(640U, 480U, GX_TF_I8, 1U, 1U);
    lbl_1_bss_1A0 = HuMemDirectMallocNum(HEAP_MODEL, bufferSizeBytes, HU_MEMNUM_OVL);
    DCInvalidateRange(lbl_1_bss_1A4, (u32) bufferSizeBytes);
    bufferSizeBytes = GXGetTexBufferSize(640U, 480U, GX_TF_RGB565, 0U, 0U);
    lbl_1_bss_1B0 = HuMemDirectMallocNum(HEAP_MODEL, bufferSizeBytes, HU_MEMNUM_OVL);
    DCInvalidateRange(lbl_1_bss_1A4, (u32) bufferSizeBytes);
}

extern void *lbl_1_bss_1A0;
extern void *lbl_1_bss_1B0;

/* Called by fn_1_16C0 to build the softened inset from captured color and intensity buffers. */
void fn_1_2380(void) {
    Mtx44 projectionMatrix;
    Mtx modelViewMatrix;
    GXTexObj textureObject;
    GXColor tevColor;
    HU3D_CAMERA *camera;
    f32 vertexOffset;
    s32 offsetMagnitude;
    s16 loopOffset;

    camera = &Hu3DCamera[Hu3DCameraNo];
    Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_I8, 1, lbl_1_bss_1A0);
    Hu3DFbCopyExec(0, 0, 200, 150, GX_TF_RGB565, 0, lbl_1_bss_1B0);
    C_MTXOrtho(projectionMatrix, (0.0f), (480.0f), (0.0f), (640.0f), (0.0f), (8000.0f));
    GXSetProjection(projectionMatrix, GX_ORTHOGRAPHIC);
    PSMTXIdentity(modelViewMatrix);
    GXLoadPosMtxImm(modelViewMatrix, 0U);
    GXSetScissor(0U, 0U, 200U, 150U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(0U, GX_ALWAYS, 0U);
    GXSetNumTexGens(1U);
    GXSetNumTevStages(1U);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (200.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (200.0f);
    GXWGFifo.f32 = (150.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (150.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    {
        tevColor.r = tevColor.g = tevColor.b = SAF_INSET_BASE_INTENSITY;
        GXSetTevColor(GX_TEVREG0, tevColor);
    }
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    /* Keep only bright captured pixels while accumulating their spread. */
    GXSetAlphaCompare(GX_GEQUAL, SAF_INSET_MIN_ALPHA, GX_AOP_OR, GX_GEQUAL, SAF_INSET_MIN_ALPHA);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZMode(0U, GX_ALWAYS, 0U);
    GXInitTexObj(&textureObject, lbl_1_bss_1A0, 320U, 240U, GX_TF_I8, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&textureObject, GX_NEAR, GX_NEAR, (0.0f), (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
    GXPixModeSync();
    GXLoadTexObj(&textureObject, GX_TEXMAP0);
    /* Draw offset copies from -7 through +6, dimming those farther from the center. */
    for (loopOffset = -7; loopOffset < 7; loopOffset += 1) {
        if (loopOffset < 0) {
            offsetMagnitude = -loopOffset;
        } else {
            offsetMagnitude = loopOffset;
        }
        tevColor.r = tevColor.g = tevColor.b =
            ((16.0) + ((192.0) * ((1.0) - (f64) ((f32) offsetMagnitude / (7.0f)))));
        GXSetTevColor(GX_TEVREG0, tevColor);
        vertexOffset = (f32) loopOffset;
        GXBegin(GX_QUADS, GX_VTXFMT0, 8U);
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = ((200.0f) + vertexOffset);
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = ((200.0f) + vertexOffset);
        GXWGFifo.f32 = ((150.0f) + vertexOffset);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = ((150.0f) + vertexOffset);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = -vertexOffset;
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = ((200.0f) + vertexOffset);
        GXWGFifo.f32 = -vertexOffset;
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = ((200.0f) + vertexOffset);
        GXWGFifo.f32 = ((150.0f) - vertexOffset);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = (1.0f);
        GXWGFifo.f32 = vertexOffset;
        GXWGFifo.f32 = ((150.0f) - vertexOffset);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (0.0f);
        GXWGFifo.f32 = (1.0f);
    }
    Hu3DFbCopyExec(0, 0, 200, 150, GX_TF_I8, 0, lbl_1_bss_1A0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXInitTexObj(&textureObject, lbl_1_bss_1B0, 200U, 150U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&textureObject, GX_NEAR, GX_NEAR, (0.0f), (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&textureObject, GX_TEXMAP0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (200.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (200.0f);
    GXWGFifo.f32 = (150.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (150.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXSetScissor((u32) camera->scissorX, (u32) camera->scissorY, (u32) camera->scissorW,
                 (u32) camera->scissorH);
}

extern void *lbl_1_bss_1A0;

typedef struct {
    GXTexObj texObj;       /* Captured inset intensity texture. */
    Mtx identity;          /* Identity model-view matrix for screen-space vertices. */
    Mtx44 ortho;           /* Projection in 640x480 screen coordinates. */
} Saf301CWork;

/* Called by fn_1_16C0 to draw the captured intensity image over the full screen. */
void fn_1_301C(void) {
    HU3D_CAMERA *camera;
    Saf301CWork work;

    camera = &Hu3DCamera[Hu3DCameraNo];
    C_MTXOrtho(work.ortho, (0.0f), (480.0f), (0.0f),
               (640.0f), (0.0f), (8000.0f));
    GXSetProjection(work.ortho, GX_ORTHOGRAPHIC);
    PSMTXIdentity(work.identity);
    GXLoadPosMtxImm(work.identity, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetNumTexGens(1U);
    GXSetNumTevStages(1U);
    GXInitTexObj(&work.texObj, lbl_1_bss_1A0, 200U, 150U, GX_TF_I8, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&work.texObj, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f),
                    (0.0f), 0U, 0U, GX_ANISO_1);
    GXPixModeSync();
    GXLoadTexObj(&work.texObj, GX_TEXMAP0);
    GXSetZMode(0U, GX_ALWAYS, 0U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (640.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (640.0f);
    GXWGFifo.f32 = (480.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (480.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
}

extern void *lbl_1_bss_198;
extern void *lbl_1_bss_19C;

f64 exp(f64);
f64 log(f64);

/* Called during fn_1_6E0 setup to allocate the depth image and build its alpha lookup table. */
void fn_1_33FC(void) {
    s32 bufferSizeBytes;
    s16 paletteEntry;
    f32 alphaRamp;
    s32 alphaLevel;

    bufferSizeBytes = GXGetTexBufferSize(640U, 480U, GX_TF_Z8, 0U, 0U);
    lbl_1_bss_19C = HuMemDirectMallocNum(HEAP_MODEL, bufferSizeBytes, HU_MEMNUM_OVL);
    DCInvalidateRange(lbl_1_bss_19C, (u32)bufferSizeBytes);
    bufferSizeBytes = 512;
    lbl_1_bss_198 = HuMemDirectMallocNum(HEAP_MODEL, bufferSizeBytes, HU_MEMNUM_OVL);
    /* The first 240 entries stay transparent; the last 16 fade into the depth ramp. */
    for (paletteEntry = 0; paletteEntry < SAF_DEPTH_RAMP_FIRST; ++paletteEntry) {
        ((u16 *)lbl_1_bss_198)[paletteEntry] = 0;
    }
    for (paletteEntry = SAF_DEPTH_RAMP_FIRST; paletteEntry < SAF_DEPTH_PALETTE_ENTRIES;
         ++paletteEntry) {
        alphaRamp = (f32)((1.0) -
                     ((f32)(paletteEntry - SAF_DEPTH_RAMP_FIRST) / (15.0f)));
        alphaLevel = (s32)((255.0) *
                      exp((f64)((-8.0f) * alphaRamp) *
                          log((2.0))));
        ((u16 *)lbl_1_bss_198)[paletteEntry] = (u16)(((u8)alphaLevel << 8) | (u8)alphaLevel);
    }
    DCFlushRange(lbl_1_bss_198, (u32)bufferSizeBytes);
}

extern void *lbl_1_bss_198;
extern void *lbl_1_bss_19C;

/* When invoked for the depth view, captures Z8 depth and draws it through the alpha palette. */
void fn_1_35CC(void) {
    f32 ortho[4][4];
    Mtx identity;
    GXTexObj texObj;
    GXTlutObj tlutObj;

    Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_Z8, 0, lbl_1_bss_19C);
    C_MTXOrtho(ortho, (0.0f), (480.0f), (0.0f),
               (640.0f), (0.0f), (8000.0f));
    GXSetProjection(ortho, GX_ORTHOGRAPHIC);
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1U);
    GXSetNumTevStages(1U);
    GXInitTlutObj(&tlutObj, lbl_1_bss_198, GX_TL_IA8, SAF_DEPTH_PALETTE_ENTRIES);
    GXLoadTlut(&tlutObj, 0U);
    GXInitTexObjCI(&texObj, lbl_1_bss_19C, 640U, 480U, GX_TF_C8,
                   GX_CLAMP, GX_CLAMP, 0U, 0U);
    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, (0.0f),
                    (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
    GXPixModeSync();
    GXLoadTexObj(&texObj, GX_TEXMAP0);
    GXSetZMode(0U, GX_ALWAYS, 0U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (640.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (640.0f);
    GXWGFifo.f32 = (480.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (1.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (480.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (0.0f);
    GXWGFifo.f32 = (1.0f);
}

extern HUPROCESS *lbl_1_bss_1B8;

void fn_1_3BB0(void);
void fn_1_4114(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

/* Called by fn_1_6E0 to create the animated test surface, install its material hook, and start
 * updates. */
void fn_1_3A24(void) {
    lbl_1_bss_4.model =
        Hu3DModelCreate(HuDataSelHeapReadNum(SAF_SURFACE_HSF_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_4.model, 1);
    Hu3DModelPosSet(lbl_1_bss_4.model, (0.0f), (130.0f), (0.0f));
    Hu3DModelScaleSet(lbl_1_bss_4.model, (0.2f), (0.2f), (0.2f));
    Hu3DModelMatHookSet(lbl_1_bss_4.model, fn_1_4114);
    lbl_1_bss_4.textureBuffer = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(640U, 480U, GX_TF_RGB565, 0U, 0U), HU_MEMNUM_OVL);
    lbl_1_bss_4.bumpAnimation =
        HuSprAnimRead(HuDataSelHeapReadNum(SAF_WATER_BUMP_ANIM_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_4.reflectionAnimation =
        HuSprAnimRead(HuDataSelHeapReadNum(SAF_REFLECTION_ANIM_ID, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_4.textureScrollOffset = (0.0f);
    HuPrcChildCreate(fn_1_3BB0, 100U, 12288U, 0, lbl_1_bss_1B8);
}

extern char lbl_1_data_54;

extern SafState lbl_1_bss_4;

/* Started by fn_1_3A24; finds the model's top edge and animates ripples on it each frame. */
void fn_1_3BB0(void) {
    HSF_OBJECT *object;
    s32 unused = 0;
    Vec surfacePosition;
    f32 topEdgeY;
    f32 wavePhaseDegrees;
    f32 distance;
    f32 absoluteDistance;
    s32 vertexIndex;
    s32 peakVertexIndex;
    s32 rippleIndex;
    s32 topEdgeVertexCount;
    Vec **topEdgeVertices;
    Vec *vertices;

    object = Hu3DModelObjPtrGet(lbl_1_bss_4.model, &lbl_1_data_54);
    Hu3DModelObjPosGet(lbl_1_bss_4.model, &lbl_1_data_54, &surfacePosition);
    /* -100000 marks each empty ripple slot until the A button starts a wave. */
    for (vertexIndex = 0; vertexIndex < 32; vertexIndex += 1) {
        lbl_1_bss_4.rippleX[vertexIndex] = (-100000.0f);
    }
    DCInvalidateRange(object->mesh.vertex->data, object->mesh.vertex->count * 12);
    topEdgeY = (-100000.0f);
    vertices = (Vec *)object->mesh.vertex->data;
    for (vertexIndex = 0; vertexIndex < object->mesh.vertex->count;
         vertexIndex += 1, vertices += 1) {
        if (topEdgeY < vertices->y) {
            topEdgeY = vertices->y;
        }
    }
    vertices = (Vec *)object->mesh.vertex->data;
    for (vertexIndex = topEdgeVertexCount = 0; vertexIndex < object->mesh.vertex->count;
         vertexIndex += 1, vertices += 1) {
        if (topEdgeY == vertices->y) {
            topEdgeVertexCount += 1;
        }
    }
    topEdgeVertices =
        (Vec **) HuMemDirectMallocNum(HEAP_HEAP, topEdgeVertexCount * 4, HU_MEMNUM_OVL);
    vertices = (Vec *)object->mesh.vertex->data;
    for (vertexIndex = peakVertexIndex = 0; vertexIndex < object->mesh.vertex->count;
         vertexIndex += 1, vertices += 1) {
        if (topEdgeY == vertices->y) {
            topEdgeVertices[peakVertexIndex] = vertices;
            peakVertexIndex += 1;
        }
    }
    wavePhaseDegrees = (0.0f);
    for (;;) {
        HuPrcVSleep();
        if ((HuPadBtnDown[0] & PAD_BUTTON_A) != 0) {
            /* Reuse the first free slot; the wave starts at a random point on model X. */
            for (rippleIndex = 0; rippleIndex < 32; rippleIndex += 1) {
                if ((-100000.0f) == lbl_1_bss_4.rippleX[rippleIndex]) {
                    lbl_1_bss_4.rippleX[rippleIndex] =
                        (f32) ((f64) (f32) (u32) frandmod(300) - (150.0));
                    lbl_1_bss_4.rippleHeight[rippleIndex] = (30.0f);
                    lbl_1_bss_4.rippleRadius[rippleIndex] = (0.0f);
                    break;
                }
            }
        }
        for (vertexIndex = 0; vertexIndex < topEdgeVertexCount; vertexIndex += 1) {
            vertices = topEdgeVertices[vertexIndex];
            vertices->y = topEdgeY + (5.0) *
                sin((3.141592653589793) * (2.0f * vertices->x + wavePhaseDegrees) / (180.0));
        }
        for (vertexIndex = 0; vertexIndex < 32; vertexIndex += 1) {
            if ((-100000.0f) != lbl_1_bss_4.rippleX[vertexIndex]) {
                for (rippleIndex = 0; rippleIndex < topEdgeVertexCount; rippleIndex += 1) {
                    vertices = topEdgeVertices[rippleIndex];
                    distance = lbl_1_bss_4.rippleX[vertexIndex] - vertices->x;
                    if (distance < (0.0f)) {
                        absoluteDistance = -distance;
                    } else {
                        absoluteDistance = distance;
                    }
                    distance = absoluteDistance;
                    /* Displace only vertices within 50 model units of the expanding crest. */
                    if (!(distance < (lbl_1_bss_4.rippleRadius[vertexIndex] - (50.0f))) &&
                        !(distance > ((50.0f) + lbl_1_bss_4.rippleRadius[vertexIndex]))) {
                        distance -= lbl_1_bss_4.rippleRadius[vertexIndex];
                        vertices->y = (f32)((f64)vertices->y +
                            ((f64)lbl_1_bss_4.rippleHeight[vertexIndex] *
                            cos(((3.141592653589793) *
                            (f64)((90.0f) *
                            (distance / (50.0f)))) /
                            (180.0))));
                    }
                }
                lbl_1_bss_4.rippleRadius[vertexIndex] += (2.0f);
                lbl_1_bss_4.rippleHeight[vertexIndex] *= (0.99);
                if (lbl_1_bss_4.rippleRadius[vertexIndex] > (500.0f)) {
                    lbl_1_bss_4.rippleX[vertexIndex] = (-100000.0f);
                }
            }
        }
        DCFlushRangeNoSync(object->mesh.vertex->data, object->mesh.vertex->count * 12);
        wavePhaseDegrees += (2.0f);
        lbl_1_bss_4.textureScrollOffset += (0.0002);
    }
}

const GXColor lbl_1_rodata_140 = { 160, 160, 255, 0 };

extern f32 lbl_1_data_3C[2][3];
extern void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId,
                         GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);

/* Registered by fn_1_3A24 and called by the model draw path to set up the surface material. */
void fn_1_4114(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material) {
    Mtx modelMatrix;
    Mtx textureMatrix;
    Mtx projectionTextureMatrix;
    Mtx inverseCameraMatrix;
    GXColor color;
    HU3D_CAMERA *camera = &Hu3DCamera[Hu3DCameraNo];

    color = lbl_1_rodata_140;
    Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_RGB565, 0, lbl_1_bss_4.textureBuffer);
    GXPixModeSync();
    Hu3DTexLoad(lbl_1_bss_4.textureBuffer, 640, 480, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 1,
                GX_TEXMAP0);
    HuSprTexLoad(lbl_1_bss_4.bumpAnimation, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    HuSprTexLoad(lbl_1_bss_4.reflectionAnimation, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    C_MTXLightPerspective(projectionTextureMatrix, camera->fov, camera->aspect,
                          (0.5f), (-0.5f),
                          (0.5f), (0.5f));
    PSMTXInverse(Hu3DCameraMtx, inverseCameraMatrix);
    PSMTXConcat(inverseCameraMatrix, drawObj->matrix, modelMatrix);
    PSMTXConcat(projectionTextureMatrix, Hu3DCameraMtx, textureMatrix);
    PSMTXConcat(textureMatrix, modelMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    PSMTXIdentity(textureMatrix);
    mtxTransCat(textureMatrix, lbl_1_bss_4.textureScrollOffset, (0.0f), (0.0f));
    mtxScaleCat(textureMatrix, (0.2f), (0.2f), (0.2f));
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_data_3C, -1);
    GXSetNumIndStages(1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
}
