/* Starts THP movie playback and draws its frames as a model or sprite. */
#define _MATH_H
#include "game/thpmain.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/THPSimple.h"

#include "string.h"

#define THP_PROCESS_PRIORITY 100
#define THP_PROCESS_STACK_BYTES 12288
#define THP_DECODE_STACK_BYTES 8192
#define THP_AUDIO_MIX_CURRENT_BUFFER 2
#define THP_DEFAULT_VOLUME 110.0f
#define THP_DRAW_SPRITE 0
#define THP_DRAW_MODEL 1
#define THP_STATE_START 0
#define THP_STATE_STOP 1
#define THP_STATE_CLOSE 2
#define THP_STATE_RESTART 3
#define THP_STATE_PLAY 4

static void THPTestProc(void);
static void THPViewFunc(HU3D_MODEL *model, Mtx *matrix);
static void THPViewSprFunc(HUSPRITE *sprite);
static void THPDecodeFunc(void *callbackContext);

static char THPFileName[64]; /* DVD path copied by the movie creator. */

static UnkThpStruct04 videoInfo; /* Opened movie's width and height in pixels. */
static s32 decodeRate;
static u8 *decodeStackP; /* Stack memory for the OS idle decoder thread. */
static OSThread *decodeThread;
static s16 THPLoopF; /* Preload stores its low byte; only the value 1 enables frame wrapping. */
static s16 THPStat; /* Playback command/state consumed by THPTestProc. */
static s32 THPFrame; /* Zero-based index most recently drawn; reset to zero at creation. */
static s16 THPStart; /* Set after preload and idle decoder setup complete. */
HUPROCESS *THPProc; /* Only one movie playback process may exist at a time. */

/* Instruction and minigame menus call this to create a sprite movie with a chosen mix volume.
 * Its process opens the file and preloads frames before drawing can begin. */
HUSPRID HuTHPSprCreateVol(char *moviePath, s16 loopMovie, s16 drawPriority, float audioMixVolume)
{
    HUSPRID movieSpriteId;

    if(THPProc) {
        return HUSPR_NONE;
    }
    THPStart = 0;
    THPProc = HuPrcCreate(THPTestProc, THP_PROCESS_PRIORITY, THP_PROCESS_STACK_BYTES, 0);
    if(!THPProc) {
        return HUSPR_NONE;
    }
    movieSpriteId = HuSprFuncCreate(THPViewSprFunc, drawPriority);
    if(movieSpriteId == HUSPR_NONE) {
        /* Sprite creation failure leaves the already-created playback process alive
         * and skips movie initialization. */
        return HUSPR_NONE;
    }
    /* Decoder initialization's return value is ignored. */
    THPSimpleInit(THP_AUDIO_MIX_CURRENT_BUFFER);
    /* Configure the shared player for a sprite and set its THP audio mix gain. */
    SimpleControl.drawMode = THP_DRAW_SPRITE;
    SimpleControl.drawObjectId = movieSpriteId;
    SimpleControl.currentVolume = audioMixVolume;
    strcpy(THPFileName, moviePath);
    THPLoopF = loopMovie;
    THPStat = THP_STATE_START;
    THPFrame = 0;
    return movieSpriteId;
}

/* Sprite movie callers use this wrapper for the default THP mix volume. */
HUSPRID HuTHPSprCreate(char *moviePath, s16 loopMovie, s16 drawPriority)
{
    return HuTHPSprCreateVol(moviePath, loopMovie, drawPriority, THP_DEFAULT_VOLUME);
}

