#include "REL/framework/sound.h"
#include "REL/m634DLL/particle_effect.h"
#include "REL/m634DLL/game_manager.h"
extern "C" {
#include <math.h>
#include "dolphin/types.h"
#include "game/hu3d.h"
}
// Creates the particle effect selected by the scene event.
void GameManager::fn_1_CCA8(const EventPacket *event)
{
    switch (event->type) {
    case 0: {
        Vec *motion = (Vec *)event->context;
        new ParticleEffect634(this->animations[0], motion, 0x18, 5);
        return;
    }
    case 1: {
        Vec *motion = (Vec *)event->context;
        new ParticleEffect634(this->animations[0], motion, 0x18, 7);
        return;
    }
    case 2:
        break;
    case 3: {
        Vec *motion = (Vec *)event->context;
        s32 index;
        this->secondaryResource->reposition(motion);
        for (index = 0; index < 3; index++) {
            new ParticleRadialEffect634(this->animations[2], motion, 0x30, 5, index,
                (f32)(2.0 * (0.016666666666666666 * (f64)index)));
        }
        soundPlayAt(2, 0x740, motion);
        return;
    }
    case 4: {
        Vec *motion = (Vec *)event->context;
        s32 index;
        this->primaryResource->reposition(motion);
        for (index = 0; index < 3; index++) {
            new ParticleRadialEffect634(this->animations[2], motion, 0x30, 7, index,
                (f32)(2.0 * (0.016666666666666666 * (f64)index)));
        }
        soundPlayAt(2, 0x740, motion);
    }
    }
}
