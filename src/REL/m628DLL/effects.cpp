#include "REL/game_gravity.h"
#include "REL/m628DLL/event_displacement.h"
#include "game/sprput.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"

#include "REL/m628DLL/effects.h"

extern "C"
{
#include "game/audio.h"
}

#include <vector>

// Creates the texture used to copy the rendered frame.
TextureController::TextureController() : MinigameSingleton<TextureController>(), RenderEffect(0x60)
{
    Model::setLayer(5);
    GXInitTexObj(&texture, textureData, 640, 480, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(
        &texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
}

// Restores GX state after the frame-copy effect.
TextureController::~TextureController()
{
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_FALSE, 0, 0);
}

// Leaves the frame-copy effect unchanged between draws.
void TextureController::update() {}

// Copies the current frame into the indirect texture.
void TextureController::draw(Mtx *modelView)
{
    Mtx textureMatrix;
    Mtx lightPerspective;
    HU3D_CAMERA *camera;

    GXLoadPosMtxImm(*modelView, GX_PNMTX0);
    camera = Hu3DCamera;
    GXSetNumTexGens(1);
    C_MTXLightPerspective(lightPerspective, camera->fov, camera->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(lightPerspective, *modelView, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
    GXCopyTex(textureData, GX_FALSE);
}

// Creates the rotating line effect and its collision record.
PointLineEffect::PointLineEffect() : RenderEffect(0x60), callbacks(0)
{
    PSMTXIdentity(matrix);
    record.matrix = matrix;
    setLayer(4);
    initialize();
}

// Releases the line points and event registration.
PointLineEffect::~PointLineEffect() {}

// Builds the line's sixteen points and installs its callbacks.
void PointLineEffect::initialize()
{
    Vec position;
    f64 secondRandom;
    s32 index;
    for (index = 0; index < 16; index++)
    {
        f64 firstRandom = RandomFloat(-(100.0f * 0.3f), (100.0f * 0.3f));
        secondRandom = RandomFloat(-(100.0f * 0.3f), (100.0f * 0.3f));
        position.x = (f32)secondRandom;
        position.y = (f32)firstRandom;
        position.z = (f64)(100.0f * (0.88f * (f32)index) - 660.0f);
        if (index == 0 || index == 15)
        {
            position.y = 0.0f;
        }
        points.push_back(position);
    }
    record.position.x = 0.0f;
    record.position.y = (100.0f * 0.3f);
    record.position.z = 0.0f;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    spinRate = 0.0f;
    pointOffset = 0;
    static const GXColor initialLineColor = {0x14, 0x28, 0x8C, 0xA0};
    color = initialLineColor;
    elapsed = 0.0f;
    unk98 = 0;
    alphaScale = 0.0f;
    soundStarted = 0;

    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&PointLineEffect::fn_1_760C), this), 0);

    setCallback(&PointLineEffect::fn_1_7B74);
}

// Advances the line rotation and its collision transform.
void PointLineEffect::update()
{
    Mtx translationMatrix;
    Mtx rotationMatrix;
    f32 zOffset;
    if (callback)
    {
        (this->*callback)();
    }
    rotation.z += spinRate;
    zOffset = 660.0f;
    PSMTXTrans(
        translationMatrix, record.position.x, record.position.y, record.position.z - zOffset);
    PSMTXRotRad(rotationMatrix, 'y', MTXDegToRad(rotation.y));
    PSMTXConcat(translationMatrix, rotationMatrix, matrix);
    pointOffset++;
}

// Adds the event movement to the effect position.
static inline void applyAxisDisplacement(Vec &position, const f32 &orientation,
                                         const EventPacket &event)
{
    Vec *displacement = (Vec *)event.context;
    if (orientation == 0.0f)
    {
        position.x = position.x + displacement->x;
        return;
    }
    position.z = position.z + displacement->z;
}

// Slides the line along its open axis when the moving-platform event reports a change.
void PointLineEffect::fn_1_760C(const EventPacket &event)
{
    applyAxisDisplacement(record.position, rotation.y, event);
}

// Copies the supplied position to the line collision record.
void fn_1_7660(PointLineEffect *object, const Vec *position)
{
    Vec *destination;
    Vec *collisionPosition;

    destination = &object->record.position;
    collisionPosition = destination;
    collisionPosition->x = position->x;
    collisionPosition->y = position->y;
    collisionPosition->z = position->z;
}

// Sets the line rotation increment per update.
void PointLineEffect::setSpinRate(f32 value)
{
    spinRate = value;
}

// Sets the line rotation vector.
void PointLineEffect::setRotation(const Vec *value)
{
    rotation = *value;
}

// Sets the line color.
void fn_1_76C0(PointLineEffect *self, const GXColor *value)
{
    self->color = *value;
}

// Draws the rotating line points with their current fade.
void PointLineEffect::draw(Mtx *parent)
{
    Mtx rotationY;
    Mtx rotationZ;
    Mtx translation;
    Vec point;
    GXColor sourceColor;
    s32 pointCount;
    s32 index;
    u32 pointIndex;
    const Vec *firstSource;
    const Vec *loopSource;
    const Vec *lastSource;
    PSMTXRotRad(rotationZ, 0x7A, MTXDegToRad(this->rotation.z));
    PSMTXRotRad(rotationY, 0x79, MTXDegToRad(this->rotation.y));
    PSMTXTrans(translation, record.position.x, record.position.y, record.position.z);
    PSMTXConcat(*parent, translation, translation);
    PSMTXConcat(translation, rotationY, rotationY);
    PSMTXConcat(rotationY, rotationZ, rotationZ);
    GXLoadPosMtxImm(rotationZ, GX_PNMTX0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);
    sourceColor = this->color;
    sourceColor.a = (u8)((f32)sourceColor.a * this->alphaScale);
    GXSetTevColor(GX_TEVREG0, sourceColor);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetLineWidth(0x18, GX_TO_ZERO);
    pointCount = points.size();
    GXBegin(GX_LINESTRIP, GX_VTXFMT0, (u16)pointCount);
    firstSource = &const_cast<const std::vector<Vec> &>(points).at(0);
    point = *firstSource;
    GXPosition3f32(point.x, point.y, point.z);
    for (index = 0; index < pointCount - 2; index++)
    {
        pointIndex = (index + (this->pointOffset >> 1)) % (pointCount - 2) + 1;
        loopSource = &const_cast<const std::vector<Vec> &>(points).at(pointIndex);
        point = *loopSource;
        point.z = 100.0f * (0.88f * (f32)(index + 1)) - 660.0f;
        GXPosition3f32(point.x, point.y, point.z);
    }
    lastSource = &const_cast<const std::vector<Vec> &>(points).at(pointCount - 1);
    point = *lastSource;
    GXPosition3f32(point.x, point.y, point.z);
}

