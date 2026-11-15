#ifndef REL_M634DLL_CALLBACK_OWNER_H
#define REL_M634DLL_CALLBACK_OWNER_H

#include "dolphin/types.h"

typedef struct M634CallbackOwner {
    u8 objectState[8]; /* Unreconstructed object contents. */
    const char *callbackTable;
} M634CallbackOwner;

#endif
