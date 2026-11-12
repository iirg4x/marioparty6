// The Dossun, the stone block of this minigame: it waits, trembles, jumps and lands back on its
// spot, and each landing shakes the camera and plays a sound.
// The file name, the assert text and its line (dossun.cpp, line 54) are the game's own; the text of
// the lines before the assert is not known. NDEBUG is undefined because the build defines it for
// every object while the game kept this assert.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

#include "REL/framework/sound.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
extern "C" void fn_80028CB8(s16, s16);
#include "REL/m653DLL/moving_object.h"
#include "REL/m653DLL/allocation.h"
#include "REL/m653DLL/random.h"
#include "datadir_enum.h"
#include "dolphin/gx.h"

void *operator new(unsigned long);
void fn_1_5978(MovingObject *);

extern "C" {

s16 fn_800288B4(s16, const Vec *, const Vec *, const GXColor *);

// Where the Dossun rests: it trembles around this point and lands back at its height.
MovingPosition lbl_1_data_120 = {{cmToWorld(0.0f), cmToWorld(400.0f), cmToWorld(-720.0f)}};

// How long it trembles before each jump.
s32 lbl_1_bss_38 = secondsToFrames(2L);

// The colour of its light: white.
const GXColor lbl_1_rodata_1F8 = {0xFF, 0xFF, 0xFF, 0xFF};

}

// Places the Dossun at its resting point at 1.1 times its size with a shadow, gives it a white
// light of its own and starts its first wait.
MovingObject::MovingObject()
    : GameObject(0x100), OwnedModel(DATANUM(0x700000, 10)), callback(&MovingObject::fn_1_5840),
      position(lbl_1_data_120), velocityY(0.0f), transition(0.0f), countdown(0)
{
    MovingPosition displacement;
    this->setShadow();
    this->setUniformScale(1.1f);
    static Vec lightPosition = {0.0f, cmToWorld(100.0f), cmToWorld(-2000.0f)};
    displacement = lbl_1_data_120;
    PSVECSubtract(&displacement.vector, &lightPosition, &displacement.vector);
    this->m_light_id = fn_800288B4(*this, &displacement.vector, &lightPosition, & lbl_1_rodata_1F8);
    assert(m_light_id >= 0);
    fn_1_5978(this);
    return;
}

void MovingObject::fn_1_5840() {}

MovingObject::~MovingObject()
{
    fn_80028CB8(*this, m_light_id);
}

void fn_1_5F4C(MovingObject *owner);

void fn_1_5900(MovingObject *object)
{
    (object->*object->callback)();
    s32 completed = 0;
    if (object->countdown > 0) {
        if ((object->countdown -= 1) == 0) {
            completed = 1;
        }
    }
    object->transition.update();
    fn_1_5F4C(object);
}

inline void MovingObject::resetCallback() {
    void (MovingObject::*initialCallback)() = &MovingObject::fn_1_5A54;
    callback = initialCallback;
}
extern "C" {
extern u8 lbl_1_bss_64, lbl_1_bss_65;
}

void fn_1_5D78(MovingObject *);
extern float RandomFloat(float,float);
void sendEvent(int,unsigned long,float);
void fn_1_5FD4(void *);

void fn_1_5978(MovingObject *owner)
{
    s32 primaryDuration;
    s32 secondaryDuration;

    if (lbl_1_bss_65 == 0) {
        if (lbl_1_bss_64 == 0) {
            primaryDuration = (s32)secondsToFrames(0.5);
            owner->countdown = primaryDuration;
        } else {
            secondaryDuration = (s32)secondsToFrames(4.5);
            owner->countdown = secondaryDuration;
        }
        owner->resetCallback();
    }
}

void MovingObject::fn_1_5A54()
{
    if (this->countdown <= 0) {
        MovingObjectCallback next;
        this->countdown = lbl_1_bss_38;
        next = &MovingObject::fn_1_5AB4;
        this->callback = next;
    }
}

void MovingObject::fn_1_5AB4()
{
    if (lbl_1_bss_65 == 0) {
        s32 timer = this->countdown;
        f32 fraction = (f32)timer / (f32)lbl_1_bss_38;
        f32 extent = 10.0f;
        Vec displacement = {0.0f, 0.0f, 0.0f};

        displacement.x = extent * RandomFloat(-fraction, fraction);
        displacement.z = extent * RandomFloat(-fraction, fraction);
        PSVECAdd(&lbl_1_data_120.vector, &displacement, &this->position.vector);
        if (this->countdown <= 0) {
            fn_1_5D78(this);
        }
    }
}

void fn_1_5BF0(MovingObject *object)
{
    MovingObjectCallback state;

    if (object->countdown <= 0) {
        object->transition.set(0.0f, (s32)secondsToFrames(1.5));
        state = &MovingObject::fn_1_5C8C;
        object->callback = state;
    }
}

void MovingObject::fn_1_5C8C()
{
    s32 primaryDuration;
    s32 secondaryDuration;

    if (this->transition.isFinished() && lbl_1_bss_65 == 0) {
        if (lbl_1_bss_64 == 0) {
            primaryDuration = (s32)secondsToFrames(0.5);
            this->countdown = primaryDuration;
        } else {
            secondaryDuration = (s32)secondsToFrames(4.5);
            this->countdown = secondaryDuration;
        }
        this->resetCallback();
    }
}

void fn_1_5D78(MovingObject *state)
{
    MovingObjectCallback copy;

    state->velocityY = framesToSeconds(0x320);
    copy = &MovingObject::fn_1_5E08;
    state->callback = copy;
}

void MovingObject::fn_1_5E08()
{
    f32 decrement = 0.27240694f;
    s32 primaryDuration;
    s32 secondaryDuration;

    this->velocityY -= decrement;
    if (this->position.vector.y > lbl_1_data_120.vector.y) {
        return;
    }
    fn_1_5FD4(this);
    this->position.vector.y = lbl_1_data_120.vector.y;
    this->velocityY = 0.0f;
    if (lbl_1_bss_65 == 0) {
        if (lbl_1_bss_64 == 0) {
            primaryDuration = (s32)secondsToFrames(0.5);
            this->countdown = primaryDuration;
        } else {
            secondaryDuration = (s32)secondsToFrames(4.5);
            this->countdown = secondaryDuration;
        }
        this->resetCallback();
    }
}

static inline void M653UpdateTransform(MovingObject *object)
{
    Vec position = {0.0f, 0.0f, 0.0f};
    f32 z = object->transition.value;

    position.z = z;
    object->position.vector.y = object->position.vector.y + object->velocityY;
    object->setPosition(&object->position.vector);
    object->setRotation(&position);
}

void fn_1_5F4C(MovingObject *object)
{
    M653UpdateTransform(object);
}

void fn_1_5FD4(void *object)
{
    if (lbl_1_bss_65 == 0) {
        sendEvent(0, 0, 0.0f);
        soundPlay(0x3AB);
    }
}

/* A debug output hook, empty in the release build. */
static inline void DebugValue(s32 value) { }

void MovingObject::update()
{
    s32 countdownExpired;

    (this->*this->callback)();
    countdownExpired = 0;
    if (this->countdown > 0) {
        if (--this->countdown == 0) {
            countdownExpired = 1;
        }
    }
    this->transition.update();
    fn_1_5F4C(this);
    DebugValue(countdownExpired);
}
