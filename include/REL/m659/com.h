#ifndef M659_COM_H
#define M659_COM_H
#include "REL/m659/player.h"
typedef M659ComSlot M659Com;
typedef struct M659ComManager {
    M659Com *players[2];
    s16 state;
    int frame;
    int ready;
} M659ComManager;
extern OMOBJ *lbl_1_bss_60;
#endif
