#include "REL/framework/world_units.h"
// The monitor: the screen model whose overlay motion follows the time left in a round, and the
// texture the current frame is copied into.
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include <vector>
#include "REL/framework/game_object2.h"
#include "REL/m681DLL/monitor_interface.h"
#include "REL/framework/minigame_core.h"
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"

#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"

extern "C" {
#include "game/sprite.h"

}
#include "game/sprput.h"
#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"
#include "REL/m681DLL/game_controller.h"

#include "REL/m681DLL/monitor.h"

#include "REL/m681DLL/game_constants.h"

// Copies each drawn frame into a texture; the monitor makes one after the winner-camera event.
struct TextureSetupWork : RenderObject, MinigameSingleton<TextureSetupWork> {
    std::vector<signed char> buffer;
    GXTexObj texture;
    TextureSetupWork();
    virtual void draw(Mtx *matrix);
    virtual ~TextureSetupWork();
    virtual void update();
};

// Allocates the frame-copy texture and places its draw hook on layer 6.
TextureSetupWork::TextureSetupWork()
    : RenderObject(4), MinigameSingleton<TextureSetupWork>(),
      buffer(GXGetTexBufferSize(0x140, 0xF0, GX_TF_RGB565, GX_FALSE, GX_FALSE), s8())
{
    setLayer(6);
    GXInitTexObj(&texture, &buffer[0], 0x140, 0xF0, GX_TF_RGB565,
                 GX_CLAMP, GX_CLAMP, GX_FALSE);
}

// Loads the copied frame as a texture map.
void fn_1_31E8(TextureSetupWork *object, GXTexMapID map) {
    GXLoadTexObj(&object->texture, map);
}

// Copies the 640x480 frame buffer into the 320x240 RGB565 texture.
void TextureSetupWork::draw(Mtx *)
{
    GXSetTexCopySrc(0, 0, 0x280, 0x1E0);
    GXSetTexCopyDst(0x140, 0xF0, GX_TF_RGB565, 1);
    GXCopyTex(&buffer[0], 0);
    GXPixModeSync();
}

extern "C" const s32 lbl_1_rodata_200[3] = { DATANUM(0x8c0000, 14), DATANUM(0x8c0000, 15), DATANUM(0x8c0000, 13) };

extern "C" void fn_1_3E38(HU3D_DRAW_OBJ *, HSF_MATERIAL *);

// Loads the monitor model, its overlay motion and the three sprite animations it shows.
Monitor::Monitor()
    : MinigameSingleton<Monitor>(), GameObject(4), ModelMotion(DATANUM(0x8c0000, 11)), EventListener(),
      callback(&Monitor::fn_1_3C24), max_frames(0.0f), textureScale(0x30), delay(0x24),
      animations(), remaining(0), duration(0), firstChoice(0), secondChoice(0), updateCount(0), active(0)
{
    s32 index;

    setLayer(3);
    setAttribute( ((1 << 30) | (1 << 0)));
    addMotion(DATANUM(0x8c0000, 12));
    setOverlayMotion(DATANUM(0x8c0000, 12));
    setAttribute(((1 << 30) | (1 << 10)));
    this->max_frames = getOverlayMotionMaxTime();
    // The overlay motion must contain at least one frame.
    assert(max_frames > 0);
    setOverlayMotionTime(0.0f);

    for (index = 0; index < 3; ++index) {
        animations.push_back(HuSprAnimRead(fn_80008DD0(
            lbl_1_rodata_200[index], 0x10000000, 2)));
    }

    fn_80027884(static_cast<ModelMotion *>(this)->id(), fn_1_3E38);
    registerChannel(0);
}

// Releases the monitor model and animation list.
Monitor::~Monitor()
{
}

// Sets the displayed choices and starts their countdown.
void fn_1_381C(Monitor *state,
                            InitializationValues *values,
                            f32 duration)
{
    state->firstChoice = values->values[0];
    state->secondChoice = values->values[1];
    s32 elapsed = (s32)secondsToFrames(duration);
    state->remaining = elapsed;
    state->duration = elapsed;
    Monitor::Callback initialValues = &Monitor::fn_1_3C28;
    state->callback = initialValues;
}

// Updates the countdown overlay and creates the frame-copy owner when active.
void Monitor::update()
{
    f64 fade;
    f32 fadeAmount;

    (this->*this->callback)();
    ++this->updateCount;
    if (this->duration <= 0) {
        fade = 0.0;
    } else {
        fade = 1.0 - (f32)this->remainingFrames() / (f32)this->duration;
    }
    fadeAmount = (f32)fade;
    setOverlayMotionTime(fadeAmount * this->max_frames);

    if (this->active != 0) {
        if (MinigameSingleton<TextureSetupWork>::current() == 0) {
            new TextureSetupWork;
        }
    }
}

