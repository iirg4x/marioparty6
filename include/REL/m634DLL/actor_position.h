#ifndef REL_M634DLL_ACTOR_POSITION_H
#define REL_M634DLL_ACTOR_POSITION_H

extern "C" {
#include "game/mg/actman.h"
}

struct M634ActorPosition {

    unsigned char unknown00[0x3C]; /* No code names this field. */

    MGACTOR *actor;
};

#endif