// Leaves the line idle before its fade sequence.
void PointLineEffect::fn_1_7B74() {}

// Leaves the line unchanged after its fade sequence.
void PointLineEffect::fn_1_7B78() {}

// Flashes the effect, then restores full opacity.
void PointLineEffect::fn_1_7B7C()
{
    if (elapsed > 1.1f)
    {
        if (soundStarted == 0)
        {
            Vec position;
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            soundId = HuAudFXPlay(0x71F);
            soundPanTo(1, soundId, &position);
            soundStarted = 1;
        }
        if ((pointOffset & 7U) == 0)
        {
            alphaScale = 1.0f;
        }
        else
        {
            alphaScale = 0.0f;
        }
        if (elapsed > 2.8f)
        {
            Vec position;
            alphaScale = 1.0f;
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            soundPlayAt(1, 0x71B, &position);

            setCallback(&PointLineEffect::fn_1_7B78);
        }
    }
    elapsed += (1.0 / 60.0);
}

// Fades out the line and stops its sound.
void PointLineEffect::fn_1_7D0C()
{
    s32 volume;
    alphaScale -= 0.01f;
    volume = (s32)(127.0f * alphaScale);
    HuAudFXVolSet(soundId, (s16)volume);
    if (!(alphaScale > 0.0f))
    {
        alphaScale = 0.0f;
        HuAudFXStop(soundId);
        setCallback(&PointLineEffect::fn_1_7B74);
    }
}

// Selects the line fade state from the event mode.
void PointLineEffect::handle(int mode)
{
    switch (mode)
    {
    case 0:
        elapsed = 0.0f;
        unk98 = 0;
        setCallback(&PointLineEffect::fn_1_7B7C);
        break;
    case 3:
        setCallback(&PointLineEffect::fn_1_7D0C);
        break;
    }
}

// Creates a textured particle effect from the supplied animation.
SpriteParticleEffect::SpriteParticleEffect(ANIMDATA *animation)
    : RenderEffect(0x60), animation(animation), callbacks(0)
{
    setLayer(4);
    fn_1_8008();
}

// Initializes the sprite particle effect and its movement and state callbacks.
void SpriteParticleEffect::fn_1_8008()
{
    s32 index;
    RenderPoint element;
    for (index = 0; index < 4; ++index)
    {
        this->initializeElement(&element);
        this->elements.push_back(element);
    }
    this->position.x = 0.0f;
    this->position.y = 50.0f;
    this->position.z = 0.0f;
    this->rotation.x = 0.0f;
    this->rotation.y = 0.0f;
    this->rotation.z = 0.0f;
    static const GXColor initialElementColor = {0x14, 0x28, 0x8C, 0xA0};
    this->color = initialElementColor;
    this->updateCount = 0;
    this->elapsed = 0.0f;
    this->unk48 = 0;
    this->alphaScale = 0.0f;

    registerEventCallback(
        &this->callbacks, std::bind1st(std::mem_fun(&SpriteParticleEffect::fn_1_879C), this), 0);

    setCallback(&SpriteParticleEffect::fn_1_8D40);
}

// Builds one randomized render element using the owner's orientation mode.
void SpriteParticleEffect::initializeElement(RenderPoint *element)
{
    element->visible = 0;
    element->randomOffset = RandomFloat(-40.0f, 40.0f);
    if (rotation.y == 0.0f)
    {
        f64 positionX;
        f64 positionY;
        f64 positionZ;
        positionZ = RandomFloat(-600.0f, 600.0f);
        positionY = position.y;
        positionX = position.x + element->randomOffset;
        element->position.x = positionX;
        element->position.y = positionY;
        element->position.z = positionZ;
        element->angle = element->randomOffset < 0.0f ? 0.0f : 180.0f;
    }
    else
    {
        f64 positionX;
        f64 positionY;
        f64 positionZ;
        positionZ = position.z + element->randomOffset;
        positionY = position.y;
        positionX = RandomFloat(-600.0f, 600.0f);
        element->position.x = positionX;
        element->position.y = positionY;
        element->position.z = positionZ;
        element->angle = element->randomOffset < 0.0f ? -90.0f : 90.0f;
    }
    element->size = 200.0f;
    element->state = 0;
    element->delay = RandomUnsigned() & 0x7F;
    element->age = 0;
}

// Randomizes one textured particle for reuse.
static inline void InitializeElement(SpriteParticleEffect *owner, RenderPoint *element)
{
    f64 rotatedPositionZ;
    f64 rotatedPositionY;
    f64 rotatedPositionX;
    f64 alignedPositionZ;
    f64 alignedPositionY;
    f64 alignedPositionX;

    element->visible = 0;
    element->randomOffset = RandomFloat(-40.0f, 40.0f);
    if (owner->rotation.y == 0.0f)
    {
        alignedPositionZ = RandomFloat(-600.0f, 600.0f);
        alignedPositionY = owner->position.y;
        alignedPositionX = owner->position.x + element->randomOffset;
        element->position.x = alignedPositionX;
        element->position.y = alignedPositionY;
        element->position.z = alignedPositionZ;
        element->angle = element->randomOffset < 0.0f ? 0.0f : 180.0f;
    }
    else
    {
        rotatedPositionZ = owner->position.z + element->randomOffset;
        rotatedPositionY = owner->position.y;
        rotatedPositionX = RandomFloat(-600.0f, 600.0f);
        element->position.x = rotatedPositionX;
        element->position.y = rotatedPositionY;
        element->position.z = rotatedPositionZ;
        element->angle = element->randomOffset < 0.0f ? -90.0f : 90.0f;
    }
    element->size = 200.0f;
    element->state = 0;
    element->delay = RandomUnsigned() & 0x7F;
    element->age = 0;
}

