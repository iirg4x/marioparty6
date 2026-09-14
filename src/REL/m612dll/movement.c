#include "game/hu3d.h"

/* Recovered names: the original local symbol names are unavailable. */
void fn_1_5E90(s32 direction, HuVecF *pos, float distance)
{
    switch (direction) {
        case 1:
            pos->x -= distance;
            break;
        case 2:
            pos->x = distance;
            break;
        case 3:
            pos->z -= distance;
            break;
        case 4:
            pos->z += distance;
            break;
    }
}

void fn_1_5EF0(s32 direction, s32 *x, s32 *y)
{
    switch (direction) {
        case 1:
            *x -= 1;
            break;
        case 2:
            *x += 1;
            break;
        case 3:
            *y -= 1;
            break;
        case 4:
            *y += 1;
            break;
    }
}
