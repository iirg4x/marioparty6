#include "game/hu3d.h"

void fn_1_2DCD0(s16 layerNo);

void fn_1_2E68C(void)
{
    Hu3DLayerHookSet(14, fn_1_2DCD0);
}

void fn_1_2E6B8(void)
{
    Hu3DLayerHookReset(14);
}
