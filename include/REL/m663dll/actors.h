#ifndef REL_M663DLL_ACTORS_H
#define REL_M663DLL_ACTORS_H

#include "dolphin/types.h"

struct MgPlayer_s;

typedef struct M663PlayerActorMap {
    s16 modelId;
    struct MgPlayer_s *players[2];
} M663PlayerActorMap;

extern M663PlayerActorMap lbl_1_bss_418;

#endif
