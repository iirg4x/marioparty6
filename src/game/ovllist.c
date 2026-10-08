/* Maps overlay IDs to the REL files loaded by the game's overlay manager. */
#define _MATH_H
#include "game/object.h"

/* Each name from ovl_table.h becomes its corresponding file under dll/. */
#define DLL(name) { "dll/" #name ".rel", 0 },

OVLTBL _ovltbl[] = {
    #include "ovl_table.h"
    { NULL, -1 }
};

#undef DLL
