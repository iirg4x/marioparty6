// The attackers: each one waits a random time, shakes, jumps and lands back on its start point,
// and the map reacts to each landing (man_map.cpp).
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m624DLL/game_constants.h"
#include "REL/framework/world_units.h"
#include "REL/m624DLL/callback_singleton.h"
#include "REL/m624DLL/attacker.h"
#include "REL/m624DLL/scene_sequence_contract.h"
#include "REL/framework/sound.h"

extern "C" {
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

float RandomFloat();
float RandomFloat(float from, float to);

// Every attacker starts from here and is lit from a point in front of it.
static Vec startPosition = { cmToWorld(0.0f), cmToWorld(400.0f), cmToWorld(-720.0f) };
// How long an attacker shakes before it jumps.
static long shakeFrames = secondsToFrames(2L);

static const GXColor lightColor = { 0xFF, 0xFF, 0xFF, 0xFF };

// A new attacker stands on the start point with its swing at rest.
Attacker::Attacker()
    : state(&Attacker::none),
      model(DATANUM(0x530000, 6)),
      position(startPosition),
      speed(0.0f),
      transition(0.0f),
      frame(0)
{
    // Half of the attackers take the second selection.
    selection = RandomFloat() < 0.5 ? 0 : 1;
    // The model is drawn on layer 4, with a shadow and slightly enlarged.
    model.setLayer(4);
    model.setShadow();
    model.setUniformScale(1.1f);
    // The light is registered on the attacker's model.

    // All attackers share one light source; the light shines from it towards the start point.
    static Vec lightPosition = { 0.0f, cmToWorld(100.0f), cmToWorld(-2000.0f) };
    Vec direction = startPosition;
    PSVECSubtract(&direction, &lightPosition, &direction);
    m_light_id = fn_800288B4(model, &direction, &lightPosition, const_cast<GXColor *>(&lightColor));
    assert(m_light_id >= 0);
}

Attacker::~Attacker()
{
    fn_80028CB8(model, m_light_id);
}

void Attacker::update()
{
    (this->*state)();
    countDown();
    transition.update();
    move();
}

void Attacker::start()
{
    setFrame(RandomFloat(secondsToFrames(2L), secondsToFrames(10L)));
    setState(&Attacker::wait);
}

void Attacker::wait()
{
    if (currentRequest() == 3) {
        setState(&Attacker::none);
    } else if (frame <= 0) {
        frame = shakeFrames;
        setState(&Attacker::shake);
    }
}

void Attacker::shake()
{
    if (currentRequest() != 2) {
        position.x = startPosition.x;
        position.y = startPosition.y;
        position.z = startPosition.z;
        setState(&Attacker::none);
        return;
    }
    f32 fraction = (f32)getFrame() / (f32)shakeFrames;
    f32 extent = 10.0f;
    Vec displacement = { 0.0f, 0.0f, 0.0f };
    displacement.x = extent * RandomFloat(-fraction, fraction);
    displacement.z = extent * RandomFloat(-fraction, fraction);
    PSVECAdd(&startPosition, &displacement, &position);
    if (frame <= 0) {
        jump();
    }
}

void Attacker::lean()
{
    updatePose();
    if (transition.isFinished()) {
        fn_1_5344(&SceneOwner::getChecked());
        fn_1_528C(&SceneOwner::getChecked());
        setFrame(secondsToFrames(0.5));
        setState(&Attacker::hold);
    }
}

void Attacker::hold()
{
    if (frame <= 0) {
        transition.set(0.0f, (s32)secondsToFrames(1.5));
        setState(&Attacker::recover);
    }
}

void Attacker::recover()
{
    updatePose();
    if (transition.isFinished()) {
        start();
    }
}

template <class T> inline T sign(const T &value)
{
    T zero(0.0f);
    if (value == zero) {
        return zero;
    } else {
        return value < zero ? T(-1.0f) : T(1.0f);
    }
}

static inline void inverseTranslation(Mtx matrix, const Point3d &translation)
{
    f32 negativeZ;
    f32 negativeY;
    f32 negativeX;
    negativeZ = -translation.z;
    negativeY = -translation.y;
    negativeX = -translation.x;
    PSMTXTrans(matrix, negativeX, negativeY, negativeZ);
}

static inline void rotation(Mtx matrix, const FrameInterpolator<float> &pose)
{
    f32 angle = pose;
    PSMTXRotRad(matrix, 'z', MTXDegToRad(angle));
}

static inline void applyTranslation(Mtx matrix, const Point3d &translation)
{
    f32 translationZ;
    f32 translationY;
    f32 translationX;
    translationZ = translation.z;
    translationY = translation.y;
    translationX = translation.x;
    PSMTXTransApply(matrix, matrix, translationX, translationY, translationZ);
}

void Attacker::updatePose()
{
    f32 offsetScale = 110.0f;
    Point3d translation = startPosition;
    Mtx translationMatrix;
    Mtx rotationMatrix;
    translation.x -= (f64)offsetScale * sign(transition);
    inverseTranslation(translationMatrix, translation);
    rotation(rotationMatrix, transition);
    PSMTXConcat(rotationMatrix, translationMatrix, translationMatrix);
    applyTranslation(translationMatrix, translation);
    PSMTXMultVec(translationMatrix, &startPosition, &position);
}

void Attacker::jump()
{
    speed = framesToSeconds(0x320L);
    setState(&Attacker::fall);
}

void Attacker::fall()
{
    speed -= gravity;
    if (!(position.y > startPosition.y)) {
        fn_1_5344(&SceneOwner::getChecked());
        fn_1_528C(&SceneOwner::getChecked());
        soundPlayAt(1, 0x3AB, &position);
        position.y = startPosition.y;
        speed = 0.0f;
        setFrame(secondsToFrames(0.5));
        start();
    }
}

void Attacker::move()
{
    Vec rotation = { 0.0f, 0.0f, 0.0f };
    f32 angle = transition.value;

    rotation.z = angle;
    position.y = position.y + speed;
    model.setPosition(&position);
    model.setRotation(&rotation);
}
