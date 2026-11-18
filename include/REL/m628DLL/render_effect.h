#ifndef REL_M628DLL_RENDER_EFFECT_H
#define REL_M628DLL_RENDER_EFFECT_H
#include "REL/framework/game_object2.h"
#include "REL/framework/event_hm.h"
#include "dolphin/gx.h"
#include "game/animdata.h"
#include <vector>
#include <functional>

// Registers a renderable game object in an effect category.
class RenderEffect : public RenderObject
{
  public:
    // Registers this object in the supplied category.
    explicit RenderEffect(int category) : RenderObject(category) {}
    // Destroys this RenderEffect and its members.
    virtual ~RenderEffect() {}
    // Leaves unhandled events unchanged.
    virtual void handle(int mode) {}
};

#include "REL/m628DLL/range_record.h"
// Registers the radius and half-height of a cylindrical collision record.
struct RangeRecord : RangeRecordBase
{
    f32 radius;
    f32 halfHeight;
    // Sets the cylinder dimensions and registers this collision record.
    RangeRecord()
    {
        radius = (100.0f * 0.3f);
        halfHeight = 1200.0f;
        lbl_1_bss_34.push_back(this);
    }
    // Destroys this RangeRecord and its members.
    virtual ~RangeRecord() {}
    // Returns the collision radius.
    f32 getRadius() const
    {
        return radius;
    }
    // Returns the collision half-height.
    f32 getHalfHeight() const
    {
        return halfHeight;
    }
};

class PointLineEffect;
typedef void (PointLineEffect::*PointLineEventCallback)(const EventPacket &);
typedef void (PointLineEffect::*PointLineStateCallback)();

// Rotates and fades a sixteen-point line with a collision record.
class PointLineEffect : public RenderEffect
{
  public:
    std::vector<Vec> points;
    RangeRecord record;
    Mtx matrix;
    Vec rotation;
    f32 spinRate;    // Angular speed applied to the line each update.
    u32 pointOffset; // Advances the line vertex and color sampling index.
    GXColor color;
    f32 elapsed;
    s32 unk98;      /* The game's code never reads this field. */
    f32 alphaScale; // Multiplier applied to the drawn alpha.
    s32 soundId;
    u8 soundStarted;                 // Prevents the line sound from starting twice.
    EventHandlerManager callbacks;   // Receives the object movement and state events.
    PointLineStateCallback callback; // Runs the current object state.
    // Selects the current update callback.
    void setCallback(PointLineStateCallback value)
    {
        callback = value;
    }

    PointLineEffect();
    virtual ~PointLineEffect();
    virtual void update();
    virtual void draw(Mtx *matrix);
    virtual void handle(int mode);
    void initialize();
    void fn_1_760C(const EventPacket &event);
    void setSpinRate(f32 value);
    void setRotation(const Vec *value);
    void fn_1_7B74();
    void fn_1_7B78();
    void fn_1_7B7C();
    void fn_1_7D0C();
};

// Stores the position, appearance and lifetime of one textured particle.
struct RenderPoint
{
    Vec position;
    f32 size;
    f32 angle;
    f32 randomOffset;
    s32 state; // Selects the particle lifetime phase.
    s32 delay; // Updates before the particle becomes visible.
    s32 age;
    u8 visible;
};

class SpriteParticleEffect;
typedef void (SpriteParticleEffect::*RenderEventCallback)(const EventPacket &);
typedef void (SpriteParticleEffect::*RenderStateCallback)();

// Updates and draws a group of textured sprite particles.
class SpriteParticleEffect : public RenderEffect
{
  public:
    std::vector<RenderPoint> elements; // Per-particle positions, sizes and lifetime states.
    ANIMDATA *animation;               // Animation data used to load the sprite particle texture.
    Vec position;
    Vec rotation;
    GXColor color;
    s32 updateCount; // Number of effect updates since initialization.
    f32 elapsed;
    s32 unk48;                     /* The game's code never reads this field. */
    f32 alphaScale;                // Multiplier applied to the drawn alpha.
    EventHandlerManager callbacks; // Receives the object movement and state events.
    RenderStateCallback callback;  // Runs the current object state.
    // Selects the current update callback.
    void setCallback(RenderStateCallback value)
    {
        callback = value;
    }

    explicit SpriteParticleEffect(ANIMDATA *animation);
    // Destroys this SpriteParticleEffect and its members.
    virtual ~SpriteParticleEffect() {}
    virtual void update();
    virtual void draw(Mtx *matrix);
    virtual void handle(int mode);
    void fn_1_879C(const EventPacket &event);
    void fn_1_87F0(const Vec *value);
    void fn_1_8828(const GXColor *value);
    void fn_1_8D40();
    void fn_1_8D44();
    void fn_1_8D48();
    void fn_1_8E1C();
    void fn_1_8008();
    void initializeElement(RenderPoint *element);
};

// Stores one fading line vertex and its alpha.
struct LinePoint
{
    f32 x;
    f32 y;
    f32 z;
    f32 alpha;
};

// One line of sixteen fading points.
struct LineFadeState
{
    LinePoint points[16]; // Vertices and alpha values of this line.
    Vec rotation;
    s32 state; // Selects the line fade phase.
    s32 duration; // Number of updates in the current phase.
    s32 age; // Updates elapsed in the current phase.
    u8 faded; // Set once the line has finished fading.
    u8 active; // Enables updates and drawing for this line.
};

class FadingLineEffect;
typedef void (FadingLineEffect::*FadingLineEventCallback)(const EventPacket &);
typedef void (FadingLineEffect::*FadingLineStateCallback)();

// Updates and draws groups of independently fading lines.
class FadingLineEffect : public RenderEffect
{
  public:
    std::vector<LineFadeState> frames; // Independently updated fading lines.
    Vec position;
    GXColor color;
    s32 updateCount; // Number of effect updates since initialization.
    f32 elapsed;
    s32 unk38;                        /* The game's code never reads this field. */
    f32 alphaScale;                   // Multiplier applied to the drawn alpha.
    EventHandlerManager callbacks;    // Receives the object movement and state events.
    FadingLineStateCallback callback; // Runs the current object state.
    // Selects the current update callback.
    void setCallback(FadingLineStateCallback value)
    {
        callback = value;
    }

    FadingLineEffect();
    // Destroys this FadingLineEffect and its members.
    virtual ~FadingLineEffect() {}
    virtual void update();
    virtual void draw(Mtx *matrix);
    virtual void handle(int mode);
    void initialize();
    void fn_1_9934(const EventPacket &event);
    void fn_1_9D28();
    void fn_1_9D2C();
    void fn_1_9D30();
    void fn_1_9E04();
};

void fn_1_7660(PointLineEffect *object, const Vec *position);
void fn_1_76C0(PointLineEffect *object, const GXColor *color);
void fn_1_880C(SpriteParticleEffect *object, Vec *rotation);
#endif
