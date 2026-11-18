#ifndef REL_M628DLL_PLAYER_INPUT_BINDING_H
#define REL_M628DLL_PLAYER_INPUT_BINDING_H
#include "REL/m628DLL/player_input_controller.h"
// Reads one configured controller pad for a player character.
class PlayerInputBinding : public PlayerInputController
{
  public:
    s32 padNo;
    PlayerInputBinding(ActPlayer *owner);
    virtual void invoke(StickState *);
};
#endif
