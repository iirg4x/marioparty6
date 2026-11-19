#include "REL/m605DLL/value_bounds.h"
#include "REL/m605DLL/animation_owner.h"
#include "REL/m605DLL/particle_objects.h"
#include "REL/framework/sound.h"
#include "REL/framework/event.h"
#include "REL/framework/misc_rotarc.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/m605DLL/scene_stage.h"
#include <memory>
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/registered_value.h"
#include "REL/framework/sphere.h"
#include "REL/m605DLL/falling_rock.h"
extern "C" {
#include "game/audio.h"
}

s32 lbl_1_bss_50;
s32 lbl_1_bss_54;
s32 lbl_1_bss_58;

// Creates the moving model, its registered spheres and its sound.
FallingRock::FallingRock()
    : GameObject(1), model(DATANUM(0x400000, 1)), body(0), sweep(0), lower(0)
{
    lbl_1_bss_30 = this;
    model.setShadow();
    f32 radius = 450.0f;
    body.radius = radius;
    body.center.x = 0.0f;
    body.center.y = (f64)(2400.0f + radius);
    body.center.z = -2400.0f;
    previousPosition = body.center;
    fn_1_1BA8(this);
    lower = body;
    lower.center.y = radius;
    lower.setFlag7();
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    rotation.w = 1.0f;
    displacement.x = 0.0f;
    displacement.y = 0.0f;
    displacement.z = 0.0f;
    callback = &FallingRock::fn_1_136C;
    limitReached = false;
    fallEnded = false;
    verticalStep = 0.0f;
    angle = 0.0f;
    body.clearFlag7();
    sweep.clearFlag7();
    registerChannel(0);
    lbl_1_bss_50 = soundPlay(0x644);
    lbl_1_bss_54 = 0;
}

// Applies the event to vertical acceleration or the motion impulse.
void FallingRock::receive(const EventPacket *packet)
{
    switch (packet->type) {
    case 0:
        verticalStep = (-10.0f / 6.0f);
        return;
    case 1:
        displacement.y += (100.0 / 6.0);
        lbl_1_bss_54 += 1;
        if (lbl_1_bss_54 == 10 || lbl_1_bss_54 == 9) {
            displacement.z *= 0.8f;
        }
    }
}

// Starts the fall when the motion is requested.
void FallingRock::fn_1_136C()
{
    f32 verticalDelta;

    if (!limitReached) {
        return;
    }

    Vec position = body.center;
    verticalDelta = body.center.y - body.radius;
    position.y = verticalDelta;
    effect.reset(fn_1_794());
    fn_1_34D0(effect.get(), &position);
    callback = &FallingRock::fn_1_1514;
    lower.clearFlag7();
    body.setFlag7();
    sweep.setFlag7();
    sendEvent(1, 0, 0.0f);
    lbl_1_bss_58 = soundPlay(0x63F);
    HuAudFXFadeOut(lbl_1_bss_50, 100);
}

// Returns the stage floor height, including the final drop.
f32 fn_1_14B8(f32 value)
{
    if (value > 47500.0f) {
        return -5000.0f;
    }
    return fn_1_4A0C(value);
}

// Accelerates to the fall's top speed.
void FallingRock::fn_1_1514()
{
    displacement.z += 1.0 / 6.0;
    if (displacement.z > (100.0f / 6.0f)) {
        displacement.z = (100.0f / 6.0f);
        callback = &FallingRock::fn_1_15B8;
        sendEvent(1, 2, 0.0f);
    }
}

// Lands at the end of the stage.
void FallingRock::fn_1_15B8()
{
    if (body.center.z > 47500.0f) {
        body.clearFlag7();
        sweep.clearFlag7();
        callback = &FallingRock::fn_1_1658;
        model.resetShadow();
        lower = sweep;
        lower.setFlag7();
        HuAudFXFadeOut(lbl_1_bss_58, 100);
    }
}

// Finishes the fall once the model reaches its height limit.
void FallingRock::fn_1_1658()
{
    if (limitReached) {
        sendEvent(1, 3, 0.0f);
        callback = 0;
        fallEnded = true;
    }
}

// Returns the height at the bottom of the sphere.
static inline f32 bottomOf(const Sphere *sphere)
{
    return sphere->center.y - sphere->radius;
}

// Advances motion, constrains its position and updates the model and effect.
void FallingRock::update()
{
    f32 nextY;

    if (callback) {
        (this->*callback)();
    }
    previousPosition = body.center;
    displacement.y += verticalStep;
    PSVECAdd(&displacement, &body.center, &body.center);
    nextY = fn_1_14B8(body.center.z) + body.getRadius();
    if (body.center.y < nextY) {
        body.center.y = nextY;
        displacement.y = 0.0f;
        limitReached = true;
    } else {
        limitReached = false;
    }
    body.center.z = minimumValue(body.center.z, 48500.0f - body.getRadius());
    fn_1_191C();
    fn_1_1A30();
    fn_1_1BA8(this);
    if (effect.get() != 0) {
        Vec position = body.center;
        position.y = bottomOf(&body);
        fn_1_34D0(effect.get(), &position);
    }
    // Nothing further happens once the fall has ended.
    if (fallEnded) return;
}

