#ifndef REL_M648_LIST_OBJECT_H
#define REL_M648_LIST_OBJECT_H

extern "C" {
#include "dolphin/types.h"
}
#include "REL/framework/game_object.h"
#include "REL/m648DLL/node_owner_198.h"

extern "C" { extern Node lbl_1_bss_63C; }

/* The base of the objects kept on the list at .bss 0x63C. */
struct RequestObject : NodeOwner305B0_198 {
    RequestObject() : state(0) { insert4(&lbl_1_bss_63C); }
    virtual ~RequestObject() {}
    virtual void clearRequest(int request) = 0;
    virtual void update();
    u32 state; // One bit for each pending request.
};

#endif
