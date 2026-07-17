#ifndef _GAME_OMOVL_H
#define _GAME_OMOVL_H

#define DLL(name) DLL_##name,

typedef enum omOvl_e {
    DLL_NONE = -1,
    #include "ovl_table.h"
    DLL_MAX
} OMOVL;

#undef DLL

#endif
