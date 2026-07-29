#ifndef _BOARD_CAPSULE_H
#define _BOARD_CAPSULE_H

#define MBCapsuleEffRandF() \
    (mbRandMod(1 << 28) * (1.0f / (1 << 28)))

#endif
