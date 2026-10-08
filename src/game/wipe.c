/* Draws screen wipes and the loading banner while the game changes scenes or loads. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
double cos(double x);
#include "game/wipe.h"
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/disp.h"
#include "game/armem.h"

#include "datanum/win.h"

typedef BOOL (*FADEFUNC)(void);

/* Current wipe effect, color, frame images, mode, and progress used by all wipe renderers. */
WIPEWORK wipeData;
/* Set when WipeCreate starts an incoming wipe; game-specific code clears it when needed. */
BOOL wipeFadeInF;

/* Save-banner animation drawn while the OS idle thread initializes game data. */
static ANIMDATA *wipeLoadAnim;
/* Save-banner pulse phase in degrees; it advances by 3 degrees per rendered frame. */
static float wipeLoadTime;

static void WipeLoadDraw(void);

static BOOL WipeNormalFade(void);
static BOOL WipeCrossFade(void);
static BOOL WipeDissolve(void);
static BOOL WipeViewShift(void);
static BOOL WipeStar(void);
static BOOL WipeWave(void);
static BOOL WipeKoopa(void);
static BOOL WipeMoon(void);
static BOOL WipeSun(void);
static BOOL WipeSunMoon(void);

static FADEFUNC fadeInFunc[WIPE_TYPE_MAX] = {
    WipeNormalFade, //WIPE_TYPE_NORMAL
    WipeCrossFade, //WIPE_TYPE_CROSS
    WipeDissolve, //WIPE_TYPE_DISSOLVE_IN
    WipeDissolve, //WIPE_TYPE_DISSOLVE_OUT
    WipeViewShift, //WIPE_TYPE_VIEW_SHIFT
    WipeNormalFade, //WIPE_TYPE_PREV
    WipeNormalFade, //WIPE_TYPE_WHITE
    WipeStar, //WIPE_TYPE_STAR
    WipeWave, //WIPE_TYPE_WAVE
    WipeKoopa, //WIPE_TYPE_KOOPA
    WipeMoon, //WIPE_TYPE_MOON
    WipeSun, //WIPE_TYPE_SUN
    WipeSunMoon //WIPE_TYPE_SUNMOON
};

static FADEFUNC fadeOutFunc[WIPE_TYPE_MAX] = {
    WipeNormalFade, //WIPE_TYPE_NORMAL
    WipeCrossFade, //WIPE_TYPE_CROSS
    WipeDissolve, //WIPE_TYPE_DISSOLVE_IN
    WipeDissolve, //WIPE_TYPE_DISSOLVE_OUT
    WipeViewShift, //WIPE_TYPE_VIEW_SHIFT
    WipeNormalFade, //WIPE_TYPE_PREV
    WipeNormalFade, //WIPE_TYPE_WHITE
    WipeStar, //WIPE_TYPE_STAR
    WipeWave, //WIPE_TYPE_WAVE
    WipeKoopa, //WIPE_TYPE_KOOPA
    WipeMoon, //WIPE_TYPE_MOON
    WipeSun, //WIPE_TYPE_SUN
    WipeSunMoon //WIPE_TYPE_SUNMOON
};

/* Resets wipe state during main-system startup; the render mode is not used here. */
void WipeInit(GXRenderModeObj *rmode)
{
    wipeData.color.r = wipeData.color.g = wipeData.color.b = 0;
    wipeData.color.a = 255;
    wipeData.type = WIPE_TYPE_NORMAL;
    wipeData.mode = WIPE_MODE_END;
    wipeData.maxTime = wipeData.time = 100;
    wipeData.image[0] = NULL;
}

/* Advances the active wipe and draws its overlay once per main-loop retrace. */
void WipeExecAlways(void)
{
    switch(wipeData.mode) {
        case WIPE_MODE_IN:
            wipeData.time++;
            if(!fadeInFunc[wipeData.type & WIPE_TYPE_MASK]()) {
                wipeData.mode = WIPE_MODE_DUMMY;
                WipeColorSet(0, 0, 0);
            }
            break;
        
        case WIPE_MODE_OUT:
            wipeData.time++;
            if(!fadeInFunc[wipeData.type & WIPE_TYPE_MASK]()) {
                wipeData.mode = WIPE_MODE_END;
            }
            break;
        
        case WIPE_MODE_END:
            fadeOutFunc[wipeData.type & WIPE_TYPE_MASK]();
            WipeLoadDraw();
            break;
    }
}