// Advances the textured particles and their fade state.
void SpriteParticleEffect::update()
{
    RenderPoint *element;
    s32 index;
    s32 count;

    if (callback)
    {
        (this->*callback)();
    }
    count = elements.size();
    for (index = 0; index < count; index++)
    {
        element = &elements.at(index);
        switch (element->state)
        {
        case 0:
            element->age++;
            if (element->age >= element->delay)
            {
                element->visible = 1;
                element->state++;
            }
            else
            {
                break;
            }
            // fall through
        case 1:
            element->size -= 8.0f;
            if (rotation.y == 0.0f)
            {
                element->position.x = position.x + element->randomOffset;
            }
            else
            {
                element->position.z = position.z + element->randomOffset;
            }
            if (!(element->size > 0.0f))
            {
                element->size = 0.0f;
                element->visible = 0;
                element->state++;
            }
            else
            {
                break;
            }
            // fall through
        case 2:
            InitializeElement(this, element);
            break;
        }
    }
    updateCount++;
}

// Slides the effect along its open axis when the moving-platform event reports a change.
void SpriteParticleEffect::fn_1_879C(const EventPacket &event)
{
    applyAxisDisplacement(position, rotation.y, event);
}

// Sets the textured particle effect position.
void SpriteParticleEffect::fn_1_87F0(const Vec *value)
{
    position = *value;
}

// Sets the textured particle effect rotation.
void fn_1_880C(SpriteParticleEffect *object, Vec *rotation)
{
    object->rotation.x = rotation->x;
    object->rotation.y = rotation->y;
    object->rotation.z = rotation->z;
}

// Sets the textured particle effect color.
void SpriteParticleEffect::fn_1_8828(const GXColor *value)
{
    color = *value;
}

// Draws the visible particles as camera-facing textured quads.
void SpriteParticleEffect::draw(Mtx *matrix)
{
    Mtx translation;
    Mtx rotation;
    Mtx transposed;
    f32 halfWidth;
    f32 halfLength;
    s32 index;
    s32 entryCount;
    GXColor tint;

    PSMTXTranspose(*matrix, transposed);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    HuSprTexLoad(animation, 0, 0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    halfWidth = 40.0f;
    halfLength = 80.0f;
    entryCount = elements.size();
    index = 0;
    while (index < entryCount)
    {
        RenderPoint element = const_cast<const std::vector<RenderPoint> &>(elements).at(index);
        if (element.visible != 0)
        {
            Vec translationPosition = element.position;
            PSMTXTrans(
                translation, translationPosition.x, translationPosition.y, translationPosition.z);
            PSMTXRotRad(rotation, 0x7A, MTXDegToRad(element.angle));
            PSMTXConcat(transposed, rotation, rotation);
            PSMTXConcat(translation, rotation, rotation);
            PSMTXConcat(*matrix, rotation, rotation);
            GXLoadPosMtxImm(rotation, GX_PNMTX0);

            tint = color;
            tint.a = (u8)(element.size * alphaScale);
            GXSetTevColor(GX_TEVREG0, tint);

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-halfWidth, -halfLength, 0.0f);
            GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32(halfWidth, -halfLength, 0.0f);
            GXTexCoord2f32(0.0f, 1.0f);
            GXPosition3f32(halfWidth, halfLength, 0.0f);
            GXTexCoord2f32(1.0f, 1.0f);
            GXPosition3f32(-halfWidth, halfLength, 0.0f);
            GXTexCoord2f32(1.0f, 0.0f);
        }
        ++index;
    }
}

// Leaves the textured particles idle before fading.
void SpriteParticleEffect::fn_1_8D40() {}

// Leaves the textured particles unchanged after fading.
void SpriteParticleEffect::fn_1_8D44() {}

// Flashes the effect, then restores full opacity.
void SpriteParticleEffect::fn_1_8D48()
{
    if (elapsed > 1.1f)
    {
        if ((updateCount & 7) == 0)
        {
            alphaScale = 1.0f;
        }
        else
        {
            alphaScale = 0.0f;
        }
        if (elapsed > 2.8f)
        {
            alphaScale = 1.0f;
            setCallback(&SpriteParticleEffect::fn_1_8D44);
        }
    }
    elapsed += (1.0 / 60.0);
}

// Fades out the textured particles.
void SpriteParticleEffect::fn_1_8E1C()
{
    alphaScale = alphaScale - 0.01f;
    if (!(alphaScale > 0.0f))
    {
        alphaScale = 0.0f;
        setCallback(&SpriteParticleEffect::fn_1_8D40);
    }
}

// Chooses a line group rotation and resets its fade timing.
static inline void resetLineRotation(LineFadeState *group)
{
    f64 zAngle = RandomFloat(360.0f);
    f64 xAngle = RandomFloat(360.0f);
    group->rotation.x = (f32)xAngle;
    group->rotation.y = 0.0f;
    group->rotation.z = (f32)zAngle;
    group->state = 0;
    group->duration = RandomUnsigned() & 0x7F;
    group->age = 0;
    group->faded = 0;
    group->active = 0;
}

// Selects the textured particle fade state from the event mode.
void SpriteParticleEffect::handle(int mode)
{
    switch (mode)
    {
    case 0:
        elapsed = 0.0f;
        unk48 = 0;
        setCallback(&SpriteParticleEffect::fn_1_8D48);
        break;
    case 3:
        setCallback(&SpriteParticleEffect::fn_1_8E1C);
        break;
    }
}

void fn_1_9290(FadingLineEffect *context, LineFadeState *work);

// Creates the groups of fading line particles.
FadingLineEffect::FadingLineEffect() : RenderEffect(0x60), callbacks(0)
{
    setLayer(6);
    initialize();
}

// Creates the line groups and registers their scene events.
void FadingLineEffect::initialize()
{
    s32 frameIndex = 0;
    LineFadeState work;

    while (frameIndex < 64)
    {
        fn_1_9290(this, &work);
        frames.push_back(work);
        ++frameIndex;
    }
    position.x = 0.0f;
    position.y = (100.0f * 0.3f);
    position.z = 0.0f;
    static const GXColor initialFadeColor = {0x8C, 0xC8, 0xC8, 0xA0};
    color = initialFadeColor;
    updateCount = 0;
    elapsed = 0.0f;
    unk38 = 0;
    alphaScale = 0.0f;

    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&FadingLineEffect::fn_1_9934), this), 0);

    setCallback(&FadingLineEffect::fn_1_9D28);
}

