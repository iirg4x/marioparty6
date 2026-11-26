#ifndef REL_MINIGAME_PAD_H
#define REL_MINIGAME_PAD_H

#include "dolphin/types.h"

/* Controller state captured by the minigame input update. */
typedef struct SystemPadInput {
    s32 unk00; /* The game's code never reads this field. */
    s32 buttonsDown;
    u16 unk08; /* The game's code never reads this field. */
    s8 stickX;
    s8 unk0B; /* The game's code never reads this field. */
    s8 unk0C; /* The game's code never reads this field. */
    s8 unk0D; /* The game's code never reads this field. */
    u8 unk0E; /* The game's code never reads this field. */
    u8 unk0F; /* The game's code never reads this field. */
    u8 unk10; /* The game's code never reads this field. */
    u8 unk11; /* The game's code never reads this field. */
} SystemPadInput;

#endif
