#ifndef MIKEACTDLL_POSITION_H
#define MIKEACTDLL_POSITION_H
#include "humath.h"
typedef struct Position {
    HuVecF position;
    unsigned char unknown_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;
#endif
