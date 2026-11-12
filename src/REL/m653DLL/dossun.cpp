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
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m653DLL/moving_object.h"
#include "REL/m653DLL/allocation.h"
#include "REL/m653DLL/random.h"
#include "datadir_enum.h"
#include "dolphin/gx.h"
#include "REL/framework/event.h"
#include "REL/m653DLL.h"

extern "C" {

// Where the Dossun rests: it trembles around this point, jumps from it and lands back at its
// height.
Vec lbl_1_data_120 = {cmToWorld(0.0f), cmToWorld(400.0f), cmToWorld(-720.0f)};

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
    this->setShadow();
    this->setUniformScale(1.1f);
    static Vec lightPosition = {0.0f, cmToWorld(100.0f), cmToWorld(-2000.0f)};
    Vec displacement = lbl_1_data_120;
    PSVECSubtract(&displacement, &lightPosition, &displacement);
    this->m_light_id =
        fn_800288B4(*this, &displacement, &lightPosition, const_cast<GXColor *>(&lbl_1_rodata_1F8));
    assert(m_light_id >= 0);
    fn_1_5978(this);
    return;
}

void MovingObject::fn_1_5840() {}

MovingObject::~MovingObject()
{
    fn_80028CB8(*this, m_light_id);
}

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

// Unless the round is over, waits half a second before the first round and 4.5 seconds during it.
inline void MovingObject::restartWait()
{
    if (lbl_1_bss_65 == 0) {
        if (lbl_1_bss_64 == 0) {
            setCountdown((s32)secondsToFrames(0.5));
        } else {
            setCountdown((s32)secondsToFrames(4.5));
        }
        setCallback(&MovingObject::fn_1_5A54);
    }
}

void fn_1_5978(MovingObject *owner)
{
    owner->restartWait();
}

void MovingObject::fn_1_5A54()
{
    if (this->countdown <= 0) {
        this->countdown = lbl_1_bss_38;
        setCallback(&MovingObject::fn_1_5AB4);
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
        PSVECAdd(&lbl_1_data_120, &displacement, &this->position);
        if (this->countdown <= 0) {
            fn_1_5D78(this);
        }
    }
}

void fn_1_5BF0(MovingObject *object)
{
    if (object->countdown <= 0) {
        object->transition.set(0.0f, (s32)secondsToFrames(1.5));
        object->setCallback(&MovingObject::fn_1_5C8C);
    }
}

void MovingObject::fn_1_5C8C()
{
    if (this->transition.isFinished()) {
        restartWait();
    }
}

void fn_1_5D78(MovingObject *state)
{
    state->velocityY = framesToSeconds(0x320);
    state->setCallback(&MovingObject::fn_1_5E08);
}

void MovingObject::fn_1_5E08()
{
    f32 decrement = 0.27240694f;

    this->velocityY -= decrement;
    if (this->position.y > lbl_1_data_120.y) {
        return;
    }
    fn_1_5FD4(this);
    this->position.y = lbl_1_data_120.y;
    this->velocityY = 0.0f;
    restartWait();
}

void fn_1_5F4C(MovingObject *object)
{
    Vec rotation = {0.0f, 0.0f, 0.0f};
    f32 z = object->transition.value;

    rotation.z = z;
    object->position.y = object->position.y + object->velocityY;
    object->setPosition(&object->position);
    object->setRotation(&rotation);
}

void fn_1_5FD4(MovingObject *object)
{
    if (lbl_1_bss_65 == 0) {
        sendEvent(0, 0, 0.0f);
        soundPlay(0x3AB);
    }
}

void MovingObject::update()
{
    fn_1_5900(this);
}