/* Starts a requested transition unless a wipe is active; at system exit, an incoming wipe is
 * discarded to start an outgoing wipe. */
void WipeCreate(s16 mode, s16 type, s16 maxTime)
{
    /* Keep the outgoing/final screen in place while system exit is pending. */
    if(omSysExitReq && (wipeData.mode == WIPE_MODE_OUT || wipeData.mode == WIPE_MODE_END)) {
        return;
    }
    if(omSysExitReq && wipeData.mode == WIPE_MODE_IN && mode == WIPE_MODE_OUT) {
        wipeData.mode = WIPE_MODE_DUMMY;
    }
    if(wipeData.mode == WIPE_MODE_OUT || wipeData.mode == WIPE_MODE_IN) {
        return;
    }
    if(type == WIPE_TYPE_PREV) {
        /* Save/load flows request the most recently selected wipe with this sentinel. */
        type = wipeData.type;
    }
    if(type == WIPE_TYPE_WHITE) {
        /* Maps white to the normal fade and requests a white tint; WipeColorSet ignores that
         * request while the current mode is END. */
        WipeColorSet(255, 255, 255);
        type = WIPE_TYPE_NORMAL;
    }
    if(mode == WIPE_MODE_IN) {
        wipeFadeInF = TRUE;
        if(!(type & WIPE_TYPE_FBKEEP)) {
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] = NULL;
            if(wipeData.image[1]) {
                HuMemDirectFree(wipeData.image[1]);
            }
            wipeData.image[1] = NULL;
        }
        WipeLoadKill();
    } else if(mode == WIPE_MODE_OUT) {
        
    }
    wipeData.maxTime = maxTime;
    wipeData.type = type;
    wipeData.mode = mode;
    wipeData.time = 0;
}

/* Changes the wipe tint while a transition can still accept color updates. */
void WipeColorSet(u8 r, u8 g, u8 b)
{
    if(wipeData.mode == WIPE_MODE_OUT || wipeData.mode == WIPE_MODE_END) {
        return;
    }
    wipeData.color.r = r;
    wipeData.color.g = g;
    wipeData.color.b = b;
}

/* Returns the selected wipe effect, including any framebuffer-keep flag. */
u8 WipeTypeGet(void)
{
    return wipeData.type;
}

/* Reports whether a transition is active, excluding idle and dummy states. */
u8 WipeCheck(void)
{
    if(wipeData.mode == WIPE_MODE_END || wipeData.mode == WIPE_MODE_DUMMY) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Reports whether the wipe has left the dummy state. */
u8 WipeCheckIn(void)
{
    if(wipeData.mode == WIPE_MODE_DUMMY) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void WipeGXInit(void);

/* fadeInFunc/fadeOutFunc select this full-screen color fade; it advances alpha each frame. */
static BOOL WipeNormalFade(void)
{
    GXColor fadeColor = wipeData.color;
    fadeColor.a = 255*(wipeData.time/wipeData.maxTime);
    if(wipeData.mode == WIPE_MODE_IN) {
        if(wipeData.time <= 1) {
            /* Begin the incoming transition fully covered before reducing alpha. */
            fadeColor.a = 255;
        } else {
            fadeColor.a = 255-fadeColor.a;
            
        }
    }
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, fadeColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXEnd();
    if(wipeData.mode == WIPE_MODE_END) {
        return TRUE;
    }
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Draws the saved framebuffer during incoming cross wipes, falling back to the normal color fade
 * when no saved image exists. */
static BOOL WipeCrossFade(void)
{
    GXColor fadeColor;
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuMemDirectMalloc(HEAP_HEAP, GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                                GX_TF_RGB565, GX_FALSE, 0));
            DCFlushRange(wipeData.image[0], GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                               GX_TF_RGB565, GX_FALSE, 0));
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        wipeData.time = wipeData.maxTime;
        return FALSE;
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                wipeData.image[0] = NULL;
                return FALSE;
            }
        }
    }
    fadeColor.a = 255*(wipeData.time/wipeData.maxTime);
    if(wipeData.mode == WIPE_MODE_IN) {
        fadeColor.a = 255-fadeColor.a;
    }
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_CLAMP,
                GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, fadeColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXTexCoord2f32(1, 1);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXTexCoord2f32(0, 1);
    GXEnd();
    return TRUE;
}

