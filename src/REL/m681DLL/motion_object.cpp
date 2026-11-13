#include "REL/framework/world_units.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "REL/m681DLL/motion_object.h"
#include "REL/framework/random.h"
#include "REL/m681DLL/game_constants.h"

struct MotionPreset {
    const unsigned long *modelData;
    s32 motionIndex;
};

extern "C" {
extern const s32 lbl_1_rodata_178[2] = { 0x008C0010, 0x008C0013 };
extern const unsigned long lbl_1_rodata_180[2] = { 0x008C0011, 0x008C0012 };
extern const unsigned long lbl_1_rodata_188[2] = { 0x008C0014, 0x008C0015 };
extern const MotionPreset lbl_1_rodata_190[2] = {
    { lbl_1_rodata_180, 2 },
    { lbl_1_rodata_188, 2 }
};
}

// Creates a stage model with its two motions and a randomized playback speed.
MotionObject::MotionObject(Vec *position, int variant)
    : GameObject(2), ModelMotion(lbl_1_rodata_178[variant]), EventListener()
{
    modelData = lbl_1_rodata_190[variant].modelData;
    enabled = 0;
    setPosition(position);
    setLayer(5);
    const MotionPreset *preset = lbl_1_rodata_190 + variant;
    addMotions(preset->modelData, preset->motionIndex);
    setMotion(*preset->modelData);
    setMotionSpeed(RandomFloat(0.9f, 1.1f));
    setAttribute(0x40000001);
    registerChannel(0);
}

// Releases the stage model and motion resources.
MotionObject::~MotionObject() {}

// Restarts the first motion when the enabled motion ends.
void MotionObject::update()
{
    f32 value = getMotionTime();
    if (enabled != 0) {
        resetAttribute(0x40000001);
        if (isMotionEnd() != 0) {
            shiftMotion(modelData[0], (f32)secondsToFrames(0.2),
                       0.0f, 0x40000001);
            enabled = 0;
        }
    }
}

// Enables playback or shifts to the second motion for game events.
void MotionObject::receive(const EventPacket *input)
{
    switch (input->type) {
    case 4:
    case 7:
    case 10:
        enabled = 1;
        break;
    case 3:
    case 8:
    case 12:
        shiftMotion(modelData[1], (f32)secondsToFrames(0.2),
                   0.0f, 0x40000001);
        break;
    default:
        break;
    }
}
