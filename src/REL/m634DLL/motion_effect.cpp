#include "REL/m634DLL/random.h"
#include "REL/m634DLL/motion_effect.h"
extern "C" {
void HuSprTexLoad(ANIMDATA *, short, short, GXTexWrapMode, GXTexWrapMode, GXTexFilter);
}

// Initializes the motion resource, position, and state.
MotionObject::MotionObject(ANIMDATA *resourceId)
    : ModeRenderObject(0x60)
{
    active = 0;
    resource = resourceId;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    phase = 0.0f;
    state = 0;
    unk3C = 0;
}
// Releases the object and its member-owned resources.
MotionObject::~MotionObject() { }
// Creates the requested motion records and configures drawing.
MotionEffect::MotionEffect(ANIMDATA *resourceId, int iterations, int selector)
    : MotionObject(resourceId)
{
    setCamera(2);
    setLayer((short)selector);
    motionCount = iterations;
    for (int index = 0; index < motionCount; index++) {
        MotionRecord temporary;
        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        initializeMotion(&temporary, 0);
        motions.push_back(temporary);
    }
}

// Releases the object and its member-owned resources.
MotionEffect::~MotionEffect() { }
struct MotionColor {
    unsigned char bytes[4];
};
static const MotionColor motionColor = { { 0xFF, 0xDC, 0x32, 0xFF } };

// Initializes a sprite motion with random displacement and duration.
void MotionEffect::initializeMotion(MotionRecord *motion, int duration)
{
    MotionColor flags = motionColor;
    Vec displacement;
    displacement.x = RandomFloat(-100.0f, 100.0f);
    displacement.y = RandomFloat(-50.0f, 50.0f);
    displacement.z = RandomFloat(-100.0f, 100.0f);

    motion->state = 0;
    motion->duration = duration;
    motion->frameCount = (int)((60.0 * (double)duration / 5.0) * RandomFloat(0.8f, 1.2f));
    motion->position = position;
    PSVECAdd(&motion->position, &displacement, &motion->destination);
    motion->scale.x = 0.0f;
    motion->scale.y = 0.0f;
    motion->scale.z = 0.0f;
    motion->width = 100.0;
    motion->height = 100.0;
    motion->rotation = 45.0;
    motion->spriteScale = 0.0;
    motion->scaleFactor = 1.05;
    motion->scaleLimit = 1.0;
    motion->fadeStep = 1.0;
    motion->color.r = flags.bytes[0];
    motion->color.g = flags.bytes[1];
    motion->color.b = flags.bytes[2];
    motion->color.a = flags.bytes[3];
}
static const double halfExtentScale = 0.5;

