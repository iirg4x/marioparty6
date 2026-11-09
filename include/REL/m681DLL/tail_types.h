#ifndef REL_M681DLL_TAIL_TYPES_H
#define REL_M681DLL_TAIL_TYPES_H

#include "dolphin/mtx.h"
struct omObj_s;
typedef struct omObj_s OMOBJ;
struct MgActor_s;
typedef struct MgActor_s MGACTOR;
struct MgPlayer_s;
typedef struct MgPlayer_s MGPLAYER;
typedef void (*MGPLAYER_MODE_FUNC)(MGPLAYER *playerP);
struct MgPlayer_s {
    s16 playerNo;
    s16 charNo;
    s8 padNo;
    u16 camBit;
    OMOBJ *omObj;
    u32 actionFlag;
    u16 motNo;
    s8 stunType;
    float stunAngle;
    float stunTime;
    float squishTime;
    u16 attr;
    u16 modeAttr;
    u16 vibAttr;
    s16 subMode;
    int mode;
    float timer;
    float squishOldHeight;
    s16 work[4];
    MGACTOR *actor;
    MGPLAYER_MODE_FUNC modeFunc[18];
};

extern "C" {
BOOL fn_80086DD0(MGACTOR *actorP, u32 *code);
void fn_80087158(MGPLAYER *playerP, u16 attr);
}
#include "REL/m681DLL/framework/linknode.h"
#include "REL/m681DLL/framework/range.h"

#endif