// Initializes the positions and timing of one line group.
void fn_1_9290(FadingLineEffect *context, LineFadeState *work)
{
    s32 index = 0;
    LinePoint *frame;

    while (index < 16)
    {
        frame = &work->points[index];
        frame->x = RandomFloat(-10.0f, 10.0f);
        frame->y = 100.0 * (1.1f * sin(M_PI * (10.0f * index) / 180.0));
        frame->z = 100.0 * (1.1f * cos(M_PI * (10.0f * index) / 180.0));
        if (index == 0 || index == 15)
        {
            frame->alpha = 0.0f;
        }
        else
        {
            frame->alpha = 255.0f;
        }
        ++index;
    }
    resetLineRotation(work);
}

// Resets the fade, rotation and delay of one line group.
static inline void resetParticleGroup(LineFadeState *group)
{
    s32 particleIndex = 0;
    LinePoint *particle;

    while (particleIndex < 16)
    {
        particle = &group->points[particleIndex];
        if (particleIndex == 0 || particleIndex == 15)
        {
            particle->alpha = 0.0f;
        }
        else
        {
            particle->alpha = 255.0f;
        }
        particleIndex += 1;
    }
    resetLineRotation(group);
}

// Resets one fading line group.
void fn_1_94CC(FadingLineEffect *context, LineFadeState *work)
{
    resetParticleGroup(work);
}

// Reduces each line point alpha and reports completion for the group.
static inline void fadeParticleGroup(FadingLineEffect *owner, LineFadeState *group)
{
    s32 fadedCount = 0;
    s32 index = 0;

    while (index < 16)
    {
        LinePoint *particle = &group->points[index];
        particle->alpha -= 2.0f * (f32)(index + 1);
        if (!(particle->alpha > 0.0f))
        {
            particle->alpha = 0.0f;
            fadedCount += 1;
        }
        index += 1;
    }
    if (fadedCount == 16)
    {
        group->faded = 1;
    }
}

// Advances the fade of one line group.
void fn_1_95E8(FadingLineEffect *owner, LineFadeState *group)
{
    fadeParticleGroup(owner, group);
}

// Advances each line group through its delay, fade and reset states.
void FadingLineEffect::update()
{
    s32 groupCount;
    s32 groupIndex;
    LineFadeState *group;

    if (callback)
    {
        (this->*callback)();
    }
    groupCount = frames.size();
    groupIndex = 0;
    while (groupIndex < groupCount)
    {
        group = &frames.at(groupIndex);
        switch (group->state)
        {
        case 0:
            group->age += 1;
            if (group->age < group->duration)
            {
                break;
            }
            group->active = 1;
            group->state += 1;
            // fall through
        case 1:
            fadeParticleGroup(this, group);
            if (group->faded == 0)
            {
                break;
            }
            group->active = 0;
            group->state += 1;
            // fall through
        case 2:
            resetParticleGroup(group);
            break;
        }
        groupIndex += 1;
    }
    updateCount += 1;
}

// Applies the event offset to the fading line position.
void FadingLineEffect::fn_1_9934(const EventPacket &event)
{
    addEventDisplacement(&position, event);
}

// Sets the fading line color.
void fn_1_996C(FadingLineEffect *self, const GXColor *value)
{
    self->color = *value;
}

// Draws the fading line groups with their current point alpha.
void FadingLineEffect::draw(Mtx *matrix)
{
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetLineWidth(0x1E, GX_TO_ZERO);

    GXColor lineColor = color;
    Mtx xRotation;
    Mtx zRotation;
    Mtx translation;
    Mtx combined;
    PSMTXTrans(translation, position.x, position.y, position.z);
    s32 end;
    end = frames.size();
    for (s32 index = 0; index < end; ++index)
    {
        LineFadeState lineState = const_cast<const std::vector<LineFadeState> &>(frames).at(index);
        if (lineState.active != 0)
        {
            PSMTXRotRad(xRotation, 'x', MTXDegToRad(lineState.rotation.x));
            PSMTXRotRad(zRotation, 'z', MTXDegToRad(lineState.rotation.z));
            PSMTXConcat(zRotation, xRotation, xRotation);
            PSMTXConcat(translation, xRotation, xRotation);
            PSMTXConcat(*matrix, xRotation, xRotation);
            GXLoadPosMtxImm(xRotation, GX_PNMTX0);
            GXBegin(GX_LINESTRIP, GX_VTXFMT0, 16);
            for (s32 vertex = 0; vertex < 16; ++vertex)
            {
                Vec vertexPosition;
                LinePoint point = lineState.points[vertex];
                u8 alpha = (u8)(point.alpha * alphaScale);
                vertexPosition.x = point.x;
                vertexPosition.y = point.y;
                vertexPosition.z = point.z;
                f32 positionX;
                f32 positionY;
                f32 positionZ;
                positionZ = vertexPosition.z;
                positionY = vertexPosition.y;
                positionX = vertexPosition.x;
                GXPosition3f32(positionX, positionY, positionZ);
                GXColor4u8(lineColor.r, lineColor.g, lineColor.b, alpha);
            }
        }
    }
}

// Leaves the fading lines idle before fading.
void FadingLineEffect::fn_1_9D28() {}

// Leaves the fading lines unchanged after fading.
void FadingLineEffect::fn_1_9D2C() {}

// Flashes the effect, then restores full opacity.
void FadingLineEffect::fn_1_9D30()
{
    if (elapsed > 1.1f)
    {
        if ((updateCount & 7) == 0)
        {
            alphaScale = 1.0f;
        }
        else
        {
            alphaScale = 0.0f;
        }
        if (elapsed > 2.8f)
        {
            alphaScale = 1.0f;
            setCallback(&FadingLineEffect::fn_1_9D2C);
        }
    }
    elapsed += (1.0 / 60.0);
}

// Fades out the line effect.
void FadingLineEffect::fn_1_9E04()
{
    alphaScale = alphaScale - 0.01f;
    if (!(alphaScale > 0.0f))
    {
        alphaScale = 0.0f;
        setCallback(&FadingLineEffect::fn_1_9D28);
    }
}

// Selects the line effect fade state from the event mode.
void FadingLineEffect::handle(int mode)
{
    switch (mode)
    {
    case 0:
        elapsed = 0.0f;
        unk38 = 0;
        setCallback(&FadingLineEffect::fn_1_9D30);
        break;
    case 3:
        setCallback(&FadingLineEffect::fn_1_9E04);
        break;
    }
}

static const GXColor lbl_1_rodata_224[2] = {{100, 100, 200, 160}, {100, 200, 200, 160}};

