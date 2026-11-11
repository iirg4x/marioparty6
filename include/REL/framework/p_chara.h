// A player's configuration slot together with the character model it selects (framework
// p_chara.cpp). The file and class names are descriptive: no symbol or string names them.
#ifndef REL_FRAMEWORK_P_CHARA_H
#define REL_FRAMEWORK_P_CHARA_H

#include "REL/framework/p_info.h"
#include "REL/framework/chara.h"

struct PlayerConfigSelection : PlayerInfo, Character {
    PlayerConfigSelection(s32 playerNo, s32 mode);
    ~PlayerConfigSelection();
};

#endif