/* Uses a rotating saved-frame overlay for outgoing dissolve wipes; incoming dissolves use the
 * normal color fade. */
static BOOL WipeDissolve(void)
{
    if(wipeData.mode == WIPE_MODE_IN) {
        if(wipeData.image[0]) {
            HuMemDirectFree(wipeData.image[0]);
        }
        wipeData.image[0] = NULL;
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_END) {
        WipeNormalFade();
        return TRUE;
    } else {
        GXColor overlayColor;
        Mtx translationMatrix;
        Mtx rotationMatrix;
        Mtx modelView;
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuMemDirectMalloc(HEAP_HEAP, GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                                GX_TF_RGB565, GX_FALSE, 0));
        }
        /* Capture the outgoing frame when the transition reaches its first frame. */
        if(wipeData.time == 1.0) {
            Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        }
        
        overlayColor.r = overlayColor.g = overlayColor.b = 255*(wipeData.time/wipeData.maxTime);
        overlayColor.a = 224;
        WipeGXInit();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_CLAMP,
                    GX_CLAMP, GX_FALSE, GX_TEXMAP0);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColor(GX_COLOR1, overlayColor);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        MTXTrans(translationMatrix, -(HU_FB_WIDTH/2), -(HU_FB_HEIGHT/2), 0);
        MTXRotDeg(rotationMatrix, 'Z', wipeData.time/10.0);
        MTXConcat(rotationMatrix, translationMatrix, modelView);
        mtxTransCat(modelView, (HU_FB_WIDTH/2), (HU_FB_HEIGHT/2), 0);
        GXLoadPosMtxImm(modelView, GX_PNMTX0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        if(wipeData.type == WIPE_TYPE_DISSOLVE_IN_BLUR) {
            GXPosition2s16(-19, -14);
            GXTexCoord2f32(0, 0);
            GXPosition2s16(HU_FB_WIDTH+19, -14);
            GXTexCoord2f32(1, 0);
            GXPosition2s16(HU_FB_WIDTH+19, HU_FB_HEIGHT+14);
            GXTexCoord2f32(1, 1);
            GXPosition2s16(-19, HU_FB_HEIGHT+14);
            GXTexCoord2f32(0, 1);
        } else {
            GXPosition2s16(10, 10);
            GXTexCoord2f32(0, 0);
            GXPosition2s16(HU_FB_WIDTH-10, 10);
            GXTexCoord2f32(1, 0);
            GXPosition2s16(HU_FB_WIDTH-10, HU_FB_HEIGHT-10);
            GXTexCoord2f32(1, 1);
            GXPosition2s16(10, HU_FB_HEIGHT-10);
            GXTexCoord2f32(0, 1);
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        WipeNormalFade();
        if(wipeData.maxTime <= wipeData.time) {
            wipeData.time = wipeData.maxTime;
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

static void TransformPoint(float fieldOfView, float viewportWidth, float viewportHeight,
                           Mtx modelView, Vec *point, Vec *transformedPoint);

/* Rotates a six-by-six grid of saved-frame tiles by 270 degrees with row-and-column-staggered
 * progress; incoming wipes without a saved image use the normal fade. */
static BOOL WipeViewShift(void)
{
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuMemDirectMalloc(HEAP_HEAP, GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                                GX_TF_RGB565, GX_FALSE, 0));
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        wipeData.time = wipeData.maxTime;
        return FALSE;
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                wipeData.image[0] = NULL;
                return FALSE;
            }
        }
    }
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeCrossFade();
    } else {
        Mtx44 projection;
        Mtx modelview;
        Vec cameraPosition;
        Vec cameraTarget;
        Vec cameraUp;
        Vec topLeftViewPoint, bottomRightViewPoint;
        Vec panelNearPoint, panelFarPoint;
        s16 column, row;
        s16 halfPanelWidth;
        
        MTXPerspective(projection, 20.0f, (float)HU_FB_WIDTH/HU_FB_HEIGHT, 100, 3000);
        GXSetProjection(projection, GX_PERSPECTIVE);
        cameraPosition.x = cameraPosition.y = 0;
        cameraPosition.z = 1000;
        cameraUp.x = cameraUp.z = 0;
        cameraUp.y = 1;
        cameraTarget.x = cameraTarget.y = cameraTarget.z = 0;
        MTXLookAt(modelview, &cameraPosition, &cameraUp, &cameraTarget);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
        GXSetScissor(0, 0, 640, 480);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetCullMode(GX_CULL_NONE);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_CLAMP,
                    GX_CLAMP, GX_FALSE, GX_TEXMAP0);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        cameraPosition.x = cameraPosition.y = 0;
        cameraPosition.z = 1000;
        TransformPoint(20.0f, HU_FB_WIDTH / 6.0, HU_FB_HEIGHT / 6, modelview, &cameraPosition,
                       &topLeftViewPoint);
        cameraPosition.x = HU_FB_WIDTH/6.0;
        cameraPosition.y = HU_FB_HEIGHT/6;
        cameraPosition.z = 1000;
        TransformPoint(20.0f, HU_FB_WIDTH / 6.0, HU_FB_HEIGHT / 6, modelview, &cameraPosition,
                       &bottomRightViewPoint);
        halfPanelWidth = (bottomRightViewPoint.x-topLeftViewPoint.x)/2;
        for(row=5; row>=0; row--) {
            for(column=0; column<6; column++) {
                /* Stagger each tile's 270-degree turn by its row and column position. */
                float rotationProgress = (column+(wipeData.time-(row*6)))/(wipeData.maxTime-36);
                if(rotationProgress < 0.0f) {
                    rotationProgress = 0.0f;
                }
                if(rotationProgress > 1.0) {
                    rotationProgress = 1.0f;
                }
                GXSetViewport((HU_FB_WIDTH / 6.0) * column, (HU_FB_HEIGHT / 6.0) * row,
                              HU_FB_WIDTH / 6.0, HU_FB_HEIGHT / 6.0, 0, 1);
                panelNearPoint.x =
                    halfPanelWidth +
                    ((halfPanelWidth * HuCos(270.0f * rotationProgress)) + topLeftViewPoint.x);
                panelNearPoint.y = topLeftViewPoint.y;
                panelNearPoint.z =
                    (halfPanelWidth * HuSin(270.0f * rotationProgress)) + topLeftViewPoint.z;
                panelFarPoint.x = halfPanelWidth +
                                  ((halfPanelWidth * HuCos((270.0f * rotationProgress) + 180.0f)) +
                                   topLeftViewPoint.x);
                panelFarPoint.y = bottomRightViewPoint.y;
                panelFarPoint.z = (halfPanelWidth * HuSin((270.0f * rotationProgress) + 180.0f)) +
                                  topLeftViewPoint.z;
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(panelNearPoint.x, panelNearPoint.y, panelNearPoint.z);
                GXTexCoord2f32((1/6.0)+((1/6.0)*column), (1/6.0)*row);
                GXPosition3f32(panelFarPoint.x, panelNearPoint.y, panelFarPoint.z);
                GXTexCoord2f32(((1/6.0)*column), (1/6.0)*row);
                GXPosition3f32(panelFarPoint.x, panelFarPoint.y, panelFarPoint.z);
                GXTexCoord2f32(((1/6.0)*column), (1/6.0)+((1/6.0)*row));
                GXPosition3f32(panelNearPoint.x, panelFarPoint.y, panelNearPoint.z);
                GXTexCoord2f32((1/6.0)+((1/6.0)*column), (1/6.0)+((1/6.0)*row));
            }
        }
        return TRUE;
    }
    
}

