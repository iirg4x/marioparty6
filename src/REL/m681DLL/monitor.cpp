#include "REL/m681DLL/frame_time.h"
#include <vector>
#include "REL/m681DLL/render_object.h"
#include "REL/m681DLL/monitor_interface.h"
#include "REL/framework/minigame_core.h"
#include "dolphin/mtx.h"

#include "dolphin/gx/GXStruct.h"

#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "REL/m681DLL/owned_model.h"
#include "REL/m681DLL/model_motion.h"

extern "C" {
#include "game/sprite.h"
#include "REL/m681DLL.h"
}

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
#include "REL/m681DLL/owner_182c.h"
#include "REL/m681DLL/monitor.h"

extern "C" {


extern char lbl_1_data_27A[];


}

extern "C" {

extern s32 fn_1_2140(s32 value);

extern s32 lbl_1_bss_50;

extern f32 lbl_1_bss_54;

extern s32 lbl_1_bss_78;


extern f32 lbl_1_bss_7C;


}

#include "REL/m681DLL/character_sequence.h"

struct TextureSetupWork : RenderObject, MinigameSingleton<TextureSetupWork> {
    std::vector<signed char> buffer;
    GXTexObj texture;
    TextureSetupWork();
    virtual void draw(Mtx *matrix);
    virtual ~TextureSetupWork();
    virtual void update();
};

extern void fn_1_33CB0__5ModelFs(void *object, s32 mode);



TextureSetupWork::TextureSetupWork()
    : RenderObject(4), MinigameSingleton<TextureSetupWork>(),
      buffer(GXGetTexBufferSize(0x140, 0xF0, GX_TF_RGB565, GX_FALSE, GX_FALSE), s8())
{
    fn_1_33CB0(6);
    GXInitTexObj(&texture, &buffer[0], 0x140, 0xF0, GX_TF_RGB565,
                 GX_CLAMP, GX_CLAMP, GX_FALSE);
}

void fn_1_31E8(TextureSetupWork *object, GXTexMapID map) {
    GXLoadTexObj(&object->texture, map);
}


void TextureSetupWork::draw(Mtx *)
{
    GXSetTexCopySrc(0, 0, 0x280, 0x1E0);
    GXSetTexCopyDst(0x140, 0xF0, GX_TF_RGB565, 1);
    GXCopyTex(&buffer[0], 0);
    GXPixModeSync();
}

typedef struct AnimData_s AnimData329C;


extern "C" { extern const u8 lbl_1_data_2EC[]; }

extern "C" const s32 lbl_1_rodata_200[3] = { DATANUM(0x8c0000, 14), DATANUM(0x8c0000, 15), DATANUM(0x8c0000, 13) };

extern "C" { void __msl_assertion_failed(const char *, const char *, int); }

extern "C" { void __ct__10GameObjectFi(void *, s32); }

extern "C" { void fn_1_19EA0(void *, s32); }

extern "C" { void __ct__13EventListenerFv(void *); }

extern "C" { void fn_1_192C0(void *, s32); }

extern "C" { void fn_1_18F60(void *, s32); }

extern "C" { void fn_1_19B38(void *, f32); }

extern "C" { f32 fn_1_19C5C(void *); }

extern "C" void fn_1_3E38(HU3D_DRAW_OBJ *, HSF_MATERIAL *);

extern "C" { void registerChannel__13EventListenerFi(void *, s32); }

Monitor::Monitor()
    : MinigameSingleton<Monitor>(), GameObject(4), ModelMotion(DATANUM(0x8c0000, 11)), EventListener(),
      callback(&Monitor::fn_1_3C24), max_frames(0.0f), textureScale(0x30), delay(0x24),
      animations(), remaining(0), duration(0), firstTexture(0), secondTexture(0), updateCount(0), active(0)
{
    s32 index;

    fn_1_33CB0(3);
    fn_1_33950( ((1 << 30) | (1 << 0)));
    fn_1_1A058(DATANUM(0x8c0000, 12));
    fn_1_1A2CC(DATANUM(0x8c0000, 12));
    fn_1_33950(((1 << 30) | (1 << 10)));
    this->max_frames = fn_1_3464C();
    this->max_frames > 0.0f ? (void)0 : __msl_assertion_failed("max_frames > 0", "monitor.cpp", 0x77);
    fn_1_34528(0.0f);

    for (index = 0; index < 3; ++index) {
        animations.push_back(HuSprAnimRead(fn_80008DD0(
            lbl_1_rodata_200[index], 0x10000000, 2)));
    }

    fn_80027884(static_cast<ModelMotion *>(this)->id(), fn_1_3E38);
    registerChannel(0);
}

Monitor::~Monitor()
{
}

typedef void (Monitor::*Callback381C)();


void fn_1_381C(Monitor *state,
                            InitializationValues *values,
                            f32 duration)
{
    state->firstTexture = values->values[0];
    state->secondTexture = values->values[1];
    s32 elapsed = (s32)secondsToFrames(duration);
    state->remaining = elapsed;
    state->duration = elapsed;
    Callback381C initialValues = &Monitor::fn_1_3C28;
    state->callback = initialValues;
}


static inline TextureSetupWork *CurrentTexture38C0()
{
    return MinigameSingleton<TextureSetupWork>::self;
}

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
    fn_1_34528(fadeAmount * this->max_frames);

    if (this->active != 0) {
        if (CurrentTexture38C0() == 0) {
            new TextureSetupWork;
        }
    }
}


