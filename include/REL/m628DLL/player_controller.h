#ifndef REL_M628DLL_PLAYER_CONTROLLER_H
#define REL_M628DLL_PLAYER_CONTROLLER_H
#include "REL/m628DLL/player_actor.h"
// Reads pad input for the lever actor.
class LeverPadController : public ActorController
{
  public:
    LeverPadController(PlayerActor *actor);
    virtual void invoke(StickState *);

    u8 unk20[4]; /* No code names this field. */
};

#endif