// Clears the choices or activates the winner frame-copy display.
void Monitor::receive(const EventPacket *action)
{
    switch (action->type) {
    case 3:
        firstChoice = 0;
        secondChoice = 0;
        break;
    case 11: {
        M681GameState *object = &MinigameSingleton<M681GameState>::getChecked();
        if (object->winnerPlayer >= 0) {
            active = 1;
        }
        break;
    }
    default:
        break;
    }
}

// Reports whether both choices are set and their countdown has expired.
s32 Monitor::choicesExpired()
{
    s32 result = 0;
    u8 active = 0;
    bool reachedLimit;

    if (firstChoice != 0 && secondChoice != 0) {
        active = 1;
    }
    if (active != 0) {
        reachedLimit = remaining <= 0;
        if (reachedLimit) {
            result = 1;
        }
    }
    return result;
}

// Performs no monitor update.
void Monitor::fn_1_3C24()
{
}

// Interpolates between values using the remaining countdown fraction.
template <class Value>
static inline Value interpolateCountdown(const Monitor *state, Value first, Value second)
{
    float progress = (float)state->remainingFrames() / (float)state->duration;
    return (1.0 - (double)progress) * first + (double)progress * second;
}

// Plays countdown sounds and returns to the idle callback when time expires.
void Monitor::fn_1_3C28()
{
    s32 finished;
    s32 previousStep;
    s32 currentStep;
    Monitor::Callback callbackData;

    previousStep = interpolateCountdown(this, 1, 4);
    // Computes the expiry flag without using its result.
    finished = 0;
    if (this->remaining > 0) {
        if (--this->remaining == 0) {
            finished = 1;
        }
    }
    currentStep = interpolateCountdown(this, 1, 4);
    if (this->remaining > 0) {
        if (previousStep != currentStep) {
            soundPlay(0x91B);
        }
    } else {
        soundPlay(0x91C);
        callbackData = &Monitor::fn_1_3C24;
        this->callback = callbackData;
    }
}

void fn_1_40C0(Monitor *);
extern "C" void fn_1_4880(Monitor *);

// Dispatches the model draw hook to the monitor singleton.
void fn_1_3E38(HU3D_DRAW_OBJ *, HSF_MATERIAL *)
{
    MinigameSingleton<Monitor>::getChecked().fn_1_3EAC();
}

// Draws either the choice textures or the copied winner frame.
void Monitor::fn_1_3EAC() {
    if (active != 0) {
        fn_1_4880(this);
        return;
    }
    fn_1_40C0(this);
}

// Builds a rotated and translated texture matrix.
void fn_1_3EF4(Mtx transform, f32 rotation, f32 x, f32 y)
{
    Mtx translation;

    PSMTXRotRad(transform, 0x7A, rotation);
    PSMTXScaleApply(transform, transform, 2.0f, 2.0f, 1.0f);
    PSMTXTrans(translation, -0.5f, -0.5f, 0.0f);
    PSMTXConcat(transform, translation, transform);
    PSMTXTransApply(transform, transform, 0.5f, 0.5f, 0.0f);
    PSMTXTrans(translation, x, y, 0.0f);
    PSMTXConcat(transform, translation, transform);
}

// Configures a texture-combiner stage with the supplied color inputs.
void configureMonitorTevStage(GXTevStageID stage,
                           GXTexCoordID textureCoordinate,
                           GXTexMapID textureMap,
                           GXTevColorArg colorA,
                           GXTevColorArg colorB,
                           GXTevColorArg colorC,
                           GXTevColorArg colorD,
                           GXTevAlphaArg alphaD)
{
    GXSetTevOrder(stage, textureCoordinate, textureMap, GX_COLOR_NULL);
    GXSetTevColorIn(stage, colorA, colorB, colorC, colorD);
    GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(stage, (GXTevAlphaArg)7, (GXTevAlphaArg)7,
                    (GXTevAlphaArg)7, alphaD);
    GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
}