// Initializes a particle at its emitter with a random velocity, size and angle.
void fn_1_9F38(ParticleOwner *emitter, HU3D_PARTICLE_DATA *particle)
{
    particle->time = 0;
    particle->parManId = (s16)((RandomUnsigned() & 0xF) + 5);
    particle->pos = emitter->position;
    particle->vel.x = (1.0 / 60.0) * (4.0f * RandomFloat(-200.0f, 200.0f));
    particle->vel.y = (1.0 / 60.0) * (4.0f * RandomFloat(-200.0f, 200.0f));
    particle->vel.z = (1.0 / 60.0) * (4.0f * RandomFloat(-200.0f, 200.0f));
    particle->color = lbl_1_rodata_224[RandomUnsigned() & 1];
    particle->color.a = 255;
    particle->scale = 100.0f * (0.1f + RandomFloat(0.1f));
    particle->zRot = MTXDegToRad(RandomFloat(360.0f));
}

void fn_1_A6C0(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

// Creates the particle model and installs its update hook.
ParticleOwner::ParticleOwner(ANIMDATA *animation, s32 particleCount)
    : ModeOwner(0x50), stopRecycling(0), callbacks(0)
{
    s16 particleModelId = fn_8003216C(animation, (s16)particleCount);
    this->particleSystem = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[particleModelId].hookData);
    this->particleSystem->work = this;
    fn_800326D0((s16)particleModelId, fn_1_A6C0);
    fn_8003269C((s16)particleModelId, 1);
    fn_80026B8C((s16)particleModelId, 7);
    fn_80026B64((s16)particleModelId, 1);
    fn_80026760((s16)particleModelId, 0.0f, 0.0f, 0.0f);
    fn_800268B0((s16)particleModelId, 0.0f, 0.0f, 0.0f);
    this->modelId = (s16)particleModelId;
    HU3D_PARTICLE_DATA *particle = this->particleSystem->data;
    HU3D_PARTICLE_DATA *end = this->particleSystem->data + this->particleSystem->maxCnt;
    s32 particleIndex = 0;
    while (particle != end)
    {
        fn_1_9F38(this, particle);
        particle++;
        particleIndex++;
    }
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 0.0f;
    this->elapsed = 0.0f;
    this->unk30 = 0;
    this->frame = 0;
    registerEventCallback(
        &this->callbacks, std::bind1st(std::mem_fun(&ParticleOwner::fn_1_A688), this), 0);
    setCallback(&ParticleOwner::fn_1_A988);
}

// Releases the particle model.
ParticleOwner::~ParticleOwner()
{
    particleSystem->work = 0;
    fn_800262D0(modelId);
}

// Runs the current particle emitter state.
void ParticleOwner::update()
{
    if (callbackState)
    {
        (this->*callbackState)();
    }
    ++frame;
}

// Applies the event offset to the particle emitter.
void ParticleOwner::fn_1_A688(const EventPacket &argument)
{
    addEventDisplacement(&position, argument);
}

void fn_1_A710(ParticleOwner *emitter, HU3D_MODEL *, HU3D_PARTICLE *pool, Mtx);

// Dispatches the model hook to its particle emitter.
void fn_1_A6C0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    ParticleOwner *work = static_cast<ParticleOwner *>(particle->work);
    fn_1_A710(work, model, particle, matrix);
}

// Moves, fades and recycles the emitter particles.
void fn_1_A710(ParticleOwner *emitter, HU3D_MODEL *, HU3D_PARTICLE *pool, Mtx)
{
    HU3D_PARTICLE_DATA *particle;
    HU3D_PARTICLE_DATA *end;

    particle = pool->data;
    end = pool->data + pool->maxCnt;
    while (particle != end)
    {
        if (particle->time != 0xFF)
        {
            PSVECAdd(&particle->pos, &particle->vel, &particle->pos);
            PSVECScale(&particle->vel, &particle->vel, 0.96f);
            particle->zRot += MTXDegToRad(2.0f);
            particle->color.a -= particle->parManId;
            if (particle->color.a <= 0x15)
            {
                particle->color.a = 0;
                particle->time = 0xFF;
                if (emitter->stopRecycling != 1)
                {
                    fn_1_9F38(emitter, particle);
                }
            }
        }
        particle++;
    }
}

// Leaves the particle emitter idle.
void ParticleOwner::fn_1_A988() {}

// Advances the timed particle emitter state.
void ParticleOwner::fn_1_A98C()
{
    if (elapsed > 1.1f)
    {
        if ((frame & 7) == 0)
        {
            fn_800268B0(modelId, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            fn_800268B0(modelId, 0.0f, 0.0f, 0.0f);
        }
        if (elapsed > 2.8f)
        {
            fn_800268B0(modelId, 1.0f, 1.0f, 1.0f);
            setCallback(&ParticleOwner::fn_1_AAD0);
        }
    }
    elapsed += (1.0 / 60.0);
}

// Leaves the particle emitter unchanged in this state.
void ParticleOwner::fn_1_AAD0() {}
// Leaves the particle emitter unchanged after its sequence.
void ParticleOwner::fn_1_AAD4() {}

// Selects the particle emitter state from the event mode.
void ParticleOwner::handle(int mode)
{
    switch (mode)
    {
    case 0:
        elapsed = 0.0f;
        unk30 = 0;
        setCallback(&ParticleOwner::fn_1_A98C);
        break;
    case 3:
        stopRecycling = 1;
        setCallback(&ParticleOwner::fn_1_AAD4);
        break;
    }
}

// Creates the animated textured quad effect.
TexturedQuadEffect::TexturedQuadEffect(ANIMDATA *value)
    : RenderEffect(0x60), animation(value), callbacks(0)
{
    Model::setLayer(4);
    fn_1_AC18();
}

// Initializes the quad transform, color and scene events.
void TexturedQuadEffect::fn_1_AC18()
{
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    static const GXColor particleColor = {0x14, 0x28, 0x8C, 0xA0};
    color = particleColor;
    updateCount = 0;
    elapsed = 0.0f;
    unk3C = 0;
    alphaScale = 0.0f;
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&TexturedQuadEffect::fn_1_AE30), this), 0);
    setCallback(&TexturedQuadEffect::fn_1_B2B8);
}

// Runs the current textured quad state.
void TexturedQuadEffect::update()
{
    if (callback)
    {
        (this->*callback)();
    }
    ++updateCount;
}

// Applies the movement event to the textured quad.
void TexturedQuadEffect::fn_1_AE30(const EventPacket &source)
{
    applyAxisDisplacement(position, rotation.y, source);
}