void Monitor::receive(const EventPacket *action)
{
    switch (action->type) {
    case 3:
        firstTexture = 0;
        secondTexture = 0;
        break;
    case 11: {
        M681GameState *object = &MinigameSingleton<M681GameState>::getChecked();
        if (object->negativeState >= 0) {
            active = 1;
        }
        break;
    }
    default:
        break;
    }
}

s32 Monitor::fn_1_3BAC()
{
    s32 result = 0;
    u8 active = 0;
    bool reachedLimit;

    if (firstTexture != 0 && secondTexture != 0) {
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

typedef void (Monitor::*CallbackData3C28)();

void Monitor::fn_1_3C24()
{
}

extern "C" { s32 fn_1_88F8(s32 message); }


template <class Value>
static inline Value fadeStep3C28(const Monitor *state, Value first, Value second)
{
    float progress = (float)state->remainingFrames() / (float)state->duration;
    return (1.0 - (double)progress) * first + (double)progress * second;
}

void Monitor::fn_1_3C28()
{
    s32 finished;
    s32 previousStep;
    s32 currentStep;
    CallbackData3C28 callbackData;

    previousStep = fadeStep3C28(this, 1, 4);
    finished = 0;
    if (this->remaining > 0) {
        if (--this->remaining == 0) finished = 1;
    }
    currentStep = fadeStep3C28(this, 1, 4);
    if (this->remaining > 0) {
        if (previousStep != currentStep) {
            fn_1_88F8(0x91B);
        }
    } else {
        fn_1_88F8(0x91C);
        callbackData = &Monitor::fn_1_3C24;
        this->callback = callbackData;
    }
}

struct Transition {
    int frames, elapsed;
    float current, start, end;
};

void fn_1_40C0(Monitor *);
extern "C" void fn_1_4880(Monitor *);

void fn_1_3E38(HU3D_DRAW_OBJ *, HSF_MATERIAL *)
{
    MinigameSingleton<Monitor>::getChecked().fn_1_3EAC();
}

void Monitor::fn_1_3EAC() {
    if (active != 0) {
        fn_1_4880(this);
        return;
    }
    fn_1_40C0(this);
}

extern "C" {

}
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
extern "C" {

}

void fn_1_4000(GXTevStageID stage,
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


extern void HuSprTexLoad(ANIMDATA *, s16, s16, GXTexWrapMode, GXTexWrapMode, GXTexFilter);
extern void __msl_assertion_failed(const char *expression, const char *file, int line);
extern void *lbl_1_bss_88;
extern const char lbl_1_data_293[];

}

static inline s32 remaining40C0(const Monitor *object)
{
    return object->remaining;
}

template <class Value>
static inline Value fadeStep40C0(const Monitor *object, Value first, Value second)
{
    f32 progress = (f32)remaining40C0(object) / (f32)object->duration;
    return (1.0 - (f64)progress) * first + (f64)progress * second;
}

extern "C" {

const f32 lbl_1_rodata_240[] = { 0.0f, 1.5707964f, -1.5707964f, 3.1415927f, 0.0f };

}
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
        u8 textureSelection2;
        useTextures = 0;
        transitionEnded = 0;
        if (object->firstTexture && 0 != object->secondTexture)
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
        firstStep = fadeStep40C0(object, 1, 4);
    }
    s32 textureMode;
    textureMode = firstStep;
    Mtx secondMatrix;
    Mtx scaleMatrix;
    HuSprTexLoad(object->animations[2], (s16)textureMode, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad(object->animations[0], 0, 1, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(object->animations[1], 0, 2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    firstRotationAddress = & lbl_1_rodata_240[object->firstTexture];
    secondRotationAddress = & lbl_1_rodata_240[object->secondTexture];
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
    fn_1_4000(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, GX_CA_ZERO);
    blendAlpha2 = object->finished() ? GX_CC_TEXA : GX_CC_ZERO;
    fn_1_4000(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_CC_CPREV, GX_CC_TEXC, blendAlpha2, GX_CC_ZERO, GX_CA_ZERO);
    fn_1_4000(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP1, GX_CC_CPREV, GX_CC_TEXC, blendAlpha2, GX_CC_ZERO, GX_CA_ZERO);
    fn_1_4000(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP2, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXA, GX_CC_ZERO, GX_CA_KONST);
}
extern "C" {

}

void fn_1_4880(Monitor *object)
{
    Mtx textureMatrix;
    TextureSetupWork *textureManager;
    TextureSetupWork *textureContext;

    textureContext = MinigameSingleton<TextureSetupWork>::self;
    if (textureContext != 0) {
        textureManager = &MinigameSingleton<TextureSetupWork>::getChecked();
        GXLoadTexObj(&textureManager->texture, GX_TEXMAP0);

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
        fn_1_4000(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                           GX_CC_TEXC, GX_CC_C0, GX_CC_A0, GX_CC_ZERO,
                           GX_CA_ZERO);
        fn_1_4000(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1,
                           GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXA, GX_CC_ZERO,
                           GX_CA_KONST);
    }
}

TextureSetupWork::~TextureSetupWork()
{
}

void TextureSetupWork::update()
{
}

extern "C" { extern s32 lbl_1_data_C; }

extern "C" { extern s32 lbl_1_data_0; }

static inline f32 accelerationPerFrame(f64 accelerationPerSecondSquared)
{
    return (f32)((accelerationPerSecondSquared * 100.0) / (60.0 * 60.0));
}

extern "C" {
s32 lbl_1_bss_80 = minutesToFrames(60L);
f32 lbl_1_bss_84 = accelerationPerFrame(9.80665);
}
