#include "REL/framework/world_units.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "REL/m681DLL/global_state.h"
#include "REL/framework/frame_interpolator.h"
extern "C" {
#include <math.h>

#include "game/hu3d.h"
#include "game/audio.h"
}
#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/m681DLL/texture_node.h"

#include "REL/m681DLL/character_model.h"
#include "REL/m681DLL/game_start.h"
#include "REL/m681DLL/game_constants.h"

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

// Creates the stand model, motions, rotation interpolator and floor circle.
PlayerStand::PlayerStand(int player)
    : GameObject(5), ModelMotion(lbl_1_rodata_428[player]), EventListener(),
      player(player), standNode(lbl_1_data_534[player]),
      firstTimer(0), secondTimer(0), nestedResource(), transition(-180.0f, 0)
{
    Vec position;
    f32 currentValue;

    nodeRotation = &getObject(standNode)->mesh.base.rot.y;
    setLayer(5);
    resetAttribute(((1 << 30) | (1 << 0)));
    setAttribute(((1 << 30) | (1 << 1)));
    addMotions(lbl_1_rodata_438, 2);
    updateMotion();
    currentValue = transition.value;
    *nodeRotation = currentValue;
    fn_1_8184(this, &position);
    position.y = 0.0f;
    new FloorCircle(&position, 95.0f);
    lbl_1_bss_C0.flag = 0;
    lbl_1_data_544 = -1;
    registerChannel(player + 2);
    registerChannel(0);
}

// Releases the stand model and owned resources.
PlayerStand::~PlayerStand() {}

// Returns the position of the stand node.
Vec *fn_1_8184(PlayerStand *owner, Vec *value)
{
    return &owner->getObjectPosition(owner->standNode, value);
}

// Advances stand rotation and plays sounds when its timers expire.
void PlayerStand::update()
{
    u8 firstTimerExpired;
    u8 secondTimerExpired;

    if (!transition.isFinished()) {
        *nodeRotation = transition.update();
        if (transition.isFinished() && lbl_1_data_544 != -1) {
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
        HuAudFXPlayPan(0x91D, fn_1_2508(player));
    }

    secondTimerExpired = 0;
    if (secondTimer > 0) {
        if ((secondTimer -= 1) == 0) {
            secondTimerExpired = 1;
        }
    }
    if (secondTimerExpired != 0) {
        HuAudFXPlayPan(0x91E, fn_1_2508(player));
    }
}

// Starts the stand rotation or dispatches its player-specific motion event.
void PlayerStand::receive(const EventPacket *request)
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

// Plays the first stand motion and requests its sound.
void PlayerStand::fn_1_83F8()
{
    setMotion(DATANUM(0x8c0000, 6));
    resetAttribute(((1 << 30) | (1 << 1)));
    lbl_1_bss_C0.flag = 1;
}

// Plays the second stand motion and starts its sound timers.
void PlayerStand::fn_1_8450()
{
    setMotion(DATANUM(0x8c0000, 7));
    resetAttribute(((1 << 30) | (1 << 1)));
    firstTimer = 50;
    secondTimer = 120;
}

extern "C" { StandSoundState lbl_1_bss_C0; }
