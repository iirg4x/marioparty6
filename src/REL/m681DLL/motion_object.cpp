#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "REL/m681DLL/motion_object.h"

struct MotionPreset {
    const unsigned long *modelData;
    s32 motionIndex;
};

extern "C" {
f32 RandomFloat__Fff(f32, f32);
extern const s32 lbl_1_rodata_178[2] = { 0x008C0010, 0x008C0013 };
extern const unsigned long lbl_1_rodata_180[2] = { 0x008C0011, 0x008C0012 };
extern const unsigned long lbl_1_rodata_188[2] = { 0x008C0014, 0x008C0015 };
extern const MotionPreset lbl_1_rodata_190[2] = {
    { lbl_1_rodata_180, 2 },
    { lbl_1_rodata_188, 2 }
};
}

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
    setMotionSpeed(RandomFloat__Fff(0.9f, 1.1f));
    setAttribute(0x40000001);
    registerChannel(0);
}

MotionObject::~MotionObject() {}

extern "C" {
#include "REL/m681DLL.h"
#include "REL/framework/world_units.h"
}

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

extern "C" {
extern s32 lbl_1_bss_50;
extern f32 lbl_1_bss_54;

}

extern "C" {
s32 lbl_1_bss_50 = minutesToFrames(60L);
f32 lbl_1_bss_54 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
