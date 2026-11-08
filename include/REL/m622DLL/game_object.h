// Objects on the module's update list at .bss 0x11C, built inline. flags takes a bit per scene event.
#ifndef REL_M622DLL_GAME_OBJECT_H
#define REL_M622DLL_GAME_OBJECT_H

#include "dolphin/types.h"
#include "REL/m622DLL/framework/linknode.h"

extern "C" Node lbl_1_bss_11C;

struct NodeOwner : Node {};

struct GameObject : NodeOwner {
    GameObject() : flags(0) { insert4(&lbl_1_bss_11C); }
    virtual ~GameObject() {}
    virtual void clearFlag(int bit) = 0;
    virtual void update() {}
    unsigned int flags;
};

#endif
