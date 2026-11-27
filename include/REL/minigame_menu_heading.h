#ifndef REL_MINIGAME_MENU_HEADING_H
#define REL_MINIGAME_MENU_HEADING_H

#include "game/object.h"

/* Start fading the selection heading out. */
static inline void minigameMenuHeadingFadeOut(OMOBJ *heading)
{
    heading->work[2] = 2;
    heading->work[3] = 0;
}

#endif