// Restarts a sprite motion with a new random displacement.
void MotionEffect::restartMotion(MotionRecord *motion)
{
    Vec displacement;

    displacement.x = RandomFloat(-100.0f, 100.0f);
    displacement.y = RandomFloat(-50.0f, 50.0f);
    displacement.z = RandomFloat(-100.0f, 100.0f);
    motion->state = 2;
    motion->frameCount = (int)(((60.0 * motion->duration) / 5.0) * RandomFloat(0.8f, 1.2f));
    motion->destination = motion->position;
    PSVECAdd(&motion->position, &displacement, &motion->destination);
    motion->rotation = 45.0;
    motion->spriteScale = 0.0;
    motion->scaleFactor = 1.05;
    if (motion->scaleLimit > halfExtentScale) {
        motion->scaleLimit -= 0.05;
    }
    motion->fadeStep = 1.0;
    motion->color.a = 255;
}
// Moves the effect and restarts its motion records.
void MotionEffect::reposition(const Vec *position)
{
    this->position = *position;
    s32 index = 0;
    s32 motionId = 0;
    for (; index < motionCount; index++) {
        MotionRecord *motion = &motions.at(index);
        if (motionId == 5) {
            break;
        }
        if (motion->state == 0) {
            initializeMotion(motion, motionId);
            motion->state = 1;
            motionId++;
        }
    }
}
// Runs the effect callback and advances active sprite motions.
void MotionEffect::update()
{
    s32 index;
    MotionRecord *motion;

    if (callback) {
        (this->*callback)();
    }
    for (index = 0; index < motionCount; index++) {
        motion = &motions.at(index);
        if (motion->state != 0) {
            if (motion->frameCount > 1) {
                motion->frameCount--;
            } else {
                if (motion->frameCount == 1) {
                    motion->frameCount--;
                    motion->spriteScale = 0.2;
                }
                motion->color.a -= motion->fadeStep;
                motion->rotation += 2.0;
                motion->spriteScale = motion->spriteScale * motion->scaleFactor;
                if (motion->spriteScale > motion->scaleLimit && motion->scaleFactor > 1.0) {
                    motion->scaleFactor = 1.0 - (motion->scaleFactor - 1.0);
                }
                if (!(motion->scaleFactor > 1.0) && !(motion->spriteScale > 0.2)) {
                    if (motion->duration > 2) {
                        motion->spriteScale = 0.0;
                        motion->state = 3;
                    } else {
                        restartMotion(motion);
                    }
                }
            }
        }
    }
}
// Draws the active motion sprites.
void MotionEffect::draw(Mtx *parentMatrix)
{
    s32 index;
    Mtx spriteScale;
    Mtx spriteTranslation;
    Mtx spriteRotation;
    Mtx parentRotation;

    PSMTXTranspose(*parentMatrix, parentRotation);
    GXSetCullMode(GX_CULL_BACK);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    HuSprTexLoad(resource, 0, 0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    for (index = 0; index < motionCount; index++) {
        MotionRecord sprite = static_cast<const std::vector<MotionRecord> &>(motions).at(index);
        if (0.0 != sprite.spriteScale) {
            GXSetTevColor(GX_TEVREG0, sprite.color);
            {
                f32 halfWidth;
                f32 halfHeight;

                PSMTXScale(spriteScale, (f32)sprite.spriteScale, (f32)sprite.spriteScale, 1.0f);
                PSMTXTrans(spriteTranslation, sprite.destination.x, sprite.destination.y,
                    sprite.destination.z);
                PSMTXRotRad(spriteRotation, 'z', (f32)(MTXDegToRad(sprite.rotation)));
                PSMTXConcat(*parentMatrix, spriteTranslation, spriteTranslation);
                PSMTXConcat(spriteTranslation, parentRotation, spriteTranslation);
                PSMTXConcat(spriteTranslation, spriteScale, spriteTranslation);
                PSMTXConcat(spriteTranslation, spriteRotation, spriteTranslation);
                GXLoadPosMtxImm(spriteTranslation, GX_PNMTX0);

                halfWidth = (f32)(sprite.width / 2.0);
                halfHeight = (f32)(sprite.height / 2.0);
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(-halfWidth, halfHeight, 0.0f);
                GXTexCoord2f32(1.0f, 0.0f);
                GXPosition3f32(halfWidth, halfHeight, 0.0f);
                GXTexCoord2f32(0.0f, 0.0f);
                GXPosition3f32(halfWidth, -halfHeight, 0.0f);
                GXTexCoord2f32(0.0f, 1.0f);
                GXPosition3f32(-halfWidth, -halfHeight, 0.0f);
                GXTexCoord2f32(1.0f, 1.0f);
            }
        }
    }
}
// Selects the motion effect callback for an event mode.
void MotionEffect::mode(int value)
{
    switch (value) {
    case 0: {
        phase = 0.0f;
        state = 0;
        void (MotionEffect::*next)() = &MotionEffect::action1;
        callback = next;
        break;
    }
    case 2: {
        void (MotionEffect::*next)() = &MotionEffect::action2;
        callback = next;
        break;
    }
    case 3: {
        active = 1;
        void (MotionEffect::*next)() = &MotionEffect::action3;
        callback = next;
        break;
    }
    }
}