extern "C" {

const f32 lbl_1_rodata_240[] = { 0.0f, 1.5707964f, -1.5707964f, 3.1415927f, 0.0f };

}
// Draws the choice textures with their countdown transition.
void fn_1_40C0(Monitor *object)
{
    const f32 *firstRotationAddress;
    const f32 *secondRotationAddress;
    GXTevColorArg blendAlpha2;
    u8 useTextures;
    u8 transitionEnded;
    s32 selectedStep;
    s32 firstStep;
    bool frameEnded;
    Mtx firstMatrix;
    f32 firstRotation;
    frameEnded = object->remaining <= 0;
    if (frameEnded)
    {
        useTextures = 0;
        transitionEnded = 0;
        if (object->firstChoice && 0 != object->secondChoice)
        {
            transitionEnded = 1;
        }
        if (transitionEnded)
        {
            bool transitionFrameEnded2 = object->remaining <= 0;
            if (0 != transitionFrameEnded2)
            {
                useTextures = 1;
            }
        }
        u8 textureSelection22 = useTextures;
        if (textureSelection22 != 0) {
            selectedStep = 0;
        } else {
            selectedStep = 4;
        }
        firstStep = selectedStep;
    }
    else
    {
        firstStep = interpolateCountdown(object, 1, 4);
    }
    s32 textureMode;
    textureMode = firstStep;
    Mtx secondMatrix;
    Mtx scaleMatrix;
    HuSprTexLoad(object->animations[2], (s16)textureMode, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad(object->animations[0], 0, 1, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(object->animations[1], 0, 2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    firstRotationAddress = & lbl_1_rodata_240[object->firstChoice];
    secondRotationAddress = & lbl_1_rodata_240[object->secondChoice];
    firstRotation = * firstRotationAddress;
    fn_1_3EF4(firstMatrix, firstRotation, 0.25f, 0.0f);
    fn_1_3EF4(secondMatrix, (* secondRotationAddress), - 0.25f, 0.0f);
    PSMTXScale(scaleMatrix, (f32) object->textureScale, (f32) object->textureScale, 1.0f);
    GXLoadTexMtxImm(firstMatrix, 0x1E, GX_MTX2x4);
    GXLoadTexMtxImm(secondMatrix, 0x21, GX_MTX2x4);
    GXLoadTexMtxImm(scaleMatrix, 0x24, GX_MTX2x4);
    GXSetNumTexGens(4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(4);
    configureMonitorTevStage(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, GX_CA_ZERO);
    blendAlpha2 = object->finished() ? GX_CC_TEXA : GX_CC_ZERO;
    configureMonitorTevStage(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_CC_CPREV, GX_CC_TEXC, blendAlpha2, GX_CC_ZERO, GX_CA_ZERO);
    configureMonitorTevStage(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP1, GX_CC_CPREV, GX_CC_TEXC, blendAlpha2, GX_CC_ZERO, GX_CA_ZERO);
    configureMonitorTevStage(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP2, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXA, GX_CC_ZERO, GX_CA_KONST);
}

// Draws the copied frame with the monitor overlay.
void fn_1_4880(Monitor *object)
{
    Mtx textureMatrix;
    TextureSetupWork *textureManager;
    TextureSetupWork *textureContext;

    textureContext = MinigameSingleton<TextureSetupWork>::self;
    if (textureContext != 0) {
        textureManager = &MinigameSingleton<TextureSetupWork>::getChecked();
        fn_1_31E8(textureManager, GX_TEXMAP0);

        HuSprTexLoad(object->animations[1], 0, 1, GX_REPEAT, GX_REPEAT, GX_LINEAR);

        PSMTXScale(textureMatrix, 0.6f, 0.7f, 1.0f);
        PSMTXTransApply(textureMatrix, textureMatrix, 0.2f, 0.15f, 0.0f);
        GXLoadTexMtxImm(textureMatrix, 0x1E, GX_MTX2x4);

        PSMTXScale(textureMatrix, (f32)object->textureScale,
                   (f32)object->textureScale, 1.0f);
        GXLoadTexMtxImm(textureMatrix, 0x21, GX_MTX2x4);
        GXSetNumTexGens(2);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

        GXColor tevColor = { 255, 192, 128, 80 };
        GXSetTevColor(GX_TEVREG0, tevColor);
        GXSetNumTevStages(2);
        configureMonitorTevStage(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                           GX_CC_TEXC, GX_CC_C0, GX_CC_A0, GX_CC_ZERO,
                           GX_CA_ZERO);
        configureMonitorTevStage(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1,
                           GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXA, GX_CC_ZERO,
                           GX_CA_KONST);
    }
}

// Releases the texture buffer and render-object bases.
TextureSetupWork::~TextureSetupWork()
{
}

// Performs no update; frame copying happens during drawing.
void TextureSetupWork::update()
{
}
