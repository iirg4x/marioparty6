#include "REL/m681DLL/frame_time.h"
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
    fn_1_33A3C(position);
    fn_1_33CB0(5);
    const MotionPreset *preset = lbl_1_rodata_190 + variant;
    fn_1_1A0F4(preset->modelData, preset->motionIndex);
    fn_1_1A1B8(*preset->modelData);
    fn_1_34680(RandomFloat__Fff(0.9f, 1.1f));
    fn_1_33950(0x40000001);
    registerChannel(0);
}

MotionObject::~MotionObject() {}

extern "C" {
#include "REL/m681DLL.h"
}

void MotionObject::update()
{
    f32 value = fn_1_34560();
    if (enabled != 0) {
        fn_1_33988(0x40000001);
        if ((u8)fn_1_346CC() != 0) {
            fn_1_1A218(modelData[0], (f32)secondsToFrames(0.2),
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
        fn_1_1A218(modelData[1], (f32)secondsToFrames(0.2),
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


static inline f32 accelerationPerFrame(f64 value)
{
    return (f32)((value * 100.0) / (60.0 * 60.0));
}
extern "C" {
s32 lbl_1_bss_50 = minutesToFrames(60L);
f32 lbl_1_bss_54 = accelerationPerFrame(9.80665);
}
