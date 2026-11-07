#ifndef _GAME_OMOVL_H
#define _GAME_OMOVL_H

typedef int OMOVL;

enum {
#define DLL(name) DLL_##name,
#include "ovl_table.h"
#undef DLL
};

#endif
