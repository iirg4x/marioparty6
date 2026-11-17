#ifndef REL_MENU_HEADING_H
#define REL_MENU_HEADING_H

#include "game/object.h"

static inline void MenuHeadingStateReset(OMOBJ *obj)
{
    obj->work[2] = 2;
    obj->work[3] = 0;
}

#endif
