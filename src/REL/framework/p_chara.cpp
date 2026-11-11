/* A player slot's character: the model of the character the player's GameWork entry selects. It
 * links right before p_info.cpp in every module that has it. */
#include "REL/framework/p_chara.h"
extern "C" {
#include "game/gamework.h"
}

PlayerConfigSelection::PlayerConfigSelection(s32 playerNo, s32 mode)
    : PlayerInfo(playerNo), Character(lbl_80265728[playerNo].charNo, mode)
{
}

PlayerConfigSelection::~PlayerConfigSelection() {}