/* Movie scenes call this to create a model player; its process opens and plays the file. */
HU3D_MODELID HuTHP3DCreateVol(char *moviePath, s16 loopMovie, float audioMixVolume)
{
    HU3D_MODELID movieModelId;

    if(THPProc) {
        return HU3D_MODELID_NONE;
    }
    THPStart = 0;
    THPProc = HuPrcCreate(THPTestProc, THP_PROCESS_PRIORITY, THP_PROCESS_STACK_BYTES, 0);
    if(!THPProc) {
        return HU3D_MODELID_NONE;
    }
    /* Decoder initialization's return value is ignored. */
    THPSimpleInit(THP_AUDIO_MIX_CURRENT_BUFFER);
    /* Configure the shared player for a model and set its THP audio mix gain. */
    SimpleControl.drawMode = THP_DRAW_MODEL;
    /* Model-slot exhaustion is unchecked; the invalid model ID is saved and passed
     * to model-attribute setup. */
    movieModelId = Hu3DHookFuncCreate(THPViewFunc);
    SimpleControl.drawObjectId = movieModelId;
    SimpleControl.currentVolume = audioMixVolume;
    strcpy(THPFileName, moviePath);
    THPLoopF = loopMovie;
    THPStat = THP_STATE_START;
    THPFrame = 0;
    /* Keep the model hidden until playback setup and preload finish. */
    Hu3DModelAttrSet(SimpleControl.drawObjectId, HU3D_ATTR_DISPOFF);
    return movieModelId;
}

/* Model movie callers use this wrapper for the default THP mix volume. */
HU3D_MODELID HuTHP3DCreate(char *moviePath, s16 loopMovie) {
    return HuTHP3DCreateVol(moviePath, loopMovie, THP_DEFAULT_VOLUME);
}

/* Movie owners request this state to stop audio, cancel reads and clear queued frames. */
void HuTHPStop(void) {
    THPStat = THP_STATE_STOP;
}

/* A movie-owning scene calls this to make the playback process release the movie. */
void HuTHPClose(void) {
    THPStat = THP_STATE_CLOSE;
}

/* Movie owners request this state to stop the current playback and preload from its start. */
void HuTHPRestart(void) {
    THPStat = THP_STATE_RESTART;
}

/* Movie owners call this to detect completion after the final decoded frame is drawn. */
BOOL HuTHPEndCheck(void) {
    s32 lastFrameIndex;

    if(!THPProc) {
        return TRUE;
    }
    if(!THPStart) {
        return FALSE;
    }
    /* Frame indices start at zero, so compare against the final index. */
    lastFrameIndex = THPSimpleGetTotalFrame() - 1;
    /* Movies with at most one frame never report completion while a process is active. */
    if(lastFrameIndex <= 0) {
        return FALSE;
    }
    return (lastFrameIndex <= THPFrame);
}

/* Movie scenes use this to read the index of the frame most recently drawn. */
s32 HuTHPFrameGet(void) {
    return THPFrame;
}

/* Movie scenes use this to read the frame count from the opened THP header. */
s32 HuTHPTotalFrameGet(void) {
    return THPSimpleGetTotalFrame();
}

/* Movie owners request a target mix level (0-127) and ramp duration (0-60000).
 * Only an opened movie with audio accepts the update and clamps both values; zero duration
 * applies the level immediately. The returned success/failure status is ignored. */
void HuTHPSetVolume(s32 targetVolume, s32 rampDuration) {
    THPSimpleSetVolume(targetVolume, rampDuration);
}