static BOOL WipeImage(int animationDataNumber);

/* fadeInFunc/fadeOutFunc route star wipes here to draw their WIN animation. */
static BOOL WipeStar(void)
{
    return WipeImage(WIN_ANM_wipe_star);
}

/* fadeInFunc/fadeOutFunc route Koopa wipes here to draw their WIN animation. */
static BOOL WipeKoopa(void)
{
    return WipeImage(WIN_ANM_wipe_koopa);
}

/* fadeInFunc/fadeOutFunc route moon wipes here to draw their WIN animation. */
static BOOL WipeMoon(void)
{
    return WipeImage(WIN_ANM_wipe_moon);
}

/* fadeInFunc/fadeOutFunc route sun wipes here to draw their WIN animation. */
static BOOL WipeSun(void)
{
    return WipeImage(WIN_ANM_wipe_sun);
}

/* fadeInFunc/fadeOutFunc route sun-and-moon wipes here to draw their WIN animation. */
static BOOL WipeSunMoon(void)
{
    return WipeImage(WIN_ANM_wipe_sunmoon);
}

/* Loads WIN animation art and draws it with solid panels to close or open the screen around the
 * current scene. */
static BOOL WipeImage(int animationDataNumber)
{
    GXColor wipeColor;
    Mtx modelview;
    float imageScale;
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuSprAnimRead(HuAR_ARAMtoMRAMFileRead(animationDataNumber, -256, HEAP_MODEL));
        }
        imageScale = HuSin(90.0*(wipeData.time/wipeData.maxTime));
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(wipeData.time == 1.0) {
            /* Start the incoming wipe with fresh art, even if an earlier image remains. */
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] =
                HuSprAnimRead(HuAR_ARAMtoMRAMFileRead(animationDataNumber, -256, HEAP_MODEL));
        }
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuSprAnimRead(HuAR_ARAMtoMRAMFileRead(animationDataNumber, -256, HEAP_MODEL));
        }
        if(wipeData.maxTime <= wipeData.time) {
            wipeData.time = wipeData.maxTime;
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] = NULL;
            return FALSE;
        } else if(wipeData.time == 1) {
            /* Keep the effect image full-size on its first incoming frame. */
            imageScale = 1;
        } else {
            imageScale = HuCos(90.0*(wipeData.time/wipeData.maxTime));
        }
    }
    wipeColor = wipeData.color;
    wipeColor.a = 255.0f*imageScale;
    WipeGXInit();
    MTXTrans(modelview, -HU_FB_WIDTH, -HU_FB_HEIGHT, 0);
    mtxScaleCat(modelview, 3.0f, 3.0f, 3.0f);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    HuSprTexLoad(wipeData.image[0], 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, wipeColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(imageScale*(HU_FB_WIDTH/2), imageScale*(HU_FB_HEIGHT/2));
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH-(imageScale*(HU_FB_WIDTH/2)), imageScale*(HU_FB_HEIGHT/2));
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH - (imageScale * (HU_FB_WIDTH / 2)),
                   HU_FB_HEIGHT - (imageScale * (HU_FB_HEIGHT / 2)));
    GXTexCoord2f32(1, 1);
    GXPosition2u16(imageScale*(HU_FB_WIDTH/2), HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, wipeColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    GXPosition2u16(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXPosition2u16(HU_FB_WIDTH, imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(0, imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(0, HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXPosition2u16(0, imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(imageScale*(HU_FB_WIDTH/2), imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(imageScale*(HU_FB_WIDTH/2), HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXPosition2u16(0, HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH-(imageScale*(HU_FB_WIDTH/2)), imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(HU_FB_WIDTH, imageScale*(HU_FB_HEIGHT/2));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT-(imageScale*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH - (imageScale * (HU_FB_WIDTH / 2)),
                   HU_FB_HEIGHT - (imageScale * (HU_FB_HEIGHT / 2)));
    GXEnd();
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Index of the wave texture variant selected for the next outgoing wave wipe. */
static s16 waveSprIdx;
/* Indirect-texture matrix scale used to reduce wave distortion through the transition. */
static float waveTexMtx[2][3] = {
    0.02f, 0, 0,
    0, 0.02f, 0
};

/* Distorts the framebuffer with a wave texture; incoming wipes without a saved image fall back to
 * the normal color fade. */
static BOOL WipeWave(void)
{
    Mtx textureTranslation;
    Mtx textureScale;
    Mtx textureMatrix;
    GXColor waveColor;
    float waveProgress;
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] =
                HuMemDirectMalloc(HEAP_HEAP, GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                                GX_TF_RGB565, GX_FALSE, 0));
            DCFlushRange(wipeData.image[0], GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2,
                                                               GX_TF_RGB565, GX_FALSE, 0));
            wipeData.image[1] = HuSprAnimRead(HuDataRead(WIN_ANM_wipe_wave+waveSprIdx));
            waveSprIdx++;
            if(waveSprIdx >= 1) {
                /* The wave wipe cycles through one texture entry. */
                waveSprIdx = 0;
            }
        }
        waveProgress = 1.0-(wipeData.time/wipeData.maxTime);
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                if(wipeData.image[1]) {
                    HuMemDirectFree(wipeData.image[1]);
                }
                wipeData.image[0] = NULL;
                wipeData.image[1] = NULL;
                return FALSE;
            } else {
                waveProgress = wipeData.time/wipeData.maxTime;
            }
        }
    }
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
    waveColor.a = 255*waveProgress;
    waveTexMtx[0][0] = 1.0-waveProgress;
    waveTexMtx[1][1] = 1.0-waveProgress;
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_CLAMP,
                GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    HuSprTexLoad(wipeData.image[1], 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumTexGens(2);
    GXSetNumTevStages(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    MTXTrans(textureTranslation, 0, 0, 0);
    MTXScale(textureScale, 1, 1, 1);
    MTXConcat(textureScale, textureTranslation, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTevColor(GX_COLOR1, waveColor);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waveTexMtx, -1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXTexCoord2f32(1, 1);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTevDirect(GX_TEVSTAGE0);
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Wipe renderers and the banner call this to establish shared orthographic GX state. */
static void WipeGXInit(void)
{
    Mtx44 projection;
    Mtx modelview;
    MTXOrtho(projection, 0, HU_FB_HEIGHT, 0, HU_FB_WIDTH, 0, 10);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetCullMode(GX_CULL_NONE);
}

/* WipeViewShift uses this to convert camera-space tile corners through the inverse view matrix. */
static void TransformPoint(float fieldOfView, float viewportWidth, float viewportHeight,
                           Mtx modelView, Vec *point, Vec *transformedPoint)
{
    float halfFovTangent = HuSin(fieldOfView/2)/HuCos(fieldOfView/2);
    float frustumHeight = 2.0f*(halfFovTangent*point->z);
    float frustumWidth = frustumHeight*(viewportWidth/viewportHeight);
    float normalizedX = point->x/viewportWidth;
    float normalizedY = point->y/viewportHeight;
    Mtx invModelview;
    transformedPoint->x = (normalizedX-0.5)*frustumWidth;
    transformedPoint->y = -(normalizedY-0.5)*frustumHeight;
    transformedPoint->z = -point->z;
    MTXInverse(modelView, invModelview);
    MTXMultVec(invModelview, transformedPoint, transformedPoint);
}

/* HuLoadProcStart loads the save banner before starting asynchronous game initialization. */
void WipeLoadCreate(void)
{
    wipeLoadAnim = HuSprAnimDataRead(WIN_ANM_save_banner);
    wipeLoadTime = 0;
}

/* LoadProcWatch releases the banner after loading; WipeCreate also releases it for incoming
 * wipes. */
void WipeLoadKill(void)
{
    if(wipeLoadAnim) {
        HuSprAnimKill(wipeLoadAnim);
        wipeLoadAnim = NULL;
    }
}

/* WipeExecAlways calls this in the idle wipe state to draw the pulsing loading banner. */
static void WipeLoadDraw(void)
{
    Mtx modelview;
    ANIMBMP *bannerBitmap;
    float pulseOffsetX;
    float pulseOffsetY;
    float bannerWidth;
    float bannerHeight;
    if(!wipeLoadAnim) {
        return;
    }
    WipeGXInit();
    MTXTrans(modelview, 0, 0, 0);
    mtxScaleCat(modelview, 1, 1, 1);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    HuSprTexLoad(wipeLoadAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    bannerBitmap = &wipeLoadAnim->bmp[0];
    bannerWidth = bannerBitmap->sizeX*2;
    bannerHeight = bannerBitmap->sizeY*2;
    pulseOffsetX = (bannerWidth/20.0)*HuSin(wipeLoadTime);
    pulseOffsetY = (bannerHeight/20.0)*HuSin(wipeLoadTime);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    
    GXPosition2f32(400-pulseOffsetX, 350-pulseOffsetY);
    GXTexCoord2f32(0, 0);
    GXPosition2f32(400+bannerWidth+pulseOffsetX, 350-pulseOffsetY);
    GXTexCoord2f32(1, 0);
    GXPosition2f32(400+bannerWidth+pulseOffsetX, 350+bannerHeight+pulseOffsetY);
    GXTexCoord2f32(1, 1);
    GXPosition2f32(400-pulseOffsetX, 350+bannerHeight+pulseOffsetY);
    GXTexCoord2f32(0, 1);
    GXEnd();
    wipeLoadTime += 3;
    if(wipeLoadTime > 360) {
        wipeLoadTime -= 360;
    }
}
