#include "REL/m681DLL/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "REL/m681DLL/global_state.h"
#include "REL/m681DLL/transition.h"
extern "C" {
#include <math.h>
#include "REL/m681DLL.h"
#include "game/hu3d.h"
#include "game/audio.h"
}
#include "REL/m681DLL/owned_model.h"
#include "REL/m681DLL/model_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/m681DLL/texture_node.h"

#include "REL/m681DLL/character_model.h"

extern "C" {
}
s16 fn_1_2508(s32 player);
extern "C" {
void fn_1_8920(s32 value);
s32 fn_1_88F8(s32 sound);
char *lbl_1_data_534[4] = { "st01", "st02", "st03", "st04" };
s32 lbl_1_data_544 = -1;
extern const s32 lbl_1_rodata_428[4] = {
    DATANUM(0x8c0000, 2), DATANUM(0x8c0000, 3),
    DATANUM(0x8c0000, 4), DATANUM(0x8c0000, 5)
};
extern const unsigned long lbl_1_rodata_438[2] = {
    DATANUM(0x8c0000, 6), DATANUM(0x8c0000, 7)
};
}

ModelOwnerComposite84::ModelOwnerComposite84(int character)
    : GameObject(5), ModelMotion(lbl_1_rodata_428[character]), EventListener(),
      character(character), motionData(lbl_1_data_534[character]),
      firstTimer(0), secondTimer(0), nestedResource()
{
    PositionVector callbackData;
    f32 currentValue;

    transition.set( -180.0f, 0);
    modelValue = &fn_1_33F28(motionData)->mesh.base.rot.y;
    fn_1_33CB0(5);
    fn_1_33988(((1 << 30) | (1 << 0)));
    fn_1_33950(((1 << 30) | (1 << 1)));
    fn_1_1A0F4(lbl_1_rodata_438, 2);
    fn_1_34734();
    currentValue = transition.current;
    *modelValue = currentValue;
    fn_1_8184(this, &callbackData.value);
    callbackData.value.y = 0.0f;
    new TextureNode0(&callbackData, 95.0f);
    lbl_1_bss_C0.flag = 0;
    lbl_1_data_544 = -1;
    registerChannel(character + 2);
    registerChannel(0);
}

ModelOwnerComposite84::~ModelOwnerComposite84() {}

Vec *fn_1_8184(ModelOwnerComposite84 *owner, Vec *value)
{
    return owner->fn_1_344A0(owner->motionData, value);
}

void ModelOwnerComposite84::update()
{
    u8 firstTimerExpired;
    u8 secondTimerExpired;

    if (transition.elapsed <= transition.frames) {
        *modelValue = transition.advance();
        if (transition.elapsed > transition.frames && lbl_1_data_544 != -1) {
            fn_1_8920(lbl_1_data_544);
            lbl_1_data_544 = -1;
            fn_1_88F8(0x921);
        }
    }

    if (lbl_1_bss_C0.flag != 0) {
        lbl_1_bss_C0.flag = 0;
        fn_1_88F8(0x91F);
    }

    firstTimerExpired = 0;
    if (firstTimer > 0) {
        if ((firstTimer -= 1) == 0) {
            firstTimerExpired = 1;
        }
    }
    if (firstTimerExpired != 0) {
        HuAudFXPlayPan(0x91D, fn_1_2508(character));
    }

    secondTimerExpired = 0;
    if (secondTimer > 0) {
        if ((secondTimer -= 1) == 0) {
            secondTimerExpired = 1;
        }
    }
    if (secondTimerExpired != 0) {
        HuAudFXPlayPan(0x91E, fn_1_2508(character));
    }
}

void ModelOwnerComposite84::receive(const EventPacket *request)
{
    if (request->channel == 0) {
        if (request->type == 0) {
            transition.set( 0.0f, (s32)secondsToFrames(1.2));
            if (lbl_1_data_544 == -1) {
                lbl_1_data_544 = fn_1_88F8(0x920);
            }
        }
        return;
    }
    switch (request->type) {
    case 0:
        fn_1_83F8();
        break;
    case 1:
        fn_1_8450();
        break;
    }
}

void ModelOwnerComposite84::fn_1_83F8()
{
    fn_1_1A1B8(DATANUM(0x8c0000, 6));
    fn_1_33988(((1 << 30) | (1 << 1)));
    lbl_1_bss_C0.flag = 1;
}

void ModelOwnerComposite84::fn_1_8450()
{
    fn_1_1A1B8(DATANUM(0x8c0000, 7));
    fn_1_33988(((1 << 30) | (1 << 1)));
    firstTimer = 50;
    secondTimer = 120;
}

static inline f32 accelerationPerFrame(f64 value)
{
    return (f32)((value * 100.0) / (60.0 * 60.0));
}
extern "C" {
s32 lbl_1_bss_B8 = minutesToFrames(60L);
f32 lbl_1_bss_BC = accelerationPerFrame(9.80665);
}

extern "C" { M681C0State lbl_1_bss_C0; }
