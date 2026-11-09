#ifndef REL_M652DLL_TAIL_TYPES_H
#define REL_M652DLL_TAIL_TYPES_H

#include "dolphin/mtx.h"
struct omObj_s;
typedef struct omObj_s OMOBJ;
struct MgActor_s;
typedef struct MgActor_s MGACTOR;
struct MgPlayer_s;
typedef struct MgPlayer_s MGPLAYER;
typedef void (*MGPLAYER_MODE_FUNC)(MGPLAYER *playerP);
extern "C" {
BOOL fn_80086DD0(MGACTOR *actorP, u32 *code);
void fn_80087158(MGPLAYER *playerP, u16 attr);
}
#include "REL/m652DLL/framework/linknode.h"
#include "REL/m652DLL/framework/range.h"

/* Shared event, timer, and player interfaces. */

struct MT;

/* CA90/CF60/CD78 consume a Range at +0x4, followed by two angles and two
 * pad-state words.  The leading link is a consumed word in the owner view;
 * it is kept named rather than hidden as padding. */
#include "REL/m652DLL/camera_arc_motion.h"

#endif