/* Instruction and minigame menu teardown poll this; TRUE means a movie process still exists. */
BOOL HuTHPProcCheck(void)
{
    if(THPProc) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Movie draw setup uses this to wait until playback starts and the first decoded frame is ready. */
BOOL HuTHPStartCheck(void)
{
    /* The decoder marks the first frame ready by replacing this negative sentinel. */
    if(!THPStart || SimpleControl.videoBuffers[0].frameIndex < 0) {
        return FALSE;
    }
    return TRUE;
}

/* Creator calls start this process, which opens, preloads, plays, and releases the movie. */
static void THPTestProc(void) {
    s32 requiredBufferBytes;
    u32 startAudioPlayback;
    void *movieBuffer;

    while(THPSimpleOpen(THPFileName) == 0) {
        OSReport("THPSimpleOpen fail");
        HuPrcVSleep();
    }
    /* The helper returns the movie's pixel width and height for frame drawing. */
    THPSimpleGetVideoInfo(&videoInfo);
    requiredBufferBytes = THPSimpleCalcNeedMemory();
    OSReport("Size %x\n", requiredBufferBytes);
    movieBuffer = HuMemDirectMalloc(HEAP_MODEL, requiredBufferBytes);
    /* Clear the movie workspace before the decoder uses it during preload. */
    memset(movieBuffer, 0, requiredBufferBytes);
    DCFlushRange(movieBuffer, requiredBufferBytes);
    /* Allocation failure is reported here; the earlier workspace clear assumes success. */
    if(movieBuffer == 0) {
        OSReport("Can't allocate the memory\n");
    }
    THPSimpleSetBuffer(movieBuffer);
    while(THPSimplePreLoad(THPLoopF) == 0) {
        OSReport("THPSimplePreLoad fail");
        HuPrcVSleep();
    }
    HuPrcVSleep();
    if(VIGetNextField() == 1) {
        HuPrcVSleep();
    }
    decodeStackP = HuMemDirectMalloc(HEAP_HEAP, THP_DECODE_STACK_BYTES);
    /* Idle-thread creation is unchecked: a busy idle slot returns NULL, but startup
     * is still marked complete and shutdown uses the returned handle. */
    decodeThread = OSSetIdleFunction(THPDecodeFunc, NULL,
                                   decodeStackP + THP_DECODE_STACK_BYTES, THP_DECODE_STACK_BYTES);
    decodeRate = 0;
    THPStart = 1;
    if(SimpleControl.drawMode == THP_DRAW_MODEL) {
        /* Reveal the model after video setup and preload finish. */
        Hu3DModelAttrReset(SimpleControl.drawObjectId, HU3D_ATTR_DISPOFF);
    } else {
        HUSPRITE *sprite = &HuSprData[SimpleControl.drawObjectId];
        sprite->attr &= ~HUSPR_ATTR_DISPOFF;
    }
    while(1) {
        if(THPStat == THP_STATE_CLOSE) {
            break;
        }
        switch (THPStat) {
            case THP_STATE_START:
    startAudioPlayback = 1;
                THPStat = THP_STATE_PLAY;
                break;
            case THP_STATE_STOP:
                /* Repeat the stop/reset each update until another command replaces this state. */
                THPSimpleAudioStop();
                THPSimpleLoadStop();
                break;
            case THP_STATE_RESTART:
                THPSimpleAudioStop();
                THPSimpleLoadStop();
                while(THPSimplePreLoad(THPLoopF) == 0) {
                    OSReport("THPSimplePreLoad fail");
                    HuPrcVSleep();
                }
                THPStat = THP_STATE_START;
                startAudioPlayback = 1;
                break;
        }
        /* A stop requested before the first START leaves this flag uninitialized
         * at its first test, which can enable audio again after the stop/reset. */
        if(startAudioPlayback != 0) {
            THPSimpleAudioStart();
            startAudioPlayback = 0;
        }
        HuPrcVSleep();
    }
    if(SimpleControl.drawMode == THP_DRAW_MODEL) {
        Hu3DModelKill(SimpleControl.drawObjectId);
    } else {
        HuSprKill(SimpleControl.drawObjectId);
    }
    OSCancelThread(decodeThread);
    HuMemDirectFree(decodeStackP);
    THPSimpleAudioStop();
    THPSimpleLoadStop();
    THPSimpleClose();
    HuMemDirectFree(movieBuffer);
    THPSimpleQuit();
    THPProc = NULL;
    HuPrcKill(HuPrcCurrentGet());
    while(1) {
        HuPrcVSleep();
    }
}

/* Hu3DHookFuncCreate installs this callback; the model renderer invokes it to draw each movie
 * frame. */
static void THPViewFunc(HU3D_MODEL *model, Mtx *matrix) {
    GXColor frameColor = { 255, 255, 255, 255 };

    /* Do not draw until setup completes and the decoder has published a frame. */
    if(!THPStart || SimpleControl.videoBuffers[0].frameIndex < 0) {
        return;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetCullMode(GX_CULL_NONE);
    /* Unsigned -width / 2 wraps before division, giving the model a nonnegative X origin. */
    THPFrame = THPSimpleDrawCurrentFrame(RenderMode, &frameColor, *matrix, -videoInfo.width / 2,
                                         videoInfo.height / 2, videoInfo.width, -videoInfo.height);
    /* Each model draw that passes the readiness checks enables audio mixing again. */
    THPSimpleAudioStart();
}

/* HuSprFuncCreate installs this callback; the sprite renderer invokes it to draw each movie
 * frame. */
static void THPViewSprFunc(HUSPRITE *sprite) {
    Vec axis = { 0.0f, 0.0f, 1.0f };
    GXColor frameColor;
    s32 frameOriginX;
    s32 frameOriginY;
    Mtx modelview;
    Mtx rotationMatrix;
    /* Do not draw until setup completes and the decoder has published a frame. */
    if(!THPStart || SimpleControl.videoBuffers[0].frameIndex < 0) {
        return;
    }
    frameColor.r = sprite->r;
    frameColor.g = sprite->g;
    frameColor.b = sprite->b;
    frameColor.a = sprite->a;
    if(sprite->zRot != 0.0f) {
        MTXRotAxisRad(rotationMatrix, &axis, MTXDegToRad(sprite->zRot));
        MTXScale(modelview, sprite->scale.x, sprite->scale.y, 1.0f);
        MTXConcat(rotationMatrix, modelview, modelview);
    } else {
        MTXScale(modelview, sprite->scale.x, sprite->scale.y, 1.0f);
    }
    mtxTransCat(modelview, sprite->pos.x, sprite->pos.y, 0.0f);
    PSMTXConcat(*sprite->groupMtx, modelview, modelview);
    /* Place the movie's top-left corner around the centered sprite origin. */
    frameOriginX = -((s32) videoInfo.width / 2);
    frameOriginY = -((s32) videoInfo.height / 2);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    THPFrame = THPSimpleDrawCurrentFrame(RenderMode, &frameColor, modelview, frameOriginX,
                                         frameOriginY, videoInfo.width, videoInfo.height);
}

/* OSSetIdleFunction runs this decoder thread; it yields for a retrace after each decode attempt
 * until the movie process requests close. */
static void THPDecodeFunc(void *callbackContext)
{
    while(1) {
        if(THPStat == THP_STATE_CLOSE) {
            break;
        }
        if(THPSimpleDecode() == 1) {
            OSReport("Fail to decode video data");
        }
        VIWaitForRetrace();
    }
}

/* Material hook for movie-textured models; binds the latest decoded YUV planes during drawing. */
void HuTHPMatHook(HU3D_DRAW_OBJ *drawObject, HSF_MATERIAL *material)
{
    /* Retain the ordinary material setup while no decoded frame is available. */
    if(!THPStart || SimpleControl.videoBuffers[0].frameIndex < 0) {
        if(material->attrNum == 0) {
            Hu3DTevStageNoTexSet(drawObject, material);
        } else {
            Hu3DTevStageTexSet(drawObject, material);
        }
        return;
    } else {
        /* Movie dimensions are pixels; each chroma plane is half-size in both axes. */
        u16 videoWidth = SimpleControl.videoInfo.width;
        u16 videoHeight = SimpleControl.videoInfo.height;
        GXTexObj texY;
        GXTexObj texCb;
        GXTexObj texCr;
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetNumTexGens(2);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(4);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
        /* These coefficients and offsets convert the movie's YUV planes to RGB. */
        GXSetTevColorS10(GX_TEVREG0, (GXColorS10) { 65446, 0, 65422, 135 });
        GXSetTevKColor(GX_KCOLOR0, (GXColor) { 0, 0, 226, 88 });
        GXSetTevKColor(GX_KCOLOR1, (GXColor) { 179, 0, 0, 182 });
        GXSetTevKColor(GX_KCOLOR2, (GXColor) { 255, 0, 255, 128 });
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
        GXInitTexObj(&texY, SimpleControl.videoBuffers[bufIdx].yPlane, (s32) videoWidth,
                     (s32) videoHeight, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texY, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXLoadTexObj(&texY, GX_TEXMAP0);
        /* Chroma planes are half the luma width and height. */
        GXInitTexObj(&texCb, SimpleControl.videoBuffers[bufIdx].cbPlane, videoWidth >> 1,
                     videoHeight >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texCb, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXLoadTexObj(&texCb, GX_TEXMAP1);
        GXInitTexObj(&texCr, SimpleControl.videoBuffers[bufIdx].crPlane, videoWidth >> 1,
                     videoHeight >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texCr, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
        GXLoadTexObj(&texCr, GX_TEXMAP2);
        THPFrame = SimpleControl.videoBuffers[0].frameIndex;
    }
}