// Draws the textured quad with its current rotation and alpha.
void TexturedQuadEffect::draw(Mtx *matrix)
{
    Mtx rotationMatrix;
    Mtx transform;
    GXColor drawColor;
    f32 halfWidth;
    f32 halfLength;

    PSMTXRotRad(rotationMatrix, 'y', MTXDegToRad(rotation.y));
    PSMTXTrans(transform, position.x, position.y, position.z);
    PSMTXConcat(*matrix, transform, transform);
    PSMTXConcat(transform, rotationMatrix, rotationMatrix);
    GXLoadPosMtxImm(rotationMatrix, GX_PNMTX0);
    HuSprTexLoad(animation, 0, 0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    drawColor = color;
    drawColor.a = (u8)((f32)drawColor.a * alphaScale);
    GXSetTevColor(GX_TEVREG0, drawColor);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    halfWidth = (100.0f * 1.2f);
    halfLength = 520.0f;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-halfWidth, 0.0f, -halfLength);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(halfWidth, 0.0f, -halfLength);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(halfWidth, 0.0f, halfLength);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-halfWidth, 0.0f, halfLength);
    GXTexCoord2f32(1.0f, 0.0f);
}

// Leaves the textured quad idle before fading.
void TexturedQuadEffect::fn_1_B2B8() {}

// Leaves the textured quad unchanged after fading.
void TexturedQuadEffect::fn_1_B2BC() {}

// Flashes the effect, then restores full opacity.
void TexturedQuadEffect::fn_1_B2C0()
{
    if (elapsed > 1.1f)
    {
        if ((updateCount & 7) == 0)
        {
            alphaScale = 1.0f;
        }
        else
        {
            alphaScale = 0.0f;
        }
        if (elapsed > 2.8f)
        {
            alphaScale = 1.0f;
            setCallback(&TexturedQuadEffect::fn_1_B2BC);
        }
    }
    elapsed += (1.0 / 60.0);
}

// Fades out the textured quad.
void TexturedQuadEffect::fn_1_B394()
{
    alphaScale = alphaScale - 0.01f;
    if (!(alphaScale > 0.0f))
    {
        alphaScale = 0.0f;
        setCallback(&TexturedQuadEffect::fn_1_B2B8);
    }
}

// Selects the textured quad fade state from the event mode.
void TexturedQuadEffect::handle(int mode)
{
    switch (mode)
    {
    case 0:
        elapsed = 0.0f;
        unk3C = 0;
        setCallback(&TexturedQuadEffect::fn_1_B2C0);
        break;
    case 3:
        setCallback(&TexturedQuadEffect::fn_1_B394);
        break;
    }
}

// Resets one drifting particle after its fade cycle.
static inline void resetDriftingParticle(HU3D_PARTICLE_DATA *state)
{
    state->time = 0;
    state->parManId = (s16)((RandomUnsigned() & 0x1F) + 15);
    state->pos.x = RandomFloat(-20.0f, 20.0f);
    state->pos.y = RandomFloat(-20.0f, 20.0f);
    state->pos.z = RandomFloat(-20.0f, 20.0f);
    state->scale = 100.0f * (0.6f + RandomFloat(0.4f));
    state->zRot = MTXDegToRad(RandomFloat(360.0f));
    state->color.a = 0;
}

// Reinitializes the completed particle and returns its next phase.
s32 fn_1_B4C8(DriftingParticleEffect *context, HU3D_PARTICLE_DATA *state)
{
    resetDriftingParticle(state);
    return 0;
}

void fn_1_BADC(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
// Creates the drifting particles and their model update hook.
DriftingParticleEffect::DriftingParticleEffect(ANIMDATA *animation, u8 axis)
    : ModeOwner(0x50), axis(axis), callbacks(0)
{
    HU3D_PARTICLE_DATA *record;
    HU3D_PARTICLE_DATA *end;
    s16 particleId;
    s32 recordIndex;
    particleId = fn_8003216C(animation, 0x20);
    this->particleSystem =
        static_cast<HU3D_PARTICLE *>(lbl_802C01C8[(HU3D_MODELID)particleId].hookData);
    this->particleSystem->work = this;
    fn_800326D0((s32)particleId, fn_1_BADC);
    static const GXColor initialParticleColor = {0x40, 0x40, 0xF0, 0xFF};
    fn_800325B0(
        (s32)particleId, initialParticleColor.r, initialParticleColor.g, initialParticleColor.b);
    fn_8003269C((s32)particleId, 1);
    fn_80026B8C((s32)particleId, 5);
    fn_80026B64((s32)particleId, 1);
    fn_80026760((s32)particleId, 0.0f, 0.0f, 2000.0f);
    this->modelId = (s32)particleId;
    record = this->particleSystem->data;
    end = this->particleSystem->data + this->particleSystem->maxCnt;
    for (recordIndex = 0; record != end; recordIndex++)
    {
        fn_1_B4C8(this, record);
        record++;
    }
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 2000.0f;
    registerEventCallback(
        &this->callbacks, std::bind1st(std::mem_fun(&DriftingParticleEffect::fn_1_BA40), this), 0);
}

// Releases the drifting particle model.
DriftingParticleEffect::~DriftingParticleEffect()
{
    particleSystem->work = 0;
    fn_800262D0(modelId);
}

// Moves the drifting particle emitter.
void DriftingParticleEffect::fn_1_B988(const Vec *value)
{
    position = *value;
    fn_80026760(modelId, value->x, value->y, value->z);
}

// Sets the drifting particle model color.
void fn_1_B9E8(DriftingParticleEffect *self, const GXColor *color)
{
    fn_800325B0(self->modelId, color->r, color->g, color->b);
}

// Leaves particle updates to the model hook.
void DriftingParticleEffect::update() {}

// Applies a movement event to the drifting particle emitter.
void DriftingParticleEffect::fn_1_BA40(const EventPacket &event)
{
    Vec *displacement = (Vec *)event.context;
    if (axis == 1)
    {
        position.x += displacement->x;
    }
    else
    {
        position.z += displacement->z;
    }
    fn_1_B988(&position);
}

void fn_1_BB2C(DriftingParticleEffect *context, HU3D_MODEL *, HU3D_PARTICLE *particleSystem, Mtx);

// Dispatches the model hook to its drifting particle owner.
void fn_1_BADC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    DriftingParticleEffect *work = static_cast<DriftingParticleEffect *>(particle->work);
    fn_1_BB2C(work, model, particle, matrix);
}