// Accumulates the rolling rotation from the position change.
void FallingRock::fn_1_191C()
{
    f64 deltaX;
    f64 deltaZ;
    f64 turn;
    f64 turnForDirection;
    Vec *end;
    Vec *start;
    Vec direction;
    Vec axis;
    Qtrn step;

    turn = body.radius;
    turnForDirection = turn;
    end = &previousPosition;
    start = &body.center;
    deltaZ = end->z - start->z;
    deltaX = end->x - start->x;
    direction.x = deltaX;
    direction.y = -turnForDirection;
    direction.z = deltaZ;
    axis.x = 0.0f;
    axis.y = -turnForDirection;
    axis.z = 0.0f;
    makeRotationArc(&step, &axis, &direction);
    PSQUATMultiply(&step, &rotation, &rotation);
    PSQUATNormalize(&rotation, &rotation);
}

// Checks the angle range and returns its sine.
static inline f32 transformSin(double angle)
{
    ASSERT(angle > -100000.0 && angle < 100000.0);
    return (f32)sin(angle);
}

// Combines rolling and oscillation rotations into the model matrix.
void FallingRock::fn_1_1A30()
{
    Mtx orientationMatrix;
    Mtx rotationMatrix;
    MtxPtr modelMatrix;
    f32 angularStep;
    f32 scaledSine;
    f32 oscillationAngle;
    f32 translationX;
    f32 translationY;
    f32 translationZ;
    PSMTXQuat(orientationMatrix, &rotation);
    angularStep = displacement.z / (100.0f / 6.0f);
    oscillationAngle = angle = static_cast<f32>(static_cast<f64>(angle) +
                                        (float) M_PI / 60.0f * static_cast<f64>(angularStep));
    scaledSine = 6.0f * transformSin((double)oscillationAngle);
    PSMTXRotRad(rotationMatrix, 'Z', MTXDegToRad(scaledSine));
    PSMTXConcat(rotationMatrix, orientationMatrix, orientationMatrix);
    translationZ = body.center.z;
    translationY = body.center.y;
    translationX = body.center.x;
    PSMTXTransApply(orientationMatrix, orientationMatrix, translationX, translationY, translationZ);
    modelMatrix = reinterpret_cast<MtxPtr>(model.getMatrix());
    PSMTXCopy(orientationMatrix, modelMatrix);
}

// Positions the side and forward spheres relative to the rock.
void fn_1_1BA8(FallingRock *rock)
{
    f32 rangeDistance;
    f32 lowerLimit;
    f32 distance;
    distance = rock->body.radius;
    rangeDistance = distance;
    f32 range = 550.0f - rangeDistance;

    rock->sweepStart.center = rock->body.center;
    rock->sweepStart.radius = rock->body.radius;
    rock->sweepEnd.center = rock->body.center;
    rock->sweepEnd.radius = rock->body.radius;
    rock->sweepStart.center.x -= range;
    rock->sweepEnd.center.x += range;
    rock->sweep.center.x = 0.0f;
    rock->sweep.center.z = 1200.0f + rock->body.center.z;

    f32 mapped = fn_1_14B8(rock->sweep.center.z);
    f32 clamped;
    if (mapped > -700.0f) clamped = mapped;
    else clamped = -700.0f;
    lowerLimit = clamped;
    rock->sweep.center.y = lowerLimit;
    lbl_1_bss_34 = rock->body.center.z;
}

// Tests a fixed-radius request against the model and its side spheres.
s32 fn_1_1D60(FallingRock *rock, const Sphere *request)
{
    Sphere localRequest(*request);
    f32 requestRadius;
    requestRadius = 50.0f;
    localRequest.radius = requestRadius;

    if (spheresOverlap(&rock->body, &localRequest)) {
        return 1;
    }
    if (spheresOverlap(&rock->sweepStart, &localRequest)) {
        return 1;
    }
    if (spheresOverlap(&rock->sweepEnd, &localRequest)) {
        return 1;
    }
    return 0;
}

// Resolves contact against the side and body spheres.
bool fn_1_1E38(FallingRock *rock, Sphere *resource)
{
    bool first = resolveSphereContact(&rock->sweepStart, resource, 0.5f, 1.0f);
    bool second = resolveSphereContact(&rock->sweepEnd, resource, 0.5f, 1.0f);
    bool third = resolveSphereContact(&rock->body, resource, 0.5f, 1.0f);

    return third | first | second;
}

// Destroys the model and its owned members.
FallingRock::~FallingRock() {}
