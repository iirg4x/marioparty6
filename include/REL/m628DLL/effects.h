#ifndef REL_M628DLL_EFFECTS_H
#define REL_M628DLL_EFFECTS_H
#include <memory>
#include "dolphin/mtx.h"
#include "REL/m628DLL/render_effect.h"
// Registers a game object in the supplied event category.
class ModeOwner : public GameObject
{
  public:
    // Registers this object in the supplied category.
    explicit ModeOwner(int category) : GameObject(category) {}
    // Destroys this ModeOwner and its members.
    virtual ~ModeOwner() {}
    // Leaves unhandled events unchanged.
    virtual void handle(int) {}
};
// Owns a particle system and its event-driven fade state.
class ParticleOwner : public ModeOwner
{
  public:
    s16 modelId;
    u8 unk12[6];                   /* No code names this field. */
    HU3D_PARTICLE *particleSystem; // Particle system associated with the owned model.
    Vec position;
    u8 stopRecycling; // Stops particle recycling when set.
    f32 elapsed;
    s32 unk30; /* The game's code never reads this field. */
    s32 frame;
    void (ParticleOwner::*callbackState)(); // Runs the particle fade state.
    // Selects the current update callback.
    void setCallback(void (ParticleOwner::*value)())
    {
        callbackState = value;
    }
    EventHandlerManager callbacks; // Receives the object movement and state events.
    ParticleOwner(ANIMDATA *, s32);
    virtual ~ParticleOwner();
    virtual void update();
    virtual void handle(int);
    void fn_1_A688(const EventPacket &);
    void fn_1_A988();
    void fn_1_A98C();
    void fn_1_AAD0();
    void fn_1_AAD4();
};
// Draws an animated textured quad with position, rotation and alpha.
class TexturedQuadEffect : public RenderEffect
{
  public:
    ANIMDATA *animation;
    Vec position;
    Vec rotation;
    GXColor color;
    s32 updateCount; // Number of effect updates since initialization.
    f32 elapsed;
    s32 unk3C;                     /* The game's code never reads this field. */
    f32 alphaScale;                // Multiplier applied to the drawn alpha.
    EventHandlerManager callbacks; // Receives the object movement and state events.
    void (TexturedQuadEffect::*callback)(); // Runs the quad fade state.
    // Selects the current update callback.
    void setCallback(void (TexturedQuadEffect::*value)())
    {
        callback = value;
    }
    void fn_1_AC18();
    void fn_1_AE30(const EventPacket &);
    void fn_1_B2B8();
    void fn_1_B2BC();
    void fn_1_B2C0();
    void fn_1_B394();
    TexturedQuadEffect(ANIMDATA *);
    // Destroys this TexturedQuadEffect and its members.
    virtual ~TexturedQuadEffect() {}
    virtual void handle(int);
    virtual void update();
    virtual void draw(Mtx *);
};
// Owns a drifting particle system and its movement events.
class DriftingParticleEffect : public ModeOwner
{
  public:
    s16 modelId;
    u8 unk12[6];                   /* No code names this field. */
    HU3D_PARTICLE *particleSystem; // Particle system associated with the drifting model.
    Vec position;
    u8 axis;                       // Selects movement along x or z.
    EventHandlerManager callbacks; // Receives the object movement and state events.
    DriftingParticleEffect(ANIMDATA *, u8);
    virtual ~DriftingParticleEffect();
    virtual void update();
    void fn_1_B988(const Vec *);
    void fn_1_BA40(const EventPacket &);
};
// Owns a model, its textured draw hooks and a collision record.
class TexturedModelEffect : public ModeOwner
{
  public:
    std::auto_ptr<OwnedModel> resource; // Owns the model receiving the effect material hook.
    std::auto_ptr<OwnedModel> shadowModel;
    std::auto_ptr<OwnedModel> rotatingModel;
    std::auto_ptr<OwnedModel> temporaryModel;
    ANIMDATA *lightTexture;    // Texture used by the camera-space light material.
    ANIMDATA *shadowTexture;   // Texture used by the scaled quad.
    ANIMDATA *rotatingTexture; // Texture used by the rotating quad.
    ActorRangeRecord entry; // Collision shape moved with the effect model.
    Vec texturePosition; // Translation used by the indirect texture matrix.
    f32 scale;
    f32 phase; // Time used by the sinusoidal scale and texture offsets.
    f32 wave; // Current sinusoidal scale displacement.
    f32 offsets[2]; // Sinusoidal offsets for the two textured quads.
    f32 spinAngle; // Angle used to rotate the textured model.
    f32 sequenceElapsed;
    s32 state;
    EventHandlerManager callbacks; // Receives the object movement and state events.
    void (TexturedModelEffect::*callback)(); // Runs the textured model state.
    // Selects the current update callback.
    void setCallback(void (TexturedModelEffect::*value)())
    {
        callback = value;
    }
    void fn_1_C170();
    void fn_1_C484(f32);
    void fn_1_C6B0(const EventPacket &);
    void fn_1_D360();
    void fn_1_D364();
    void fn_1_D5F0();
    void fn_1_D5F4();
    TexturedModelEffect(const OwnedModel &, ANIMDATA *, ANIMDATA *, ANIMDATA *);
    TexturedModelEffect(OwnedModel *, ANIMDATA *, ANIMDATA *, ANIMDATA *);
    virtual ~TexturedModelEffect();
    virtual void update();
    virtual void handle(int);
};
#include "REL/framework/singleton.h"
// Copies the rendered frame into a texture for the model effect.
class TextureController : public MinigameSingleton<TextureController>, public RenderEffect
{
  public:
    GXTexObj texture;

    u8 textureData[640 * 480 * 2] ATTRIBUTE_ALIGN(32);
    TextureController();
    virtual ~TextureController();
    virtual void update();
    virtual void draw(Mtx *);
};
void fn_1_B9E8(DriftingParticleEffect *, const GXColor *);
void fn_1_C420(TexturedModelEffect *, Vec *);
#endif
