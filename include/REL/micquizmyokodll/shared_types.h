#ifndef MICQUIZMYOKODLL_SHARED_TYPES_H
#define MICQUIZMYOKODLL_SHARED_TYPES_H

#include "humath.h"

typedef struct Position {
    HuVecF position;
    unsigned char unk_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

#endif
