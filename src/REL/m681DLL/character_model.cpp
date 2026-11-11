#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
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
#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/m681DLL/texture_node.h"

#include "REL/m681DLL/character_model.h"
#include "REL/framework/world_units.h"

extern "C" {
}
s16 fn_1_2508(s32 player);
extern "C" {
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
      firstTimer(0), secondTimer(0), nestedResource(), transition(-180.0f, 0)
{
    Vec callbackData;
    f32 currentValue;

    modelValue = &getObject(motionData)->mesh.base.rot.y;
    setLayer(5);
    resetAttribute(((1 << 30) | (1 << 0)));
    setAttribute(((1 << 30) | (1 << 1)));
    addMotions(lbl_1_rodata_438, 2);
    updateMotion();
    currentValue = transition.current;
    *modelValue = currentValue;
    fn_1_8184(this, &callbackData);
    callbackData.y = 0.0f;
    new TextureNode0(&callbackData, 95.0f);
    lbl_1_bss_C0.flag = 0;
    lbl_1_data_544 = -1;
    registerChannel(character + 2);
    registerChannel(0);
}

ModelOwnerComposite84::~ModelOwnerComposite84() {}

Vec *fn_1_8184(ModelOwnerComposite84 *owner, Vec *value)
{
    return &owner->getObjectPosition(owner->motionData, value);
}

void ModelOwnerComposite84::update()
{
    u8 firstTimerExpired;
    u8 secondTimerExpired;

    if (transition.elapsed <= transition.frames) {
        *modelValue = transition.advance();
        if (transition.elapsed > transition.frames && lbl_1_data_544 != -1) {
            soundStop(lbl_1_data_544);
            lbl_1_data_544 = -1;
            soundPlay(0x921);
        }
    }

    if (lbl_1_bss_C0.flag != 0) {
        lbl_1_bss_C0.flag = 0;
        soundPlay(0x91F);
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
                lbl_1_data_544 = soundPlay(0x920);
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
    setMotion(DATANUM(0x8c0000, 6));
    resetAttribute(((1 << 30) | (1 << 1)));
    lbl_1_bss_C0.flag = 1;
}

void ModelOwnerComposite84::fn_1_8450()
{
    setMotion(DATANUM(0x8c0000, 7));
    resetAttribute(((1 << 30) | (1 << 1)));
    firstTimer = 50;
    secondTimer = 120;
}

extern "C" {
s32 lbl_1_bss_B8 = minutesToFrames(60L);
f32 lbl_1_bss_BC = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}

extern "C" { M681C0State lbl_1_bss_C0; }
