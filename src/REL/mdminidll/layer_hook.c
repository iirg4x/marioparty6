/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

void fn_1_20068(s16 layerNo);

void fn_1_20A24(void)
{
    Hu3DLayerHookSet(14, fn_1_20068);
}

void fn_1_20A50(void)
{
    Hu3DLayerHookReset(14);
}