// Advances drifting particles through their fade and recycle states.
void fn_1_BB2C(DriftingParticleEffect *context, HU3D_MODEL *, HU3D_PARTICLE *particleSystem, Mtx)
{
    HU3D_PARTICLE_DATA *entry = particleSystem->data;
    HU3D_PARTICLE_DATA *end = particleSystem->data + particleSystem->maxCnt;

    for (; entry != end; ++entry)
    {
        switch (entry->time)
        {
        case 0:
            entry->color.a += 0x20;
            if (entry->color.a >= 0xC8)
            {
                entry->color.a = 0xC8;
                ++entry->time;
            }
            else
            {
                break;
            }
            // fall through
        case 1:
            entry->color.a -= entry->parManId;
            if (entry->color.a <= 0x32)
            {
                entry->color.a = 0;
                ++entry->time;
            }
            else
            {
                break;
            }
            // fall through
        case 2:
            resetDriftingParticle(entry);
            continue;
        default:
            break;
        }
        entry->zRot += MTXDegToRad(1.0f);
    }
}

void fn_1_C738(TexturedModelEffect *, HU3D_DRAW_OBJ *, HSF_MATERIAL *);
void fn_1_CAC8(TexturedModelEffect *, Mtx);
void fn_1_CF60(TexturedModelEffect *, Mtx);

// Dispatches the material hook to the textured model effect.
void fn_1_BD08(HU3D_DRAW_OBJ *object, HSF_MATERIAL *material)
{
    fn_1_C738(static_cast<TexturedModelEffect *>(object->model->hookData), object, material);
}

// Dispatches the rotating texture draw hook.
void fn_1_BD48(HU3D_MODEL *model, Mtx *matrix)
{
    fn_1_CAC8(static_cast<TexturedModelEffect *>(model->hookData), *matrix);
}

// Dispatches the textured quad draw hook.
void fn_1_BD80(HU3D_MODEL *model, Mtx *matrix)
{
    fn_1_CF60(static_cast<TexturedModelEffect *>(model->hookData), *matrix);
}

// Takes ownership of the supplied model and initializes the textured effect.
TexturedModelEffect::TexturedModelEffect(OwnedModel *model,
                                         ANIMDATA *lightAnimation,
                                         ANIMDATA *shadowAnimation,
                                         ANIMDATA *rotatingAnimation)
    : ModeOwner(0x50), resource(model), shadowModel(0), rotatingModel(0), temporaryModel(0),
      lightTexture(lightAnimation), shadowTexture(shadowAnimation),
      rotatingTexture(rotatingAnimation), entry(150.0f), callbacks(0)
{
    fn_1_C170();
}

// Copies the supplied model and creates the textured draw-hook models.
TexturedModelEffect::TexturedModelEffect(const OwnedModel &model,
                                         ANIMDATA *lightAnimation,
                                         ANIMDATA *shadowAnimation,
                                         ANIMDATA *rotatingAnimation)
    : ModeOwner(0x50), resource(new OwnedModel(model)), shadowModel(new OwnedModel(fn_1_BD80)),
      rotatingModel(new OwnedModel(fn_1_BD48)), temporaryModel(0), lightTexture(lightAnimation),
      shadowTexture(shadowAnimation), rotatingTexture(rotatingAnimation), entry(100.0f),
      callbacks(0)
{
    fn_1_C170();
    entry.collisionClass = 0;
}

// Configures the model textures, collision record and scene events.
void TexturedModelEffect::fn_1_C170()
{
    s32 i;
    resource->setAttribute(1 << 0);
    resource->setLayer(6);
    resource->setShadow();
    resource->setHookData(this);
    rotatingModel->setAttribute(1 << 0);
    rotatingModel->setLayer(6);
    rotatingModel->setHookData(this);
    shadowModel->setAttribute(1 << 0);
    shadowModel->setLayer(6);
    shadowModel->setHookData(this);
    fn_80027884(*resource.get(), fn_1_BD08);
    texturePosition.x = 0.0f;
    texturePosition.y = 0.0f;
    texturePosition.z = 0.0f;
    fn_1_C484(0.0f);
    entry.position.x = 0.0f;
    entry.position.y = 0.0f;
    entry.position.z = 0.0f;
    phase = 0.0f;
    wave = 0.0f;
    spinAngle = 0.0f;
    sequenceElapsed = 0.0f;
    state = 0;
    for (i = 0; i < 2; i++)
    {
        offsets[i] = 0.0f;
    }
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&TexturedModelEffect::fn_1_C6B0), this), 0);
    setCallback(&TexturedModelEffect::fn_1_D360);
}

// Copies the effect position to the collision record and model.
void fn_1_C420(TexturedModelEffect *owner, Vec *position)
{
    owner->entry.setPosition(*position);
    owner->resource->setPosition(position);
}

// Sets the effect scale and applies it to the model.
void TexturedModelEffect::fn_1_C484(f32 time)
{
    resource->setUniformScale(time);
    scale = time;
}

// Updates the texture oscillation, vertical drift, scale and spin angle.
void TexturedModelEffect::update()
{
    f32 adjustedPosition;
    s32 index;

    if (callback)
    {
        (this->*callback)();
    }
    texturePosition.y = (f32)(texturePosition.y - 0.006);
    wave = (f32)sin((M_PI * (4.0f * (360.0f * phase))) / 180.0);
    for (index = 0; index < 2; ++index)
    {
        offsets[index] =
            (f32)sin((M_PI * (0.5f * (360.0f * phase) * (0.5f * (f32)index))) / 180.0);
    }
    adjustedPosition = scale + 0.1f * wave;
    resource->setUniformScale(adjustedPosition);
    spinAngle += 10.0 / 60.0;
    phase += (1.0 / 60.0);
}

// Applies the event offset to the effect model and collision record.
void TexturedModelEffect::fn_1_C6B0(const EventPacket &event)
{
    addEventDisplacement(&entry.position, event);
    entry.setPosition(entry.position);
    resource->setPosition(&entry.position);
}

static f32 lbl_1_data_46C[2][3] = {{0.3f, 0.0f, 0.0f}, {0.0f, 0.3f, 0.0f}};

// Sets the camera-space light texture and material for the model.
void fn_1_C738(TexturedModelEffect *effect, HU3D_DRAW_OBJ *drawObject, HSF_MATERIAL *material)
{
    Mtx translated;
    Mtx scaled;
    Mtx lightTexture;
    Mtx perspective;
    Mtx modelTexture;
    GXTexObj texture;
    HU3D_CAMERA *camera;
    TextureController *textureOwner = &MinigameSingleton<TextureController>::getChecked();
    GXTexObj *textureObject = &textureOwner->texture;
    texture = *textureObject;
    GXColor color = {0x60, 0x60, 0xE0, 0x40};
    camera = &Hu3DCamera[lbl_802C01A6];
    GXLoadTexObj(&texture, GX_TEXMAP0);
    HuSprTexLoad(effect->lightTexture, 0, 1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumTexGens(2);
    GXSetNumTevStages(1);
    C_MTXLightPerspective(perspective, camera->fov, 1.2f, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXInverse(lbl_802613F4, modelTexture);
    PSMTXConcat(modelTexture, drawObject->matrix, translated);
    PSMTXConcat(perspective, lbl_802613F4, lightTexture);
    PSMTXConcat(lightTexture, translated, lightTexture);
    GXLoadTexMtxImm(lightTexture, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumIndStages(1);
    PSMTXTrans(translated, effect->texturePosition.x, effect->texturePosition.y,
               effect->texturePosition.z);
    PSMTXScale(scaled, 0.1f, 0.1f, 1.0f);
    PSMTXConcat(scaled, translated, lightTexture);
    GXLoadTexMtxImm(lightTexture, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_data_46C, -1);
    fn_8001CDF0(drawObject->model, 0, material->hiliteScale);
}

// Draws the two rotating textured effect quads.
void fn_1_CAC8(TexturedModelEffect *effect, Mtx matrix)
{
    Mtx translated;
    Mtx transform;
    Mtx transposed;

    PSMTXTranspose(matrix, transposed);
    PSMTXTrans(
        translated, effect->entry.position.x, effect->entry.position.y, effect->entry.position.z);
    HuSprTexLoad(effect->rotatingTexture, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXColor color = {0xC8, 0xC8, 0xFA, 0x80};
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetCullMode(GX_CULL_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    for (s32 side = 0; side < 2; side++)
    {
        f32 halfWidth = 100.0f;
        f32 adjustedSize = effect->scale + 0.6f * effect->offsets[side];
        halfWidth *= adjustedSize;
        f32 angle;
        if ((side & 1) == 0)
        {
            angle = effect->spinAngle;
        }
        else
        {
            angle = -effect->spinAngle;
        }
        f32 rotationAngle = angle;

        PSMTXRotRad(transform, 'z', MTXDegToRad(rotationAngle));
        PSMTXConcat(transposed, transform, transform);
        PSMTXConcat(translated, transform, transform);
        PSMTXConcat(matrix, transform, transform);
        GXLoadPosMtxImm(transform, GX_PNMTX0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-halfWidth, -halfWidth, 0.0f);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(-halfWidth, halfWidth, 0.0f);
        GXTexCoord2f32(0.0f, 1.0f);
        GXPosition3f32(halfWidth, halfWidth, 0.0f);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(halfWidth, -halfWidth, 0.0f);
        GXTexCoord2f32(1.0f, 0.0f);
    }
}

// Draws the scaled textured effect quad.
void fn_1_CF60(TexturedModelEffect *effect, Mtx matrix)
{
    Mtx transform;
    Mtx transposed;

    PSMTXTranspose(matrix, transposed);
    PSMTXTrans(
        transform, effect->entry.position.x, effect->entry.position.y, effect->entry.position.z);
    PSMTXConcat(transform, transposed, transform);
    PSMTXConcat(matrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    HuSprTexLoad(effect->shadowTexture, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    GXColor color = {0xDC, 0xC8, 0xFA, 0xC8};
    GXSetTevColor(GX_TEVREG0, color);

    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    f32 quadHalfWidth = 52.0f;
    f32 widthScale = effect->scale + 0.05f * effect->wave;
    quadHalfWidth *= widthScale;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-quadHalfWidth, -quadHalfWidth, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(-quadHalfWidth, quadHalfWidth, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(quadHalfWidth, quadHalfWidth, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(quadHalfWidth, -quadHalfWidth, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
}

// Leaves the textured model idle before its sequence.
void TexturedModelEffect::fn_1_D360() {}

// Advances the textured model fade and replacement sequence.
void TexturedModelEffect::fn_1_D364()
{
    switch (state)
    {
    case 0:
        if (sequenceElapsed < 2.8f)
        {
            break;
        }

        temporaryModel.reset(new OwnedModel(DATANUM(0x570000, 7)));
        temporaryModel->setLayer(7);
        temporaryModel->setAttribute(1 << 22);
        resource->resetAttribute(1 << 0);
        rotatingModel->resetAttribute(1 << 0);
        shadowModel->resetAttribute(1 << 0);
        state += 1;

        // fall through
    case 1:
        if (fn_8002D75C(*temporaryModel) != 0)
        {
            temporaryModel.reset(0);
            setCallback(&TexturedModelEffect::fn_1_D5F0);
            state += 1;
        }
        break;
    }

    if (sequenceElapsed > 3.0f)
    {
        f32 fade = 7.0f - (5.0f * ((sequenceElapsed - 3.0f) / 0.4f));
        if (sequenceElapsed > 3.4f)
        {
            fade = 2.0f;
        }
        fn_1_C484(fade);
    }
    sequenceElapsed += (1.0 / 60.0);
}

// Leaves the textured model unchanged in this state.
void TexturedModelEffect::fn_1_D5F0() {}

// Shrinks the textured model until its sequence completes.
void TexturedModelEffect::fn_1_D5F4()
{
    scale -= 0.02f;
    fn_1_C484(scale);
    if (!(scale > 0.0f))
    {
        rotatingModel->setAttribute(1 << 0);
        resource->setAttribute(1 << 0);
        shadowModel->setAttribute(1 << 0);
        setCallback(&TexturedModelEffect::fn_1_D360);
    }
}

// Selects the textured model state from the event mode.
void TexturedModelEffect::handle(int mode)
{
    switch (mode)
    {
    case 0:
        sequenceElapsed = 0.0f;
        state = 0;
        setCallback(&TexturedModelEffect::fn_1_D364);
        break;
    case 3:
        setCallback(&TexturedModelEffect::fn_1_D5F4);
        break;
    }
}

// Releases the effect models and event registration.
TexturedModelEffect::~TexturedModelEffect() {}
